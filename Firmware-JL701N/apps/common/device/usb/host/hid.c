#include "includes.h"
#include "asm/includes.h"
#include "app_config.h"
#include "system/timer.h"
#include "device/ioctl_cmds.h"
#include "device_drive.h"
#if TCFG_HID_HOST_ENABLE
#include "usb/host/usb_host.h"
#include "usb_ctrl_transfer.h"
#include "usb_bulk_transfer.h"
#include "hid.h"
#include "usb_config.h"
#include "usb_hid_keys.h"

#define MAIN_ITEM    0
#define GLOBAL_ITEM  1
#define LOCAL_ITEM   2

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[HID]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


struct hid_device_t hid_device[USB_MAX_HW_NUM][MAX_HOST_INTERFACE];

static int set_power(struct usb_host_device *host_dev, u32 value)
{
    const usb_dev usb_id = host_device2id(host_dev);
    memset(hid_device[usb_id], 0, sizeof(hid_device[usb_id]));
    return DEV_ERR_NONE;
}

static int get_power(struct usb_host_device *host_dev, u32 value)
{
    return DEV_ERR_NONE;
}

static const struct interface_ctrl hid_ctrl = {
    .interface_class = USB_CLASS_HID,
    .set_power = set_power,
    .get_power = get_power,
    .ioctl = NULL,
};

static const struct usb_interface_info _usb_if[USB_MAX_HW_NUM][MAX_HOST_INTERFACE] = {
    {
        {
            .ctrl = (struct interface_ctrl *) &hid_ctrl,
            .dev.hid = &hid_device[0][0],
        },
        {
            .ctrl = (struct interface_ctrl *) &hid_ctrl,
            .dev.hid = &hid_device[0][1],
        },
        {
            .ctrl = (struct interface_ctrl *) &hid_ctrl,
            .dev.hid = &hid_device[0][2],
        },
        {
            .ctrl = (struct interface_ctrl *) &hid_ctrl,
            .dev.hid = &hid_device[0][3],
        },
    },
#if USB_MAX_HW_NUM > 1
    {
        {
            .ctrl = (struct interface_ctrl *) &hid_ctrl,
            .dev.hid = &hid_device[1][0],
        },
        {
            .ctrl = (struct interface_ctrl *) &hid_ctrl,
            .dev.hid = &hid_device[1][1],
        },
        {
            .ctrl = (struct interface_ctrl *) &hid_ctrl,
            .dev.hid = &hid_device[1][2],
        },
        {
            .ctrl = (struct interface_ctrl *) &hid_ctrl,
            .dev.hid = &hid_device[1][3],
        },
    },
#endif
};

static u8 interval[USB_MAX_HW_NUM][16];

int usb_hid_parser(struct usb_host_device *host_dev, u8 interface_num, const u8 *pBuf)
{
    struct usb_interface_descriptor *interface = (struct usb_interface_descriptor *)pBuf;
    pBuf += sizeof(struct usb_interface_descriptor);
    int len = 0;
    const usb_dev usb_id = host_device2id(host_dev);

    struct usb_endpoint_descriptor *endpoint;

    pBuf += 9;//hid desc;

    const struct usb_interface_info *usb_if = &_usb_if[usb_id][interface_num];

    memset(usb_if->dev.p, 0, sizeof(struct hid_device_t));

    host_dev->interface_info[interface_num] = usb_if;
    usb_if->dev.hid->parent = host_dev;

    log_info("hid eps %d  %d %x %x", interface->bNumEndpoints, interface_num, usb_if, usb_if->dev.p);

    log_info("parent %x hid @ interface %d usb_if %x hid %x",
             host_dev, interface_num, usb_if, usb_if->dev.hid);

    if ((interface->bInterfaceProtocol == 0x02) ||
        (interface->bInterfaceProtocol == 0x01)) { //mouse & keyboard

        usb_if->dev.hid->bNumEndpoints = interface->bNumEndpoints;
        usb_if->dev.hid->report_list[0].usage = interface->bInterfaceProtocol;
        for (int i = 0 ; i < interface->bNumEndpoints; i++) {
            endpoint = (struct usb_endpoint_descriptor *)pBuf;
            if (USB_DIR_IN & endpoint->bEndpointAddress) {
                const u8 ep = endpoint->bEndpointAddress & 0x0f;
                usb_if->dev.hid->ep_pair[i] = ep;
                interval[usb_id][ep] = endpoint->bInterval;
                log_info("interfacenum = %d,endpoint = %x interval = %x",
                         interface->bInterfaceNumber, endpoint->bEndpointAddress, endpoint->bInterval);
            }
            pBuf += endpoint->bLength;
        }
    } else {
        log_info("vendor");
        host_dev->interface_info[interface_num] = NULL; //???
        for (int i = 0 ; i < interface->bNumEndpoints; i++) {
            endpoint = (struct usb_endpoint_descriptor *)pBuf;
            if (USB_DIR_IN & endpoint->bEndpointAddress) {
                /* interface_endpoint[interface->bInterfaceNumber] = endpoint->bEndpointAddress & 0x0f; */
                log_info("interfacenum = %d,endpoint = %x interval = %x",
                         interface->bInterfaceNumber, endpoint->bEndpointAddress, endpoint->bInterval);
            }
            pBuf += endpoint->bLength;
        }
        return sizeof(struct usb_interface_descriptor);
    }

    return pBuf - (u8 *)interface;
}

static u32 _hid_report_parse(struct hid_device_t *hid, const u8 *report, u32 len)
{
    hid->report_count = 0;
    struct report_info_t null_rpt;
    struct report_info_t *const rpt = &null_rpt;

    memset(rpt, 0, sizeof(*rpt));

    unsigned char ops;
    int index = 0;
    u8 report_size = 0;
    u8 report_count = 0;
    u8 cur_ops_is_report_size_count = 0;
    u8 old_ops_is_report_size_count = 0;
    s8 cur_section_bit = 0;
    u8 input_bit_index = 0;
    u8 total_bits = 0;
    u8 output_bit_index = 0;
    u8 cur_usage = 0;
    u32 undef_type = 0;
    u8 undef_usage = 0;
    u8 collection_deep = 0;

    while (index < len) {
        ops = (char)report[index++];
        char bSize = ops & 0x03;
        bSize = bSize == 3 ? 4 : bSize; // size is 4 when bSize is 3
        char bType = (ops >> 2) & 0x03;
        char bTag = (ops >> 4) & 0x0F;

        cur_ops_is_report_size_count = 0;
        char bSizeActual = 0;
        int itemVal = 0;
        for (int j = 0; j < bSize; j++) {
            if (index + j < len) {
                itemVal += report[index + j] << (8 * j);
                bSizeActual++;
            }
        }
        if (undef_type) {
            undef_type ++;
            if (bTag == 0x0A) {
                undef_usage ++;
            } else if (bTag == 0x0C) {
                undef_usage --;
            }
            if (undef_usage == 0 && undef_type > 2) {
                undef_type = 0;
            }
            index += bSize;
            continue;
        }
        if (undef_type) {
            index += bSize;
            continue;
        }
        if (itemVal == 0xffb5) {
            undef_type = 1;
            index += bSize;
            continue;
        } else {
            undef_type = 0;
        }
        if (bType == MAIN_ITEM) {
            if (old_ops_is_report_size_count) {
                cur_section_bit += report_size * report_count;
            }
            if (bTag == 0x08) {
                /* log_info("input %X", itemVal);                       */
                /* log_info("\tusage %x", cur_usage);                   */
                /* log_info("\t\tcur_section_bit %d", cur_section_bit); */
                if (rpt->usage == 0x02) { //mouse
                    if (cur_usage == 1) {
                        if (rpt->btn_start_bit == 0) {
                            rpt->btn_start_bit = total_bits ;
                        }
                        rpt->btn_width += cur_section_bit ;
                        /* log_info("btn_width %d-%d", rpt->btn_start_bit, rpt->btn_width); */
                    } else if ((cur_usage == 0x30) || (cur_usage == 0x31)) {

                        if (rpt->xy_start_bit == 0) {
                            rpt->xy_start_bit = total_bits;
                        }
                        rpt->xy_width = cur_section_bit;
                        /* log_info("xy_width %d-%d", rpt->xy_start_bit, rpt->xy_width); */
                    } else if (cur_usage == 0x38) {
                        if (rpt->wheel_start_bit == 0) {
                            rpt->wheel_start_bit = total_bits;
                        }
                        if (rpt->xy_width || cur_section_bit < 24) {
                            rpt->wheel_width = cur_section_bit;
                            /* log_info("wheel_width %d-%d", rpt->wheel_start_bit, rpt->wheel_width); */
                        } else {
                            rpt->wheel_width = rpt->xy_width = cur_section_bit / 3;

                            rpt->xy_start_bit = total_bits;
                            rpt->wheel_start_bit = rpt->xy_start_bit + rpt->xy_width * 2;
                            /* log_info("wheel_width %d-%d", rpt->wheel_start_bit, rpt->wheel_width); */
                            /* log_info("xy_width %d-%d", rpt->xy_start_bit, rpt->xy_width);          */
                        }
                    } else if (cur_usage == 0xb8) {
                        rpt->wheel_width = rpt->xy_width = cur_section_bit / 4;
                        rpt->xy_start_bit = total_bits;
                        rpt->wheel_start_bit = rpt->xy_start_bit + rpt->xy_width * 2;
                    }
                }
                total_bits += cur_section_bit;
                /* input_bit[input_bit_index++] = cur_section_bit; */
                cur_section_bit = -1;
            } else if (bTag == 0x09) {
                /* log_info("OUTPUT %X", itemVal);                      */
                /* log_info("\tusage %x", cur_usage);                   */
                /* log_info("\t\tcur_section_bit %d", cur_section_bit); */
                /* output_bit[output_bit_index++] = cur_section_bit; */
                cur_section_bit = -1;
            } else if (bTag == 0x0B) {
                /* log_info("Feature %X", itemVal);                     */
                /* log_info("\tusage %x", cur_usage);                   */
                /* log_info("\t\tcur_section_bit %d", cur_section_bit); */
                /* output_bit[output_bit_index++] = cur_section_bit; */
                cur_section_bit = -1;
            } else if (bTag == 0x0A) {
                collection_deep ++ ;
                log_info("Collection %d %x", collection_deep, rpt->usage);
            } else if (bTag == 0x0C) {
                collection_deep --;
                log_info("End Collection %d %x", collection_deep, rpt->usage);
                if (collection_deep == 0) {
                    if (rpt->usage == 0x02 ||
                        rpt->usage == 0x06 ||
                        rpt->usage == 0x07) {

                        memcpy(&hid->report_list[hid->report_count], rpt, sizeof(*rpt));
                        memset(rpt, 0, sizeof(*rpt));

                        hid->report_count ++;
                    }
                }
                if (index < len) {
                    continue;
                }
            } else {
                log_info("MAIN_ITEM Unknown btag :%x", bTag);
                return 1;
            }
        } else if (bType == GLOBAL_ITEM) {
            /* log_info("GLOBAL_ITEM"); */
            if (bTag == 0x00) {
                /* log_info("Usage Page %x", itemVal); */

                if (rpt->usage == 0x06) {
                    if (itemVal == 0x07) {
                        rpt->usage = 0x07;
                        log_info("re set type %x", 0x07);
                    }
                }
                if (itemVal == 0x02) {
                    rpt->usage = 0x02;
                    log_info("re set type %x", 0x02);
                }

            } else if (bTag == 0x01) {
                //log_info("Logical Minimum %x", itemVal);
            } else if (bTag == 0x02) {
                //log_info("Logical Maximum %x", itemVal);
            } else if (bTag == 0x03) {
                /* log_info("Physical Minimum %x", itemVal); */
            } else if (bTag == 0x04) {
                /* log_info("Physical Maximum %x", itemVal); */
            } else if (bTag == 0x05) {
                /* log_info("Unit Exponent %x", itemVal); */
            } else if (bTag == 0x06) {
                /* log_info("Unit %x", itemVal); */
            } else if (bTag == 0x07) {
                /* log_info("Report Size %x", itemVal); */
                report_size = itemVal;
                cur_ops_is_report_size_count = 1;
            } else if (bTag == 0x08) {
                log_info("Report ID %x", itemVal, rpt->usage);
                rpt->report_id = itemVal;
            } else if (bTag == 0x09) {
                /* log_info("Report Count %x", itemVal); */
                report_count = itemVal;
                cur_ops_is_report_size_count = 1;
            } else if (bTag == 0x0A) {
                /* log_info("Push %x", bSizeActual); */
            } else if (bTag == 0x0B) {
                /* log_info("Pop %x", bSizeActual); */
            } else {
                log_info("GLOBAL_ITEM Unknown btag :%x", bTag);
                return 2;
            }
        } else if (bType == LOCAL_ITEM) {
            /* log_info("LOCAL_ITEM"); */
            if (bTag == 0x00) {
                if (rpt->usage == 0) {
                    rpt->usage = itemVal;
                    log_info("set type %x", rpt->usage);
                }
                if (itemVal == 0x30) { //X
                } else if (itemVal == 0x31) { //y
                } else if (itemVal == 0x38) { //wheel
                } else {
                }
                /* log_info("\t change usage %x -> %x", cur_usage, itemVal); */
                cur_usage = itemVal;
                if (!collection_deep) {
                    if (itemVal == 0x06 || itemVal == 0x07 || itemVal == 0x02) {
                        //仅限键盘和鼠标
                        rpt->usage = itemVal;
                        log_info("set typee %x", rpt->usage);
                    }
                }
                /* type = itemVal; */
            } else if (bTag == 0x01) {
                // log_info("Usage Minimum %x", itemVal);
            } else if (bTag == 0x02) {
                // log_info("Usage Maximum %x", itemVal);
            } else if (bTag == 0x03) {
                /* log_info("Designator Index %x", itemVal); */
            } else if (bTag == 0x04) {
                /* log_info("Designator Minimum %x", itemVal); */
            } else if (bTag == 0x05) {
                /* log_info("Designator Maximum %x", itemVal); */
            } else if (bTag == 0x07) {
                /* log_info("String Index %x", itemVal); */
            } else if (bTag == 0x08) {
                /* log_info("String Minimum %x", itemVal); */
            } else if (bTag == 0x09) {
                /* log_info("String Maximum %x", itemVal); */
            } else if (bTag == 0x0A) {
                /* log_info("Delimiter %x", itemVal); */
            } else {
                log_info("LOCAL_ITEM Unknown btag :%x", bTag);
                return 3;
            }
        } else {
            log_info("OTHER Unknown btag :%x", bTag);
            return 4;
        }
        if (!cur_ops_is_report_size_count && old_ops_is_report_size_count) {
            if (cur_section_bit != -1) {
                cur_section_bit += report_size * report_count;
            } else {
                cur_section_bit = 0;
            }
        }
        if (cur_section_bit == -1) {
            cur_section_bit = 0;
        }
        old_ops_is_report_size_count = cur_ops_is_report_size_count;
        index += bSize;

    }
    return 0;
}
static u32 hid_report_parse(struct hid_device_t *hid, const u8 *report, u32 len)
{
    u8 type = _hid_report_parse(hid, report, len);
    for (int i = 0; i < hid->report_count; i++) {
        struct report_info_t *rpt = &hid->report_list[i];
        if (rpt->usage == 0x02) {
            if (rpt->report_id == 0) {
                rpt->report_id = 0xff;
            }
            rpt->btn_start_bit /= 8;
            rpt->btn_width /= 8;
            rpt->xy_start_bit /= 8;
            rpt->xy_width /= 8;
            rpt->wheel_start_bit /= 8;
            rpt->wheel_width /= 8;

            log_info("mouse report_id %d",
                     rpt->report_id);

            log_info("btn_width %d-%d",
                     rpt->btn_start_bit, rpt->btn_width);
            log_info("xy_width %d-%d",
                     rpt->xy_start_bit, rpt->xy_width);
            log_info("wheel_width %d-%d",
                     rpt->wheel_start_bit, rpt->wheel_width);

            if (rpt->btn_width != 2) {
                rpt->btn_width = 1;
            }

            log_info("btn_width %d-%d",
                     rpt->btn_start_bit, rpt->btn_width);

        } else if (rpt->usage == 6 || rpt->usage == 7) {
            if (rpt->report_id == 0) {
                rpt->report_id = 0xff;
            }
            log_info("keyboard report_id %d", rpt->report_id);
        } else {
            log_info("unknown usage %d", rpt->usage);
        }
    }
    return 0;
}

void mouse_route(const struct mouse_data_t *p);
/* __attribute__((weak)) void mouse_route(const struct mouse_data_t *p) */
/* {                                                                    */
/*     log_info("btn: %x x-y %d %d wheel %d ac_pan %d",                 */
/*              p->btn, p->x, p->y, p->wheel, p->ac_pan);               */
/* }                                                                    */
static void hid_convert_mouse(const struct report_info_t *mouse, const u8 *buffer)
{
    struct mouse_data_t mouse_data;
    memset(&mouse_data, 0, sizeof(mouse_data));
    if (mouse->report_id != 0xff) {
        if (mouse->report_id != buffer[0]) {
            log_error("report_id = %x buffer[0] = %x", mouse->report_id, buffer[0]);
            return;
        }
        buffer++;
    }
    const u8 *ptr;
    ptr = &buffer[mouse->btn_start_bit];
    if (mouse->btn_width == 2) {
        mouse_data.btn = ptr[0] | (ptr[1] << 8);
    } else {
        mouse_data.btn = ptr[0] ;
    }
    s16 tmp;
    ptr = &buffer[mouse->xy_start_bit];
    if (mouse->xy_width == 1 || mouse->xy_width == 2) {
        mouse_data.x = (char)ptr[0];
        mouse_data.y = (char)ptr[1];
    } else if (mouse->xy_width == 4) {
        mouse_data.x = ptr[0] | (ptr[1] << 8);
        ptr += 2;
        mouse_data.y = ptr[0] | (ptr[1] << 8);
    } else if (mouse->xy_width == 3) {
        tmp = (ptr[1] & 0xf) << 12 | ptr[0] << 4;
        tmp = tmp >> 4;
        mouse_data.x = tmp;

        tmp = (ptr[2] << 8) | ((ptr[1] >> 4) << 4);
        tmp = tmp >> 4;
        mouse_data.y = tmp;
    } else {
        log_error("error mouse xy_width %d", mouse->xy_width);
    }

    ptr = &buffer[mouse->wheel_start_bit];

    if (mouse->wheel_width == 1) {
        mouse_data.wheel = (char)ptr[0];
    } else {
        mouse_data.wheel = ptr[0] | (ptr[1] << 8);
    }

    mouse_route(&mouse_data);
}
__attribute__((weak)) void keyboard_route(const u8  *p)
{
    log_info("keyboard_buffer:");
    printf_buf(p, 12);
}
void hid_convert_krbd(const struct report_info_t *kbd, u8 *buffer)
{
#if 0
    u8 keyboard_buffer[12];
    memset(keyboard_buffer, 0, sizeof(keyboard_buffer));
    if (kbd->report_id != 0xff) {
        if (kbd->report_id != buffer[0]) {
            log_error("report_id = %x buffer[0] = %x", kbd->report_id, buffer[0]);
            return;
        }
        buffer++;
    }

    u8 idx = 0;
    u8 index = 0;
    int i = 0;
    for (i = 0; i < 8; i++) {
        if (buffer[0] & BIT(i)) {
            keyboard_buffer[idx++] = 0xe0 + i;
        }
    }

    if (buffer[1] == 0) {
        buffer += 2;
    }
    index = idx;
    for (; idx < 12; idx++) {
        if (*buffer) {
            keyboard_buffer[index] = *buffer;
            index++;
        }
        buffer ++;
    }
    keyboard_route(keyboard_buffer);
#else
    if (kbd->report_id != 0xff) {
        if (kbd->report_id != buffer[0]) {
            log_error("report_id = %x buffer[0] = %x", kbd->report_id, buffer[0]);
            return;
        }
        buffer++;
    }
    u8 keyboard_buffer[8];
    memset(keyboard_buffer, 0, sizeof(keyboard_buffer));

    keyboard_buffer[0] = *buffer;
    buffer ++;

    if (*buffer == 0) {
        buffer ++;
    }
    keyboard_buffer[1] = 0;

    /* memcpy(&keyboard_buffer[2], buffer, 6); */
    u8 pos = 2;
    for (int i = pos; i < 8; i++) {
        if (*buffer) {
            keyboard_buffer[pos] = *buffer;
            pos++;
        }
        buffer++;
    }
    keyboard_route(keyboard_buffer);
#endif
}

static void hid_route(const struct hid_device_t *hid, const u8 *buffer)
{
    for (int i = 0; i < hid->report_count; i++) {
        if (hid->report_list[i].usage == 0x02) {
            hid_convert_mouse(&hid->report_list[i], buffer);
        } else if ((hid->report_list[i].usage == 0x06) ||
                   (hid->report_list[i].usage == 0x07)) {
            hid_convert_krbd(&hid->report_list[i], buffer);
        } else {
            r_printf("usage %x", hid->report_list[i].usage);
        }
    }
}
static void hid_isr(struct usb_interface_info *usb_if, u32 ep)
{
    u8 buffer[64] = {0};
    struct usb_host_device *host_dev = usb_if->dev.hid->parent;
    usb_dev usb_id = host_device2id(host_dev);
    u32 target_ep = usb_if->dev.hid->ep_pair[ep];
    u32 rx_len = usb_h_ep_read_async(usb_id, ep, target_ep, buffer, 64, USB_ENDPOINT_XFER_INT, 0);

    if (rx_len) {
        hid_route(usb_if->dev.hid, buffer);
    }
    /* printf_buf(buffer, rx_len);    */

    usb_h_ep_read_async(usb_id, ep, target_ep, buffer, 8, USB_ENDPOINT_XFER_INT, 1);
}
void hid_process(u32 id)
{
    struct usb_host_device *host_dev = host_id2device(id);
    u8 report[256 + 2];
    u8 ep_pair[4];

    for (u8 i = 0; i < MAX_HOST_INTERFACE; i++) {
        struct usb_interface_info  *usb_if = host_dev->interface_info[i];
        log_info("parent %x hid @ interface %d usb_if %x hid %x",
                 host_dev, i, usb_if, usb_if ? usb_if->dev.hid : 0);
        if (usb_if &&
            (usb_if->ctrl->interface_class == USB_CLASS_HID)) {
            hid_set_idle(host_dev, i);
            memset(report, 0, sizeof(report));
            hid_get_report(host_dev, report, i, 0xff);
            printf_buf(report, 256);
            hid_report_parse(usb_if->dev.hid, report, 256);
            memcpy(ep_pair, usb_if->dev.hid->ep_pair, 4);

            if (usb_if->dev.hid->report_count == 0) {
                continue;
            }

            for (int i = 0; i < usb_if->dev.hid->bNumEndpoints; i++) {

                u32 host_ep = usb_get_ep_num(id, USB_DIR_IN, USB_ENDPOINT_XFER_INT);

                ASSERT(host_ep != -1, "ep not enough");

                u32 target_ep = ep_pair[i];

                usb_if->dev.hid->ep_pair[host_ep] = target_ep;

                log_info("D2H ep: %x --> %x interval %d",
                         target_ep, host_ep, interval[id][target_ep]);

                usb_h_set_ep_isr(host_dev, host_ep | USB_DIR_IN, hid_isr, usb_if);

                u8 *ep_buffer = usb_h_get_ep_buffer(id, host_ep | USB_DIR_OUT);
                usb_h_ep_config(id,  host_ep | USB_DIR_IN, USB_ENDPOINT_XFER_INT, 1,
                                interval[id][target_ep], ep_buffer, 64);
                int r = usb_h_ep_read_async(id, host_ep, target_ep, NULL, 0, USB_ENDPOINT_XFER_INT, 1);
            }
        } else {
            if (usb_if) {
                log_error("error hid class %x", usb_if->ctrl->interface_class);
            }
        }
    }

}

#endif
