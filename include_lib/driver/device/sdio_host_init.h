#ifndef _SDIO_HOST_INIT_H_
#define _SDIO_HOST_INIT_H_

/*
SDIO_GRP_0|SDIO_PORT_0: PA1-PA4->CMD,CLK,D2,D0; PH13,PH15->D3,D1
SDIO_GRP_0|SDIO_PORT_1: PH6-PH11->D3,CMD,CLK,D2,D0,D1
SDIO_GRP_0|SDIO_PORT_2: PC8-PC13->D2,D3,CMD,CLK,D0,D1
SDIO_GRP_0|SDIO_PORT_3: PG8-PG13->D2,D3,CMD,CLK,D0,D1
SDIO_GRP_1|SDIO_PORT_0: PF4-PF9->D2,D3,CMD,CLK,D0,D1
SDIO_GRP_1|SDIO_PORT_1: PG0-PG5->D2,D3,CMD,CLK,D0,D1
SDIO_GRP_1|SDIO_PORT_2: PD6-PD11->D2,D3,CMD,CLK,D0,D1
SDIO_GRP_1|SDIO_PORT_3: PA10-PA15->D2,D3,CMD,CLK,D0,D1
*/

//选择SDIO0? SDIO1? 作为输出,默认为SDIO1
#define SDIO_GRP_0  0
#define SDIO_GRP_1  (1 << 31)
#define SDIO_GRP_2  (1 << 30)
#define SDIO_GRP_MASK  (1 << 31|1 << 30)

//选择SDIOx的哪个出口作为输出,默认为出口0
#define SDIO_PORT_0  0
#define SDIO_PORT_1  (1 << 29)
#define SDIO_PORT_2  (1 << 28)
#define SDIO_PORT_3  (1 << 29|1 << 28)
#define SDIO_PORT_MASK  (1 << 29|1 << 28)

//是否使用四线模式, 默认为单线模式
#define SDIO_1_BIT_DATA  0
#define SDIO_4_BIT_DATA  (1 << 27)
#define SDIO_4_BIT_DATA_MASK  (1 << 27)

//是否使用硬件中断检测外设事件,例如接收到数据. , 默认使用轮询方式查询事件, //对接收速度不敏感使用SDIO_POLLING即可 ,使用SDIO_DATA1_IRQ如果接收数据量过大会导致CPU不足的问题.
#define SDIO_POLLING  0
#define SDIO_DATA1_IRQ  (1 << 26)
#define SDIO_DATA1_IRQ_MASK  (1 << 26)

//配置SDIO时钟(HZ), 默认为40MHZ
#define SDIO_MAX_CLK_MASK ((1 << 26)-1)

#define SDIO_CLOCK_80M  (0) //80M 写0
#define SDIO_CLOCK_40M  (40 * 1000000)
#define SDIO_CLOCK_26M  (26 * 1000000)
#define SDIO_CLOCK_20M  (20 * 1000000)
#define SDIO_CLOCK_16M  (16 * 1000000)
#define SDIO_CLOCK_8M  (8 * 1000000)
#define SDIO_CLOCK_4M  (4 * 1000000)
#define SDIO_CLOCK_2M (2 * 1000000)

/*
sdio_host_init(0);
sdio_host_init(SDIO_GRP_1|SDIO_4_BIT_DATA);
sdio_host_init(SDIO_GRP_1|SDIO_PORT_1|SDIO_DATA1_IRQ);
sdio_host_init(SDIO_GRP_0|SDIO_PORT_0|SDIO_4_BIT_DATA|SDIO_DATA1_IRQ|(10*1000000));
*/

extern  void sdio_host_init(unsigned int parm);
extern unsigned char *SDIO_GET_CMD_BUF(void);
extern void  SDIO_SET_GRP_PORT(u32 grp, u32 port);
extern void cpu_sdio_host_uninit(void);
extern void sdio_dat1_irq_uninit(void);
extern void sdio_dat1_irq_init(void);
extern void host_set_timing(void *host, unsigned int timing);
extern void mmc_set_bus_width(void *host, unsigned int width);
extern void mmc_set_clock(void *host,  unsigned int hz);
extern void  SDIO_CONTROLLER_RESET(void);
extern void  SDIO_IDLE_CLK_EN(u8 enable) ;
extern void  SDIO_CONTROLLER_START(void);
extern void SDIO_CONTROLLER_SET_IRQ(void);
extern  void  SDIO_SET_4WIRE_MODE(u8 enable);

#endif  //_SDIO_HOST_INIT_H_
