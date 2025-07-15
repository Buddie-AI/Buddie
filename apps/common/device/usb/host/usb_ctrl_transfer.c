/**
 * @file usb_ctrl_transfer.c
 * @brief usb 控制传输接口
 * @author chenrixin@zh-jieli.com
 * @version 1.00
 * @date 2017-02-09
 */

#include "usb/host/usb_host.h"
#include "usb_ctrl_transfer.h"
#include "app_config.h"
#include "device_drive.h"
#include "gpio.h"
#include "usb/scsi.h"

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if USB_HOST_ENABLE

_WEAK_
void usb_dis_ep0_txdly(const usb_dev id)
{
}
static void ep0_h_isr(struct usb_host_device *host_dev, u32 ep)
{
    usb_dev usb_id = host_device2id(host_dev);
    usb_sem_post(host_dev);
}
/**
 * @brief usb_ctlXfer
 *
 * @param host_dev
 * @param urb
 *
 * @return
 */
static int usb_ctlXfer(struct usb_host_device *host_dev, struct ctlXfer *urb)
{
    u32 ret = DEV_ERR_NONE;
    u8 reg = 0;
    u32 data_len;
    usb_dev usb_id = host_device2id(host_dev);
    u8 devnum = host_dev->private_data.devnum;
    u32 max_packet_size = host_dev->private_data.ep0_max_packet_size;

    usb_write_faddr(usb_id, devnum);
#ifdef USB_HW_20
    usb_write_txfuncaddr(usb_id, 0, devnum);
#endif

    switch (urb->stage) {
    case USB_PID_SETUP :
        usb_write_ep0(usb_id, (u8 *)&urb->setup, 8);
        reg = CSR0H_SetupPkt | CSR0H_TxPktRdy;
        break;
    case USB_PID_IN :
        if (urb->setup.wLength) {
            reg = CSR0H_ReqPkt;
        } else {
            reg = CSR0H_StatusPkt | CSR0H_ReqPkt;
        }
        break;
    case USB_PID_OUT:
        if (urb->setup.wLength) {
            data_len = min(urb->setup.wLength, max_packet_size);
            reg = CSR0H_TxPktRdy;
            usb_write_ep0(usb_id, urb->buffer, data_len);
            urb->setup.wLength -= data_len;
            urb->buffer += data_len;
        } else {
            reg = CSR0H_StatusPkt | CSR0H_TxPktRdy;
        }
        break;
    default :
        break;
    }

#if USB_HOST_ASYNC
    //config ep0 callback fun
    usb_h_set_ep_isr(host_dev, 0, ep0_h_isr, host_dev);
    usb_set_intr_txe(usb_id, 0);
#endif

#ifdef USB_HW_20
    usb_write_csr0(usb_id, reg | CSR0H_DISPING);
#else
    usb_write_csr0(usb_id, reg);
#endif

    u32 st = 0;
    u32 ot = usb_get_jiffies() + 500;
    while (1) {
        if (usb_host_timeout(ot)) {
            log_error("time out %x\n", reg);
            ret = -DEV_ERR_TIMEOUT;
            goto __exit;
        }
        if (usb_h_dev_status(usb_id)) {
            st ++;
        } else {
            st = 0;
        }
        if (((usb_read_devctl(usb_id) & BIT(2)) == 0) || (st > 1000)) {
            log_error("usb%d_offline\n", usb_id);
            ret = -DEV_ERR_OFFLINE;
            goto __exit;
        }

#if USB_HOST_ASYNC
        ret = usb_sem_pend(host_dev, 250); //wait isr
        if (ret) {
            log_error("usb%d_offline\n", usb_id);
            ret = -DEV_ERR_OFFLINE;
            goto __exit;
        }
#endif

        reg = usb_read_csr0(usb_id);
        if (reg & CSR0H_RxStall) {
            log_error(" rxStall CSR0:0x%x", reg);
            ret = -DEV_ERR_CONTROL_STALL;
            goto __exit;
        }
        if (reg & CSR0H_Error) {
            log_error(" Error CSR0:0x%x", reg);
            usb_write_csr0(usb_id, 0);
            ret = -DEV_ERR_CONTROL;
            goto __exit;
        }
        if (USB_PID_IN == urb->stage) {

            if (reg & CSR0H_RxPktRdy) {
                data_len = usb_read_count0(usb_id);
                data_len = min(data_len, urb->setup.wLength);
                usb_read_ep0(usb_id, urb->buffer, data_len);;
                urb->buffer += data_len;
                urb->setup.wLength -= data_len;
                if (data_len < max_packet_size) {
                    urb->setup.wLength = 0;
                }
                if (urb->setup.wLength) {
#ifdef USB_HW_20
                    usb_write_csr0(usb_id, CSR0H_ReqPkt | CSR0H_DISPING);
#else
                    usb_write_csr0(usb_id, CSR0H_ReqPkt);
#endif
                } else {
#ifdef USB_HW_20
                    usb_write_csr0(usb_id, CSR0H_DISPING);
#else
                    usb_write_csr0(usb_id, 0);
#endif
                    break;
                }
            }
        } else {
            if (!(reg & CSR0H_TxPktRdy)) {
                break;
            }
        }
    }
__exit:
    usb_clr_intr_txe(usb_id, 0);
    usb_dis_ep0_txdly(usb_id);
    return ret;
}
/**
 * @brief usb_control_transfers
 *
 * @param struct host_dev
 * @param urb
 *
 * @return
 */
static int usb_control_transfers(struct usb_host_device *host_dev, struct ctlXfer *urb)
{
    usb_dev usb_id = host_device2id(host_dev);

    int res;
    /*SETUP*/

    urb->stage = USB_PID_SETUP;		//SETUP transaction

    res = usb_ctlXfer(host_dev, urb);

    if (res) {
        return res;
    }

    /*IN or OUT*/
    urb->stage = USB_PID_IN;


    while (urb->setup.wLength) {
        if (urb->setup.bRequestType & USB_DIR_IN) {	//Request Direction
            urb->stage = USB_PID_IN;	//IN transaction

            res = usb_ctlXfer(host_dev, urb);

            if (res) {
                return res;
            }

            urb->stage = USB_PID_OUT;
        } else {
            urb->stage = USB_PID_OUT;	//OUT transaction

            res = usb_ctlXfer(host_dev, urb);

            if (res) {
                return res;
            }

            urb->stage = USB_PID_IN;
        }
    }

    res = usb_ctlXfer(host_dev, urb);

    if (res) {
        return res;
    }

    return DEV_ERR_NONE;
}

/**
 * @brief usb_control_msg
 *
 * @param host_dev
 * @param request
 * @param requesttype
 * @param value
 * @param index
 * @param data
 * @param size
 *
 * @return
 */
int usb_control_msg(struct usb_host_device *host_dev,
                    u8 request, u8 requesttype,
                    u16 value, u16 index,
                    void *data, u16 size)
{
    struct ctlXfer urb;
    urb.setup.bRequestType = requesttype;
    urb.setup.bRequest = request;
    urb.setup.wValue = cpu_to_le16(value);
    urb.setup.wIndex = cpu_to_le16(index);
    urb.setup.wLength = cpu_to_le16(size);
    urb.buffer = data;

    return usb_control_transfers(host_dev, &urb);

}

/**
 * @brief usb_clear_feature
 *
 * @param host_dev
 * @param ep
 *
 * @return
 */
int usb_clear_feature(struct usb_host_device *host_dev, u32 ep)
{
    return usb_control_msg(host_dev,  USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0, ep, NULL, 0);
}

int set_address(struct usb_host_device *host_dev, u8 devnum)
{
    return usb_control_msg(host_dev, USB_REQ_SET_ADDRESS, 0,  devnum, 0, NULL, 0);
}

int usb_get_device_descriptor(struct usb_host_device *host_dev, struct usb_device_descriptor *desc)
{
    return usb_control_msg(host_dev,
                           USB_REQ_GET_DESCRIPTOR,
                           USB_DIR_IN,
                           (USB_DT_DEVICE << 8),
                           0,
                           desc,
                           USB_DT_DEVICE_SIZE
                          );
}

int usb_get_string_descriptor(struct usb_host_device *host_dev, struct usb_device_descriptor *desc)
{
    int ret = DEV_ERR_NONE;
#if 0
    struct usb_private_data *private_data = host_dev->private_data ;
    /**********get string language*********/
    u8 buf[16];
    memset(buf, 0x0, sizeof(buf));
    ret = usb_control_msg(host_dev,
                          USB_REQ_GET_DESCRIPTOR,
                          USB_DIR_IN,
                          (USB_DT_STRING << 8),
                          0,
                          desc,
                          sizeof(buf)
                         );
    if (ret) {
        return ret;
    }
    memcpy(&private_data->language, buf + 2, sizeof(private_data->language));

    /**********get manufacturer string**********/
    memset(private_data->manufacturer, 0x0, sizeof(private_data->manufacturer));
    if (desc->iManufacturer) {
        ret = usb_control_msg(host_dev,
                              USB_REQ_GET_DESCRIPTOR,
                              USB_DIR_IN,
                              (USB_DT_STRING << 8) | desc->iManufacturer,
                              0,
                              private_data->manufacturer,
                              sizeof(private_data->manufacturer)
                             );
        if (ret) {
            return ret;
        }
    }
    /**********get product string**********/
    memset(private_data->product, 0x0, sizeof(private_data->product));
    if (desc->iProduct) {
        ret = usb_control_msg(host_dev,
                              USB_REQ_GET_DESCRIPTOR,
                              USB_DIR_IN,
                              (USB_DT_STRING << 8) | desc->iProduct,
                              0,
                              private_data->product,
                              sizeof(private_data->product)
                             );
        if (ret) {
            return ret;
        }
    }
#endif
    return ret;
}

int set_configuration(struct usb_host_device *host_dev)
{
    return  usb_control_msg(host_dev, USB_REQ_SET_CONFIGURATION, 0, 1, 0, NULL, 0);
}
int set_configuration_add_value(struct usb_host_device *host_dev, u16 value)
{
    return  usb_control_msg(host_dev, USB_REQ_SET_CONFIGURATION, 0, value, 0, NULL, 0);
}
int get_config_descriptor(struct usb_host_device *host_dev, void *cfg_desc, u32 len)
{
    return usb_control_msg(host_dev,
                           USB_REQ_GET_DESCRIPTOR,
                           USB_DIR_IN,
                           (USB_DT_CONFIG << 8),
                           0,
                           cfg_desc,
                           len);
}

int get_config_descriptor_add_value_l(struct usb_host_device *host_dev, void *cfg_desc, u32 len, u8 value_l)
{
    return usb_control_msg(host_dev,
                           USB_REQ_GET_DESCRIPTOR,
                           USB_DIR_IN,
                           (USB_DT_CONFIG << 8) | value_l,
                           0,
                           cfg_desc,
                           len);
}


int get_msd_max_lun(struct usb_host_device *host_dev, void *lun)
{
    return usb_control_msg(host_dev,
                           USB_MSD_MAX_LUN,
                           USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                           0,
                           0,
                           lun,
                           1);
}
int set_msd_reset(struct usb_host_device *host_dev)
{
    return usb_control_msg(host_dev,
                           0xff,
                           USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                           0,
                           0,
                           NULL,
                           0);
}

int hid_set_idle(struct usb_host_device *host_dev, u32 id)
{
    return usb_control_msg(host_dev, 0x0a, 0x21, 0, id << 8, NULL, 0);
}
int hid_get_report(struct usb_host_device *host_dev, u8 *report, u8 report_id, u16 report_len)
{
    return usb_control_msg(host_dev,
                           USB_REQ_GET_DESCRIPTOR,
                           USB_DIR_IN | USB_RECIP_INTERFACE,
                           0x2200,
                           report_id,
                           report,
                           report_len);
}
int hid_set_output_report(struct usb_host_device *host_dev, u8 *report, u8 report_id, u8 report_len)
{
    return usb_control_msg(host_dev,
                           0x09,
                           USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                           0x0201,
                           report_id,
                           report,
                           report_len);
}
int usb_set_remote_wakeup(struct usb_host_device *host_dev)
{
    return usb_control_msg(host_dev, USB_REQ_SET_FEATURE, USB_RECIP_DEVICE,
                           USB_DEVICE_REMOTE_WAKEUP, 0, NULL, 0);
}
int get_device_status(struct usb_host_device *host_dev)
{
    u16 status;
    return usb_control_msg(host_dev, USB_REQ_GET_STATUS, USB_DIR_IN, 0, 0, (u8 *)&status, 2);
}
int usb_get_device_qualifier(struct usb_host_device *host_dev, u8 *buffer)
{
    return usb_control_msg(host_dev,
                           USB_REQ_GET_DESCRIPTOR,
                           USB_DIR_IN,
                           (USB_DT_DEVICE_QUALIFIER << 8),
                           0,
                           buffer,
                           0x0a);

}
#define     AOA_CMD51   0x33
#define     AOA_CMD52   0x34
#define     AOA_CMD53   0x35




int usb_get_aoa_version(struct usb_host_device *host_dev, u16 *version)
{
    return usb_control_msg(host_dev,
                           AOA_CMD51,
                           USB_DIR_IN | USB_TYPE_VENDOR,
                           0,
                           0,
                           version,
                           2);
}
int usb_set_credentials(struct usb_host_device *host_dev, const char *string, int index)
{
    return usb_control_msg(host_dev,
                           AOA_CMD52,
                           USB_DIR_OUT | USB_TYPE_VENDOR,
                           0,
                           index,
                           (u8 *)string,
                           strlen(string));
}
int usb_switch2aoa(struct usb_host_device *host_dev)
{
    return usb_control_msg(host_dev,
                           AOA_CMD53,
                           USB_DIR_OUT | USB_TYPE_VENDOR,
                           0,
                           0,
                           NULL,
                           0);
}
int usb_switch2slave(struct usb_host_device *host_dev)
{
    return usb_control_msg(host_dev,
                           0x51,
                           USB_DIR_OUT | USB_TYPE_VENDOR,
                           0,
                           0,
                           NULL,
                           0);
}
/* Control request for registering a HID device.
 * Upon registering, a unique ID is sent by the accessory in the
 * value parameter. This ID will be used for future commands for
 * the device
 *
 *  requestType:    USB_DIR_OUT | USB_TYPE_VENDOR
 *  request:        ACCESSORY_REGISTER_HID_DEVICE
 *  value:          Accessory assigned ID for the HID device
 *  index:          total length of the HID report descriptor
 *  data            none
 */
#define ACCESSORY_REGISTER_HID         54
int usb_aoa_register_hid(struct usb_host_device *host_dev, u16 value, u16 index)
{
    return usb_control_msg(host_dev,
                           ACCESSORY_REGISTER_HID,
                           USB_DIR_OUT | USB_TYPE_VENDOR,
                           value,
                           index,
                           NULL,
                           0);
}
/* Control request for unregistering a HID device.
 *
 *  requestType:    USB_DIR_OUT | USB_TYPE_VENDOR
 *  request:        ACCESSORY_REGISTER_HID
 *  value:          Accessory assigned ID for the HID device
 *  index:          0
 *  data            none
 */
#define ACCESSORY_UNREGISTER_HID         55
int usb_aoa_unregister_hid(struct usb_host_device *host_dev, u16 value)
{
    return usb_control_msg(host_dev,
                           ACCESSORY_UNREGISTER_HID,
                           USB_DIR_OUT | USB_TYPE_VENDOR,
                           value,
                           0,
                           NULL,
                           0);
}

/* Control request for sending the HID report descriptor.
 * If the HID descriptor is longer than the endpoint zero max packet size,
 * the descriptor will be sent in multiple ACCESSORY_SET_HID_REPORT_DESC
 * commands. The data for the descriptor must be sent sequentially
 * if multiple packets are needed.
 *
 *  requestType:    USB_DIR_OUT | USB_TYPE_VENDOR
 *  request:        ACCESSORY_SET_HID_REPORT_DESC
 *  value:          Accessory assigned ID for the HID device
 *  index:          offset of data in descriptor
 *                      (needed when HID descriptor is too big for one packet)
 *  data            the HID report descriptor
 */
#define ACCESSORY_SET_HID_REPORT_DESC         56
int usb_aoa_set_hid_report_desc(struct usb_host_device *host_dev, u16 value, u16 offset, const char *pbuf, u32 len)
{
    return usb_control_msg(host_dev,
                           ACCESSORY_SET_HID_REPORT_DESC,
                           USB_DIR_OUT | USB_TYPE_VENDOR,
                           value,
                           offset,
                           (u8 *)pbuf,
                           len);
}

/* Control request for sending HID events.
 *
 *  requestType:    USB_DIR_OUT | USB_TYPE_VENDOR
 *  request:        ACCESSORY_SEND_HID_EVENT
 *  value:          Accessory assigned ID for the HID device
 *  index:          0
 *  data            the HID report for the event
 */
#define ACCESSORY_SEND_HID_EVENT         57
int usb_aoa_send_hid_event(struct usb_host_device *host_dev, u16 value, const u8 *pbuf, u32 len)
{
    return usb_control_msg(host_dev,
                           ACCESSORY_SEND_HID_EVENT,
                           USB_DIR_OUT | USB_TYPE_VENDOR,
                           value,
                           0,
                           (u8 *)pbuf,
                           len);
}
int get_ms_extended_compat_id(struct usb_host_device *host_dev,  u8 *buffer)
{
    return usb_control_msg(host_dev,
                           0x01,
                           USB_DIR_IN | USB_RECIP_DEVICE | USB_TYPE_VENDOR,
                           0x0000,
                           4,
                           buffer,
                           0x28);
}


int usb_set_interface(struct usb_host_device *host_dev, u8 interface, u8 alternateSetting)
{
    log_info("%s Set Interface:%d AlternateSetting:%d", __func__, interface, alternateSetting);
    return usb_control_msg(host_dev,
                           USB_REQ_SET_INTERFACE,
                           USB_RECIP_INTERFACE,
                           alternateSetting,
                           interface,
                           NULL,
                           0);

}

int usb_audio_sampling_frequency_control(struct usb_host_device *host_dev, u32 ep, u32 sampe_rate)
{
    log_info("%s ep:%d sampe_rate:%d", __func__, ep, sampe_rate);
    return usb_control_msg(host_dev,
                           1,
                           USB_TYPE_CLASS | USB_RECIP_ENDPOINT,
                           0x0100,
                           ep,
                           &sampe_rate,
                           3);
}
int usb_audio_volume_control(struct usb_host_device *host_dev, u8 feature_id, u8 channel_num, u16 volume)
{
    log_info("%s featureID:%d vol:%x", __func__, feature_id, volume);
    return usb_control_msg(host_dev,
                           1,
                           USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                           (0x02 << 8) | channel_num,
                           feature_id << 8,
                           &volume,
                           2);
}
int usb_audio_mute_control(struct usb_host_device *host_dev, u8 feature_id, u8 mute)
{
    log_info("%s featureID:%d mute:%d", __func__, feature_id, mute);
    return usb_control_msg(host_dev,
                           1,
                           USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                           0x0100,
                           feature_id << 8,
                           &mute,
                           1);
}
#endif
