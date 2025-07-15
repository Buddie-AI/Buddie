#include "adapter_wireless_enc.h"
//#include "adapter_encoder.h"
#include "media/sbc_enc.h"
#include "app_config.h"

#if TCFG_WIRELESS_MIC_ENABLE

/* #define WIRELESS_ENCODER_SEL	AUDIO_CODING_SBC */
#define WIRELESS_ENCODER_SEL	AUDIO_CODING_LC3

#if (WIRELESS_ENCODER_SEL == AUDIO_CODING_SBC)
#define WIRELESS_ENC_IN_SIZE			512
#else
#define WIRELESS_ENC_IN_SIZE			480
#endif
#define WIRELESS_ENC_OUT_SIZE			256
#define WIRELESS_FRAME_SUM          	(1)/*一包多少帧*/
#if (WIRELESS_ENCODER_SEL == AUDIO_CODING_LC3)
#define WIRELESS_PACKET_HEADER_LEN  	(2)//(2 + 2)//(2)/*包头Max Length*/
#else
#define WIRELESS_PACKET_HEADER_LEN  	(2)//(2)/*包头Max Length*/
#endif
#define WIRELESS_PACKET_REPEAT_SUM  	(1)/*包重发次数*/
#define WIRELESS_ENC_OUT_BUF_SIZE		(WIRELESS_ENC_OUT_SIZE * WIRELESS_FRAME_SUM)
#define WIRELESS_ENC_PCM_BUF_LEN		(WIRELESS_ENC_IN_SIZE * 4)

struct __adapter_wireless_enc {
    struct audio_encoder 	encoder;
    cbuffer_t 			 	cbuf_pcm;
    s16 				 	pcm_buf[WIRELESS_ENC_PCM_BUF_LEN / 2];
    s16 				 	output_frame[WIRELESS_ENC_OUT_SIZE / 2];		// 编码输出帧
    int 				 	input_frame[WIRELESS_ENC_IN_SIZE / 4];
    u8  				 	out_buf[WIRELESS_PACKET_HEADER_LEN + WIRELESS_ENC_OUT_BUF_SIZE];	// sbc帧缓存

    u16 				 	frame_in_size;	// 帧输入大小
    u8  				 	frame_cnt;		// 帧打包计数
    u16 				 	frame_len;		// 帧长
    u16 				 	tx_seqn;		// 传输计数
    volatile u8  		 	start; 			// 启动标志

};


static struct audio_encoder_task *wireless_encode_task = NULL;



#define     MY_IO_DEBUG_0(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}
#define     MY_IO_DEBUG_1(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}

static int adapter_wireless_enc_push(struct __adapter_wireless_enc *hdl, void *buf, int len)
{
    if (!hdl->start) {
        return 0;
    }

//	printf("len = %d\n", len);

    int ret = 0;
    extern int wireless_mic_ble_send(void *priv, u8 * buf, u16 len);
    for (int i = 0; i < WIRELESS_PACKET_REPEAT_SUM; i++) {
        ret = wireless_mic_ble_send(NULL, buf, len);
        if (ret) {
//            printf("lost ###################\n");
            putchar('E');
        } else {
//            putchar('O');
        }
    }
    return len;
}

static int adapter_wireless_enc_write_pcm(struct __adapter_wireless_enc *hdl, void *buf, int len)
{
    if (!hdl->start) {
        return 0;
    }
    //putchar('w');
    int wlen = cbuf_write(&hdl->cbuf_pcm, buf, len);
    if (cbuf_get_data_len(&hdl->cbuf_pcm) >= (hdl->frame_in_size)) {
        audio_encoder_resume(&hdl->encoder);
    }
    return wlen;
}


static void adapter_wireless_enc_packet_pack(struct __adapter_wireless_enc *hdl, u16 len)
{
    //将包的序列号写到头部
    hdl->tx_seqn++;
#if (WIRELESS_PACKET_HEADER_LEN)
    hdl->out_buf[0] = hdl->tx_seqn >> 8;
    hdl->out_buf[1] = hdl->tx_seqn & 0xff;

#if (WIRELESS_ENCODER_SEL == AUDIO_CODING_LC3)
    //memcpy(&hdl->out_buf[2], &len, 2);
#endif//AUDIO_CODING_LC3

#endif
}

static int adapter_wireless_enc_pcm_get(struct audio_encoder *encoder, s16 **frame, u16 frame_len)
{
    struct __adapter_wireless_enc *hdl = container_of(encoder, struct __adapter_wireless_enc, encoder);
    int pcm_len = 0;
    //printf("frame_len = %d   ", frame_len);

#if (WIRELESS_ENCODER_SEL == AUDIO_CODING_SBC)
    frame_len = hdl->frame_in_size;
#else
    if (frame_len > hdl->frame_in_size) {
        printf("frame_len over limit !!!!, %d\n", frame_len);
        return 0;
    }
#endif
    //printf("frame_len = %d\n", frame_len);
    pcm_len = cbuf_read(&hdl->cbuf_pcm, hdl->input_frame, frame_len);
    if (pcm_len != frame_len) {
        /* putchar('L'); */
    } else {
        /* put_u16hex(frame_len); */
    }


    *frame = (s16 *)hdl->input_frame;
    return pcm_len;
}

static void adapter_wireless_enc_pcm_put(struct audio_encoder *encoder, s16 *frame)
{
}

static const struct audio_enc_input adapter_wireless_enc_input = {
    .fget = adapter_wireless_enc_pcm_get,
    .fput = adapter_wireless_enc_pcm_put,
};


static int adapter_wireless_enc_probe_handler(struct audio_encoder *encoder)
{
    return 0;
}

static int adapter_wireless_enc_output_handler(struct audio_encoder *encoder, u8 *frame, int len)
{
    struct __adapter_wireless_enc *enc = container_of(encoder, struct __adapter_wireless_enc, encoder);
    //检查上次是否还有数据没有发出
    if (enc->frame_cnt >= WIRELESS_FRAME_SUM) {
        //putchar('&');
        int wlen = adapter_wireless_enc_push(enc, enc->out_buf, enc->frame_len);
        if (wlen != enc->frame_len) {
            return 0;
        }
        enc->frame_cnt = 0;
    }
    if (enc->frame_cnt == 0) {
        //重新打包， 加包头
        adapter_wireless_enc_packet_pack(enc, len);
        enc->frame_len = WIRELESS_PACKET_HEADER_LEN;
    }
    //拼接数据内容
    memcpy(&enc->out_buf[enc->frame_len], frame, len);
    enc->frame_len += len;
    enc->frame_cnt ++;
    //检查是否达到发送条件
    if (enc->frame_cnt >= WIRELESS_FRAME_SUM) {
        //putchar('@');
        int wlen = adapter_wireless_enc_push(enc, enc->out_buf, enc->frame_len);
        if (wlen == enc->frame_len) {
            enc->frame_cnt = 0;
            //audio_encoder_resume(&enc->encoder);
        }
    }
    return len;
}

const static struct audio_enc_handler adapter_wireless_enc_handler = {
    .enc_probe = adapter_wireless_enc_probe_handler,
    .enc_output = adapter_wireless_enc_output_handler,
};


static void adapter_wireless_enc_stop(struct __adapter_wireless_enc *hdl)
{
    if (hdl->start) {
        hdl->start = 0;
        audio_encoder_close(&hdl->encoder);
    }
}

static struct __adapter_wireless_enc *wireless_enc = NULL;
void adapter_wireless_enc_close(void)
{
    if (!wireless_enc) {
        return ;
    }
    adapter_wireless_enc_stop(wireless_enc);

    local_irq_disable();
    free(wireless_enc);
    wireless_enc = NULL;
    local_irq_enable();
}

#if 1
int adapter_wireless_enc_open_base(struct audio_fmt *fmt, u16 frame_in_size)
{
    int err = 0;
    if (fmt == NULL) {
        log_e("wireless_enc_open parm error\n");
        return -1;
    }
    struct __adapter_wireless_enc *hdl = zalloc(sizeof(struct __adapter_wireless_enc));
    if (!hdl) {
        return -1;
    }

    hdl->frame_in_size = frame_in_size;

    // 变量初始化
    cbuf_init(&hdl->cbuf_pcm, hdl->pcm_buf, sizeof(hdl->pcm_buf));

    if (!wireless_encode_task) {
        ///这里不用SDK本身的编码线程因为编码线程成不能高于协议栈线程
        wireless_encode_task = zalloc(sizeof(struct audio_encoder_task));
        if (!wireless_encode_task) {
            printf("wireless_encode_task NULL !!!\n");
        }
        printf("wireless_encode_task 1111\n");
        audio_encoder_task_create(wireless_encode_task, "wl_mic_enc");
    }


    printf("wireless_encode_task 2222\n");
    audio_encoder_open(&hdl->encoder, &adapter_wireless_enc_input, wireless_encode_task);
    audio_encoder_set_handler(&hdl->encoder, &adapter_wireless_enc_handler);
    audio_encoder_set_fmt(&hdl->encoder, fmt);
    audio_encoder_set_output_buffs(&hdl->encoder, hdl->output_frame, sizeof(hdl->output_frame), 1);

    if (!hdl->encoder.enc_priv) {
        log_e("encoder err, maybe coding(0x%x) disable \n", fmt.coding_type);
        err = -EINVAL;
        goto __err;
    }

    printf("wireless_encode_task 3333\n");
    audio_encoder_start(&hdl->encoder);
    printf("wireless_encode_task 4444\n");

    hdl->start = 1;

    local_irq_disable();
    wireless_enc = hdl;
    local_irq_enable();

    return 0;

__err:
    audio_encoder_close(&hdl->encoder);
    local_irq_disable();
    free(hdl);
    hdl = NULL;
    local_irq_enable();
    return err;
}

#endif

int adapter_wireless_enc_write(void *buf, int len)
{
    if (!wireless_enc) {
        return 0;
    }
    int wlen = adapter_wireless_enc_write_pcm(wireless_enc, buf, len);
    if (wlen != len) {
        putchar('L');
    }
    return wlen;
}



//static struct audio_stream_entry g_entry;	// 音频流入口
static sbc_t g_sbc_param = {0};
u16 g_sample_rate = 44100;
u8 g_channel = 2;


static int adapter_wireless_enc_data_handler(struct audio_stream_entry *entry,  struct audio_data_frame *in,
        struct audio_data_frame *out)
{
    int wlen = adapter_wireless_enc_write(in->data, in->data_len);
    if (wlen == in->data_len) {
        //putchar('W');
    } else {
        //putchar('M');
    }
    return in->data_len;
}

int adapter_wireless_enc_open(void)
{
    struct audio_fmt fmt = {0};
    u16 frame_in_size = 0;
#if (WIRELESS_ENCODER_SEL == AUDIO_CODING_SBC)
    sbc_t *sbc_param = &g_sbc_param;

    sbc_param->frequency = SBC_FREQ_44100;
    sbc_param->blocks = SBC_BLK_16;
    sbc_param->subbands = SBC_SB_8;
    sbc_param->mode = SBC_MODE_STEREO;
    sbc_param->allocation = 0;
    sbc_param->endian = SBC_LE;
    sbc_param->bitpool = 19;//53;

    printf("sbc_param->bitpool ============================ %d\n", sbc_param->bitpool);

    switch (g_sample_rate) {
    case 16000:
        sbc_param->frequency = SBC_FREQ_16000;
        break;
    case 32000:
        sbc_param->frequency = SBC_FREQ_32000;
        break;
    case 44100:
        sbc_param->frequency = SBC_FREQ_44100;
        break;
    case 48000:
        sbc_param->frequency = SBC_FREQ_48000;
        break;
    }
    if (g_channel == 1) {
        sbc_param->mode = SBC_MODE_MONO;
        frame_in_size = WIRELESS_ENC_IN_SIZE / 2;
    } else {
        sbc_param->mode = SBC_MODE_STEREO;
        frame_in_size = WIRELESS_ENC_IN_SIZE;
    }

    fmt.coding_type = AUDIO_CODING_SBC;
    fmt.sample_rate = g_sample_rate;
    fmt.channel = g_channel;
    fmt.priv = &g_sbc_param;
#else
    extern const int LC3_DMS_VAL;      //单位ms, 【只支持 25,50,100】
    fmt.coding_type = AUDIO_CODING_LC3;
    fmt.sample_rate = 44100;//g_sample_rate;
#if (TCFG_WIRELESS_MIC_STEREO_EN)
    fmt.channel = 2;
#else
    fmt.channel = 1;
#endif
    fmt.bit_rate = 64000;//128000;//64000;//128000;
    fmt.frame_len = LC3_DMS_VAL;//2.5 * 10;
    if (fmt.channel == 1) {
        frame_in_size = WIRELESS_ENC_IN_SIZE / 2;
    } else {
        frame_in_size = WIRELESS_ENC_IN_SIZE;
    }
#endif

    adapter_wireless_enc_open_base(&fmt, frame_in_size);
    return 0;
}


#endif//TCFG_WIRELESS_MIC_ENABLE

