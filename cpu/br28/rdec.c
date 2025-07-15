#include "asm/includes.h"
#include "generic/gpio.h"
#include "asm/rdec.h"
#include "timer.h"

#define RDEC_DEBUG_ENABLE
#ifdef RDEC_DEBUG_ENABLE
#define rdec_debug(fmt, ...) 	printf("[RDEC] "fmt, ##__VA_ARGS__)
#else
#define rdec_debug(...)
#endif
#define RDEC_IRQ_MODE  0
#define rdec_error(fmt, ...) 	printf("[RDEC] ERR: "fmt, ##__VA_ARGS__)
struct  rdec {
    u8 init;
    const struct rdec_platform_data  *user_data;
};
static  struct  rdec _rdec = {0};
#define  __this     (&_rdec)

typedef struct {
    u32 con;
    int dat;
    int smp;
} RDEC_REG;

RDEC_REG *rdec_get_reg(u8 index)
{
    RDEC_REG *reg = NULL;
    switch (index) {
    case 0:
        reg = (RDEC_REG *)JL_RDEC0;
        break;
    case 1:
        reg = (RDEC_REG *)JL_RDEC1;
        break;
    case 2:
        reg = (RDEC_REG *)JL_RDEC2;
        break;
    }
    return reg;
}

s8 get_rdec_rdat(int i);
___interrupt
static void rdec0_isr()
{
    get_rdec_rdat(0);
}

___interrupt
static void rdec1_isr()
{
    get_rdec_rdat(1);
}

___interrupt
static void rdec2_isr()
{
    get_rdec_rdat(2);
}
static void __rdec_port_init(u8 port)
{
    gpio_set_pull_down(port, 0);
    gpio_set_pull_up(port, 1);
    gpio_set_die(port, 1);
    gpio_set_direction(port, 1);

}
static void rdec_port_init(const struct rdec_device *rdec)
{
    const u32 d0_table[] = {PFI_RDEC0_DAT0, PFI_RDEC1_DAT0, PFI_RDEC2_DAT0};
    const u32 d1_table[] = {PFI_RDEC0_DAT1, PFI_RDEC1_DAT1, PFI_RDEC2_DAT1};
    u32 index = rdec->index;

    __rdec_port_init(rdec->sin_port0);
    __rdec_port_init(rdec->sin_port1);

    printf("rdec->sin_port0 = %d", rdec->sin_port0);
    printf("rdec->sin_port1 = %d", rdec->sin_port1);

    gpio_set_fun_input_port(rdec->sin_port0, d0_table[index], LOW_POWER_FREE);
    gpio_set_fun_input_port(rdec->sin_port1, d1_table[index], LOW_POWER_FREE);
}
static void log_rdec_info(u32 index)
{
    RDEC_REG *reg = NULL;
    reg = rdec_get_reg(index);
    rdec_debug("RDEC CON = 0x%x", reg->con);
    printf("FI_RDEC3_DA0 = 0x%x", JL_IMAP->FI_RDEC0_DAT0);
    printf("FI_RDEC3_DA1 = 0x%x", JL_IMAP->FI_RDEC0_DAT1);

    printf("PB_DIR = 0x%x", JL_PORTB->DIR);
    printf("PB_OUT = 0x%x", JL_PORTB->OUT);
    printf("PB_PU = 0x%x",  JL_PORTB->PU);
    printf("PB_PD = 0x%x",  JL_PORTB->PD);
}

int rdec_init(const struct rdec_platform_data *user_data)
{
    u8 i;
    RDEC_REG *reg = NULL;
    rdec_debug("rdec init...");
    __this->init = 0;
    if (user_data == NULL) {
        return -1;
    }
    for (i = 0; i < user_data->num; i++) {
        u32 index = user_data->rdec[i].index;
        reg = rdec_get_reg(index);
        rdec_port_init(&(user_data->rdec[i]));
        //module init
        reg->con = 0;
        reg->con |= (0xe << 2); //2^14, fpga lsb = 24MHz, 2^14 / 24us = 682us = 0.68ms
        reg->con &= ~BIT(1); //pol = 0, io should pull up
        reg->con |= BIT(6); //clear pending
        reg->con |= BIT(8); //mouse mode
        reg->con |= BIT(0); //RDECx EN

#if RDEC_IRQ_MODE
        const u32 irq_table[] = {IRQ_RDEC0_IDX, IRQ_RDEC1_IDX, IRQ_RDEC2_IDX};
        const void (*irq_fun[3])(void) = {rdec0_isr, rdec1_isr, rdec2_isr};
        request_irq(irq_table[index], 1, irq_fun[index], 0);
#endif
        log_rdec_info(index);
    }
    __this->init = 1;
    __this->user_data = user_data;
    return 0;
}
s8 get_rdec_rdat(int i)
{
    RDEC_REG *reg = NULL;
    s8 rdat = 0;
    s8 _rdat = 0;
    reg = rdec_get_reg(i);
    if (reg->con & BIT(7)) {
        asm("csync");
        rdat = reg->dat;
        reg->con |= BIT(6);
        asm("csync");
        _rdat = reg->dat;
    }
    if (_rdat != 0) {
        rdec_debug("RDEC: %d , rdat: %d, _rdat: %d", i, rdat, _rdat);
    }
    return _rdat;
}

#define JL_RDEC_SEL 0
#define RDEC_TEST   1
#if RDEC_TEST
const struct rdec_device rdec_device_list[] = {
    {
        .index = JL_RDEC_SEL,
        .sin_port0 = IO_PORTA_01,
        .sin_port1 = IO_PORTA_02,
    },
};

RDEC_PLATFORM_DATA_BEGIN(rdec_data)
.enable = 1,
 .num = ARRAY_SIZE(rdec_device_list),
  .rdec = rdec_device_list,
   RDEC_PLATFORM_DATA_END()
#endif

   static void rdec_poll(void *p)
{
    for (int i = 0; i < 3; i++) {
        get_rdec_rdat(i);
    }
}

void rdec_test()
{
    rdec_init(&rdec_data);
#if RDEC_TEST
    sys_timer_add(NULL, rdec_poll, 100);
#endif
}
