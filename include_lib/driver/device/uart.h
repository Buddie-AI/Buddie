#ifndef DEVICE_UART_H
#define DEVICE_UART_H

#include "typedef.h"
#include "device/device.h"
#include "generic/ioctl.h"
#include "system/task.h"

#define UART_DISABLE        0x00000000
#define UART_DMA_SUPPORT 	0x00000001
#define UART_TX_USE_DMA 	0x00000003
#define UART_RX_USE_DMA 	0x00000005
#define UART_DEBUG 			0x00000008

struct uart_outport {
    u8  tx_pin;
    u8  rx_pin;
    u16 value;
};

extern void putbyte(char a);




enum uart_clk_src {
    LSB_CLK,
    OSC_CLK,
    PLL_48M,
};


enum _uart_port_out {
    //uart0
    PORTC_0_1 = 0x00001000,
    PORTG_6_7 = 0x00002000,
    PORTH_12_13 = 0x00003000,
    PORTB_14_15 = 0x00004000,
    //uart1
    PORTC_2_3 = 0x00005000,
    PORTH_2_5 = 0x00006000,
    PORTH_14_15 = 0x00007000,
    PORTC_6_7 = 0x00008000,
    //uart3
    PORTE_0_1 = 0x00009000,
    PORTB_4_3 = 0x0000A000,
    PORTD_9_10 = 0x0000B000,
    PORTD_14_15 = 0x0000C000,

    PORT_REMAP = 0x0000D000,
};

struct uart_platform_data {
    u8 *name;

    u8  irq;
    u8  tx_pin;
    u8  rx_pin;
    u32 flags;
    u32 baudrate;

    enum _uart_port_out port;
    void (*port_remap_func)(void);
    u32 max_continue_recv_cnt;
    u32 idle_sys_clk_cnt;
    enum uart_clk_src clk_src;
};

enum {
    UART_CIRCULAR_BUFFER_WRITE_OVERLAY = -1,
    UART_RECV_TIMEOUT = -2,
    UART_RECV_EXIT = -3,
};

#define UART_MAGIC                          'U'
#define UART_FLUSH                          _IO(UART_MAGIC,1)
#define UART_SET_RECV_ALL                   _IOW(UART_MAGIC,2,bool)
#define UART_SET_RECV_BLOCK                 _IOW(UART_MAGIC,3,bool)
#define UART_SET_RECV_TIMEOUT               _IOW(UART_MAGIC,4,u32)
#define UART_SET_RECV_TIMEOUT_CB            _IOW(UART_MAGIC,5,int (*)(void))
#define UART_GET_RECV_CNT                   _IOR(UART_MAGIC,6,u32)
#define UART_START                          _IO(UART_MAGIC,7)
#define UART_SET_CIRCULAR_BUFF_ADDR         _IOW(UART_MAGIC,8,void *)
#define UART_SET_CIRCULAR_BUFF_LENTH        _IOW(UART_MAGIC,9,u32)


#define UART_PLATFORM_DATA_BEGIN(data) \
    static const struct uart_platform_data data = {


#define UART_PLATFORM_DATA_END() \
    };


struct uart_device {
    char *name;
    const struct uart_operations *ops;
    struct device dev;
    const struct uart_platform_data *priv;
    OS_MUTEX mutex;
};




struct uart_operations {
    int (*init)(struct uart_device *);
    int (*read)(struct uart_device *, void *buf, u32 len);
    int (*write)(struct uart_device *, void *buf, u16 len);
    int (*ioctl)(struct uart_device *, u32 cmd, u32 arg);
    int (*close)(struct uart_device *);
};



#define REGISTER_UART_DEVICE(dev) \
    static struct uart_device dev sec(.uart)

extern struct uart_device uart_device_begin[], uart_device_end[];

#define list_for_each_uart_device(p) \
    for (p=uart_device_begin; p<uart_device_end; p++)



extern const struct device_operations uart_dev_ops;



#endif

