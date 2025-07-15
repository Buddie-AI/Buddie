#ifndef HWI_H
#define HWI_H

//=================================================
#define IRQ_EMUEXCPT_IDX   		0		//0
#define IRQ_EXCEPTION_IDX  		1		//0
#define IRQ_SYSCALL_IDX    		2		//0
#define IRQ_TICK_TMR_IDX   		3		//0
#define IRQ_TIME0_IDX      		4   	//0
#define IRQ_TIME1_IDX      		5   	//0
#define IRQ_TIME2_IDX      		6   	//0
#define IRQ_TIME3_IDX      		7   	//0
#define IRQ_TIME4_IDX      		8   	//0
#define IRQ_TIME5_IDX      		9   	//0

#define IRQ_UART0_IDX      		12		//0
#define IRQ_UART1_IDX      		13		//0
#define IRQ_UART2_IDX      		14		//0

#define IRQ_SPI0_IDX       		16		//0
#define IRQ_SPI1_IDX       		17		//0
#define IRQ_SPI2_IDX       		18		//0

#define IRQ_SD0_IDX        		20		//0

#define IRQ_IIC_IDX        		24		//0

#define IRQ_USB_SOF_IDX    		28   //1
#define IRQ_USB_CTRL_IDX   		29  	//1

#define IRQ_P2M_IDX        		32  	//0
#define IRQ_LP_TIMER0_IDX  		33
#define IRQ_LP_TIMER1_IDX  		34
#define IRQ_LP_TIMER2_IDX  		35
#define IRQ_LP_TIMER3_IDX  		36

#define IRQ_PORT_IDX		   	38		//0
#define IRQ_GPADC_IDX		   	39		//0
#define IRQ_CTM_IDX             40
#define IRQ_PWM_LED_IDX		   	41		//1
#define IRQ_OSA_IDX			   	42
#define IRQ_LRCT_IDX		   	43       //1
#define IRQ_GPC_IDX			   	44		//1
#define IRQ_FMTX_IDX		   	45		//1
#define IRQ_TDM_IDX			   	46		//1
#define IRQ_RDEC0_IDX		   	47		//1
#define IRQ_SBC_IDX			   	48		//1
#define IRQ_AES_IDX        		49
#define IRQ_RDEC1_IDX		   	50		//1
#define IRQ_RDEC2_IDX		   	51		//1
#define IRQ_MCPWM_CHX_IDX       52
#define IRQ_MCPWM_TMR_IDX       53

#define IRQ_ANC_IDX             67
#define IRQ_AUDIO_IDX      		68		//1
#define IRQ_ALINK0_IDX     		69  	//1
#define IRQ_PDM_LINK_IDX   		70		//1

#define IRQ_BT_CLKN_IDX    		76		//2
#define IRQ_BT_DBG_IDX     		77		//1
#define IRQ_BLE_RX_IDX     		78		//2
#define IRQ_BLE_EVENT_IDX  		79		//1

#define IRQ_BT_TIMEBASE_IDX  	80		//1
#define IRQ_WL_LOFC_IDX    		81		//2
#define IRQ_BREDR_IDX      		82		//2

#define IRQ_SYNC_IDX      		88		//2
#define IRQ_SRC_SYNC_IDX        89		//1
#define IRQ_SRC_IDX       		90		//1
#define IRQ_EQ_IDX         		91		//1
#define IRQ_DCP_IDX	       		93		//1

#define IRQ_IMG_CP_IDX	       	103		//1
#define IRQ_IMD_IDX	       		104		//1
#define IRQ_IMB_IDX	       		105		//1
#define IRQ_JPG_IDX	       		106		//1

#define IRQ_FFT_IDX	       		116		//1

#define IRQ_SOFT0_IDX      		124
#define IRQ_SOFT1_IDX      		125
#define IRQ_SOFT2_IDX      		126
#define IRQ_SOFT3_IDX      		127

#define MAX_IRQ_ENTRY_NUM   128

//=================================================

//=================================================

void interrupt_init();


/* --------------------------------------------------------------------------*/
/**
 * @brief 中断注册函数
 *
 * @param index 中断号
 * @param priority 优先级，范围0-6可用
 * @param handler 中断服务函数
 * @param cpu_id 相应中断服务函数的CPU
 */
/* ----------------------------------------------------------------------------*/
void request_irq(u8 index, u8 priority, void (*handler)(void), u8 cpu_id);

void unrequest_irq(u8 index);

void reg_set_ip(unsigned char index, unsigned char priority, u8 cpu_id);


/* --------------------------------------------------------------------------*/
/**
 * @brief 设置不可屏蔽中断（不可屏蔽中断不区分优先级）
 * cpu多核同步默认使用0，其他使用1。
 *
 * @param index 中断号
 * @param priority 不可屏蔽优先级，范围 0、1 （CPU_IRQ_IPMASK_LEVEL == 6）
 * @param cpu_id 相应中断服务函数的CPU
 */
/* ----------------------------------------------------------------------------*/
void irq_unmask_set(u8 index, u8 priority, u8 cpu_id);

void bit_clr_ie(unsigned char index);
void bit_set_ie(unsigned char index);
bool irq_read(u32 index);

#define irq_disable(x)  bit_clr_ie(x)
#define irq_enable(x)   bit_set_ie(x)

void unmask_enter_critical(void);
void unmask_exit_critical(void);
//---------------------------------------------//
// low power waiting
//---------------------------------------------//
__attribute__((always_inline))
static void lp_waiting(int *ptr, int pnd, int cpd, char inum)
{
    q32DSP(core_num())->IWKUP_NUM = inum;
    while (!(*ptr & pnd)) {
        asm volatile("idle");
    }
    *ptr |= cpd;
}

//---------------------------------------------//
// interrupt cli/sti
//---------------------------------------------//

static inline int int_cli(void)
{
    int msg;
    asm volatile("cli %0" : "=r"(msg) :);
    return msg;
}

static inline void int_sti(int msg)
{
    asm volatile("sti %0" :: "r"(msg));
}

#ifdef IRQ_TIME_COUNT_EN
void irq_handler_enter(int irq);

void irq_handler_exit(int irq);

void irq_handler_times_dump();
#else

#define irq_handler_enter(irq)      do { }while(0)
#define irq_handler_exit(irq)       do { }while(0)
#define irq_handler_times_dump()    do { }while(0)

#endif


#endif

