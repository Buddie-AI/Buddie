#include "usb_config.h"
#include "usb/scsi.h"
#include "irq.h"
#include "init.h"
#include "gpio.h"
#include "timer.h"
#include "app_config.h"
#include "lbuf.h"

#ifdef CONFIG_ADAPTER_ENABLE
#include "adapter_usb_hid.h"
#endif//CONFIG_ADAPTER_ENABLE

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define     SET_INTERRUPT   ___interrupt


#define     MAX_EP_TX   5
#define     MAX_EP_RX   5

static usb_interrupt usb_interrupt_tx[USB_MAX_HW_NUM][MAX_EP_TX];// SEC(.usb_g_bss);
static usb_interrupt usb_interrupt_rx[USB_MAX_HW_NUM][MAX_EP_RX];// SEC(.usb_h_bss);

static u8 ep0_dma_buffer[EP0_SETUP_LEN]     __attribute__((aligned(4))) SEC(.usb_ep0)    ;

#if TCFG_USB_SLAVE_MSD_ENABLE
#define     MSD_DMA_SIZE (MAXP_SIZE_BULKOUT + MAXP_SIZE_BULKIN)
#else
#define     MSD_DMA_SIZE 0
#endif

#if TCFG_USB_SLAVE_HID_ENABLE
#define     HID_DMA_SIZE    64
#else
#define     HID_DMA_SIZE    0
#endif

#if TCFG_USB_SLAVE_AUDIO_ENABLE
#define     AUDIO_DMA_SIZE  256+192
#else
#define     AUDIO_DMA_SIZE  0
#endif

#if TCFG_USB_SLAVE_CDC_ENABLE
#if CDC_INTR_EP_ENABLE
#define     CDC_DMA_SIZE    (64*3)
#else
#define     CDC_DMA_SIZE    (64*2)
#endif
#else
#define     CDC_DMA_SIZE    0
#endif

struct usb_config_var_t {
    u8 usb_setup_buffer[USB_SETUP_SIZE];
    struct usb_ep_addr_t usb_ep_addr;
    struct usb_setup_t usb_setup;
};
static struct usb_config_var_t *usb_config_var = {NULL};

#if USB_MALLOC_ENABLE
#else
static struct usb_config_var_t _usb_config_var SEC(.usb_config_var);
#endif


#define USB_DMA_BUF_ALIGN	(8)
#ifndef USB_DMA_BUF_MAX_SIZE
#define USB_DMA_BUF_MAX_SIZE (HID_DMA_SIZE +USB_DMA_BUF_ALIGN+ AUDIO_DMA_SIZE +USB_DMA_BUF_ALIGN+ MSD_DMA_SIZE*2 + USB_DMA_BUF_ALIGN+CDC_DMA_SIZE + USB_DMA_BUF_ALIGN + 100)
#endif//USB_DMA_BUF_MAX_SIZE

static u8 usb_dma_buf[USB_DMA_BUF_MAX_SIZE] SEC(.usb_msd_dma) __attribute__((aligned(8)));
struct lbuff_head *usb_dma_lbuf = NULL;
void usb_memory_init()
{
    usb_dma_lbuf = lbuf_init(usb_dma_buf, sizeof(usb_dma_buf), USB_DMA_BUF_ALIGN, 0);
    log_info("%s() total dma size %x @%x", __func__, sizeof(usb_dma_buf), usb_dma_buf);
}

__attribute__((always_inline_when_const_args))
void *usb_alloc_ep_dmabuffer(const usb_dev usb_id, u32 ep, u32 dma_size)
{
    u8 *ep_buffer = NULL;
    u32 _ep = ep & 0xf;
    if (ep & USB_DIR_IN) {
        switch (_ep) {
        case 0:
            ep_buffer = ep0_dma_buffer;
            break;
        default :
            ep_buffer = lbuf_alloc(usb_dma_lbuf, dma_size);
            break;
        }
    } else {
        switch (_ep) {
        case 0:
            ep_buffer = ep0_dma_buffer;
            break;
        default :
            ep_buffer = lbuf_alloc(usb_dma_lbuf, dma_size);
            break;
        }
    }
    ASSERT(ep_buffer, "usb_alloc_ep_dmabuffer ep_buffer = NULL!!!, _ep = %x, dma_size = %d\n", ep, dma_size);

    log_info("ep_buffer = %x, ep = %x, dma_size = %d\n", ep_buffer, ep, dma_size);

    return ep_buffer;
}


static u8 sleep_flag = 0;
u8 get_sleep_flag()
{
    return sleep_flag ;
}

void set_sleep_flag(u8 sl_flag)
{
    sleep_flag = sl_flag ;
}

static void usb_resume_sign(void *priv)
{
    usb_dev usb_id = usb_device2id(priv);
    u32 reg = usb_read_power(usb_id);
    usb_write_power(usb_id, reg | BIT(2));//send resume
    os_time_dly(2);//10ms~20ms
    usb_write_power(usb_id, reg);//clean resume
}

void usb_remote_wakeup(const usb_dev usb_id)
{
    struct usb_device_t *usb_device = usb_id2device(usb_id);
    if (usb_device->bRemoteWakup) {
        sys_timeout_add(usb_device, usb_resume_sign, 1);
    }
}
void usb_phy_resume(const usb_dev usb_id)
{
    usb_iomode(0);

    struct usb_device_t *usb_device = usb_id2device(usb_id);
    usb_write_faddr(usb_id, usb_device->baddr);

    if (usb_device->baddr == 0) {
        usb_device->bDeviceStates = USB_DEFAULT;
    } else {
        usb_device->bDeviceStates = USB_CONFIGURED;
    }

    usb_otg_resume(usb_id);
}

void usb_phy_suspend(const usb_dev usb_id)
{
    gpio_set_pull_up(IO_PORT_DP, 1);
    gpio_set_pull_down(IO_PORT_DP, 0);
    gpio_set_direction(IO_PORT_DP, 1);

    usb_iomode(1);
    /* musb_read_usb(0, MUSB_INTRUSB); */

    usb_otg_suspend(usb_id, OTG_KEEP_STATE);
}
void usb_isr(const usb_dev usb_id)
{
    u32 intr_usb, intr_usbe;
    u32 intr_tx, intr_txe;
    u32 intr_rx, intr_rxe;

    __asm__ volatile("ssync");
    usb_read_intr(usb_id, &intr_usb, &intr_tx, &intr_rx);
    usb_read_intre(usb_id, &intr_usbe, &intr_txe, &intr_rxe);
    struct usb_device_t *usb_device = usb_id2device(usb_id);

    intr_usb &= intr_usbe;
    intr_tx &= intr_txe;
    intr_rx &= intr_rxe;

    if (intr_usb & INTRUSB_SUSPEND) {
        log_error("usb suspend");
        set_sleep_flag(1);
#if USB_SUSPEND_RESUME
        usb_phy_suspend(usb_id);
#endif

#if USB_SUSPEND_RESUME_SYSTEM_NO_SLEEP
        printf("\n NULL \n");
#endif
    }
    if (intr_usb & INTRUSB_RESET_BABBLE) {
        log_error("usb reset");
        usb_reset_interface(usb_device);

#if USB_SUSPEND_RESUME || USB_SUSPEND_RESUME_SYSTEM_NO_SLEEP
        u32 reg = usb_read_power(usb_id);
        usb_write_power(usb_id, (reg | INTRUSB_SUSPEND | INTRUSB_RESUME));//enable suspend resume
#endif
    }

    if (intr_usb & INTRUSB_RESUME) {
        log_error("usb resume");
#if USB_SUSPEND_RESUME
        usb_phy_resume(usb_id);
#endif
    }

    if (intr_tx & BIT(0)) {
        if (usb_interrupt_rx[usb_id][0]) {
            usb_interrupt_rx[usb_id][0](usb_device, 0);
        } else {
            usb_control_transfer(usb_device);
        }
    }

    for (int i = 1; i < MAX_EP_TX; i++) {
        if (intr_tx & BIT(i)) {
            if (usb_interrupt_tx[usb_id][i]) {
                usb_interrupt_tx[usb_id][i](usb_device, i);
            }
        }
    }

    for (int i = 1; i < MAX_EP_RX; i++) {
        if (intr_rx & BIT(i)) {
            if (usb_interrupt_rx[usb_id][i]) {
                usb_interrupt_rx[usb_id][i](usb_device, i);
            }
        }
    }
    __asm__ volatile("csync");
}

void usb_sof_isr(const usb_dev usb_id)
{
    usb_sof_clr_pnd(usb_id);
    static u32 sof_count = 0;
    if ((sof_count++ % 1000) == 0) {
        log_d("sof 1s isr frame:%d", usb_read_sofframe(usb_id));
    }
}
void usb_suspend_check(void *p)
{
    usb_dev usb_id = (usb_dev)p;

    static u16 sof_frame = 0;
    u16 frame = usb_read_sofframe(usb_id);// sof frame 不更新，则usb进入断开或者suspend状态
    if (frame == sof_frame) {
        usb_phy_suspend(usb_id);
    }
    sof_frame = frame;
}

SET_INTERRUPT
void usb0_g_isr()
{
    usb_isr(0);
}
SET_INTERRUPT
void usb0_sof_isr()
{
    usb_sof_isr(0);
}

#if USB_MAX_HW_NUM == 2
SET_INTERRUPT
void usb1_g_isr()
{
    usb_isr(1);
}
SET_INTERRUPT
void usb1_sof_isr()
{
    usb_sof_isr(1);
}
#endif
__attribute__((always_inline_when_const_args))
u32 usb_g_set_intr_hander(const usb_dev usb_id, u32 ep, usb_interrupt hander)
{
    if (ep & USB_DIR_IN) {
        usb_interrupt_tx[usb_id][ep & 0xf] = hander;
    } else {
        usb_interrupt_rx[usb_id][ep] = hander;
    }
    return 0;
}
void usb_g_isr_reg(const usb_dev usb_id, u8 priority, u8 cpu_id)
{
    if (usb_id == 0) {
        request_irq(IRQ_USB_CTRL_IDX, priority, usb0_g_isr, cpu_id);
    } else {
#if USB_MAX_HW_NUM == 2
        request_irq(IRQ_USB1_CTRL_IDX, priority, usb1_g_isr, cpu_id);
#endif
    }
}
void usb_sof_isr_reg(const usb_dev usb_id, u8 priority, u8 cpu_id)
{
    if (usb_id == 0) {
        request_irq(IRQ_USB_SOF_IDX, priority, usb0_sof_isr, cpu_id);
    } else {
#if USB_MAX_HW_NUM == 2
        request_irq(IRQ_USB1_SOF_IDX, priority, usb1_sof_isr, cpu_id);
#endif
    }
}
u32 usb_config(const usb_dev usb_id)
{
    memset(usb_interrupt_rx[usb_id], 0, sizeof(usb_interrupt_rx[usb_id]));
    memset(usb_interrupt_tx[usb_id], 0, sizeof(usb_interrupt_tx[usb_id]));

    if (!usb_config_var) {
#if USB_MALLOC_ENABLE
        usb_config_var = (struct usb_config_var_t *)zalloc(sizeof(struct usb_config_var_t));
        if (!usb_config_var) {
            return -1;
        }
#else
        memset(&_usb_config_var, 0, sizeof(_usb_config_var));
        usb_config_var = &_usb_config_var;
#endif
    }
    log_debug("zalloc: usb_config_var = %x\n", usb_config_var);

    usb_var_init(usb_id, &(usb_config_var->usb_ep_addr));
    usb_setup_init(usb_id, &(usb_config_var->usb_setup), usb_config_var->usb_setup_buffer);
    return 0;
}

u32 usb_release(const usb_dev usb_id)
{
    log_debug("free zalloc: usb_config_var = %x\n", usb_id, usb_config_var);
    usb_var_init(usb_id, NULL);
    usb_setup_init(usb_id, NULL, NULL);
#if USB_MALLOC_ENABLE
    if (usb_config_var) {
        log_debug("free: usb_config_var = %x\n", usb_config_var);
        free(usb_config_var);
    }
#endif

    usb_config_var = NULL;

    return 0;
}
