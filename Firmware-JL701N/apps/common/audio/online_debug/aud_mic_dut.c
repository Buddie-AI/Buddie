#include "aud_mic_dut.h"
#include "audio_online_debug.h"
#include "online_db_deal.h"
#include "app_config.h"
#include "system/includes.h"

#if ((defined TCFG_AUDIO_MIC_DUT_ENABLE) && TCFG_AUDIO_MIC_DUT_ENABLE)
#include "mic_dut_process.h"

#define AUDIO_MIC_DUT_TASK_NAME     		"AudioMicDut"

typedef struct {
    u8 start;                   /*mic dut 开始和结束标志*/
    u16 mic_gain;               /*保存在线调试的mic gain*/
    u32 sr;                     /*保存在线调试的mic采样率*/
    u8 mic_idx;                 /*记录当前使用的那个mic*/
    u8 scan_start;              /*mic dut scan 开始和结束的标志*/
    u8 dac_gain;                /*保存在线调试的dac gain*/
    u32 send_timer;             /*定时器句柄*/
    u32 seqn;                   /*数据的seqn*/
    OS_SEM sem;                 /*信号量*/
} mic_online_info_t;

/*硬件信息*/
static mic_dut_info_t mic_info;

/*在线调试信息*/
static mic_online_info_t online_info_t;
static mic_online_info_t *online_info_hdl = &online_info_t;

static int spp_data_export(u8 ch, u8 *buf, u16 len)
{
    u8 data_ch;
    putchar('.');
    if (ch == 0) {
        data_ch = DB_PKT_TYPE_DAT_CH0;
    } else if (ch == 1) {
        data_ch = DB_PKT_TYPE_DAT_CH1;
    } else {
        data_ch = DB_PKT_TYPE_DAT_CH2;
    }
    int err = app_online_db_send_more(data_ch, buf, len);
    if (err) {
        printf("tx_err:%d", err);
    }
    return len;
}

/*在线数据初始化*/
static void mic_dut_online_init(void)
{
    if (online_info_hdl) {
        online_info_hdl->start = 0;
        online_info_hdl->mic_gain = mic_info.mic_gain_default;
        online_info_hdl->sr = mic_info.sr_default;
        online_info_hdl->mic_idx = 0;
        online_info_hdl->scan_start = 0;
        online_info_hdl->dac_gain = mic_info.dac_gain_default;
        online_info_hdl->send_timer = 0;
        online_info_hdl->seqn = 1;
    } else {
        printf("online_info_hdl is NULL !!!\n");
    }
}

static void audio_mic_dut_data_export_run(void)
{
    if (online_info_hdl) {
        u8 buf[644];
        u8 *data = &buf[4];
        int len = 640;
        int rlen = 0;
        if (audio_mic_dut_get_data_len() >= len) {
            memcpy(buf, &online_info_hdl->seqn, 4);
            rlen = audio_mic_dut_data_get(data, len);
            extern void audio_mic_dut_eq_run(s16 * data, int len);
            audio_mic_dut_eq_run(data, len);
            /* printf("%d, %d", len, rlen); */
            int ret = spp_data_export(0, buf, rlen + 4);

            online_info_hdl->seqn ++;
            /* printf("seqn %d %d\n", mic_online_info.seqn,*((int *) buf)); */
        }
    }
}

static void audio_mic_dut_timer(void *p)
{
    audio_mic_dut_data_export_run();
}

static void audio_mic_dut_task(void *p)
{
    while (1) {
        os_sem_pend(&online_info_hdl->sem, 0);
        audio_mic_dut_data_export_run();
    }
}

static void mic_dut_online_start(void)
{
    if (online_info_hdl) {
#if MIC_DUT_DATA_SEND_BY_TIMER
        online_info_hdl->send_timer = usr_timer_add(NULL, audio_mic_dut_timer, MIC_DATA_SEND_INTERVAL, 1);
#else
        os_sem_create(&online_info_hdl->sem, 0);
        os_task_create(audio_mic_dut_task, NULL, 1, 768, 64, AUDIO_MIC_DUT_TASK_NAME);
#endif
    }
}

/*释放mic dut 信号量*/
void mic_dut_online_sem_post(void)
{
    if (online_info_hdl) {
#if (MIC_DUT_DATA_SEND_BY_TIMER == 0)
        os_sem_post(&online_info_hdl->sem);
#endif /*SPP_SEND_BY_TIMER*/
    }
}

static void mic_dut_online_stop(void)
{
    if (online_info_hdl) {
        if (online_info_hdl->start) {
            audio_mic_dut_stop();
            online_info_hdl->start = 0;
        }
        if (online_info_hdl->scan_start) {
            audio_mic_dut_scan_stop();
            online_info_hdl->scan_start = 0;
        }

#if MIC_DUT_DATA_SEND_BY_TIMER
        if (online_info_hdl->send_timer) {
            usr_timer_del(online_info_hdl->send_timer);
            online_info_hdl->send_timer = 0;
        }
#else
        os_task_del(AUDIO_MIC_DUT_TASK_NAME);
        os_sem_del(&online_info_hdl->sem, 0);
#endif
        /* online_info_hdl = NULL; */
    }
}

/*数据解析*/
static int mic_dut_online_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size)
{
    int res_data = 0;
    online_cmd_t mic_dut_cmd;
    int err = 0;
    u8 parse_seq = ext_data[1];
    //AEC_ONLINE_LOG("[MIC_DUT]spp_rx,seq:%d,size:%d\n", parse_seq, size);
    put_buf(packet, size);
    memcpy(&mic_dut_cmd, packet, sizeof(online_cmd_t));
    printf("[MIC_DUT]cmd:0x%x\n", mic_dut_cmd.cmd);
    switch (mic_dut_cmd.cmd) {
    case AUD_RECORD_COUNT:
        y_printf("AEC_RECORD_COUNT\n");
        res_data = 1;
        err = app_online_db_ack(parse_seq, (u8 *)&res_data, 4);
        break;
    case ONLINE_OP_QUERY_RECORD_PACKAGE_LENGTH:
        y_printf("ONLINE_OP_QUERY_RECORD_PACKAGE_LENGTH\n");
        if (mic_dut_cmd.data == 0) {
            res_data = RECORD_CH0_LENGTH;
        } else if (mic_dut_cmd.data == 1) {
            res_data = RECORD_CH1_LENGTH;
        } else {
            res_data = RECORD_CH2_LENGTH;
        }
        printf("query record ch%d packet length:%d\n", mic_dut_cmd.data, res_data);
        err = app_online_db_ack(parse_seq, (u8 *)&res_data, 4); //回复对应的通道数据长度
        break;
    case MIC_DUT_INFO_QUERY:
        y_printf("MIC_DUT_INFO_QUERY\n");
        /*获取mic dut硬件数据*/
        audio_mic_dut_info_get(&mic_info);
        /*在线数据初始化*/
        mic_dut_online_init();
        err = app_online_db_ack(parse_seq, (u8 *)&mic_info, sizeof(mic_info));
        break;
    case MIC_DUT_GAIN_SET:
        y_printf("MIC_DUT_GAIN_SET:%d\n", mic_dut_cmd.data);
        if (online_info_hdl) {
            online_info_hdl->mic_gain = mic_dut_cmd.data;
        }
        audio_mic_dut_gain_set(mic_dut_cmd.data);
        err = app_online_db_ack(parse_seq, (u8 *)"OK", 2);
        break;
    case MIC_DUT_SAMPLE_RATE_SET:
        y_printf("MIC_DUT_SAMPLE_RATE_SET:%d\n", mic_dut_cmd.data);
        if (online_info_hdl) {
            online_info_hdl->sr = mic_dut_cmd.data;
        }
        audio_mic_dut_sample_rate_set(mic_dut_cmd.data);
        err = app_online_db_ack(parse_seq, (u8 *)"OK", 2);
        break;
    case MIC_DUT_START:
        y_printf("MIC_DUT_START\n");
        /*打开发数资源*/
        mic_dut_online_start();
        /*打开mic*/
        audio_mic_dut_start();
        if (online_info_hdl) {
            online_info_hdl->start = 1;
        }
        err = app_online_db_ack(parse_seq, (u8 *)"OK", 2);
        break;
    case MIC_DUT_STOP:
        y_printf("MIC_DUT_STOP\n");
        if (online_info_hdl) {
            online_info_hdl->start = 0;
        }
        /*关闭mic*/
        audio_mic_dut_stop();
        /*关闭发数资源*/
        mic_dut_online_stop();
        err = app_online_db_ack(parse_seq, (u8 *)"OK", 2);
        break;
    case MIC_DUT_AMIC_SEL:
        y_printf("MIC_DUT_AMIC_SEL:%d\n", mic_dut_cmd.data);
        if (online_info_hdl) {
            online_info_hdl->mic_idx = mic_dut_cmd.data;
        }
        audio_mic_dut_amic_select(mic_dut_cmd.data);
        err = app_online_db_ack(parse_seq, (u8 *)"OK", 2);
        break;
    case MIC_DUT_DMIC_SEL:
        y_printf("MIC_DUT_DMIC_SEL:%d\n", mic_dut_cmd.data);
        if (online_info_hdl) {
            online_info_hdl->mic_idx = mic_dut_cmd.data;
        }
        audio_mic_dut_dmic_select(mic_dut_cmd.data);
        err = app_online_db_ack(parse_seq, (u8 *)"OK", 2);
        break;
    case MIC_DUT_DAC_GAIN_SET:
        y_printf("MIC_DUT_DAC_GAIN_SET:%d\n", mic_dut_cmd.data);
        if (online_info_hdl) {
            online_info_hdl->dac_gain = mic_dut_cmd.data;
        }
        audio_mic_dut_dac_gain_set(mic_dut_cmd.data);
        err = app_online_db_ack(parse_seq, (u8 *)"OK", 2);
        break;
    case MIC_DUT_SCAN_START:
        y_printf("MIC_DUT_SCAN_START\n");
        /*打开发数资源*/
        mic_dut_online_start();
        /*打开dac*/
        audio_mic_dut_scan_start();
        if (online_info_hdl) {
            online_info_hdl->scan_start = 1;
        }
        err = app_online_db_ack(parse_seq, (u8 *)"OK", 2);
        break;
    case MIC_DUT_SCAN_STOP:
        y_printf("MIC_DUT_SCAN_STOP\n");
        if (online_info_hdl) {
            online_info_hdl->scan_start = 0;
        }
        /*关闭dac*/
        audio_mic_dut_scan_stop();
        /*关闭发数资源*/
        mic_dut_online_stop();
        err = app_online_db_ack(parse_seq, (u8 *)"OK", 2);
        break;

    default:
        break;
    }
    printf("err:%d\n", err);
    return 0;

}

/*获取当前设置的mic增益*/
int mic_dut_online_get_mic_gain(void)
{
    if (online_info_hdl) {
        return online_info_hdl->mic_gain;
    }
    return -1;
}

/*获取当前设置的dac增益*/
int mic_dut_online_get_dac_gain(void)
{
    if (online_info_hdl) {
        return online_info_hdl->dac_gain;
    }
    return -1;
}

/*获取当前设置的采样率*/
int mic_dut_online_get_sample_rate(void)
{
    if (online_info_hdl) {
        return online_info_hdl->sr;
    }
    return -1;
}

/*获取当前使能的模拟mic*/
u8 mic_dut_online_amic_enable_bit(void)
{
    return mic_info.amic_enable_bit;
}

/*获取当前使能的数字mic*/
u8 mic_dut_online_dmic_enable_bit(void)
{
    return mic_info.dmic_enable_bit;
}

/*获取当前在线使用的mic*/
int mic_dut_online_get_mic_idx(void)
{
    if (online_info_hdl) {
        return online_info_hdl->mic_idx;
    }
    return -1;
}

/*获取mic dut状态*/
int mic_dut_online_get_mic_state(void)
{
    if (online_info_hdl) {
        return online_info_hdl->start;
    }
    return -1;
}

/*获取dut scan状态*/
int mic_dut_online_get_scan_state(void)
{
    if (online_info_hdl) {
        return online_info_hdl->scan_start;
    }
    return -1;
}

/*注册mic dut解析回调*/
int aud_mic_dut_open(void)
{
    app_online_db_register_handle(DB_PKT_TYPE_MIC_DUT, mic_dut_online_parse);
    return 0;
}

int aud_mic_dut_close(void)
{
    return 0;
}
#endif /*((defined TCFG_AUDIO_MIC_DUT_ENABLE) && TCFG_AUDIO_MIC_DUT_ENABLE)*/
