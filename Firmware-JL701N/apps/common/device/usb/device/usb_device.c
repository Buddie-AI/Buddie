#include "usb/device/usb_stack.h"
#include "usb_config.h"
#include "usb/device/msd.h"
#include "usb/scsi.h"
#include "usb/device/hid.h"
#include "usb/device/uac_audio.h"
#include "usb/device/cdc.h"
#include "irq.h"
#include "init.h"
#include "gpio.h"
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

static void usb_device_init(const usb_dev usb_id)
{

    usb_config(usb_id);
    usb_g_sie_init(usb_id);
#if defined(FUSB_MODE) && FUSB_MODE
    usb_write_power(usb_id, 0x40);
#elif defined(FUSB_MODE) && (FUSB_MODE == 0)
    usb_write_power(usb_id, 0x60);
#endif
    usb_slave_init(usb_id);
    u8 *ep0_dma_buffer = usb_alloc_ep_dmabuffer(usb_id, 0, 64);

    usb_set_dma_raddr(usb_id, 0, ep0_dma_buffer);
    usb_set_dma_raddr(usb_id, 1, ep0_dma_buffer);
    usb_set_dma_raddr(usb_id, 2, ep0_dma_buffer);
    usb_set_dma_raddr(usb_id, 3, ep0_dma_buffer);
    usb_set_dma_raddr(usb_id, 4, ep0_dma_buffer);

    usb_write_intr_usbe(usb_id, INTRUSB_RESET_BABBLE | INTRUSB_SUSPEND);
    usb_clr_intr_txe(usb_id, -1);
    usb_clr_intr_rxe(usb_id, -1);
    usb_set_intr_txe(usb_id, 0);
    usb_set_intr_rxe(usb_id, 0);
    usb_g_isr_reg(usb_id, 3, 0);
    /* usb_sof_isr_reg(usb_id,3,0); */
    /* usb_sofie_enable(usb_id); */
    /* USB_DEBUG_PRINTF("ep0 addr %x %x\n", usb_get_dma_taddr(0), ep0_dma_buffer); */
}
static void usb_device_hold(const usb_dev usb_id)
{

    usb_g_hold(usb_id);
    usb_release(usb_id);
}


static int usb_ep_conflict_check(const usb_dev usb_id);
int usb_device_mode(const usb_dev usb_id, const u32 class)
{

    /* usb_device_set_class(CLASS_CONFIG); */
    u8 class_index = 0;
    if (class == 0) {
        gpio_direction_input(IO_PORT_DM + 2 * usb_id);
        gpio_set_pull_up(IO_PORT_DM + 2 * usb_id, 0);
        gpio_set_pull_down(IO_PORT_DM + 2 * usb_id, 0);
        gpio_set_die(IO_PORT_DM + 2 * usb_id, 0);

        gpio_direction_input(IO_PORT_DP + 2 * usb_id);
        gpio_set_pull_up(IO_PORT_DP + 2 * usb_id, 0);
        gpio_set_pull_down(IO_PORT_DP + 2 * usb_id, 0);
        gpio_set_die(IO_PORT_DP + 2 * usb_id, 0);

        os_time_dly(15);

        gpio_set_die(IO_PORT_DM + 2 * usb_id, 1);
        gpio_set_die(IO_PORT_DP + 2 * usb_id, 1);

#if TCFG_USB_SLAVE_MSD_ENABLE
        msd_release(usb_id);
#endif

#if TCFG_USB_SLAVE_AUDIO_ENABLE
        uac_release(usb_id);
#endif

#if TCFG_USB_SLAVE_CDC_ENABLE
        cdc_release(usb_id);
#endif
#if TCFG_USB_SLAVE_HID_ENABLE
        hid_release(usb_id);
#endif
        usb_device_hold(usb_id);
        return 0;
    }

    /* int ret = usb_ep_conflict_check(usb_id); */
    /* if (ret) {                               */
    /*     return ret;                          */
    /* }                                        */

    usb_memory_init();

    usb_add_desc_config(usb_id, MAX_INTERFACE_NUM, NULL);

#if TCFG_USB_SLAVE_MSD_ENABLE
    if ((class & MASSSTORAGE_CLASS) == MASSSTORAGE_CLASS) {
        log_info("add desc msd");
        usb_add_desc_config(usb_id, class_index++, msd_desc_config);
        msd_register(usb_id);
    }
#endif

#if TCFG_USB_SLAVE_AUDIO_ENABLE
    if ((class & AUDIO_CLASS) == AUDIO_CLASS) {
        log_info("add audio desc");
        usb_add_desc_config(usb_id, class_index++, uac_audio_desc_config);
        uac_register(usb_id, AUDIO_CLASS);
    } else if ((class & SPEAKER_CLASS) == SPEAKER_CLASS) {
        log_info("add desc speaker");
        usb_add_desc_config(usb_id, class_index++, uac_spk_desc_config);
        uac_register(usb_id, SPEAKER_CLASS);
    } else if ((class & MIC_CLASS) == MIC_CLASS) {
        log_info("add desc mic");
        usb_add_desc_config(usb_id, class_index++, uac_mic_desc_config);
        uac_register(usb_id, MIC_CLASS);
    }
#endif

#if TCFG_USB_SLAVE_HID_ENABLE
    if ((class & HID_CLASS) == HID_CLASS) {
        log_info("add desc std hid");
        usb_add_desc_config(usb_id, class_index++, hid_desc_config);
        hid_register(usb_id);
    }
#endif

#if TCFG_USB_SLAVE_CDC_ENABLE
    if ((class & CDC_CLASS) == CDC_CLASS) {
        log_info("add desc cdc");
        usb_add_desc_config(usb_id, class_index++, cdc_desc_config);
        cdc_register(usb_id);
    }
#endif

    usb_device_init(usb_id);
    user_setup_filter_install(usb_id2device(usb_id));
    return 0;
}
/* module_initcall(usb_device_mode); */

static int usb_ep_conflict_check(const usb_dev usb_id)
{
    u8 usb_ep_tx_list[] = {
#if TCFG_USB_SLAVE_MSD_ENABLE
        MSD_BULK_EP_IN,
#endif
#if TCFG_USB_SLAVE_HID_ENABLE
        HID_EP_IN,
#endif
#if TCFG_USB_SLAVE_AUDIO_ENABLE
        MIC_ISO_EP_IN,
#endif
#if TCFG_USB_SLAVE_CDC_ENABLE
        CDC_DATA_EP_IN,
#if CDC_INTR_EP_ENABLE
        CDC_INTR_EP_IN,
#endif
#endif
    };
    u8 usb_ep_rx_list[] = {
#if TCFG_USB_SLAVE_MSD_ENABLE
        MSD_BULK_EP_OUT,
#endif
#if TCFG_USB_SLAVE_HID_ENABLE
        HID_EP_OUT,
#endif
#if TCFG_USB_SLAVE_AUDIO_ENABLE
        SPK_ISO_EP_OUT,
#endif
#if TCFG_USB_SLAVE_CDC_ENABLE
        CDC_DATA_EP_OUT,
#endif
    };
    int ret = 0;
    int i, j;

    for (i = 0; i < sizeof(usb_ep_tx_list) - 1; i++) {
        for (j = i + 1; j < sizeof(usb_ep_tx_list); j++) {
            if (usb_ep_tx_list[i] == usb_ep_tx_list[j]) {
                ret = -1;
                ASSERT(0, "ep%d conflict, dir in\n", usb_ep_tx_list[i]);
                goto __exit;
            }
        }
    }
    for (i = 0; i < sizeof(usb_ep_rx_list) - 1; i++) {
        for (j = i + 1; j < sizeof(usb_ep_rx_list); j++) {
            if (usb_ep_rx_list[i] == usb_ep_rx_list[j]) {
                ret = -1;
                ASSERT(0, "ep%d conflict, dir out\n", usb_ep_rx_list[i]);
                goto __exit;
            }
        }
    }
__exit:
    return ret;
}

#endif

/*
 * @brief otg检测中sof初始化，不要放在TCFG_USB_SLAVE_ENABLE里
 * @parm id usb设备号
 * @return 0 ,忽略sof检查，1 等待sof信号
 */
u32 usb_otg_sof_check_init(const usb_dev id)
{
    /* return 0;// */
    u8 *ep0_dma_buffer = usb_alloc_ep_dmabuffer(id, 0, 64);

    usb_g_sie_init(id);

#ifdef USB_HW_20
    //husb调用完usb_g_sie_init()，或者收到usb reset中断，intrusbe，intrtxe，
    //intrrxe会复位成默认值，不手动清零以及关中断，会收到usb reset中断，由于
    //空指针问题导致异常
    usb_write_intr_usbe(id, 0);
    usb_clr_intr_txe(id, -1);
    usb_clr_intr_rxe(id, -1);
    if (id == 0) {
        unrequest_irq(IRQ_USB_CTRL_IDX);
#if USB_MAX_HW_NUM > 1
    } else {
        unrequest_irq(IRQ_USB1_CTRL_IDX);
#endif
    }
#endif

#if defined(FUSB_MODE) && FUSB_MODE
    usb_write_power(id, 0x40);
#elif defined(FUSB_MODE) && (FUSB_MODE == 0)
    usb_write_power(id, 0x60);
#endif

    usb_set_dma_raddr(id, 0, ep0_dma_buffer);

    for (int ep = 1; ep < USB_MAX_HW_EPNUM; ep++) {
        usb_disable_ep(id, ep);
    }
    usb_sof_clr_pnd(id);
    return 1;
}
