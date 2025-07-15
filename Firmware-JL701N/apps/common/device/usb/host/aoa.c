/**
 * @file aoa.c
 * @brief https://source.android.com/devices/accessories/aoa
 *        http://www.hackermi.com/2015-04/aoa-analyse/
 *        Android 开放配件协议 1.0
 * @author chenrixin@zh-jieli.com
 * @version 1
 * @date 2020-03-25
 */

#include "includes.h"
#include "app_config.h"

#include "usb_config.h"
#include "usb/host/usb_host.h"
#include "usb/usb_phy.h"
#include "device_drive.h"
#include "usb_ctrl_transfer.h"
#include "usb_bulk_transfer.h"
#include "usb_storage.h"
#include "adb.h"
#include "aoa.h"
#include "usb_hid_keys.h"
#if TCFG_AOA_ENABLE
#include "gamebox.h"

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[AOA]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

//0x2D00 有一个接口，该接口有两个批量端点，用于输入和输出通信。
//0x2D01 有两个接口，每个接口有两个批量端点，用于输入和输出通信。
//第一个接口处理标准通信，
//第二个接口则处理 ADB 通信。
//要使用接口，请找到第一个批量输入和输出端点，
//使用 SET_CONFIGURATION (0x09) 设备请求将设备配置的值设为 1，然后使用端点进行通信
//


static void aoa_timer_handler(void *priv);
static u32 aoa_timer_id;
static struct aoa_device_t aoa;
static struct device aoa_device;

static int set_power(struct usb_host_device *host_dev, u32 value)
{
    if (aoa_timer_id) {
        usr_timer_del(aoa_timer_id);
        aoa_timer_id = 0;
    }
    return DEV_ERR_NONE;
}

static int get_power(struct usb_host_device *host_dev, u32 value)
{
    return DEV_ERR_NONE;
}
static const struct interface_ctrl aoa_ctrl = {
    .interface_class = USB_CLASS_AOA,
    .set_power = set_power,
    .get_power = get_power,
    .ioctl = NULL,
};

static const struct usb_interface_info aoa_inf = {
    .ctrl = (struct interface_ctrl *) &aoa_ctrl,
    .dev.aoa = &aoa,
};

static const char *credetials[] = {"JieLiTec",
                                   "GameBox",
                                   "Android accessories devcie !",
                                   "1.0.0",
                                   "http://www.zh-jieli.com",
                                   "1234567890ABCDEF",
                                  };

void aoa_switch(struct usb_host_device *host_dev)
{
    u16 version;
    log_info("aoa_switch");
    usb_get_aoa_version(host_dev, &version);
    log_info("AOA version: %x", version);
    for (int i = 0; i < 5; i++) {
        log_info("send string [%d] %s", i, credetials[i]);
        int r = usb_set_credentials(host_dev, credetials[i], i);
        if (r < 0) {
            break;
        }
    }
    usb_switch2aoa(host_dev);
}

int usb_aoa_parser(struct usb_host_device *host_dev, u8 interface_num, const u8 *pBuf)
{
    struct usb_interface_descriptor *interface = (struct usb_interface_descriptor *)pBuf;
    pBuf += sizeof(struct usb_interface_descriptor);
    int len = 0;
    const usb_dev usb_id = host_device2id(host_dev);
    aoa_device.private_data = host_dev;
    host_dev->interface_info[interface_num] = &aoa_inf;

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
                aoa.target_epin = end_desc->bEndpointAddress & 0x0f;
#if defined(FUSB_MODE) && FUSB_MODE == 0
                aoa.rxmaxp = end_desc->wMaxPacketSize;
#endif
            } else {
                aoa.target_epout = end_desc->bEndpointAddress & 0x0f;
#if defined(FUSB_MODE) && FUSB_MODE == 0
                aoa.txmaxp = end_desc->wMaxPacketSize;
#endif
            }
        }
    }


    return len;
}

static int aoa_tx_data(const u8 *pbuf, u32 len)
{
    struct usb_host_device *host_dev = aoa_device.private_data;
    usb_dev usb_id = host_device2id(host_dev);
    g_printf("TX:");
    printf_buf(pbuf, len);
    return usb_h_ep_write_async(usb_id, aoa.host_epout, 64, aoa.target_epout, pbuf, len, USB_ENDPOINT_XFER_BULK, 1);
}
static void aoa_epin_isr(struct usb_host_device *host_dev, u32 ep)
{
    u8 buffer[64] = {0};
    usb_dev usb_id = host_device2id(host_dev);
    u32 rx_len = usb_h_ep_read_async(usb_id, ep, aoa.target_epin, buffer, sizeof(buffer), USB_ENDPOINT_XFER_BULK, 0);
    g_printf("RX:");
    printf_buf(buffer, rx_len);

    usb_h_ep_read_async(usb_id, ep, aoa.target_epin, NULL, 0, USB_ENDPOINT_XFER_BULK, 1);
}

#define     Accessory_assigned_ID   0x0001

u32 aoa_process(u32 mode, u32 id)
{
    struct usb_host_device *host_dev = aoa_device.private_data;
    struct usb_device_descriptor device_desc;
    usb_get_device_descriptor(host_dev, &device_desc);

    if ((device_desc.idVendor == 0x18d1) &&
        ((device_desc.idProduct & 0x2d00) == 0x2d00)) {
        log_info("aoa mode ready idVendor:%x idProduct: %x",
                 device_desc.idVendor, device_desc.idProduct);
    } else {
        log_info("aoa switch idVendor:%x idProduct: %x",
                 device_desc.idVendor, device_desc.idProduct);

        aoa_switch(host_dev);

        usb_host_remount(id, 3, 30, 50, 1);
        return 0;
    }


    usb_aoa_register_hid(host_dev, Accessory_assigned_ID, sizeof(hid_report_desc));

    u32 offset = 0;
    while (offset < sizeof(hid_report_desc)) {
        u32 cnt = min(sizeof(hid_report_desc) - offset, 63);
        usb_aoa_set_hid_report_desc(host_dev, Accessory_assigned_ID, offset, &hid_report_desc[offset], cnt);
        offset += cnt;
    }

    aoa.host_epout = usb_get_ep_num(id, USB_DIR_OUT, USB_ENDPOINT_XFER_BULK);
    aoa.host_epin = usb_get_ep_num(id, USB_DIR_IN, USB_ENDPOINT_XFER_BULK);
    log_debug("D(%d)->H(%d)", aoa.target_epin, aoa.host_epin);
    log_debug("H(%d)->D(%d)",  aoa.host_epout, aoa.target_epout);

    usb_h_set_ep_isr(host_dev, aoa.host_epin | USB_DIR_IN, aoa_epin_isr, host_dev);
    u8 *ep_buffer = usb_h_get_ep_buffer(id, aoa.host_epin | USB_DIR_IN);
    usb_h_ep_config(id, aoa.host_epin | USB_DIR_IN, USB_ENDPOINT_XFER_BULK, 1, 0, ep_buffer, 64);
    int r = usb_h_ep_read_async(id, aoa.host_epin, aoa.target_epin, NULL, 0, USB_ENDPOINT_XFER_BULK, 1);

    ep_buffer = usb_h_get_ep_buffer(id, aoa.host_epout | USB_DIR_OUT);
    usb_h_ep_config(id, aoa.host_epout | USB_DIR_OUT, USB_ENDPOINT_XFER_BULK, 0, 0, ep_buffer, 64);

    aoa_timer_id = usr_timer_add((void *)0, aoa_timer_handler, 4, 0);
    g_printf("aoa succ");

    return 1;
}


static void aoa_timer_handler(void *priv)
{
    struct usb_host_device *host_dev = aoa_device.private_data;

    u8 tx_buffer[32];
    if (mouse_data_send == 1) {
        tx_buffer[0] = MOUSE_POINT_ID;
        memcpy(&tx_buffer[1], &mouse_data, sizeof(mouse_data));
        usb_aoa_send_hid_event(host_dev, Accessory_assigned_ID, tx_buffer, sizeof(mouse_data) + 1);
        memset(&mouse_data, 0, sizeof(mouse_data)) ;
        mouse_data_send = 0;
    }

    tx_buffer[0] = TOUCH_SCREEN_ID;
    struct touch_screen_t t;
    memset(&t, 0, sizeof(t));
    if (point_list_pop(&t)) {
        memcpy(&tx_buffer[1], &t, sizeof(t));
        usb_aoa_send_hid_event(host_dev, Accessory_assigned_ID, tx_buffer, sizeof(t) + 1);
    }

}
#endif
