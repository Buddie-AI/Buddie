#include "app_config.h"
#ifdef AUDIO_PCM_DEBUG
#include "uartPcmSender.h"

extern int aec_uart_init();
extern int aec_uart_fill(u8 ch, void *buf, u16 size);
extern void aec_uart_write(void);
extern int aec_uart_close(void);

/*
 16k 正选波 pcm 数据
*/
static short const sin0_16k[16] = {
    0x0000, 0x30fd, 0x5a83, 0x7641, 0x7fff, 0x7642, 0x5a82, 0x30fc, 0x0000, 0xcf04, 0xa57d, 0x89be, 0x8000, 0x89be, 0xa57e, 0xcf05,
};
static u16 tx1_s_cnt = 0;
static int get_sine0_data(u16 *s_cnt, s16 *data, u16 points, u8 ch)
{
    while (points--) {
        if (*s_cnt >= 16) {
            *s_cnt = 0;
        }
        *data++ = sin0_16k[*s_cnt] / 2;
        if (ch == 2) {
            *data++ = sin0_16k[*s_cnt] / 2;
        }
        (*s_cnt)++;
    }
    return 0;
}

/*
*************************************************************
*           uart data export 接口使用说明
* 1、每个通道一次只能固定发送512byte数据,数据不够会被丢弃
* 2、aec_uart_write()一次会发送3个通道的数据，没有填数的通道会发送空白数据
* 3、UART DMA发送一次数据需要约8ms时间，两次发数间隔需 > 8ms
* 4、发送波特率、发送引脚在 uartPcmSender.h 配置
* 5、该接口的接收端需使用JL的串口导出工具接收数据
*
*************************************************************
*/
void audio_export_demo_task(void *param)
{
    printf("audio_export_demo_task\n");
    s16 data[256];
    int len = 512;
    /*生成256个点的16k的正选波数据*/
    get_sine0_data(&tx1_s_cnt, data, len / 2, 1);

    while (1) {
        putchar('.');

        {
            /*run code*/
        }

        aec_uart_fill(0, data, 512);    //往通道0填数据

        {
            /*run code*/
        }

        aec_uart_fill(1, data, 512);    //往通道1填数据

        {
            /*run code*/
        }

        aec_uart_fill(2, data, 512);    //往通道2填数据
        aec_uart_write();               //一次把3路数据发送出去
        os_time_dly(2);                 //等待发送完成，发送一次要8ms，发完才可以进行下一次发数
    }
}

/*
**********************************************************
* uart data export 接口使用示例
*********************************************************
*/
int audio_export_demo_init()
{
    printf("audio_export_demo_init\n");
    /* uartSendInit(); //串口初始化，已经在开机时调用初始化，这里不需调用 */
    aec_uart_init();    //uart export 初始化
    os_task_create(audio_export_demo_task, NULL, 1, 1024, 128, "audio_export_task");

    return 0;
}


/*
*************************************************************
*         通用 uart 发数接口（uartSendData()）使用说明
* 1、发数的数据长度可以根据需要修改
* 2、一般发送512byte,DMA模式需要2.7ms
* 3、发送波特率、发送引脚在 uartPcmSender.h 配置
* 4、可以使用该接口自定义发数方式、协议等
*
*************************************************************
*/
void audio_uart_transmit_demo_task(void *param)
{
    printf("audio_uart_transmit_demo_task\n");
    s16 data[256];
    int len = 512;
    /*生成256个点的16k的正选波数据*/
    get_sine0_data(&tx1_s_cnt, data, len / 2, 1);

    while (1) {
        putchar('.');
        uartSendData(data, len);    //uart 发送数据接口
        os_time_dly(1);             //一般发送512byte,DMA模式需要2.7ms
    }
}
/*
**********************************************************
* 通用uart发数使用示例
*********************************************************
*/
int audio_uart_transmit_demo_init()
{
    printf("audio_uart_transmit_demo_init\n");
    /* uartSendInit(); //串口初始化，已经在开机时调用初始化，这里不需调用 */
    os_task_create(audio_uart_transmit_demo_task, NULL, 1, 1024, 128, "audio_uart_transmit_task");
    return 0;
}

static u8 audio_export_demo_idle_query()
{
    return 0;
}

REGISTER_LP_TARGET(audio_export_demo_lp_target) = {
    .name = "audio_export__demo",
    .is_idle = audio_export_demo_idle_query,
};

#endif /*AUDIO_PCM_DEBUG*/

