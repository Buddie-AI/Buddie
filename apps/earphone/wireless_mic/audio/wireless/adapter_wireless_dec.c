#include "adapter_wireless_dec.h"
#include "asm/includes.h"
#include "system/includes.h"
#include "app_config.h"
#include "audio_config.h"
//#include "adapter_decoder.h"
#include "adapter_process.h"

#if TCFG_WIRELESS_MIC_ENABLE

//#define WIRELESS_DECODER_SEL	AUDIO_CODING_SBC
#define WIRELESS_DECODER_SEL	AUDIO_CODING_LC3

#define WIRELESS_PACKET_HEADER_LEN  	(2)//(2)/*包头Max Length*/
#define WIRELESS_DAC_MAX_DELAY			(16)
#define WIRELESS_DAC_START_DELAY		(10)

struct __adapter_wireless_dec {
    struct audio_decoder 			decoder;	// 解码器
    struct audio_res_wait 			wait;		// 资源等待句柄
    struct audio_mixer_ch 			mix_ch;		// 叠加句柄
    u8 								start;		// 解码开始
    u8 								wait_resume;// 需要激活
    u8								remain;		// 解码剩余数据标记
    int 							coding_type;// 解码类型
};


extern struct audio_decoder_task decode_task;
extern struct audio_dac_hdl dac_hdl;
extern struct audio_mixer mixer;

extern const int LC3_SUPPORT_CH;
extern const int LC3_DMS_VAL;      //单位ms, 【只支持 25,50,100】

#define ADAPTER_WIRELESS_FRAME_LBUF_SIZE		(150*LC3_SUPPORT_CH)

struct __adapter_wireless_dec *adapter_wireless_dec = NULL;

int adapter_wireless_media_get_packet(u8 **frame);
void adapter_wireless_media_free_packet(void *_packet);
void *adapter_wireless_media_fetch_packet(int *len, void *prev_packet);
int adapter_wireless_media_get_packet_num(void);
int adapter_wireless_dec_close(void);


// 解码获取数据
static int adapter_wireless_dec_get_frame(struct audio_decoder *decoder, u8 **frame)
{
    struct __adapter_wireless_dec *dec = container_of(decoder, struct __adapter_wireless_dec, decoder);
    u8 *packet = NULL;
    int len = 0;

    // 获取数据
    len = adapter_wireless_media_get_packet(&packet);
    if (len < 0) {
        // 失败
        putchar('X');
        return len;
    }
//	putchar('g');
    *frame = packet;

    return len;
}

// 解码释放数据空间
static void adapter_wireless_dec_put_frame(struct audio_decoder *decoder, u8 *frame)
{
    struct __adapter_wireless_dec *dec = container_of(decoder, struct __adapter_wireless_dec, decoder);

    if (frame) {
        adapter_wireless_media_free_packet((void *)(frame));
    }
}

// 解码查询数据
static int adapter_wireless_dec_fetch_frame(struct audio_decoder *decoder, u8 **frame)
{
    struct __adapter_wireless_dec *dec = container_of(decoder, struct __adapter_wireless_dec, decoder);
    u8 *packet = NULL;
    int len = 0;
    u32 wait_timeout = 0;

    if (!dec->start) {
        wait_timeout = jiffies + msecs_to_jiffies(500);
    }

__retry_fetch:
    packet = adapter_wireless_media_fetch_packet(&len, NULL);
    if (packet) {
        *frame = packet;
    } else if (!dec->start) {
        // 解码启动前获取数据来做格式信息获取等
        if (time_before(jiffies, wait_timeout)) {
            os_time_dly(1);
            goto __retry_fetch;
        }
    }
    printf("adapter_wireless_dec_fetch_frame ok\n");
    return len;
}

static const struct audio_dec_input adapter_wireless_input = {
    .coding_type = WIRELESS_DECODER_SEL,
    .data_type   = AUDIO_INPUT_FRAME,
    .ops = {
        .frame = {
            .fget = adapter_wireless_dec_get_frame,
            .fput = adapter_wireless_dec_put_frame,
            .ffetch = adapter_wireless_dec_fetch_frame,
        }
    }
};


// 解码预处理
static int adapter_wireless_dec_probe_handler(struct audio_decoder *decoder)
{
    struct __adapter_wireless_dec *dec = container_of(decoder, struct __adapter_wireless_dec, decoder);

    if (adapter_wireless_media_get_packet_num() < 1) {
        // 没有数据时返回负数，等有数据时激活解码
        dec->wait_resume = 1;
        audio_decoder_suspend(decoder, 0);
        return -EINVAL;
    }
    return 0;
}

// 解码后处理
static int adapter_wireless_dec_post_handler(struct audio_decoder *decoder)
{
    return 0;
}

static int adapter_wireless_dec_output_handler(struct audio_decoder *decoder, s16 *data, int len, void *priv)
{
    struct __adapter_wireless_dec *dec = container_of(decoder, struct __adapter_wireless_dec, decoder);
    if (!dec->remain) {
        /* put_u16hex(len); */
    }
    int wlen = 0;
    do {
        wlen = audio_mixer_ch_write(&dec->mix_ch, data, len);
    } while (0);

    int remain_len = len - wlen;

    if (remain_len == 0) {
        dec->remain = 0;
    } else {
        dec->remain = 1;
    }

    return wlen;
}

static const struct audio_dec_handler adapter_wireless_dec_handler = {
    .dec_probe  = adapter_wireless_dec_probe_handler,
    .dec_output =  adapter_wireless_dec_output_handler,
    .dec_post   = adapter_wireless_dec_post_handler,
};


// 解码释放
static void adapter_wireless_dec_release(void)
{
    // 删除解码资源等待
    audio_decoder_task_del_wait(&decode_task, &adapter_wireless_dec->wait);


    // 释放空间
    local_irq_disable();
    free(adapter_wireless_dec);
    adapter_wireless_dec = NULL;
    local_irq_enable();
}

// 解码关闭
static void adapter_wireless_audio_res_close(void)
{
    if (adapter_wireless_dec->start == 0) {
        printf("adapter_wireless_dec->start == 0");
        return ;
    }

    // 关闭数据流节点
    adapter_wireless_dec->start = 0;
    audio_decoder_close(&adapter_wireless_dec->decoder);

    audio_mixer_ch_close(&adapter_wireless_dec->mix_ch);

    //adapter_audio_stream_close(&adapter_wireless_dec->stream);

    app_audio_state_exit(APP_AUDIO_STATE_MUSIC);
}

// 解码事件处理
static void adapter_wireless_dec_event_handler(struct audio_decoder *decoder, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_DEC_EVENT_END:
        printf("AUDIO_DEC_EVENT_END\n");
        adapter_wireless_dec_close();
        break;
    }
}

// 解码数据流激活
static void adapter_wireless_out_stream_resume(void *p)
{
    struct __adapter_wireless_dec *dec = (struct __adapter_wireless_dec *)p;

    audio_decoder_resume(&dec->decoder);
}

// 收到数据后的处理
void adapter_wireless_media_rx_notice_to_decode(void)
{
    if (adapter_wireless_dec && adapter_wireless_dec->start && adapter_wireless_dec->wait_resume) {
        adapter_wireless_dec->wait_resume = 0;
        audio_decoder_resume(&adapter_wireless_dec->decoder);
    }
}

// 解码start
static int adapter_wireless_dec_start(void)
{
    int err;
    struct audio_fmt *fmt;
    struct __adapter_wireless_dec *dec = adapter_wireless_dec;

    if (!adapter_wireless_dec) {
        return -EINVAL;
    }

    printf("adapter_wireless_dec_start: in\n");

    // 打开adapter_wireless解码
    err = audio_decoder_open(&dec->decoder, &adapter_wireless_input, &decode_task);
    if (err) {
        goto __err1;
    }

    // 设置运行句柄
    audio_decoder_set_handler(&dec->decoder, &adapter_wireless_dec_handler);

#if (WIRELESS_DECODER_SEL == AUDIO_CODING_SBC)
    if (dec->coding_type != adapter_wireless_input.coding_type) {
        struct audio_fmt f = {0};
        f.coding_type = dec->coding_type;
        err = audio_decoder_set_fmt(&dec->decoder, &f);
        if (err) {
            goto __err2;
        }
    }

    // 获取解码格式
    err = audio_decoder_get_fmt(&dec->decoder, &fmt);
    if (err) {
        goto __err2;
    }
#else
    struct audio_fmt f = {0};
    f.coding_type = WIRELESS_DECODER_SEL;
    f.channel = LC3_SUPPORT_CH;
    f.sample_rate = 44100;
    f.bit_rate = 64000;
    f.frame_len = LC3_DMS_VAL;

    dec->decoder.fmt.channel = f.channel;
    dec->decoder.fmt.sample_rate = f.sample_rate;
    dec->decoder.fmt.bit_rate = f.bit_rate;
    dec->decoder.fmt.frame_len = f.frame_len;

    err = audio_decoder_set_fmt(&dec->decoder, &f);
    if (err) {
        goto __err2;
    }
#endif

    // 使能事件回调
    audio_decoder_set_event_handler(&dec->decoder, adapter_wireless_dec_event_handler, 0);

    // 设置输出声道类型
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)
    audio_decoder_set_output_channel(&dec->decoder, AUDIO_CH_LR);
#else
    audio_decoder_set_output_channel(&dec->decoder, AUDIO_CH_DIFF);
#endif


#if TCFG_AUDIO_DAC_ENABLE
    //设定延时,
    audio_dac_set_delay_time(&dac_hdl, WIRELESS_DAC_START_DELAY, WIRELESS_DAC_MAX_DELAY);
#endif//TCFG_AUDIO_DAC_ENABLE

    // 设置音频输出类型
    audio_mixer_ch_open(&dec->mix_ch, &mixer);
    audio_mixer_ch_set_sample_rate(&dec->mix_ch, f.sample_rate);
    audio_mixer_ch_set_resume_handler(&dec->mix_ch, (void *)&dec->decoder, (void (*)(void *))audio_decoder_resume);
    app_audio_state_switch(APP_AUDIO_STATE_MUSIC, get_max_sys_vol());


    // 开始解码
    dec->start = 1;
    err = audio_decoder_start(&dec->decoder);
    if (err) {
        goto __err3;
    }


    return 0;
__err3:
__err2:
    dec->start = 0;
    audio_decoder_close(&dec->decoder);
__err1:
    adapter_wireless_dec_release();

    return err;
}

// 解码资源等待回调
static int adapter_wireless_wait_res_handler(struct audio_res_wait *wait, int event)
{
    int err = 0;

    y_printf("adapter_wireless_wait_res_handler: %d\n", event);

    if (event == AUDIO_RES_GET) {
        // 可以开始解码
        err = adapter_wireless_dec_start();
    } else if (event == AUDIO_RES_PUT) {
        // 被打断
        if (adapter_wireless_dec->start) {
            adapter_wireless_audio_res_close();
        }
    }
    return err;
}

// 打开解码
//int adapter_wireless_dec_open(struct adapter_decoder_fmt *fmt, struct adapter_media_parm *media_parm)
int adapter_wireless_dec_open(void)
{
    struct __adapter_wireless_dec *dec;

    if (adapter_wireless_dec) {
        return 0;
    }

    printf("adapter_wireless_dec_open \n");

    dec = zalloc(sizeof(*dec));
    ASSERT(dec);


    /* dec->fmt = fmt; */
    /* dec->media_parm = media_parm; */

    adapter_wireless_dec = dec;
    dec->coding_type = WIRELESS_DECODER_SEL;	// 解码类型
    dec->wait.priority = 1;		// 解码优先级
    dec->wait.preemption = 0;	// 不使能直接抢断解码
    //dec->wait.snatch_same_prio = 1;	// 可抢断同优先级解码
    dec->wait.handler = adapter_wireless_wait_res_handler;

    adapter_wireless_dec_frame_init();

    audio_decoder_task_add_wait(&decode_task, &dec->wait);

    return 0;
}

// 关闭解码
int adapter_wireless_dec_close(void)
{

    printf("adapter_wireless_dec_close 1\n");
    if (!adapter_wireless_dec) {
        return 0;
    }

    if (adapter_wireless_dec->start) {
        adapter_wireless_audio_res_close();
    }
    adapter_wireless_dec_release();

    adapter_wireless_dec_frame_close();

    printf("adapter_wireless_dec_close 2\n");

    return 1;
}

/* static void adapter_wireless_dec_set_vol(u32 channel, u8 vol) */
/* { */
/* if (adapter_wireless_dec && adapter_wireless_dec->stream) { */
/* adapter_audio_stream_set_vol(adapter_wireless_dec->stream, channel, vol); */
/* } */
/* } */

/////////////////////////////////////////////////////////////////////////////////////////////
//

struct adapter_wireless_media_rx_bulk {
    struct list_head entry;
    int data_len;
    u8 data[0];
};

static u16 adapter_wireless_rx_seqn = 0;
static u32 *adapter_wireless_media_buf = NULL;
static LIST_HEAD(adapter_wireless_media_head);



// 获取frame数据
int adapter_wireless_media_get_packet(u8 **frame)
{
    struct adapter_wireless_media_rx_bulk *p;
    local_irq_disable();
    if (adapter_wireless_media_head.next != &adapter_wireless_media_head) {
        p = list_entry((&adapter_wireless_media_head)->next, typeof(*p), entry);
        __list_del_entry(&p->entry);
        *frame = p->data;
        local_irq_enable();
        return p->data_len;
    }
    local_irq_enable();

    return 0;
}

// 释放frame数据
void adapter_wireless_media_free_packet(void *data)
{
    struct adapter_wireless_media_rx_bulk *rx = container_of(data, struct adapter_wireless_media_rx_bulk, data);

    local_irq_disable();
    __list_del_entry(&rx->entry);
    local_irq_enable();

    lbuf_free(rx);
}

// 检查frame数据
void *adapter_wireless_media_fetch_packet(int *len, void *prev_packet)
{
    struct adapter_wireless_media_rx_bulk *p;
    local_irq_disable();
    if (adapter_wireless_media_head.next != &adapter_wireless_media_head) {
        if (prev_packet) {
            p = container_of(prev_packet, struct adapter_wireless_media_rx_bulk, data);
            if (p->entry.next != &adapter_wireless_media_head) {
                p = list_entry(p->entry.next, typeof(*p), entry);
                *len = p->data_len;
                local_irq_enable();
                return p->data;
            }
        } else {
            p = list_entry((&adapter_wireless_media_head)->next, typeof(*p), entry);
            *len = p->data_len;
            local_irq_enable();
            return p->data;
        }
    }
    local_irq_enable();

    return NULL;
}

// 获取数据量
int adapter_wireless_media_get_packet_num(void)
{
    struct adapter_wireless_media_rx_bulk *p;
    u32 num = 0;
    local_irq_disable();
    list_for_each_entry(p, &adapter_wireless_media_head, entry) {
        num++;
    }
    local_irq_enable();
    return num;
}

int adapter_wireless_dec_frame_write(void *data, u16 len)
{
//	printf("rx len = %d  ", len);
    struct adapter_wireless_media_rx_bulk *p;
    local_irq_disable();
    if (adapter_wireless_media_buf) {
        if (len < WIRELESS_PACKET_HEADER_LEN) {
            local_irq_enable();
            return 0;
        }
        u16 rx_seqn = 0;
        u8 *buf = data;
        rx_seqn |= (u16)(buf[0] << 8);
        rx_seqn |= buf[1];
        if (adapter_wireless_rx_seqn == rx_seqn) {
            putchar('1');
            local_irq_enable();
            return 0;
        }


        adapter_wireless_rx_seqn ++;
        if (rx_seqn != adapter_wireless_rx_seqn) {
//           putchar('2');
            adapter_wireless_rx_seqn = rx_seqn;
        }
//        putchar('R');

        len -= 2;
        buf += 2;

        p = lbuf_alloc((struct lbuff_head *)adapter_wireless_media_buf, sizeof(*p) + len);
        if (p == NULL) {
            printf("lbuf full !!\n");
            /* putchar('!'); */
            local_irq_enable();
            adapter_wireless_media_rx_notice_to_decode();
            return 0;
        }

        //putchar('W');
        // 填数
        p->data_len = len;//sizeof(sbc_data);
        memcpy(p->data, buf, len);
        list_add_tail(&p->entry, &adapter_wireless_media_head);
        // 告诉上层有数据
        adapter_wireless_media_rx_notice_to_decode();
        local_irq_enable();
        return 1;
    }
    local_irq_enable();
    return 0;
}

// 模拟定时关闭
void adapter_wireless_dec_frame_close(void)
{
    y_printf("%s,%d \n", __func__, __LINE__);

    local_irq_disable();
    __list_del_entry(&adapter_wireless_media_head);
    if (adapter_wireless_media_buf) {
        printf("meida_buf free!!!!!!!!!!!!!!!");
        free(adapter_wireless_media_buf);
        adapter_wireless_media_buf = NULL;
    }
    local_irq_enable();
}

void adapter_wireless_dec_frame_init(void)
{
    printf("%s,%d \n", __func__, __LINE__);
    if (adapter_wireless_media_buf) {
        return;
    }

    // 申请空间
    u32 buf_size = ADAPTER_WIRELESS_FRAME_LBUF_SIZE;
    void *buf = malloc(buf_size);
    ASSERT(buf);
    // 初始化lbuf
    local_irq_disable();
    lbuf_init(buf, buf_size, 4, 0);
    INIT_LIST_HEAD(&adapter_wireless_media_head);
    local_irq_enable();

    // 启动解码
    adapter_wireless_media_buf = buf;
}

#endif//TCFG_WIRELESS_MIC_ENABLE




