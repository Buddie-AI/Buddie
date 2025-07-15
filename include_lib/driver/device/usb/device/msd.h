#ifndef  __USBD_MSD_H__
#define  __USBD_MSD_H__

#include "asm/usb.h"
#include "usb_stack.h"

#define MAX_MSD_DEV                 2
#define MSD_DEV_NAME_LEN            12

struct msd_info {
    u8 bError;
    u8 bSenseKey;
    u8 bAdditionalSenseCode;
    u8 bAddiSenseCodeQualifier;
    u8 bDisk_popup[MAX_MSD_DEV];
    void *dev_handle[MAX_MSD_DEV];
    char dev_name[MAX_MSD_DEV][MSD_DEV_NAME_LEN];
    void (*msd_wakeup_handle)(struct usb_device_t *usb_device);
    void (*msd_reset_wakeup_handle)(struct usb_device_t *usb_device, u32 itf_num);
};



u32 msd_desc_config(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num);
void USB_MassStorage(const struct usb_device_t *usb_device);
u32 msd_set_wakeup_handle(void (*handle)(struct usb_device_t *usb_device));
u32 msd_register_disk(const char *name, void *arg);
u32 msd_unregister_disk(const char *name);
u32 msd_unregister_all();
u32 msd_register(const usb_dev id);
u32 msd_release();
void msd_set_reset_wakeup_handle(void (*handle)(struct usb_device_t *usb_device, u32 itf_num));
void msd_reset(struct usb_device_t *usb_device, u32 itf_num);
#endif  /*USBD_MSD_H*/
