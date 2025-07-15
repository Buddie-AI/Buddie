#include "includes.h"
#include "asm/includes.h"
#include "system/timer.h"
#include "device/ioctl_cmds.h"
#include "usb/host/usb_host.h"
#include "usb_ctrl_transfer.h"
#include "usb_bulk_transfer.h"
#include "device_drive.h"
#include "adb.h"
#include "usb_config.h"
#include "app_config.h"
#if TCFG_ADB_ENABLE
#define LOG_TAG_CONST       USB
#define LOG_TAG             "[adb]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


#include "adb_rsa_key.c"

struct adb_device_t adb;

struct device adb_device;
void aoa_switch(struct usb_host_device *host_dev);

static int set_adb_power(struct usb_host_device *host_dev, u32 value)
{
    return DEV_ERR_NONE;
}

static int get_adb_power(struct usb_host_device *host_dev, u32 value)
{
    return DEV_ERR_NONE;
}

static const struct interface_ctrl adb_ops = {
    .interface_class = USB_CLASS_ADB,
    .set_power = set_adb_power,
    .get_power = get_adb_power,
    .ioctl = NULL,
};

static const struct usb_interface_info adb_inf = {
    .ctrl = (struct interface_ctrl *) &adb_ops,
    .dev.adb = &adb,
};

u32 usb_adb_interface_ptp_mtp_parse(struct usb_host_device *host_dev, u8 interface_num, const u8 *pBuf)
{
    struct usb_interface_descriptor *interface = (struct usb_interface_descriptor *)pBuf;
    pBuf += sizeof(struct usb_interface_descriptor);
    int len = 0;
    const usb_dev usb_id = host_device2id(host_dev);

    for (int i = 0; i < interface->bNumEndpoints; i++) {
        struct usb_endpoint_descriptor *endpoint = (struct usb_endpoint_descriptor *)pBuf;
        if (endpoint->bDescriptorType == USB_DT_ENDPOINT) {
            if (endpoint->bmAttributes == USB_ENDPOINT_XFER_BULK) {
                if (endpoint->bEndpointAddress & USB_DIR_IN) {
                    adb.extr_in = endpoint->bEndpointAddress & 0xf;
                } else {
                    adb.extr_out = endpoint->bEndpointAddress;
                }
            }
            pBuf += USB_DT_ENDPOINT_SIZE;
        } else {
            return 0;
        }
    }
    printf("%s() %x %x\n", __func__, adb.extr_in, adb.extr_out);
    return pBuf - (u8 *)interface ;
}

int usb_adb_parser(struct usb_host_device *host_dev, u8 interface_num, const u8 *pBuf)
{
    struct usb_interface_descriptor *interface = (struct usb_interface_descriptor *)pBuf;
    pBuf += sizeof(struct usb_interface_descriptor);
    int len = 0;
    const usb_dev usb_id = host_device2id(host_dev);

    adb_device.private_data = host_dev;

    host_dev->interface_info[interface_num] = &adb_inf;

    for (int endnum = 0; endnum < interface->bNumEndpoints; endnum++) {
        struct usb_endpoint_descriptor *end_desc = (struct usb_endpoint_descriptor *)pBuf;

        if (end_desc->bDescriptorType != USB_DT_ENDPOINT ||
            end_desc->bLength != USB_DT_ENDPOINT_SIZE) {
            log_error("ep bDescriptorType = %d bLength = %d", end_desc->bDescriptorType, end_desc->bLength);
            return -USB_DT_ENDPOINT;
        }

        len += USB_DT_ENDPOINT_SIZE;
        pBuf += USB_DT_ENDPOINT_SIZE;

        if ((end_desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
            if (end_desc->bEndpointAddress & USB_DIR_IN) {
                adb.host_epin = usb_get_ep_num(usb_id, USB_DIR_IN, USB_ENDPOINT_XFER_BULK);
                adb.target_epin = end_desc->bEndpointAddress & 0x0f;
#if defined(FUSB_MODE) && FUSB_MODE == 0
                adb.rxmaxp = end_desc->wMaxPacketSize;
#endif
                log_debug("D(%d)->H(%d)", adb.target_epin, adb.host_epin);
            } else {
                adb.host_epout = usb_get_ep_num(usb_id, USB_DIR_OUT, USB_ENDPOINT_XFER_BULK);
                adb.target_epout = end_desc->bEndpointAddress & 0x0f;
#if defined(FUSB_MODE) && FUSB_MODE == 0
                adb.txmaxp = end_desc->wMaxPacketSize;
#endif
                log_debug("H(%d)->D(%d)",  adb.host_epout, adb.target_epout);
            }
        }
    }

    u8 *ep_buffer = usb_h_get_ep_buffer(usb_id, adb.host_epin | USB_DIR_IN);
    usb_h_ep_config(usb_id, adb.host_epin | USB_DIR_IN, USB_ENDPOINT_XFER_BULK, 0, 0, ep_buffer, 64);


    ep_buffer = usb_h_get_ep_buffer(usb_id, adb.host_epout | USB_DIR_OUT);
    usb_h_ep_config(usb_id, adb.host_epout | USB_DIR_OUT, USB_ENDPOINT_XFER_BULK, 0, 0, ep_buffer, 64);

    return len;
}

static int adb_send_packet(struct amessage *msg, const u8 *data_ptr)
{
    if (msg == NULL) {
        return usb_bulk_only_send(&adb_device, adb.host_epout, 64, adb.target_epout, NULL, 0);
    }
    u32 cnt;
    u32 count = msg->data_length;

    msg->magic = msg->command ^ 0xffffffff;
    const u8 *_data_ptr = data_ptr;
    msg->data_check = 0;
    while (count--) {
        msg->data_check += *_data_ptr++;
    }
    int ret = usb_bulk_only_send(&adb_device, adb.host_epout, 64, adb.target_epout, (u8 *)msg, sizeof(*msg));
    if (ret < 0) {
        return ret;
    }
    if (data_ptr != NULL) {
        return usb_bulk_only_send(&adb_device, adb.host_epout, 64, adb.target_epout, data_ptr, msg->data_length);
    } else {
        return 0;
    }
}
static int  adb_recv(u8 *buffer, u32 len, u32 timeout)
{
    return usb_bulk_only_receive(&adb_device, adb.host_epin, 64, adb.target_epin, buffer, len);
}
#define     check_usb_status(ret)   do{\
                                        if(ret < 0){\
                                            log_info("%s() @ %d %d\n", __func__, __LINE__, ret);\
                                            return ret;\
                                        }\
                                    }while(0);

static u8 adb_signatrue_data_index ;

u32 adb_auth()
{
    log_info("%s() %d\n", __func__, __LINE__);
    struct amessage msg;
    int ret = 0;
    u8 *cmd_string;
    adb.local_id = 0x58525047;
    adb.remote_id = 0;
    msg.command = A_CNXN;
    msg.arg0 = A_VERSION;
    msg.arg1 = 0x00001000;
    cmd_string = (u8 *)"host::";
    msg.data_length = strlen((const char *)cmd_string) + 1;
    ret = adb_send_packet(&msg, cmd_string);
    check_usb_status(ret);
    memset(&msg, 0, 24);
    ret = adb_recv((u8 *)&msg, 24, 5);
    check_usb_status(ret);

    if (msg.command == A_CNXN) {
        if (adb_recv(adb.buffer, msg.data_length, 1 * 100)) {
            log_error("auth error 0\n");
            return 0;
        }
        log_info("auth not send rsa pub key\n");
        return 0;
    } else if (msg.command == A_AUTH) {

    } else {
        log_error("auth error 1\n");
        return 1;
    }

    ret = adb_recv(adb.buffer, msg.data_length, 1 * 100);
    check_usb_status(ret);
    msg.command = A_AUTH;
    msg.arg0 = ADB_AUTH_SIGNATURE;
    msg.data_length = sizeof(adb_signatrue_data[0]);
    if (adb_signatrue_data_index > 2) {
        adb_signatrue_data_index = 0;
    }

    adb_signatrue_data_index ++;
    cmd_string = (u8 *)&adb_signatrue_data[adb_signatrue_data_index][0];
    ret = adb_send_packet(&msg, cmd_string);
    check_usb_status(ret);

    ret = adb_send_packet(NULL, NULL);//zero packet
    check_usb_status(ret);

    memset(&msg, 0, 24);
    ret = adb_recv((u8 *)&msg, 24, 1 * 100);
    check_usb_status(ret);
    if (msg.command != A_AUTH) {
        log_error("auth error 2\n");
        return 1;
    }
    ret = adb_recv(adb.buffer, msg.data_length, 1 * 100);
    check_usb_status(ret);


__RETRY:
    msg.command = A_AUTH;
    msg.arg0 = ADB_AUTH_RSAPUBLICKEY;
    msg.arg1 = 0;
    msg.data_length = sizeof(adb_rsa_pub_key);
    ret = adb_send_packet(&msg, (u8 *)adb_rsa_pub_key);
    check_usb_status(ret);

    ret = adb_recv((u8 *)&msg, 24, 30 * 100);

    if (ret < 0) {
        if (ret == -DEV_ERR_TIMEOUT) {
            goto __RETRY;
        }
        check_usb_status(ret);
    }

    ret = adb_recv(adb.buffer, msg.data_length, 1 * 100);//最长等待30s,等手机点击确认授权adb
    if (ret < 0) {
        if (ret == -DEV_ERR_TIMEOUT) {
            goto __RETRY;
        }
        check_usb_status(ret);
    }

    if (msg.command == A_AUTH) {
        goto __RETRY;
    }
    return 0;
}

u32 adb_shell_login()
{
    log_info("%s() %d\n", __func__, __LINE__);
    struct amessage msg;
    u8 *cmd_string;
    /* __AUTH_SUCCESS: */

    cmd_string = (u8 *)"shell:";
    msg.command = A_OPEN;
    msg.arg0 = adb.local_id;
    msg.arg1 = 0;
    msg.data_length = strlen((char const *)cmd_string) + 1;
    int ret = adb_send_packet(&msg, cmd_string);
    check_usb_status(ret);

    memset(&msg, 0, 24);
    ret = adb_recv((u8 *)&msg, 24, 1 * 100);
    check_usb_status(ret);

    if (msg.command != A_OKAY) {
        log_error("A_OKAY error\n");
        return 4;
    }

    ret = adb_recv((u8 *)&msg, 24, 1 * 100);
    check_usb_status(ret);
    if (msg.command != A_WRTE) {
        log_error("A_WRTE error\n");
        return 5;
    }
    adb.remote_id = msg.arg0;

    ret = adb_recv(adb.buffer, msg.data_length, 1 * 100);
    check_usb_status(ret);

    msg.command = A_OKAY;
    msg.arg0 = adb.local_id;
    msg.arg1 = adb.remote_id;
    msg.data_length = 0;
    ret = adb_send_packet(&msg, NULL);
    check_usb_status(ret);
    return 0;
}

static u32 adb_ex_cmd(const char *cmd_string, u8 *echo_buffer, u32 max_len)
{
    log_info("%s\n", cmd_string);
    int ret;
    struct amessage msg;
    msg.command = A_WRTE;
    msg.arg0 = adb.local_id;
    msg.arg1 = adb.remote_id;
    msg.data_length = strlen(cmd_string);
    ret = adb_send_packet(&msg, (u8 *)cmd_string);
    check_usb_status(ret);
    memset(&msg, 0, 24);
    memset(echo_buffer, 0, max_len);
    ret = adb_recv((u8 *)&msg, sizeof(msg), 3 * 100);
    check_usb_status(ret);

    if (msg.command != A_OKAY) {
        return true;
    }
    u32 offset = 0;
    do {
        ret = adb_recv((u8 *)&msg, sizeof(msg), 3 * 100);
        check_usb_status(ret);
        if (msg.command != A_WRTE) {
            log_info("command %x\n", msg.command);
            return true;
        }
        if ((offset + msg.data_length)  > max_len) {
            log_info("%s", echo_buffer);
            echo_buffer[offset] = 0;
            offset = 0;
        }

        ret = adb_recv(&echo_buffer[offset], msg.data_length, 3 * 100);
        check_usb_status(ret);
        offset += msg.data_length;

        if (msg.data_length == 0) {
            log_info("no data_length\n");
            break;
        }

        if (offset >= max_len) {

        }
        /* echo_buffer[offset] = '\n'; */
        echo_buffer[offset] = 0;

        if (echo_buffer[offset - 2] == 0x24 && echo_buffer[offset - 1] == 0x20) {
            /* puts("end\n"); */
            break;
        } else if (echo_buffer[offset - 2] == 0x23 && echo_buffer[offset - 1] == 0x20) {
            /* puts("end 1\n"); */
            break;
        }
        msg.command = A_OKAY;
        msg.arg0 = adb.local_id;
        msg.arg1 = adb.remote_id;
        msg.data_length = 0;
        ret = adb_send_packet(&msg, NULL);
        check_usb_status(ret);

    } while (1);

    msg.command = A_OKAY;
    msg.arg0 = adb.local_id;
    msg.arg1 = adb.remote_id;
    msg.data_length = 0;
    /* puts("exit\n"); */
    return adb_send_packet(&msg, NULL);

}
#define     APP_ACTIVITY_PATH   "com.zh-jieli.gmaeCenter/com.zh-jieli.gameCenter.activity.guide.SplashActivity\n"
#define     APP_WEBSITE         "http://www.zh-jieli.com\n"
#define     APP_BASH_IN_PATH    "/sdcard/jilei/active.bash"
#define     APP_BASH_OUT_PATH   "/data/local/tmp/active.bash"

u32 adb_game_active()
{
    log_info("%s() %d\n", __func__, __LINE__);
    u32 max_len = adb.max_len;;
    u8 *adb_buffer = adb.buffer;
    //1，启动app
    adb_ex_cmd("am start -n " APP_ACTIVITY_PATH, adb_buffer, max_len);
    puts((char *)adb_buffer);
    //查找Error字符串，如果找到跳转网页下载app，否则执行adb指令
    if (strstr((const char *)adb_buffer, "Error") != NULL) {
        adb_ex_cmd("am start -a android.intent.action.VIEW -d " APP_WEBSITE, adb_buffer, max_len);
        puts((char *)adb_buffer);
    } else {
        adb_ex_cmd("dd if=" APP_BASH_IN_PATH " of=" APP_BASH_OUT_PATH "\n", adb_buffer, max_len);
        puts((char *)adb_buffer);
        adb_ex_cmd("chown shell " APP_BASH_OUT_PATH";chmod 777 "APP_BASH_OUT_PATH "\n", adb_buffer, max_len);
        puts((char *)adb_buffer);
        adb_ex_cmd("trap \"\" HUP;sh "APP_BASH_OUT_PATH "&\n", adb_buffer, max_len);
        puts((char *)adb_buffer);
    }

    return 0;
}
static void mtp_ptp_open_session(u32 is_mtp)
{
    /* usbh_bulk_send_blocking(ADB_HOST_EP, adb_device.extr_out, (u8 *)_open_session, 16);                        */
    /* usbh_request_bulk_blocking(ADB_HOST_EP, adb_device.extr_in, get_data_buffer(), 12, 3);                     */

    /* usbh_bulk_send_blocking(ADB_HOST_EP, adb_device.extr_out, (u8 *)get_device_info, sizeof(get_device_info)); */
    /* usbh_request_bulk_blocking(ADB_HOST_EP, adb_device.extr_in, get_data_buffer(), 512, 3);                    */

    /* usbh_request_bulk_blocking(ADB_HOST_EP, adb_device.extr_in, get_data_buffer(), 12, 3);                     */
}

static void adb_open_session()
{
    struct usb_host_device *host_dev = adb_device.private_data;
    if (adb.extr_in && adb.extr_out) {
        get_ms_extended_compat_id(host_dev, adb.buffer);
        get_device_status(host_dev);
        get_config_descriptor(host_dev, adb.buffer, 0xff);
        /* mtp_ptp_open_session(ac6921_data_buffer[0x12] == 0x4d); */
    }
}
u32 adb_process()
{
    adb.max_len = 1024;
    adb.buffer = malloc(adb.max_len);
    os_time_dly(20);
    do {

        adb_open_session();

        if (adb_auth()) {
            break;
        }

        if (adb_shell_login()) {
            break;
        }

        adb_game_active();
        log_info("adb active succ");
        return 0;

    } while (0);

    free(adb.buffer);


    log_info("adb active error");
    return 1;
}
void adb_switch_aoa(u32 id)
{
    struct usb_host_device *host_dev = adb_device.private_data;
    aoa_switch(host_dev);
    /* usb_host_remount(id, 3, 30, 50, 1); */
}
#endif //TCFG_ADB_ENABLE
