#include "private_iis.h"
#include "app_config.h"
#include "audio_decoder.h"
#include "media/includes.h"
#include "audio_config.h"
#include "system/includes.h"
#include "audio_enc.h"
#include "application/audio_eq.h"
#include "application/audio_drc.h"
#include "app_config.h"
#include "audio_config.h"
#include "audio_dec.h"
#include "app_main.h"
#include "clock_cfg.h"
#include "audio_dec_eff.h"
#include "audio_codec_clock.h"
#include "audio_dvol.h"
#include "audio_link.h"
#include "circular_buf.h"
#include "sound_device.h"

#define PRIVATE_IIS_MSG_CHECK_EN	1		//使能格式检查

struct __private_iis_data {
    u32 msg[CMD_LEN / 4];
    u8 get_msg_flag;
    u8 msg_len;
};
static struct __private_iis_data private_iis_data = {0};

struct private_iis_hdl {
    OS_SEM sem;
    /* s16 *store_pcm_buf; */
    /* cbuffer_t cbuf; */
    int sample_rate;
    void *hw_alink;
    void *alink_ch;
    u32 cmd;		//要求发送或者接收到的cmd
};
static struct private_iis_hdl *private_iis = NULL;

extern ALINK_PARM alink0_platform_data;

/********************************* private iis Rx *********************************/
// 这个格式检查需要根据具体消息自己定义！！, 也可以不做检查
static u8 private_iis_check_msg(void)
{
#if PRIVATE_IIS_MSG_CHECK_EN
    if (private_iis_data.msg[0] == PRIVATE_IIS_CMD && private_iis_data.msg[1] != private_iis_data.msg[0]) {
        return 1;
    }
    return 0;	//格式检查不过
#else
    return 1;
#endif
}


// 接收消息
static void private_iis_in_data_handler(void *priv, void *_data, int len)
{
    s16 *data = (s16 *)_data;
    /* memcpy(private_iis_data.msg, data, len); */
    u8 *temp = memchr(data, private_iis->cmd, len);
    if (temp && private_iis_data.get_msg_flag == 0) {
        memcpy(private_iis_data.msg, temp, CMD_LEN);
        if (private_iis_check_msg()) {
            private_iis_data.get_msg_flag = 1;
            private_iis_data.msg_len = CMD_LEN;
        }
    }
    alink_set_shn(&alink0_platform_data.ch_cfg[1], len / 4);
}

/*
 * 打开iis接收，用来接收私有数据
 */
void private_iis_rx_open(void)
{
    if (private_iis) {
        printf("private iis is running!\n");
        return;
    }
    struct private_iis_hdl *iis_in = NULL;
    iis_in = zalloc(sizeof(struct private_iis_hdl));
    if (!iis_in) {
        return;
    }

    private_iis = iis_in;

    iis_in->cmd = PRIVATE_IIS_CMD;
    /* iis_in->store_pcm_buf = malloc(IIS_IN_STORE_PCM_SIZE); */
    /* if (!iis_in->store_pcm_buf) { */
    /* free(iis_in); */
    /* return; */
    /* } */
    /* cbuf_init(&iis_in->cbuf, iis_in->store_pcm_buf, IIS_IN_STORE_PCM_SIZE); */
    iis_in->hw_alink = (ALINK_PARM *)get_iis_alink_param();
    if (iis_in->hw_alink == NULL) {
        iis_in->hw_alink = alink_init(&alink0_platform_data);
    }
    iis_in->alink_ch = alink_channel_init(iis_in->hw_alink, 1, ALINK_DIR_RX, (void *)iis_in, private_iis_in_data_handler);
    alink_start(iis_in->hw_alink);
    iis_in->sample_rate = TCFG_IIS_SR;
}


u8 get_private_iis_get_msg_flag(void)
{
    return 	private_iis_data.get_msg_flag;
}

void show_private_msg(void)
{
    if (private_iis_data.get_msg_flag) {
        for (int i = 0; i < private_iis_data.msg_len / 4; i++) {
            y_printf(">>>msg[%d] = %d\n", i, private_iis_data.msg[i]);
        }
    } else {
        r_printf(">>>>> Show private msg failed!\n");
    }
}


void private_iis_rx_close(void)
{
    if (private_iis) {
        void *alink_param = (ALINK_PARM *)get_iis_alink_param();
        if (!alink_param) {
            //如果 alink_param 为NULL，则说明还没打开 iis tx
            alink_uninit(private_iis->hw_alink);
        } else {
            alink_channel_close(private_iis->alink_ch);
        }

        free(private_iis);
        private_iis = NULL;
    }
}


/********************************* private iis Tx *********************************/

// 发送消息
static void private_iis_out_data_handler(void *priv, void *_data, int len)
{
    u32 *buf_u32 = (u32 *)_data;
    buf_u32[0] = private_iis->cmd;
    buf_u32[1] = TCFG_IIS_SR;
    buf_u32[2] = PRIVATE_IIS_TX_LINEIN_CH_IDX;
    buf_u32[3] = PRIVATE_IIS_TX_LINEIN_GAIN;
    alink_set_shn(&alink0_platform_data.ch_cfg[0], len / 4);
}

void private_iis_tx_open(void)
{
    if (private_iis) {
        printf("private iis is running!\n");
        return;
    }
    struct private_iis_hdl *iis_out = NULL;
    iis_out = zalloc(sizeof(struct private_iis_hdl));
    if (!iis_out) {
        return;
    }

    private_iis = iis_out;

    iis_out->cmd = PRIVATE_IIS_CMD;

    iis_out->hw_alink = (ALINK_PARM *)get_iis_alink_param();
    if (iis_out->hw_alink == NULL) {
        iis_out->hw_alink = alink_init(&alink0_platform_data);
    }
    iis_out->alink_ch = alink_channel_init(iis_out->hw_alink, 0, ALINK_DIR_TX, (void *)iis_out, private_iis_out_data_handler);
    alink_start(iis_out->hw_alink);
    iis_out->sample_rate = TCFG_IIS_SR;
}

void private_iis_tx_close(void)
{
    if (private_iis) {
        void *alink_param = (ALINK_PARM *)get_iis_alink_param();
        if (!alink_param) {
            //如果 alink_param 为NULL，则说明还没打开 iis tx
            alink_uninit(private_iis->hw_alink);
        } else {
            alink_channel_close(private_iis->alink_ch);
        }

        free(private_iis);
        private_iis = NULL;
    }
}



// ************* test ************
static void private_iis_test_timer_cb(void *p)
{
    static u32 tick = 0;
    tick++;
    printf(">>>>>>>>>>>>>>> tick : %d\n", tick);
    if (get_private_iis_get_msg_flag()) {
        y_printf("(test) >>>>>>> GET MSG!\n");
        show_private_msg();
        private_iis_rx_close();
    }
}
void private_iis_rx_test(void)
{
    y_printf("-------------------- Enter %s !", __func__);
    private_iis_rx_open();
    sys_timer_add(NULL, private_iis_test_timer_cb, 1000);
}


void private_iis_tx_test(void)
{
    y_printf("-------------------- Enter %s !", __func__);
    private_iis_tx_open();
    os_time_dly(10);
    private_iis_tx_close();
    extern int iis_in_dec_open(void);
    iis_in_dec_open();
}


