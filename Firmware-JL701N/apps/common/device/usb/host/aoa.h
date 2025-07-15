#ifndef  __AOA_H__
#define  __AOA_H__

struct aoa_device_t {
    u16 version;

    u8 target_epin;
    u8 target_epout;

    u8 host_epin;
    u8 host_epout;

    struct adb_device_t *adb;
};
u32 aoa_process(u32 mode, u32 id);
int usb_aoa_parser(struct usb_host_device *host_dev, u8 interface_num, const u8 *pBuf);



#endif  /*AOA_H*/
