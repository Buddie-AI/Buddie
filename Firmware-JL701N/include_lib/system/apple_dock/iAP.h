/*************************************************************/
/** @file:		usb_device.h
    @brief:		USB 从机驱动重写，加入iAP协议
    @details:
    @author:	Bingquan Cai
    @date: 		2013-10-11,09:34
    @note:
*/
/*************************************************************/

#ifndef _IAP_H_
#define _IAP_H_

#include "generic/typedef.h"
// #include "string.h"
// #include "usb/usb_slave_api.h"

#define IAP2_LINK_EN 			1

#define IAP_USE_USB_HID_EN		1

//USB
#define MAXP_SIZE_INTERRUPT_IN		        0x40
//IIC crack
#define AUTHENTICATION_CONTROL_STATUS		0x10
#define SIGNATURE_DATA_LENGTH 				0x11
#define SIGNATURE_DATA 						0x12
#define CHALLENGE_DATA_LENGTH				0x20
#define CHALLENGE_DATA 						0x21
#define ACCESSORY_CERTIFICATE_DATA_LENGTH	0x30
#define ACCESSORY_CERTIFICATE_DATA			0x31
//iAP1 and iAP2 COMMON
#define IAP2_VERSIONS  						0x02
#define IAP1_VERSIONS  						0x01
#define DEFAULT_SEQUENCE_NUM				0x0
#define EXTRA_SEQUENCE_NUM					0x1
//"Dock speaker"
#define ACCESSORY_NAME						0x44, 0x6f, 0x63, 0x6b, 0x20, 0x73, 0x70, 0x65, 0x61, 0x6b, 0x65, 0x72, 0x00
//"IPDLI13"
#define ACCESSORY_MODEL_IDENTIFIER		   	0x49, 0x50, 0x44, 0x4c, 0x49, 0x31, 0x33, 0x00
//"I want it"
#define ACCESSORY_MANUFACTURER				0x49, 0x20, 0x77, 0x61, 0x6e, 0x74, 0x20, 0x69, 0x74, 0x00
//"iAP Interface"
#define ACCESSORY_SERIALNUMBER				0x69, 0x41, 0x50, 0x20, 0x49, 0x6e, 0x74, 0x65, 0x72, 0x66, 0x61, 0x63, 0x65, 0x00

//< hid
#define HID_PREV_FILE			USB_AUDIO_PREFILE
#define HID_NEXT_FILE			USB_AUDIO_NEXTFILE
#define HID_PP					USB_AUDIO_PP
#define HID_PLAY				USB_AUDIO_PLAY
#define HID_PAUSE				USB_AUDIO_PAUSE
#define HID_VOL_DOWN			USB_AUDIO_VOLDOWN
#define HID_VOL_UP				USB_AUDIO_VOLUP


//<iap printf
#include "uart.h"
#define 	IAP_CHIP_PRINTF

#ifdef	IAP_CHIP_PRINTF
#define	iAP_deg_str(x)				puts(x)
#define	iAP_printf					printf
#define iAP_printf_buf(x, y)		put_buf(x, y)
#define iAP_put_u8hex(x)			put_u8hex(x)
#define iAP_put_u16hex(x)			put_u16hex(x)
#define iAP_put_u32hex(x)			put_u32hex(x)
#else
#define	iAP_deg_str(x)
#define	iAP_printf(...)
#define iAP_printf_buf(x, y)
#define iAP_put_u8hex(x)
#define iAP_put_u16hex(x)
#define iAP_put_u32hex(x)
#endif

#define     _xdata
// #define     _data
#define     _root
#define     _no_init
#define     _banked_func


///variable
typedef enum {
    iAP_IDLE = 0,
    iAP_INTERRUPT,
    iAP_BULK_IN,
    iAP_BULE_OUT,
} iAP_STATE;


enum {
    NO_MFI_CHIP = 0,
    MFI_PROCESS_READY,
    MFI_PROCESS_STANDBY,
    MFI_PROCESS_PASS,

    IAP1_LINK_ERR,
    IAP1_LINK_SUCC,
    IAP2_LINK_ERR,
    IAP2_LINK_SUCC,
};

//extern var
extern u8 chip_online_status;
extern u8 mfi_pass_status;

#define apple_mfi_chip_online_lib()		chip_online_status
#define apple_link_disable()			mfi_pass_status = MFI_PROCESS_STANDBY
#define apple_link_init()				mfi_pass_status = MFI_PROCESS_STANDBY
#define apple_mfi_pass_ready_set_api()	mfi_pass_status = MFI_PROCESS_READY

#define apple_hid_key_api(key)			iap2_hid_key(key)

///outside call parameter
extern u8  *iAP_var_pRxBuf ;
extern u8  *iAP_var_pTxBuf ;
extern u16  iAP_var_wLength;

extern u8  iAP_send_pkt_self[0x40] ;
extern u8  iAP_receive_pkt_self[0x40] ;

///inside call
// void my_code_memcpy(u8  *s1, const u8 *s2, u16 len);
#define my_code_memcpy(x,y,z)		memcpy(x,(u8  *)y,z)
// u8 my_memcmp(u8  *s1, const u8 *s2, u8 len);
#define my_code_memcmp(x,y,z)		memcmp(x,(u8  *)y,z)

u8 iAP_support(void);

u8 apple_mfi_link(void *hdl);
void apple_mfi_unlink(void);

// #define iAP_host_online()		(get_usb_online_status() & BIT(0))
// #define iAP_device_online()		(get_usb_online_status() & BIT(1))

#define GET_U16L(x)				(u8)(x)
#define GET_U16H(x)				(u8)((u16)(x)>>8)
#define SW16(x)					(((u16)(x) & 0x00ffU)<<8 | ((u16)(x) & 0xff00U)>>8)

#endif	/*	_IAP_H_	*/
