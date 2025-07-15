#ifndef  __ADB_H__
#define  __ADB_H__

#include "system/task.h"
#include "device/device.h"
#include "usb/scsi.h"
#include "usb_bulk_transfer.h"
#include "usb/host/usb_host.h"
struct adb_device_t {
    u32 local_id;
    u32 remote_id;
    void *buffer;
    u32 max_len;

    u8 target_epin;
    u8 target_epout;
    u8 host_epin;
    u8 host_epout;

    u8 extr_in;
    u8 extr_out;
};
u32 usb_adb_interface_ptp_mtp_parse(struct usb_host_device *host_dev, u8 interface_num, const u8 *pBuf);
int usb_adb_parser(struct usb_host_device *host_dev, u8 interface_num, const u8 *pBuf);
u32 adb_process();
void adb_switch_aoa(u32 id);

#if 1
#define A_SYNC 0x434e5953
#define A_CNXN 0x4e584e43
#define A_OPEN 0x4e45504f
#define A_OKAY 0x59414b4f
#define A_CLSE 0x45534c43
#define A_WRTE 0x45545257
#define A_AUTH 0x48545541
//#define S_ID_LOCAL  0x00003456
/* AUTH packets first argument */
/* Request */
#define ADB_AUTH_TOKEN 1
/* Response */
#define ADB_AUTH_SIGNATURE 2
#define ADB_AUTH_RSAPUBLICKEY 3

#define A_VERSION 0x01000000 // ADB protocol version

#define ADB_VERSION_MAJOR 1 // Used for help/version information
#define ADB_VERSION_MINOR 0 // Used for help/version information

#else
#define A_SYNC 0x53594e43
#define A_CNXN 0x434e584e
#define A_OPEN 0x4f50454e
#define A_OKAY 0x4f4b4159
#define A_CLSE 0x434c5345
#define A_WRTE 0x57525445

#define A_VERSION 0x00000001 // ADB protocol version

#define ADB_VERSION_MAJOR 1 // Used for help/version information
#define ADB_VERSION_MINOR 0 // Used for help/version information

#endif

struct amessage {
    unsigned long int command;     /* command identifier constant      */
    unsigned long int arg0;        /* first argument                   */
    unsigned long int arg1;        /* second argument                  */
    unsigned long int data_length; /* length of payload (0 is allowed) */
    unsigned long int data_check;  /* checksum of data payload         */
    unsigned long int magic;       /* command ^ 0xffffffff             */
};

#endif  /*ADB_H*/
