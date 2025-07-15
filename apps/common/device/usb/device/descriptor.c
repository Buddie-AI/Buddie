/**
 * @file descriptor.c
 * @brief overwrite usb device descriptor
 * @version 1.00
 * @date 2019-05-06
 */

#include "usb/device/usb_stack.h"
#include "usb/device/descriptor.h"
#include "usb/device/uac_audio.h"

#include "app_config.h"

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if TCFG_USB_SLAVE_ENABLE

static const u8 sDeviceDescriptor[] = { //<Device Descriptor
    USB_DT_DEVICE_SIZE,      // bLength: Size of descriptor
    USB_DT_DEVICE,       // bDescriptorType: Device
#if defined(FUSB_MODE) && FUSB_MODE
    0x10, 0x01,     // bcdUSB: USB 1.1
#elif defined(FUSB_MODE) && (FUSB_MODE ==0 )
    0x00, 0x02,     // bcdUSB: USB 2.0
#else
#error "USB_SPEED_MODE not defined"
#endif
    0x00,       // bDeviceClass: none
    0x00,       // bDeviceSubClass: none
    0x00,       // bDeviceProtocol: none
    EP0_SETUP_LEN,//EP0_LEN,      // bMaxPacketSize0: 8/64 bytes
    'J', 'L',     // idVendor: 0x4a4c - JL
    'U', 'A',     // idProduct: chip id
    0x00, 0x01,     // bcdDevice: version 1.0
    0x01,       // iManufacturer: Index to string descriptor that contains the string <Your Name> in Unicode
    0x02,       // iProduct: Index to string descriptor that contains the string <Your Product Name> in Unicode
    0x03,       // iSerialNumber: none
    0x01        // bNumConfigurations: 1
};

static const u8 LANGUAGE_STR[] = {
    0x04, 0x03, 0x09, 0x04
};


static const u8 product_string[] = {
    42,
    0x03,
    'U', 0x00,
    'S', 0x00,
    'B', 0x00,
    ' ', 0x00,
    'C', 0x00,
    'o', 0x00,
    'm', 0x00,
    'p', 0x00,
    'o', 0x00,
    's', 0x00,
    'i', 0x00,
    't', 0x00,
    'e', 0x00,
    ' ', 0x00,
    'D', 0x00,
    'e', 0x00,
    'v', 0x00,
    'i', 0x00,
    'c', 0x00,
    'e', 0x00,
};

static const u8 MANUFACTURE_STR[] = {
    34,         //该描述符的长度为34字节
    0x03,       //字符串描述符的类型编码为0x03
    0x4a, 0x00, //J
    0x69, 0x00, //i
    0x65, 0x00, //e
    0x6c, 0x00, //l
    0x69, 0x00, //i
    0x20, 0x00, //
    0x54, 0x00, //T
    0x65, 0x00, //e
    0x63, 0x00, //c
    0x68, 0x00, //h
    0x6e, 0x00, //n
    0x6f, 0x00, //o
    0x6c, 0x00, //l
    0x6f, 0x00, //o
    0x67, 0x00, //g
    0x79, 0x00, //y
};

static const u8 sConfigDescriptor[] = {	//<Config Descriptor
//ConfiguraTIon
    USB_DT_CONFIG_SIZE,    //bLength
    USB_DT_CONFIG,    //DescriptorType : ConfigDescriptor
    0, 0, //TotalLength
    0,//bNumInterfaces: 在set_descriptor函数里面计算
    0x01,    //bConfigurationValue - ID of this configuration
    0x00,    //Unused
#if USB_ROOT2 || USB_SUSPEND_RESUME || USB_SUSPEND_RESUME_SYSTEM_NO_SLEEP
    0xA0,    //Attributes:Bus Power remotewakeup
#else
    0x80,    //Attributes:Bus Power
#endif
    50,     //MaxPower * 2ma
};

static const u8 serial_string[] = {
    0x22, 0x03, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x36, 0x00, 0x46, 0x00, 0x36, 0x00,
    0x34, 0x00, 0x30, 0x00, 0x39, 0x00, 0x36, 0x00, 0x42, 0x00, 0x32, 0x00, 0x32, 0x00, 0x45, 0x00,
    0x37, 0x00
};

void get_device_descriptor(u8 *ptr)
{
    memcpy(ptr, sDeviceDescriptor, USB_DT_DEVICE_SIZE);
}

void get_language_str(u8 *ptr)
{
    memcpy(ptr, LANGUAGE_STR, LANGUAGE_STR[0]);
}

void get_manufacture_str(u8 *ptr)
{
    memcpy(ptr, MANUFACTURE_STR, MANUFACTURE_STR[0]);
}

void get_iserialnumber_str(u8 *ptr)
{
#if USB_ROOT2
    memcpy(ptr, serial_string, serial_string[0]);
#else
    extern __attribute__((weak)) u8 *get_norflash_uuid(void);
    u8 flash_id[16] = {0};
    int i;
    u8 bcd;
    if (get_norflash_uuid && get_norflash_uuid()) {
        ptr[0] = 0x22;
        ptr[1] = 0x03;
        memset(&ptr[2], 0, 0x20);
        memcpy(flash_id, get_norflash_uuid(), 16);
        //take 8 bytes from flash uuid
        for (i = 0; i < 8; i++) {
            bcd = flash_id[i] >> 4;
            if (bcd > 9) {
                bcd = bcd - 0xa + 'A';
            } else {
                bcd = bcd + '0';
            }
            ptr[2 + i * 4] = bcd;
            bcd = flash_id[i] & 0xf;
            if (bcd > 9) {
                bcd = bcd - 0xa + 'A';
            } else {
                bcd = bcd + '0';
            }
            ptr[2 + i * 4 + 2] = bcd;
        }
    } else {
        memcpy(ptr, serial_string, serial_string[0]);
    }
#endif
}

#if USB_ROOT2
static const u8 ee_string[] = {0x12, 0x03, 0x4D, 0x00, 0x53, 0x00, 0x46, 0x00, 0x54,
                               0x00, 0x31, 0x00, 0x30, 0x00, 0x30, 0x00, 0x90, 0x00
                              };
void get_string_ee(u8 *ptr)
{
    memcpy(ptr, ee_string, ee_string[0]);
}
#endif

void get_product_str(u8 *ptr)
{
    memcpy(ptr, product_string, product_string[0]);
}

const u8 *usb_get_config_desc()
{
    return sConfigDescriptor;
}

const u8 *usb_get_string_desc(u32 id)
{
    const u8 *pstr = uac_get_string(id);
    if (pstr != NULL) {
        return pstr;
    }
    return NULL;
}

#endif
