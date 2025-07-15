#include "system/includes.h"
#include "usb/device/usb_stack.h"
#include "usb/device/msd.h"
#include "usb/scsi.h"
#include "usb_config.h"
#include "device/device.h"
#include "app_config.h"

#if TCFG_DEV_MANAGER_ENABLE
#include "dev_manager.h"
#endif

#if TCFG_USB_APPLE_DOCK_EN
#include "apple_dock/iAP.h"
#endif

#if TCFG_USB_SLAVE_MSD_ENABLE

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

 // *INDENT-OFF*
#define USB_MSD_BULK_DEV_USE_ASYNC			1

//               Mass Storage Class
    /*********************************************************/
static const u8 sMassDescriptor[] = {  //<Interface & Endpoint Descriptor
///Interface
    USB_DT_INTERFACE_SIZE,   //Length
    USB_DT_INTERFACE,   //DescriptorType:Inerface
    0x00,   //InterfaceNum:0
    0x00,   //AlternateSetting:0
    0x02,   //NumEndpoint:0
    USB_CLASS_MASS_STORAGE,   //InterfaceClass:MSD
    0x06,   //InterfaceSubClass SCSI transparent command set Allocated by USB-IF for SCSI. SCSI standards are defined outside of USB.
    0x50,   //InterfaceProtocol BBB USB Mass Storage Class Bulk-Only (BBB) Transport
    0x00,   //Interface String
///Endpoint IN
    USB_DT_ENDPOINT_SIZE,
    USB_DT_ENDPOINT,
    USB_DIR_IN | MSD_BULK_EP_IN,
    USB_ENDPOINT_XFER_BULK,
    LOBYTE(MAXP_SIZE_BULKIN), HIBYTE(MAXP_SIZE_BULKIN),
    0x01,
///Endpoint OUT
    USB_DT_ENDPOINT_SIZE,
    USB_DT_ENDPOINT,
    MSD_BULK_EP_OUT,
    USB_ENDPOINT_XFER_BULK,
    LOBYTE(MAXP_SIZE_BULKOUT), HIBYTE(MAXP_SIZE_BULKOUT),
    0x01,
};
#if defined(CONFIG_CPU_BR23)
#define     CPU_NAME    'B','R','2','3'
#elif defined(CONFIG_CPU_BR25)
#define     CPU_NAME    'B','R','2','5'
#elif defined(CONFIG_CPU_BD29)
#define     CPU_NAME    'B','D','2','9'
#elif defined(CONFIG_CPU_BR30)
#define     CPU_NAME    'B','R','3','0'
#elif defined(CONFIG_CPU_BD19)
#define     CPU_NAME    'B','D','1','9'
#elif defined(CONFIG_CPU_BR36)
#define     CPU_NAME    'B','R','3','6'
#elif defined(CONFIG_CPU_BR18)
#define     CPU_NAME    'B','R','1','8'
#elif defined(CONFIG_CPU_BR28)
#define     CPU_NAME    'B','R','2','8'
#elif defined(CONFIG_CPU_BR27)
#define     CPU_NAME    'B','R','2','7'
#else
#define     CPU_NAME    'U','K','N','W'
/* #error "not define cpu name" */
#endif
 // *INDENT-ON*
static const u8 SCSIInquiryData[] = {
    0x00,//  // Peripheral Device Type: direct access devices  0x05,//
    0x80,   // Removable: UFD is removable
    0x02,   // iso ecma ANSI version
    0x02,   // Response Data Format: compliance with UFI
    0x20,   // Additional Length (Number of UINT8s following this one): 31, totally 36 UINT8s
    0x00, 0x00, 0x00,   // reserved
    CPU_NAME,    //-- Vender information start
    ' ',
    ' ',
    ' ',
    ' ',   //" " -- Vend Information end

    'U',    //-- Production Identification start
    'D',
    'I',
    'S',
    'K',
    ' ',
    ' ',
    ' ',
    ' ',
    ' ',
    ' ',
    ' ',
    ' ',
    ' ',
    ' ',
    ' ',    //" " -- Production Identification end

    0x31,   //"1" -- Production Revision Level start
    0x2e,   //"."
    0x30,   //"0"
    0x30    //"0" -- Production Revision Level end
};


#define MSD_BLOCK_SIZE       1
#define MSD_BUFFER_SIZE     (MSD_BLOCK_SIZE * 512)

struct usb_msd_handle {
    struct usb_scsi_cbw cbw;
    struct usb_scsi_csw csw;
    struct msd_info info;
    u8 *msd_buf;
    u8 *ep_out_dmabuffer;
    /* u8 *ep_in_dmabuffer; */

};
struct usb_msd_handle *msd_handle;
#if USB_MALLOC_ENABLE
#else
#if USB_MSD_BULK_DEV_USE_ASYNC
static u8 msd_buf[MSD_BUFFER_SIZE * 2] SEC(.mass_storage) __attribute__((aligned(64)));
#else
static u8 msd_buf[MSD_BUFFER_SIZE] SEC(.mass_storage) __attribute__((aligned(64)));
#endif
static struct usb_msd_handle _msd_handle SEC(.mass_storage);
#endif

inline static u8 __get_max_msd_dev()
{
    u8 cnt = 0;
#if (!TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0 && TCFG_SD0_ENABLE)\
    ||(TCFG_SD0_ENABLE && TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0 && TCFG_DM_MULTIPLEX_WITH_SD_PORT != 0)
    //没有复用时候判断 sd开关
    //复用时候判断是否参与复用
    cnt++;
#endif

#if (!TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0 && TCFG_SD1_ENABLE)\
     ||(TCFG_SD1_ENABLE && TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0 && TCFG_DM_MULTIPLEX_WITH_SD_PORT != 1)
    //没有复用时候判断 sd开关
    //复用时候判断是否参与复用
    cnt++;
#endif

#if defined(TCFG_NORFLASH_DEV_ENABLE) && TCFG_NORFLASH_DEV_ENABLE
    cnt++;
#endif

#if TCFG_VIR_UDISK_ENABLE
    cnt++;
#endif
    if (!cnt) {
        cnt = 1;
    } else if (cnt >= MAX_MSD_DEV) {
        cnt = MAX_MSD_DEV;
    }
    return cnt;
}

u32 msd_set_wakeup_handle(void (*handle)(struct usb_device_t *usb_device))
{
    if (msd_handle) {
        msd_handle->info.msd_wakeup_handle = handle;
        return 1;
    }
    return 0;
}
static void msd_wakeup(struct usb_device_t *usb_device, u32 ep)
{
    if ((msd_handle) && (msd_handle->info.msd_wakeup_handle)) {
        msd_handle->info.msd_wakeup_handle(usb_device);
    }
}
static u8 get_cardreader_popup(u32 lun)
{
    return msd_handle->info.bDisk_popup[lun];//弹出
}
static void set_cardreader_popup(u32 lun, u8 status)
{
    if (msd_handle->info.dev_handle[lun] != NULL) {
        msd_handle->info.bDisk_popup[lun] = status;//弹出
    }
}
static void recover_set_cardreader_popup(u32 lun)
{
    if (msd_handle->info.dev_handle[lun] != NULL) {
        msd_handle->info.bDisk_popup[lun] = 0;//未弹出
    }
}
static void msd_endpoint_init(struct usb_device_t *usb_device, u32 itf)
{
    const usb_dev usb_id = usb_device2id(usb_device);

    usb_g_ep_config(usb_id, MSD_BULK_EP_IN | USB_DIR_IN, USB_ENDPOINT_XFER_BULK, 0, msd_handle->ep_out_dmabuffer, MAXP_SIZE_BULKIN);

    usb_g_ep_config(usb_id, MSD_BULK_EP_OUT, USB_ENDPOINT_XFER_BULK, 1, msd_handle->ep_out_dmabuffer, MAXP_SIZE_BULKOUT);
    usb_g_set_intr_hander(usb_id, MSD_BULK_EP_OUT, msd_wakeup);

    usb_enable_ep(usb_id, MSD_BULK_EP_IN);
}
static void msd_reset_wakeup(struct usb_device_t *usb_device, u32 itf_num)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    if (msd_handle && msd_handle->info.msd_reset_wakeup_handle) {
        msd_handle->info.msd_reset_wakeup_handle(usb_device, itf_num);
    }
}
void msd_set_reset_wakeup_handle(void (*handle)(struct usb_device_t *usb_device, u32 itf_num))
{
    if (msd_handle) {
        msd_handle->info.msd_reset_wakeup_handle = handle;
    }
}
void msd_reset(struct usb_device_t *usb_device, u32 itf_num)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    log_debug("%s", __func__);

#if USB_ROOT2
    usb_disable_ep(usb_id, MSD_BULK_EP_OUT);
#else
    msd_endpoint_init(usb_device, itf_num);
#endif
}
static void *check_disk_status(u8 cur_lun)
{
    u32 ret;
    u32 online_status;
    void *dev_fd = msd_handle->info.dev_handle[cur_lun]  ;
    const char *dev_name = msd_handle->info.dev_name[cur_lun];

#if TCFG_DEV_MANAGER_ENABLE
    if (dev_manager_list_check_by_logo((char *)dev_name)) {
        if (dev_fd == NULL) {
            msd_handle->info.dev_handle[cur_lun] = dev_open(dev_name, NULL);
            if (msd_handle->info.dev_handle[cur_lun]) {
#ifdef CONFIG_EARPHONE_CASE_ENABLE
                dev_ioctl(msd_handle->info.dev_handle[cur_lun], IOCTL_POWER_RESUME, (u32)dev_name);
#endif
            }
        } else {
            //FIXME:need add device state check??
            u32 dev_online = 0;
            ret = dev_ioctl(dev_fd, IOCTL_GET_STATUS, (u32)&dev_online);
            if (ret || !dev_online) {
                goto __dev_offline;
            }
        }

        if (get_cardreader_popup(cur_lun)) {
            return NULL;
        }
    } else {
        if (dev_fd) {
            goto __dev_offline;
        }
    }
#else
    if (dev_fd == NULL) {
        msd_handle->info.dev_handle[cur_lun] = dev_open(dev_name, NULL);
    } else {
        u32 dev_online = 0;
        ret = dev_ioctl(dev_fd, IOCTL_GET_STATUS, (u32)&dev_online);
        if (ret || !dev_online) {
            goto __dev_offline;
        }
    }
#endif

    return msd_handle->info.dev_handle[cur_lun];

__dev_offline:
    dev_close(dev_fd);
    recover_set_cardreader_popup(cur_lun);
    msd_handle->info.dev_handle[cur_lun] = NULL;
    return NULL;
}
static u32 msd_itf_hander(struct usb_device_t *usb_device, struct usb_ctrlrequest *setup)
{
    u32 tx_len;
    u8 *tx_payload = usb_get_setup_buffer(usb_device);
    u32 bRequestType = setup->bRequestType & USB_TYPE_MASK;
    switch (bRequestType) {
    case USB_TYPE_STANDARD:
        switch (setup->bRequest) {
        case USB_REQ_GET_STATUS:
            usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
            break;
        case USB_REQ_GET_INTERFACE:
            tx_len = 1;
            u8 i = 0;
            usb_set_data_payload(usb_device, setup, &i, tx_len);
            break;
        case USB_REQ_SET_INTERFACE:
            usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
            break;
        default:
            break;
        }
        break;
    case USB_TYPE_CLASS:
        switch (setup->bRequest) {
        case USB_MSD_MAX_LUN:
            msd_endpoint_init(usb_device, -1);
            tx_len = 1;
            tx_payload[0] = __get_max_msd_dev() - 1;
            usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
            break;
        case USB_MSD_RESET :
            usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return 0;
}
u32 msd_desc_config(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num)
{
    log_debug("%s() %d", __func__, *cur_itf_num);
    memcpy(ptr, sMassDescriptor, sizeof(sMassDescriptor));
    ptr[2] = *cur_itf_num;
#if TCFG_USB_APPLE_DOCK_EN
    if (apple_mfi_chip_online_lib()) {
        ptr[5] = 0xff;	//bInterfaceClass:Vendor-Specific
        ptr[6] = 0xf0; 	//bInterfaceSubClass:MFi Accessory
        ptr[7] = 0x00;	//bInterfaceProtocol */
        ptr[8] = MSD_STR_INDEX;	//bInterfaceString*/
    }
#endif
    if (usb_set_interface_hander(usb_id, *cur_itf_num, msd_itf_hander) != *cur_itf_num) {

    }
    if (usb_set_reset_hander(usb_id, *cur_itf_num, msd_reset_wakeup) != *cur_itf_num) {

    }
    *cur_itf_num = *cur_itf_num + 1;
    return sizeof(sMassDescriptor);
}
u32 msd_usb2mcu(const struct usb_device_t *usb_device, u8 *buffer, u32 len)
{
    usb_dev usb_id = usb_device2id(usb_device);
    return usb_g_bulk_read(usb_id, MSD_BULK_EP_OUT, buffer, len, 1);
}
u32 msd_mcu2usb(const struct usb_device_t *usb_device, const u8 *buffer, u32 len)
{
    usb_dev usb_id = usb_device2id(usb_device);
    return usb_g_bulk_write(usb_id, MSD_BULK_EP_IN, buffer, len);
}
static u32 msd_usb2mcu_64byte_fast(const struct usb_device_t *usb_device, u8 *buffer, u32 len)
{
    usb_dev usb_id = usb_device2id(usb_device);
    return usb_g_bulk_read64byte_fast(usb_id, MSD_BULK_EP_OUT, buffer, len);
}
static u32 get_cbw(const struct usb_device_t *usb_device)
{
    msd_handle->cbw.dCBWSignature = 0;
    memset((u8 *)(&(msd_handle->cbw)), 0, sizeof(struct usb_scsi_cbw));
    u32 rx_len = msd_usb2mcu(usb_device, (u8 *)(&(msd_handle->cbw)), sizeof(struct usb_scsi_cbw));
    if (rx_len == 0) {
        return 1;
    }
    if (msd_handle->cbw.dCBWSignature ^ CBW_SIGNATURE) {
        if ((msd_handle->cbw.dCBWSignature ^ CBW_SIGNATURE) != CBW_SIGNATURE) {
        }
        return 1;
    }
    msd_handle->csw.dCSWTag = msd_handle->cbw.dCBWTag;
    msd_handle->csw.uCSWDataResidue = msd_handle->cbw.dCBWDataTransferLength;

    return 0;
}
static void inquiry(const struct usb_device_t *usb_device)
{
    u32 err;
    u32 len = min(msd_handle->csw.uCSWDataResidue, sizeof(SCSIInquiryData));
    u32 scsiinquirydata[sizeof(SCSIInquiryData) / 4 + 4]; //usb dma 不能访问code区域
    memcpy(scsiinquirydata, SCSIInquiryData, sizeof(SCSIInquiryData));
    err = msd_mcu2usb(usb_device, (const u8 *)scsiinquirydata, len);
    if (err == 0) {
        log_error("write usb err %d", __LINE__);
    }
    msd_handle->csw.uCSWDataResidue -= len;
    msd_handle->csw.bCSWStatus = 0;
}
static void send_csw(const struct usb_device_t *usb_device)
{
    u32 err;
    msd_handle->csw.dCSWSignature = CSW_SIGNATURE;
    err = msd_mcu2usb(usb_device, (u8 *) & (msd_handle->csw), sizeof(struct usb_scsi_csw));
    if (err == 0) {
        log_error("write usb err %d", __LINE__);
    }
}
static void stall_inep(const struct usb_device_t *usb_device)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    usb_write_txcsr(usb_id, MSD_BULK_EP_IN, TXCSRP_SendStall);
    u32 ot = 2000;

    while (1) {
        if (ot-- == 0) {
            break;
        }
        if (usb_otg_online(usb_id) == DISCONN_MODE) {
            break;
        }
        if ((usb_read_txcsr(usb_id, MSD_BULK_EP_IN) & TXCSRP_SendStall) == 0) {
            break;
        }
    }
}

static void stall_outep(const struct usb_device_t *usb_device)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    usb_write_rxcsr(usb_id, MSD_BULK_EP_OUT, RXCSRP_SendStall);
    u32 ot = 2000;
    while (1) {
        if (ot-- == 0) {
            break;
        }
        if (usb_otg_online(usb_id) == DISCONN_MODE) {
            break;
        }
        if ((usb_read_rxcsr(usb_id, MSD_BULK_EP_OUT) & RXCSRP_SendStall) == 0) {
            break;
        }
    }
}

static void unknow(const struct usb_device_t *usb_device, u8 rw)
{
    if (rw) {
        stall_outep(usb_device);
    } else {
        stall_inep(usb_device);
    }

    msd_handle->csw.uCSWDataResidue = 0;
    msd_handle->csw.bCSWStatus = 0x01;
    msd_handle->info.bError = 0x1;
    /* msd_handle->info.bError = 0x2; */
}
static void stall_error(const struct usb_device_t *usb_device, u8 rw, u8 error)
{
    msd_handle->csw.bCSWStatus = 0x01;
    unknow(usb_device, rw);
    msd_handle->info.bError = error;
}
static void test_unit_ready(const struct usb_device_t *usb_device)
{
    int err;
    u8 cur_lun = msd_handle->cbw.bCBWLUN;

    if (NULL == check_disk_status(cur_lun)) {
        msd_handle->csw.bCSWStatus = 0x1;
        msd_handle->info.bError = 0x0f;
    } else {
        msd_handle->csw.uCSWDataResidue = 0;
        msd_handle->csw.bCSWStatus = 0x0;
    }
}
static void allow_medium_removal(const struct usb_device_t *usb_device)
{
    msd_handle->csw.bCSWStatus = 0x0;
    if (msd_handle->cbw.lba[2]) {
        msd_handle->info.bError = 0x02;
        msd_handle->csw.bCSWStatus = 0x01;
    }
    msd_handle->csw.uCSWDataResidue = 0;
}
static void request_sense(const struct usb_device_t *usb_device)
{
    u32 err;
    u32 len;
    u8 sense_data[0x12];
    len = min(msd_handle->csw.uCSWDataResidue, 0x12);
    if (msd_handle->csw.uCSWDataResidue == 0) {
        stall_inep(usb_device);
        msd_handle->info.bError = NOT_READY;
        msd_handle->csw.uCSWDataResidue = 0;
        msd_handle->csw.bCSWStatus = 0x01;
        return;
    } else {
        memset(sense_data, 0x0, sizeof(sense_data));
        sense_data[2] = msd_handle->info.bSenseKey;
        sense_data[12] = msd_handle->info.bAdditionalSenseCode;
        sense_data[13] = msd_handle->info.bAddiSenseCodeQualifier;
        sense_data[0] = 0x70;
        sense_data[7] = 0x0a;
        err = msd_mcu2usb(usb_device, sense_data, len);
        if (err == 0) {
            log_error("write usb err %d", __LINE__);
        }
    }
    msd_handle->csw.uCSWDataResidue -= len;
    msd_handle->csw.bCSWStatus = 0;
}
static u8 const scsi_mode_sense[4] = {
    0x03, 0x00, 0x00, 0x00
};
static void mode_sense(const struct usb_device_t *usb_device)
{
    u32 err;
    u32 sense;
    err = msd_mcu2usb(usb_device, (u8 *)scsi_mode_sense, sizeof(scsi_mode_sense));
    if (err == 0) {
        log_error("write usb err %d", __LINE__);
    }
    msd_handle->csw.uCSWDataResidue = 0;
    msd_handle->csw.bCSWStatus = 0;
}
static void verify(const struct usb_device_t *usb_device)
{
    msd_handle->csw.uCSWDataResidue = 0;
    msd_handle->csw.bCSWStatus = 0;
}

#if USB_MSD_BULK_DEV_USE_ASYNC
static void msd_write_10_async(const struct usb_device_t *usb_device, u8 cur_lun, u32 lba, u16 lba_num)
{
    u32 err = 0;
    u16 num = 0;
    u8  buf_idx = 0;
    u32 have_send_stall = FALSE;
    void *dev_fd = NULL;
    while (lba_num) {
        wdt_clear();
        num = lba_num > MSD_BLOCK_SIZE ? MSD_BLOCK_SIZE : lba_num;
        if (msd_handle->csw.uCSWDataResidue == 0) {
            msd_handle->csw.bCSWStatus = 1;
            break;
        }
        if (msd_handle->csw.uCSWDataResidue >= num * 0x200) {
            msd_handle->csw.uCSWDataResidue -= num * 0x200;
        }
        err = msd_usb2mcu_64byte_fast(usb_device, msd_handle->msd_buf + buf_idx * MSD_BUFFER_SIZE, num * 0x200);
        if (err != num * 0x200) {
            log_error("read usb_err %d, dev = %s",
                      __LINE__, msd_handle->info.dev_name[cur_lun]);
            stall_error(usb_device, 1, MEDIUM_ERROR);
            have_send_stall = TRUE;
            break;
        }
        dev_fd = check_disk_status(cur_lun);
        if (dev_fd) {
            dev_ioctl(dev_fd, IOCTL_SET_ASYNC_MODE, 0);
            err = dev_bulk_write(dev_fd, msd_handle->msd_buf + buf_idx * MSD_BUFFER_SIZE, lba, num);
            if (err != num) {
                log_error("write_10 write fail, %d, dev = %s",
                          err, msd_handle->info.dev_name[cur_lun]);
                stall_error(usb_device, 1, MEDIUM_ERROR);
                have_send_stall = TRUE;
                break;
            }
        } else {
            log_error("write_10 disk offline, dev = %s",
                      msd_handle->info.dev_name[cur_lun]);
            stall_error(usb_device, 1, MEDIUM_ERROR);
            have_send_stall = TRUE;
            break;
        }
        buf_idx = !buf_idx;
        lba_num -= num;
        lba += num;
    }
    dev_fd = check_disk_status(cur_lun);
    if (dev_fd) {
        //async mode last block flush
        dev_ioctl(dev_fd, IOCTL_FLUSH, 0);
    } else {
        if (have_send_stall == FALSE) {
            log_error("write_10 disk offline while sync, dev = %s",
                      msd_handle->info.dev_name[cur_lun]);
            stall_error(usb_device, 1, MEDIUM_ERROR);
        }
    }
}

static void msd_read_10_async(const struct usb_device_t *usb_device, u8 cur_lun, u32 lba, u16 lba_num)
{
    u32 err = 0;
    u16 num = 0;
    u8 buf_idx = 0;
    u32 last_lba = 0, last_num = 0;
    void *dev_fd = NULL;

    if (lba_num == 0) {
        log_error("lba_num == 0\n");
        stall_error(usb_device, 0, 0x02);
        return;
    }
    num = lba_num > MSD_BLOCK_SIZE ? MSD_BLOCK_SIZE : lba_num;
    dev_fd = check_disk_status(cur_lun);
    if (dev_fd) {
        dev_ioctl(dev_fd, IOCTL_SET_ASYNC_MODE, 0);
        err = dev_bulk_read(dev_fd, msd_handle->msd_buf + buf_idx * MSD_BUFFER_SIZE, lba, num);
        if (err != num) {
            log_error("read disk error0 = %d, dev = %s\n", err, msd_handle->info.dev_name[cur_lun]);
            stall_error(usb_device, 0, MEDIUM_ERROR);
            return;
        }
    } else {
        log_error("read_10 disk offline, dev = %s\n", msd_handle->info.dev_name[cur_lun]);
        stall_error(usb_device, 0, MEDIUM_ERROR);
        return;
    }

    while (lba_num) {
        wdt_clear();
        last_num = num;
        last_lba = lba;
        buf_idx = !buf_idx;
        lba += num;
        lba_num -= num;
        num = lba_num > MSD_BLOCK_SIZE ? MSD_BLOCK_SIZE : lba_num;
        if (msd_handle->csw.uCSWDataResidue == 0) {
            msd_handle->csw.bCSWStatus = 1;
            break;
        }
        if (msd_handle->csw.uCSWDataResidue >= last_num * 0x200) {
            msd_handle->csw.uCSWDataResidue -= last_num * 0x200;
        }
        dev_fd = check_disk_status(cur_lun);
        if (dev_fd) {
            if (num) {
                dev_ioctl(dev_fd, IOCTL_SET_ASYNC_MODE, 0);
                err = dev_bulk_read(dev_fd, msd_handle->msd_buf + buf_idx * MSD_BUFFER_SIZE, lba, num);
                if (err != num) {
                    log_error("read disk error1 = %d, dev = %s",
                              err, msd_handle->info.dev_name[cur_lun]);
                    stall_error(usb_device, 0, MEDIUM_ERROR);
                    break;
                }
            } else {
                //async mode last block flush
                dev_ioctl(dev_fd, IOCTL_FLUSH, 0);
            }
        } else {
            log_error("read_10 disk offline, dev = %s",
                      msd_handle->info.dev_name[cur_lun]);
            stall_error(usb_device, 0, MEDIUM_ERROR);
            break;
        }

        /* u8 *p = msd_handle->msd_buf + !buf_idx * MSD_BUFFER_SIZE; */
        /* for (int i = 0; i < last_num * 512; i++) {                */
        /*     p[i] = i;                                             */
        /* }                                                         */

        err = msd_mcu2usb(usb_device, msd_handle->msd_buf + !buf_idx * MSD_BUFFER_SIZE, last_num * 0x200);

        if (err != last_num * 0x200) {
            log_error("read_10 data transfer err %d, dev = %s",
                      __LINE__, msd_handle->info.dev_name[cur_lun]);
            stall_error(usb_device, 0, 0x05);
            if (num) {
                dev_ioctl(dev_fd, IOCTL_FLUSH, 0);
            }
            break;
        }
    }
}
#endif
static u32 read_32(u8 *p)
{
    return ((u32)(p[0]) << 24) | ((u32)(p[1]) << 16) | ((u32)(p[2]) << 8) | (p[3]);
}


static void write_10(const struct usb_device_t *usb_device)
{
    u32 err = 0 ;
    u32 lba;
    u16 lba_num;
    u16 num = 0;
    lba = read_32(msd_handle->cbw.lba);
    lba_num = ((u16)(msd_handle->cbw.LengthH) << 8) | (msd_handle->cbw.LengthL);
    u8 cur_lun = msd_handle->cbw.bCBWLUN;
    void *dev_fd = msd_handle->info.dev_handle[cur_lun];

    /* app_status_handler(APP_STATUS_PC_COPY); */


#if USB_MSD_BULK_DEV_USE_ASYNC
    /* u32 old_speed;                                           */
    /* dev_fd = check_disk_status(cur_lun);                     */
    /* if (dev_fd) {                                            */
    /*     dev_ioctl(dev_fd, IOCTL_GET_SPEED, (u32)&old_speed); */
    /*     dev_ioctl(dev_fd, IOCTL_SET_SPEED, 48000000);        */
    /* }                                                        */
    msd_write_10_async(usb_device, cur_lun, lba, lba_num);
    /* if (dev_fd) {                                      */
    /*     dev_ioctl(dev_fd, IOCTL_SET_SPEED, old_speed); */
    /* }                                                  */
#else

    while (lba_num) {
        wdt_clear();
        num = lba_num > MSD_BLOCK_SIZE ? MSD_BLOCK_SIZE : lba_num;
        if (msd_handle->csw.uCSWDataResidue == 0) {
            msd_handle->csw.bCSWStatus = 0x1;
            break;
        }
        if (msd_handle->csw.uCSWDataResidue >= (num * 0x200)) {
            msd_handle->csw.uCSWDataResidue -= (num * 0x200);
        }
        err = msd_usb2mcu_64byte_fast(usb_device, msd_handle->msd_buf, num * 0x200);
        if (err == 0) {
            stall_error(usb_device, 1, MEDIUM_ERROR);
            log_error("read usb, dev = %s", err, msd_handle->info.dev_name[cur_lun]);
            break;
        }
        dev_fd = check_disk_status(cur_lun);
        if (dev_fd) {
            dev_ioctl(dev_fd, IOCTL_CMD_RESUME, 0);
            err = dev_bulk_write(dev_fd, msd_handle->msd_buf, lba, num);
            dev_ioctl(dev_fd, IOCTL_CMD_SUSPEND, 0);
            if (err != num) {
                stall_error(usb_device, 1, MEDIUM_ERROR);
                log_error("write disk error =%d, dev = %s", err, msd_handle->info.dev_name[cur_lun]);
                break;
            } else {
                break;
            }
            lba += num;
            lba_num -= num;
        } else {
            log_error("write disk offline, dev = %s", msd_handle->info.dev_name[cur_lun]);
            stall_error(usb_device, 1, MEDIUM_ERROR);
            break;
        }
    }
#endif
}
static void read_10(const struct usb_device_t *usb_device)
{
    u32 err = 0;
    u32 lba;
    u16 lba_num;
    u16 num = 0;
    lba = read_32(msd_handle->cbw.lba);
    lba_num = ((u16)(msd_handle->cbw.LengthH) << 8) | (msd_handle->cbw.LengthL);
    u8 cur_lun = msd_handle->cbw.bCBWLUN;
    void *dev_fd = msd_handle->info.dev_handle[cur_lun];

#if USB_MSD_BULK_DEV_USE_ASYNC
    msd_read_10_async(usb_device, cur_lun, lba, lba_num);
#else

    while (lba_num) {
        wdt_clear();
        num = lba_num > MSD_BLOCK_SIZE ? MSD_BLOCK_SIZE : lba_num;
        if (msd_handle->csw.uCSWDataResidue == 0) {
            msd_handle->csw.bCSWStatus = 0x1;
            break;
        }
        if (msd_handle->csw.uCSWDataResidue >= (num * 0x200)) {
            msd_handle->csw.uCSWDataResidue -= (num * 0x200);
        }
        dev_fd = check_disk_status(cur_lun);
        if (dev_fd) {
            dev_ioctl(dev_fd, IOCTL_CMD_RESUME, 0);
            err = dev_bulk_read(dev_fd, msd_handle->msd_buf, lba, num);
            dev_ioctl(dev_fd, IOCTL_CMD_SUSPEND, 0);
            if (err != num) {
                log_error("read disk error =%d, dev = %s", err, msd_handle->info.dev_name[cur_lun]);
                stall_error(usb_device, 0, MEDIUM_ERROR);
                break;
            }
        } else {
            log_error("read disk offline, dev = %s", err, msd_handle->info.dev_name[cur_lun]);
            stall_error(usb_device, 0, MEDIUM_ERROR);
            break;
        }
        err = msd_mcu2usb(usb_device, msd_handle->msd_buf, num * 0x200);
        if (err == 0) {
            log_error("write usb err %d, dev = %s", __LINE__, msd_handle->info.dev_name[cur_lun]);
            stall_error(usb_device, 0, MEDIUM_ERROR);
            break;
        }
        lba += num;
        lba_num -= num;
    }
#endif
}
static void read_capacity(const struct usb_device_t *usb_device)
{
    u32 err;
    u32 capacity_temp;
    u8 capacity[8] = {0};
    u8 cur_lun = msd_handle->cbw.bCBWLUN;
    void *dev_fd = check_disk_status(cur_lun);
    if (!dev_fd) {
        stall_error(usb_device, 0, 0x0f);
        return;
    }
    dev_ioctl(dev_fd, IOCTL_GET_CAPACITY, (u32)&capacity_temp);
    capacity_temp = cpu_to_be32(capacity_temp - 1);
    memcpy(capacity, &capacity_temp, 4);
    dev_ioctl(dev_fd, IOCTL_GET_BLOCK_SIZE, (u32)&capacity_temp);
    /* r_printf(">>>[test]:capacity_temp2 = %d\n", capacity_temp); */
    capacity_temp = cpu_to_be32(capacity_temp);
    memcpy(&capacity[4], &capacity_temp, 4);
    err = msd_mcu2usb(usb_device, capacity, 8);
    if (err == 0) {
        log_error("write usb err %d", __LINE__);
    }
    msd_handle->csw.uCSWDataResidue = 0;
    msd_handle->csw.bCSWStatus = 0;

}

static void read_format_capacity(const struct usb_device_t *usb_device)
{
    u32 err;
    u8 capacity[12] = {0};
    u32 capacity_temp;
    u8 cur_lun = msd_handle->cbw.bCBWLUN;
    void *dev_fd = check_disk_status(cur_lun);
    if (!dev_fd) {
        stall_error(usb_device, 0, 0x0f);
        return;
    }

    dev_ioctl(dev_fd, IOCTL_GET_CAPACITY, (u32)&capacity_temp);

    capacity_temp = cpu_to_be32(capacity_temp);
    memcpy(&capacity[4], &capacity_temp, 4);
    dev_ioctl(dev_fd, IOCTL_GET_BLOCK_SIZE, (u32)&capacity_temp);
    capacity_temp = cpu_to_be32(capacity_temp);
    memcpy(&capacity[8], &capacity_temp, 4);
    capacity[3] = 0x08;
    capacity[8] = 0x02;
    err = msd_mcu2usb(usb_device, capacity, 12);
    if (err == 0) {
        log_error("write usb err %d", __LINE__);
    }
    msd_handle->csw.uCSWDataResidue -= 12;
    msd_handle->csw.bCSWStatus = 0;
}

static void stop_start()
{
    int i;
    u8 loej, start;
    /* set_cardreader_popup(0, !(msd_handle->cbw.lba[2])); */
    loej = msd_handle->cbw.lba[2] & BIT(1);
    start = msd_handle->cbw.lba[2] & BIT(0);
    for (i = 0; i < __get_max_msd_dev(); i++) {
        if (loej && !start) {
            set_cardreader_popup(i, 1);
        } else if (loej && start) {
            set_cardreader_popup(i, 0);
        }
    }

    msd_handle->csw.uCSWDataResidue = 0;
    msd_handle->csw.bCSWStatus = 0x0;
}


u32 private_scsi_cmd(const struct usb_device_t *usb_device, struct usb_scsi_cbw *cbw);

void USB_MassStorage(const struct usb_device_t *usb_device)
{
    const usb_dev usb_id = usb_device2id(usb_device);
#ifdef USB_HW_20
    //husb每次收到数据都有intrrx, 即使intrrxe没有打开，所以USB_MassStorage()退出
    //前一打开intrrxe就会起来一次rx中断，但是ep的rxcsr == 0
    if ((usb_read_rxcsr(usb_id, MSD_BULK_EP_OUT) & RXCSRP_RxPktRdy) == 0) {
        return;
    }
#endif
    usb_clr_intr_rxe(usb_id, MSD_BULK_EP_OUT);
    if (get_cbw(usb_device)) {
        usb_set_intr_rxe(usb_id, MSD_BULK_EP_OUT);
        return;
    }
    /* log_debug("opcode %x", msd_handle->cbw.operationCode); */
    if (private_scsi_cmd(usb_device, &(msd_handle->cbw))) {
        msd_handle->info.bError = 0;
        msd_handle->csw.uCSWDataResidue = 0;
        msd_handle->csw.bCSWStatus = 0x0;
    } else {
        wdt_clear();
        switch (msd_handle->cbw.operationCode) {
        case INQUIRY://0x12
            inquiry(usb_device);
            break;
        case TEST_UNIT_READY://0x00
            test_unit_ready(usb_device);
            break;
        case VERIFY://0x2f
            verify(usb_device);
            break;
        case ALLOW_MEDIUM_REMOVAL://0x1e
            allow_medium_removal(usb_device);
            break;
        case REQUEST_SENSE://0x03
            request_sense(usb_device);
            break;
        case MODE_SENSE://0x1a
            mode_sense(usb_device);
            break;
        case READ_10://0x28
            read_10(usb_device);
            break;
        case WRITE_10://0x2a
            write_10(usb_device);
            break;
        case READ_CAPACITY://0x25
            read_capacity(usb_device);
            break;
        case READ_FORMAT_CAPACITIES://0x23
            read_format_capacity(usb_device);
            break;
        case START_STOP://0x1b
            stop_start();
            break;

        default:
            log_error("opcode %x", msd_handle->cbw.operationCode);
            stall_error(usb_device, 0, !(msd_handle->cbw.bmCBWFlags & 0x80));
            break;
        }
    }
    switch (msd_handle->info.bError) {
    case 0x00:
        msd_handle->info.bSenseKey = NO_SENSE;
        msd_handle->info.bAdditionalSenseCode = ASC_NO_ADDITIONAL_SENSE_INFORMATION;
        msd_handle->info.bAddiSenseCodeQualifier = ASCQ_NO_ADDITIONAL_SENSE_INFORMATION;
        break;

    case 0x02:
        msd_handle->info.bSenseKey = ILLEGAL_REQUEST;
        msd_handle->info.bAdditionalSenseCode = ASC_INVALID_FIELD_IN_COMMAND_PACKET;
        msd_handle->info.bAddiSenseCodeQualifier = ASCQ_INVALID_FIELD_IN_COMMAND_PACKET;
        break;
    case MEDIUM_ERROR:
        msd_handle->info.bSenseKey = MEDIUM_ERROR;
        msd_handle->info.bAdditionalSenseCode = ASC_WRITE_FAULT;
        msd_handle->info.bAddiSenseCodeQualifier = ASCQ_WRITE_FAULT;
        break;


    case 0x0f:
    case 0x05:
        msd_handle->info.bSenseKey = NOT_READY;
        msd_handle->info.bAdditionalSenseCode = ASC_MEDIUM_PRESENT;
        msd_handle->info.bAddiSenseCodeQualifier = ASCQ_MEDIUM_PRESENT;
        break;
    case 0x06:
        msd_handle->info.bSenseKey = ABORTED_COMMAND;
        msd_handle->info.bAdditionalSenseCode = ASC_INVALID_COMMAND_OPERATION_CODE;
        msd_handle->info.bAddiSenseCodeQualifier = ASCQ_INVALID_COMMAND_OPERATION_CODE;
        break;
    }
    usb_set_intr_rxe(usb_id, MSD_BULK_EP_OUT);
    send_csw(usb_device);
}

u32 msd_register_disk(const char *name, void *arg)
{
    int i;
    u32 len, ret = -1;
    if (msd_handle) {
        for (i = 0; i < __get_max_msd_dev(); i++) {
            if (msd_handle->info.dev_name[i][0] == 0) {
                ASSERT(strlen(name) <= MSD_DEV_NAME_LEN, "MSD_DEV_NAME_LEN too small");
                strcpy(msd_handle->info.dev_name[i], name);
                msd_handle->info.dev_handle[i] = NULL;
                break;
            }
        }
        if (i == __get_max_msd_dev()) {
            ret = -3;
        }
    }
    return ret;
}
u32 msd_unregister_disk(const char *name)
{
    u32 err;
    int i;
    if (msd_handle) {
        for (i = 0; i < __get_max_msd_dev(); i++) {
            if (!strcmp(msd_handle->info.dev_name[i], name)) {
                msd_handle->info.dev_name[i][0] = 0;
                return 0;
            }
        }
    }
    return -1;
}

u32 msd_unregister_all()
{
    u32 err;
    int i;
    if (msd_handle) {
        for (i = 0; i < __get_max_msd_dev(); i++) {
            if (msd_handle->info.dev_handle[i]) {
                err = dev_close(msd_handle->info.dev_handle[i]);
                msd_handle->info.dev_handle[i] = NULL;
                /* memset(msd_handle->info.dev_name[i], 0, MSD_DEV_NAME_LEN); */
            }
        }
    }
    return 0;
}
u32 msd_register(const usb_dev usb_id)
{
    if (msd_handle == NULL) {
#if USB_MALLOC_ENABLE
        msd_handle = (struct usb_msd_handle *)zalloc(sizeof(struct usb_msd_handle));
        if (msd_handle == NULL) {
            log_error("msd_register err");
            return -1;
        }
#if USB_MSD_BULK_DEV_USE_ASYNC
        msd_handle->msd_buf = (u8 *)malloc(MSD_BUFFER_SIZE * 2);
#else
        msd_handle->msd_buf = (u8 *)malloc(MSD_BUFFER_SIZE);
#endif
        if (msd_handle->msd_buf == NULL) {
            log_error("msd_register err");
            return -1;
        }
#else
        memset(&_msd_handle, 0, sizeof(struct usb_msd_handle));
        _msd_handle.msd_buf = msd_buf;
        msd_handle = &_msd_handle;
#endif
        log_info("msd_handle = %x", msd_handle);

        msd_handle->ep_out_dmabuffer = usb_alloc_ep_dmabuffer(usb_id, MSD_BULK_EP_OUT, MAXP_SIZE_BULKIN + MAXP_SIZE_BULKOUT);

    }
    return 0;
}
u32 msd_release()
{
    if (msd_handle) {
        for (int i = 0; i < __get_max_msd_dev(); i++) {
            void *dev_fd = msd_handle->info.dev_handle[i] ;
            if (dev_fd) {
#ifdef CONFIG_EARPHONE_CASE_ENABLE
                dev_ioctl(dev_fd, IOCTL_POWER_SUSPEND, 0);
#endif
                dev_close(dev_fd);
                msd_handle->info.dev_handle[i] = NULL;
            }
        }

#if USB_MALLOC_ENABLE
        if (msd_handle->msd_buf) {
            free(msd_handle->msd_buf);
            msd_handle->msd_buf = NULL;
        }
        free(msd_handle);
#endif
        msd_handle = NULL;
    }
    return 0;
}

#if TCFG_USB_APPLE_DOCK_EN
int usb_g_ep_read_check(const usb_dev id, u32 ep, u32 rx_len)
{
    if (apple_mfi_chip_online_lib()) {
        if (ep != MSD_BULK_EP_OUT) {
            return 0;
        }
        u32 dma_size = usb_get_dma_size(id, ep);
        /* printf("dma:%d, rl:%d, ep:%d \n", dma_size, rx_len, ep); */
        if (rx_len < dma_size) {
            return -1;
        }
    }
    return 0;
}
void apple_usb_msd_wakeup(struct usb_device_t *usb_device)
{
    msd_wakeup(usb_device, MSD_BULK_EP_IN);
}
u8 MCU_SRAMToUSB_app(void *hdl, u8 *pBuf, u16 uCount)
{
    return msd_mcu2usb(hdl, pBuf, uCount);
}
u8 MCU_USBToSRAM_app(void *hdl, u8 *pBuf, u16 uCount)
{
    /* return msd_usb2mcu(hdl, pBuf, uCount); */
    usb_dev usb_id = usb_device2id(hdl);
    int ret;
    int cnt = 10000;
    while (cnt--) {
        ret = usb_g_bulk_read(usb_id, MSD_BULK_EP_OUT, pBuf, uCount, 0);
        if (ret) {
            break;
        }
    }
    // printf("%s,%d, ret:%d, cnt:%d \n", __func__, __LINE__, ret, cnt);
    // put_buf(pBuf, ret);
    return ret;
}
#endif
#endif
