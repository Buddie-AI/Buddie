#ifndef __USB_COMMON_DEFINE_H__
#define __USB_COMMON_DEFINE_H__

///<<<注意此文件不要放函数声明, 只允许宏定义, 并且差异化定义可以根据需求在对应板卡中重新定义, 除非新增，否则不要直接修改这里
///<<<注意此文件不要放函数声明, 只允许宏定义, 并且差异化定义可以根据需求在对应板卡中重新定义, 除非新增，否则不要直接修改这里
///<<<注意此文件不要放函数声明, 只允许宏定义, 并且差异化定义可以根据需求在对应板卡中重新定义, 除非新增，否则不要直接修改这里
//
/**************************************************************************/
/*
               CLASS  BITMAP
    7   |   6   |   5   |   4   |   3   |   2   |   1   |   0
                                   HID    AUDIO  SPEAKER   Mass Storage
*/
/**************************************************************************/
#define     MASSSTORAGE_CLASS   0x00000001
#define     SPEAKER_CLASS       0x00000002
#define     MIC_CLASS           0x00000004
#define     HID_CLASS           0x00000008
#define     CDC_CLASS           0x00000010

#define     AUDIO_CLASS         (SPEAKER_CLASS|MIC_CLASS)


#define     USB_ROOT2   0

/// board文件没有定义的宏,在这里定义,防止编译报warning
#ifndef TCFG_PC_ENABLE
#define TCFG_PC_ENABLE  0
#endif
#ifndef TCFG_UDISK_ENABLE
#define TCFG_UDISK_ENABLE     0
#endif
#ifndef TCFG_HID_HOST_ENABLE
#define TCFG_HID_HOST_ENABLE  0
#endif
#ifndef TCFG_AOA_ENABLE
#define TCFG_AOA_ENABLE  0
#endif
#ifndef TCFG_ADB_ENABLE
#define TCFG_ADB_ENABLE   0
#endif
#ifndef TCFG_USB_APPLE_DOCK_EN
#define TCFG_USB_APPLE_DOCK_EN 0
#endif
#ifndef TCFG_HOST_AUDIO_ENABLE
#define TCFG_HOST_AUDIO_ENABLE  0
#endif
#ifndef TCFG_CHARGE_ENABLE
#define TCFG_CHARGE_ENABLE     0
#endif
#ifndef TCFG_USB_PORT_CHARGE
#define TCFG_USB_PORT_CHARGE   0
#endif
#ifndef TCFG_USB_MIC_ECHO_ENABLE
#define TCFG_USB_MIC_ECHO_ENABLE   0
#endif
#ifndef TCFG_USB_MIC_DATA_FROM_MICEFFECT
#define TCFG_USB_MIC_DATA_FROM_MICEFFECT   0
#endif
#ifndef TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0
#define TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0   0
#endif
#ifndef TCFG_ONLY_PC_ENABLE     //只有pc模式
#define TCFG_ONLY_PC_ENABLE 0
#endif
#ifndef TCFG_TYPE_C_ENABLE      //应用于type-c场景
#define TCFG_TYPE_C_ENABLE  0
#endif
#ifndef TCFG_USB_CDC_BACKGROUND_RUN
#define TCFG_USB_CDC_BACKGROUND_RUN 0
#endif

/********************************/

#if TCFG_UDISK_ENABLE || TCFG_HID_HOST_ENABLE || TCFG_AOA_ENABLE || TCFG_ADB_ENABLE || TCFG_HOST_AUDIO_ENABLE
#define MOUNT_RETRY                         3
#define MOUNT_RESET                         40
#define MOUNT_TIMEOUT                       50
#define     USB_HOST_ENABLE                 1
#else
#define     USB_HOST_ENABLE                 0
#endif

#if TCFG_CHARGE_ENABLE && TCFG_USB_PORT_CHARGE
#define TCFG_OTG_MODE_CHARGE                (OTG_CHARGE_MODE)
#else
#define TCFG_OTG_MODE_CHARGE                0
#endif

#if (TCFG_PC_ENABLE)
#define TCFG_PC_UPDATE                      1
#define TCFG_OTG_MODE_SLAVE                 (OTG_SLAVE_MODE)
#else
#define TCFG_PC_UPDATE                      0
#define TCFG_OTG_MODE_SLAVE                 0
#endif

#if (USB_HOST_ENABLE)
#define TCFG_OTG_MODE_HOST                  (OTG_HOST_MODE)
#else
#define TCFG_OTG_MODE_HOST                  0
#endif

#if TCFG_PC_ENABLE
#define TCFG_USB_SLAVE_ENABLE               1
#if (USB_DEVICE_CLASS_CONFIG & MASSSTORAGE_CLASS)
#define TCFG_USB_SLAVE_MSD_ENABLE           1
#else
#define TCFG_USB_SLAVE_MSD_ENABLE           0
#endif

#if (USB_DEVICE_CLASS_CONFIG & AUDIO_CLASS)
#define TCFG_USB_SLAVE_AUDIO_ENABLE         1
#else
#define TCFG_USB_SLAVE_AUDIO_ENABLE         0
#endif

#if (USB_DEVICE_CLASS_CONFIG & HID_CLASS)
#define TCFG_USB_SLAVE_HID_ENABLE           1
#else
#define TCFG_USB_SLAVE_HID_ENABLE           0
#endif

#if (USB_DEVICE_CLASS_CONFIG & CDC_CLASS)
#define TCFG_USB_SLAVE_CDC_ENABLE           1
#else
#define TCFG_USB_SLAVE_CDC_ENABLE           0
#endif

#else  /* TCFG_PC_ENABLE == 0*/
#define TCFG_USB_SLAVE_ENABLE               0
#define TCFG_USB_SLAVE_MSD_ENABLE           0
#define TCFG_USB_SLAVE_AUDIO_ENABLE         0
#define TCFG_USB_SLAVE_HID_ENABLE           0
#define TCFG_USB_SLAVE_CDC_ENABLE           0
#endif

#define TCFG_OTG_SLAVE_ONLINE_CNT           3
#define TCFG_OTG_SLAVE_OFFLINE_CNT          2

#define TCFG_OTG_HOST_ONLINE_CNT            2
#define TCFG_OTG_HOST_OFFLINE_CNT           3

#ifndef TCFG_OTG_MODE
#define TCFG_OTG_MODE                       (TCFG_OTG_MODE_HOST|TCFG_OTG_MODE_SLAVE|TCFG_OTG_MODE_CHARGE)
#endif

#define TCFG_OTG_DET_INTERVAL               50

#if (TCFG_USB_CDC_BACKGROUND_RUN)

#if (USB_DEVICE_CLASS_CONFIG & CDC_CLASS)
#error  "CDC_CLASS and TCFG_COMM_TYPE==TCFG_USB_COMM 不能同时打开"
#else
#undef TCFG_USB_SLAVE_CDC_ENABLE
#define TCFG_USB_SLAVE_CDC_ENABLE           1
#endif
#undef  TCFG_USB_SLAVE_ENABLE
#define TCFG_USB_SLAVE_ENABLE               1

#if (!TCFG_PC_ENABLE)
#undef TCFG_OTG_MODE
#define TCFG_OTG_MODE   (TCFG_OTG_MODE_HOST | OTG_SLAVE_MODE | TCFG_OTG_MODE_CHARGE)
#endif

#endif

#endif
