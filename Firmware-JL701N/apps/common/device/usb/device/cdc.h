#ifndef __CDC_H__
#define __CDC_H__

#include "usb/device/usb_stack.h"

#ifdef __cplusplus
extern "C" {
#endif

u32 cdc_desc_config(const usb_dev usb_id, u8 *ptr, u32 *itf);
void cdc_set_wakeup_handler(void (*handle)(struct usb_device_t *usb_device));
void cdc_set_output_handle(void *priv, int (*output_handler)(void *priv, u8 *buf, u32 len));
u32 cdc_read_data(const usb_dev usb_id, u8 *buf, u32 len);
u32 cdc_write_data(const usb_dev usb_id, u8 *buf, u32 len);
u32 cdc_write_inir(const usb_dev usb_id, u8 *buf, u32 len);
void cdc_register(const usb_dev usb_id);
void cdc_release(const usb_dev usb_id);

#ifdef __cplusplus
}
#endif

#endif
