/*
 ****************************************************************************
 *							Audio CVP DUT
 *
 *
 ****************************************************************************
 */
#include "audio_cvp_dut.h"
#include "online_db_deal.h"
#include "system/includes.h"
#include "app_config.h"
#include "audio_adc.h"
#include "anc.h"
#include "aec_user.h"
#include "app_main.h"
#include "user_cfg_id.h"
#include "audio_config.h"
#include "math.h"

#if 0
#define cvp_dut_log	printf
#define cvp_dut_put_float put_float
#else
#define cvp_dut_log(...)
#define cvp_dut_put_float(...)
#endif

#define CVP_DUT_BIG_ENDDIAN 		1		//大端数据输出
#define CVP_DUT_LITTLE_ENDDIAN		2		//小端数据输出
#define CVP_DUT_OUTPUT_WAY			CVP_DUT_LITTLE_ENDDIAN

#if TCFG_AUDIO_CVP_DUT_ENABLE
static cvp_dut_t *cvp_dut_hdl = NULL;
static int cvp_dut_app_online_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size);
int cvp_dut_new_rx_packet(u8 *dat, u8 len);
int cvp_dut_spp_rx_packet(u8 *dat, u8 len);

#if TCFG_AUDIO_TRIPLE_MIC_ENABLE
extern void audio_aec_output_sel(u8 sel, u8 agc);
#endif/*TCFG_AUDIO_TRIPLE_MIC_ENABLE*/

extern void put_float(double fv);
extern void sys_enter_soft_poweroff(void *priv);
extern int audio_aec_toggle_set(u8 toggle);

/*未经过算法处理，主要用于分析ANC MIC的频响*/
int cvp_dut_bypass_mic_ch_sel(u8 mic_num)
{
    if (cvp_dut_hdl) {
        switch (mic_num) {
        case A_MIC0:
            cvp_dut_hdl->mic_ch_sel = AUDIO_ADC_MIC_0;
            break;
        case A_MIC1:
            cvp_dut_hdl->mic_ch_sel = AUDIO_ADC_MIC_1;
            break;
        case A_MIC2:
            cvp_dut_hdl->mic_ch_sel = AUDIO_ADC_MIC_2;
            break;
        case A_MIC3:
            cvp_dut_hdl->mic_ch_sel = AUDIO_ADC_MIC_3;
            break;
        default:
            break;
        }
    }
    return 0;
}

u8 cvp_dut_mic_ch_get(void)
{
    if (cvp_dut_hdl) {
        return cvp_dut_hdl->mic_ch_sel;
    }
    return 0;
}

u8 cvp_dut_mode_get(void)
{
    if (cvp_dut_hdl) {
        return cvp_dut_hdl->mode;
    }
    return 0;
}

void cvp_dut_mode_set(u8 mode)
{
    if (cvp_dut_hdl) {
        cvp_dut_hdl->mode = mode;
    }
}

int cvp_dut_event_deal(u8 *dat)
{
    if (cvp_dut_hdl) {
        switch (dat[3]) {
#if TCFG_AUDIO_DUAL_MIC_ENABLE
//双麦ENC DUT 事件处理
        case CVP_DUT_DMS_MASTER_MIC:
            cvp_dut_log("CMD:CVP_DUT_DMS_MASTER_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(DMS_OUTPUT_SEL_MASTER, 0);
            break;
        case CVP_DUT_DMS_SLAVE_MIC:
            cvp_dut_log("CMD:CVP_DUT_DMS_SLAVE_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(DMS_OUTPUT_SEL_SLAVE, 0);
            break;
        case CVP_DUT_DMS_OPEN_ALGORITHM:
            cvp_dut_log("CMD:CVP_DUT_DMS_OPEN_ALGORITHM\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(DMS_OUTPUT_SEL_DEFAULT, 1);
            break;
#elif TCFG_AUDIO_TRIPLE_MIC_ENABLE
        case CVP_DUT_3MIC_MASTER_MIC:
            cvp_dut_log("CMD:CVP_DUT_3MIC_MASTER_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(1, 0);
            break;
        case CVP_DUT_3MIC_SLAVE_MIC:
            cvp_dut_log("CMD:CVP_DUT_3MIC_SLAVE_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(2, 0);
            break;
        case CVP_DUT_3MIC_FB_MIC:
            cvp_dut_log("CMD:CVP_DUT_3MIC_FB_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(3, 0);
            break;
        case CVP_DUT_3MIC_OPEN_ALGORITHM:
            cvp_dut_log("CMD:CVP_DUT_3MIC_OPEN_ALGORITHM\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(0, 0);
            break;
#else
//单麦SMS/DNS DUT 事件处理
        case CVP_DUT_DMS_MASTER_MIC:
            cvp_dut_log("CMD:SMS/CVP_DUT_DMS_MASTER_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_toggle_set(0);
            break;
        case CVP_DUT_DMS_OPEN_ALGORITHM:
            cvp_dut_log("CMD:SMS/CVP_DUT_DMS_OPEN_ALGORITHM\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_toggle_set(1);
            break;
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/

#if TCFG_AUDIO_ANC_ENABLE
        case CVP_DUT_FF_MIC:	//TWS_FF_MIC测试 或者 头戴式FF_L_MIC测试
            cvp_dut_log("CMD:CVP_DUT_FF_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_BYPASS;
            if (ANC_CH & ANC_L_CH) {
                cvp_dut_bypass_mic_ch_sel(ANCL_FF_MIC);
            } else {
                cvp_dut_bypass_mic_ch_sel(ANCR_FF_MIC);
            }
            break;
        case CVP_DUT_FB_MIC:	//TWS_FB_MIC测试 或者 头戴式FB_L_MIC测试
            cvp_dut_log("CMD:CVP_DUT_FB_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_BYPASS;
            if (ANC_CH & ANC_L_CH) {
                cvp_dut_bypass_mic_ch_sel(ANCL_FB_MIC);
            } else {
                cvp_dut_bypass_mic_ch_sel(ANCR_FB_MIC);
            }
            break;
        case CVP_DUT_HEAD_PHONE_R_FF_MIC:	//头戴式FF_R_MIC测试
            cvp_dut_log("CMD:CVP_DUT_HEAD_PHONE_R_FF_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_BYPASS;
            if (ANC_CH == (ANC_L_CH | ANC_R_CH)) {
                cvp_dut_bypass_mic_ch_sel(ANCR_FF_MIC);
            }
            break;
        case CVP_DUT_HEAD_PHONE_R_FB_MIC:	//头戴式FB_R_MIC测试
            cvp_dut_log("CMD:CVP_DUT_HEAD_PHONE_R_FB_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_BYPASS;
            if (ANC_CH == (ANC_L_CH | ANC_R_CH)) {
                cvp_dut_bypass_mic_ch_sel(ANCR_FB_MIC);
            }
            break;
        case CVP_DUT_MODE_ALGORITHM_SET:				//CVP恢复正常模式
            cvp_dut_log("CMD:CVP_DUT_RESUME\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            break;
#endif/*TCFG_AUDIO_ANC_ENABLE*/

//JL自研算法指令API
#if !TCFG_CVP_DEVELOP_ENABLE
        case CVP_DUT_NS_OPEN:
            cvp_dut_log("CMD:CVP_DUT_NS_OPEN\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_cvp_ioctl(CVP_NS_SWITCH, 1, NULL);	//降噪关
            break;
        case CVP_DUT_NS_CLOSE:
            cvp_dut_log("CMD:CVP_DUT_NS_CLOSE\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_cvp_ioctl(CVP_NS_SWITCH, 0, NULL); //降噪开
            break;
#endif/*TCFG_CVP_DEVELOP_ENABLE*/

        case CVP_DUT_OPEN_ALGORITHM:
            cvp_dut_log("CMD:CVP_DUT_OPEN_ALGORITHM\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_toggle_set(1);
            break;
        case CVP_DUT_CLOSE_ALGORITHM:
            cvp_dut_log("CMD:CVP_DUT_CLOSE_ALGORITHM\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_toggle_set(0);
            break;
        case CVP_DUT_POWEROFF:
            cvp_dut_log("CMD:CVP_DUT_POWEROFF\n");
            sys_enter_soft_poweroff(NULL);
            break;
        default:
            cvp_dut_log("CMD:UNKNOW CMD!!!\n");
            return CVP_DUT_ACK_ERR_UNKNOW;
        }
        return CVP_DUT_ACK_SUCCESS;
    }
    return CVP_DUT_ACK_ERR_UNKNOW;
}

void cvp_dut_init(void)
{
    cvp_dut_log("tx dat");
    if (cvp_dut_hdl == NULL) {
        cvp_dut_hdl = zalloc(sizeof(cvp_dut_t));
    }
    //开机默认设置成算法模式
    cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
    app_online_db_register_handle(DB_PKT_TYPE_DMS, cvp_dut_app_online_parse);
}

void cvp_dut_unit(void)
{
    free(cvp_dut_hdl);
    cvp_dut_hdl = NULL;
}

int cvp_dut_spp_tx_packet(u8 command)
{
    if (cvp_dut_hdl) {
        cvp_dut_hdl->tx_buf.magic = CVP_DUT_SPP_MAGIC;
        cvp_dut_hdl->tx_buf.dat[0] = 0;
        cvp_dut_hdl->tx_buf.dat[1] = 0;
        cvp_dut_hdl->tx_buf.dat[2] = 0;
        cvp_dut_hdl->tx_buf.dat[3] = command;
        cvp_dut_log("tx dat");
#if CVP_DUT_OUTPUT_WAY == CVP_DUT_LITTLE_ENDDIAN	//	小端格式
        cvp_dut_hdl->tx_buf.crc = CRC16((&cvp_dut_hdl->tx_buf.dat), CVP_DUT_PACK_NUM - 4);
        put_buf((u8 *)&cvp_dut_hdl->tx_buf.magic, CVP_DUT_PACK_NUM);
        app_online_db_send(DB_PKT_TYPE_DMS, (u8 *)&cvp_dut_hdl->tx_buf.magic, CVP_DUT_PACK_NUM);
#else
        u16 crc_temp;
        int i;
        u8 dat[CVP_DUT_PACK_NUM];
        u8 dat_temp;
        memcpy(dat, &(cvp_dut_hdl->tx_buf), CVP_DUT_PACK_NUM);
        crc_temp = CRC16(dat + 4, 6);
        printf("crc0x%x,0x%x", crc_temp, cvp_dut_hdl->tx_buf.crc);
        for (i = 0; i < 6; i += 2) {	//小端数据转大端
            dat_temp = dat[i];
            dat[i] = dat[i + 1];
            dat[i + 1] = dat_temp;
        }
        crc_temp = CRC16(dat + 4, CVP_DUT_PACK_NUM - 4);
        dat[2] = crc_temp >> 8;
        dat[3] = crc_temp & 0xff;
        put_buf(dat, CVP_DUT_PACK_NUM);
        app_online_db_send(DB_PKT_TYPE_DMS, dat, CVP_DUT_PACK_NUM);
#endif

    }
    return 0;
}

static int cvp_dut_app_online_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size)
{
    if (cvp_dut_hdl) {
        cvp_dut_hdl->parse_seq  = ext_data[1];
        int ret = cvp_dut_spp_rx_packet(packet, size);
        if (ret) {
            cvp_dut_new_rx_packet(packet, size);
        }
    }
    return 0;
}


int cvp_dut_spp_rx_packet(u8 *dat, u8 len)
{
    if (cvp_dut_hdl) {
        u8 dat_temp;
        u16 crc = 0;
        int ret = 0;
        u8 dat_packet[CVP_DUT_PACK_NUM];
        if (len > CVP_DUT_PACK_NUM) {
            return 1;
        }
        /* cvp_dut_log("rx dat,%d\n", CVP_DUT_PACK_NUM); */
        put_buf(dat, len);
        memcpy(dat_packet, dat, len);
        crc = CRC16(dat + 4, len - 4);
#if CVP_DUT_OUTPUT_WAY == CVP_DUT_BIG_ENDDIAN	//	大端格式
        for (int i = 0; i < 6; i += 2) {		//	大端数据转小端
            dat_temp = dat_packet[i];
            dat_packet[i] = dat_packet[i + 1];
            dat_packet[i + 1] = dat_temp;
        }
#endif
        cvp_dut_log("rx dat_packet");
        memcpy(&(cvp_dut_hdl->rx_buf), dat_packet, 4);
        if (cvp_dut_hdl->rx_buf.magic == CVP_DUT_SPP_MAGIC) {
            cvp_dut_log("crc %x,cvp_dut_hdl->rx_buf.crc %x\n", crc, cvp_dut_hdl->rx_buf.crc);
            if (cvp_dut_hdl->rx_buf.crc == crc || cvp_dut_hdl->rx_buf.crc == 0x1) {
                memcpy(&(cvp_dut_hdl->rx_buf), dat_packet, len);
                ret = cvp_dut_event_deal(cvp_dut_hdl->rx_buf.dat);
                cvp_dut_spp_tx_packet(ret);		//反馈收到命令
                return 0;
            }
        }
    }
    return 1;
}

/***************************************************************************
  							CVP SPP产测 NEW STRUCT
***************************************************************************/

struct cvp_dut_packet_t {
    u8 cmd;		//命令
    u16 len;	//参数长度
    u8 dat[0];	//参数
} __attribute__((packed));

struct cvp_dut_head_t {
    u16 magic;	//标识符
    u16 crc;	//CRC, 从packet开始计算
    struct cvp_dut_packet_t packet;
} __attribute__((packed));

/*
	CVP 命令处理函数
	param: struct cvp_dut_head_t 类型的数据
	return 执行结果
*/
int cvp_dut_new_event_deal(void *packet)
{
    struct cvp_dut_packet_t *hdl = (struct cvp_dut_packet_t *)packet;
    u16 len = hdl->len;
    int wlen = 0;
    int ret = -1;
    u8 last_mode, reset_flag;
    cvp_dut_log("CVP_DUT CMD:0X%x,LEN:%d\n", hdl->cmd, len);
    put_buf((u8 *)packet, hdl->len + sizeof(struct cvp_dut_packet_t));
    if (cvp_dut_hdl) {
        reset_flag = 0;
        last_mode = cvp_dut_hdl->mode;
        switch (hdl->cmd) {
#if TCFG_AUDIO_DUAL_MIC_ENABLE
//双麦ENC DUT 事件处理
        case CVP_DUT_DMS_MASTER_MIC:
            cvp_dut_log("CMD:CVP_DUT_DMS_MASTER_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(DMS_OUTPUT_SEL_MASTER, 0);
            break;
        case CVP_DUT_DMS_SLAVE_MIC:
            cvp_dut_log("CMD:CVP_DUT_DMS_SLAVE_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(DMS_OUTPUT_SEL_SLAVE, 0);
            break;
        case CVP_DUT_DMS_OPEN_ALGORITHM:
            cvp_dut_log("CMD:CVP_DUT_DMS_OPEN_ALGORITHM\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(DMS_OUTPUT_SEL_DEFAULT, 1);
            break;
#elif TCFG_AUDIO_TRIPLE_MIC_ENABLE
        case CVP_DUT_3MIC_MASTER_MIC:
            cvp_dut_log("CMD:CVP_DUT_3MIC_MASTER_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(1, 0);
            break;
        case CVP_DUT_3MIC_SLAVE_MIC:
            cvp_dut_log("CMD:CVP_DUT_3MIC_SLAVE_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(2, 0);
            break;
        case CVP_DUT_3MIC_FB_MIC:
            cvp_dut_log("CMD:CVP_DUT_3MIC_FB_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(3, 0);
            break;
        case CVP_DUT_3MIC_OPEN_ALGORITHM:
            cvp_dut_log("CMD:CVP_DUT_3MIC_OPEN_ALGORITHM\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(0, 0);
            break;
#else
//单麦SMS/DNS DUT 事件处理
        case CVP_DUT_DMS_MASTER_MIC:
            cvp_dut_log("CMD:SMS/CVP_DUT_DMS_MASTER_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_toggle_set(0);
            break;
        case CVP_DUT_DMS_OPEN_ALGORITHM:
            cvp_dut_log("CMD:SMS/CVP_DUT_DMS_OPEN_ALGORITHM\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_toggle_set(1);
            break;
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/

#if TCFG_AUDIO_ANC_ENABLE
        case CVP_DUT_FF_MIC:	//TWS_FF_MIC测试 或者 头戴式FF_L_MIC测试
            cvp_dut_log("CMD:CVP_DUT_FF_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_BYPASS;
            if (ANC_CH & ANC_L_CH) {
                cvp_dut_bypass_mic_ch_sel(ANCL_FF_MIC);
            } else {
                cvp_dut_bypass_mic_ch_sel(ANCR_FF_MIC);
            }
            reset_flag = 1;
            break;
        case CVP_DUT_FB_MIC:	//TWS_FB_MIC测试 或者 头戴式FB_L_MIC测试
            cvp_dut_log("CMD:CVP_DUT_FB_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_BYPASS;
            if (ANC_CH & ANC_L_CH) {
                cvp_dut_bypass_mic_ch_sel(ANCL_FB_MIC);
            } else {
                cvp_dut_bypass_mic_ch_sel(ANCR_FB_MIC);
            }
            reset_flag = 1;
            break;
        case CVP_DUT_HEAD_PHONE_R_FF_MIC:	//头戴式FF_R_MIC测试
            cvp_dut_log("CMD:CVP_DUT_HEAD_PHONE_R_FF_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_BYPASS;
            if (ANC_CH == (ANC_L_CH | ANC_R_CH)) {
                cvp_dut_bypass_mic_ch_sel(ANCR_FF_MIC);
            }
            reset_flag = 1;
            break;
        case CVP_DUT_HEAD_PHONE_R_FB_MIC:	//头戴式FB_R_MIC测试
            cvp_dut_log("CMD:CVP_DUT_HEAD_PHONE_R_FB_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_BYPASS;
            if (ANC_CH == (ANC_L_CH | ANC_R_CH)) {
                cvp_dut_bypass_mic_ch_sel(ANCR_FB_MIC);
            }
            reset_flag = 1;
            break;
        case CVP_DUT_MODE_ALGORITHM_SET:				//CVP恢复正常模式
            cvp_dut_log("CMD:CVP_DUT_RESUME\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            break;
#endif/*TCFG_AUDIO_ANC_ENABLE*/

//JL自研算法指令API
#if !TCFG_CVP_DEVELOP_ENABLE
        case CVP_DUT_NS_OPEN:
            cvp_dut_log("CMD:CVP_DUT_NS_OPEN\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_cvp_ioctl(CVP_NS_SWITCH, 1, NULL);	//降噪关
            break;
        case CVP_DUT_NS_CLOSE:
            cvp_dut_log("CMD:CVP_DUT_NS_CLOSE\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_cvp_ioctl(CVP_NS_SWITCH, 0, NULL); //降噪开
            break;
#endif/*TCFG_CVP_DEVELOP_ENABLE*/

        case CVP_DUT_OPEN_ALGORITHM:
            cvp_dut_log("CMD:CVP_DUT_OPEN_ALGORITHM\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_toggle_set(1);
            break;
        case CVP_DUT_CLOSE_ALGORITHM:
            cvp_dut_log("CMD:CVP_DUT_CLOSE_ALGORITHM\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_toggle_set(0);
            break;
        case CVP_DUT_POWEROFF:
            cvp_dut_log("CMD:CVP_DUT_POWEROFF\n");
            sys_enter_soft_poweroff(NULL);
            break;

            /*enc麦克风校准补偿*/
#if TCFG_AUDIO_MIC_ARRAY_TRIM_ENABLE
        case CVP_ENC_MIC_DIFF_CMP_SET:	//主MIC - 副MIC
            cvp_dut_log("CMD:CVP_ENC_MIC_DIFF_CMP_SET\n");
            float diff;
            memcpy((u8 *)&diff, hdl->dat, len);
            cvp_dut_log("diff:");
            cvp_dut_put_float(diff);
            float dms_mic_diff = log10(app_var.enc_degradation) * 20;
            app_var.audio_mic_array_diff_cmp = pow(10, (diff + dms_mic_diff) / 20);
            wlen = syscfg_write(CFG_MIC_ARRAY_DIFF_CMP_VALUE, (u8 *)(&app_var.audio_mic_array_diff_cmp), sizeof(app_var.audio_mic_array_diff_cmp));
            if (wlen != sizeof(app_var.audio_mic_array_diff_cmp)) {
                printf("mic array diff cap vm write fail !!!");
                ret = CVP_DUT_ACK_VM_WRITE_ERR;
            }
            cvp_dut_log("mic_array_diff_cmp:");
            cvp_dut_put_float(app_var.audio_mic_array_diff_cmp);
            break;
        case CVP_ENC_MIC_DIFF_CMP_EN:
            cvp_dut_log("CMD:CVP_ENC_MIC_DIFF_CMP_EN\n");
            u8 en;
            memcpy(&en, hdl->dat, len);
            app_var.audio_mic_array_trim_en = en;
            wlen = syscfg_write(CFG_MIC_ARRAY_TRIM_EN, (u8 *)(&app_var.audio_mic_array_trim_en), sizeof(app_var.audio_mic_array_trim_en));
            if (wlen != sizeof(app_var.audio_mic_array_trim_en)) {
                cvp_dut_log("mic array trim en vm write fail !!!");
                ret = CVP_DUT_ACK_VM_WRITE_ERR;
            }
            cvp_dut_log("mic array trim en: %d\n", app_var.audio_mic_array_trim_en);
            break;
        case CVP_ENC_MIC_DIFF_CMP_CLEAN:
            cvp_dut_log("CMD:CVP_ENC_MIC_DIFF_CMP_CLEAN\n");
            app_var.audio_mic_array_diff_cmp = 1.0f;
            app_var.audio_mic_cmp.talk = 1.0f;
            app_var.audio_mic_cmp.ff = 1.0f;
            app_var.audio_mic_cmp.fb = 1.0f;

            wlen = syscfg_write(CFG_MIC_ARRAY_DIFF_CMP_VALUE, (u8 *)(&app_var.audio_mic_array_diff_cmp), sizeof(app_var.audio_mic_array_diff_cmp));
            if (wlen != sizeof(app_var.audio_mic_array_diff_cmp)) {
                cvp_dut_log("mic array diff cap vm write fail !!!");
                ret = CVP_DUT_ACK_VM_WRITE_ERR;
            }
            wlen = syscfg_write(CFG_MIC_TARGET_DIFF_CMP, (u8 *)(&app_var.audio_mic_cmp), sizeof(audio_mic_cmp_t));
            if (wlen != sizeof(audio_mic_cmp_t)) {
                printf("mic array diff cap vm write fail !!!");
                ret = CVP_DUT_ACK_VM_WRITE_ERR;
            }
            cvp_dut_log("CMD:CVP_ENC_MIC_DIFF_CMP_CLEAN\n");
            break;
        case CVP_ENC_TARGET_MIC_DIFF_SET:
            cvp_dut_log("CMD:CVP_ENC_TARGET_MIC_DIFF_SET\n");
            float talk_diff, ff_diff, fb_diff;
            memcpy((u8 *)&talk_diff, hdl->dat, 4);
            memcpy((u8 *)&ff_diff, hdl->dat + 4, 4);
            memcpy((u8 *)&fb_diff, hdl->dat + 8, 4);
            cvp_dut_log("talk_diff:");
            cvp_dut_put_float(talk_diff);
            cvp_dut_log("ff_diff:");
            cvp_dut_put_float(ff_diff);
            cvp_dut_log("fb_diff:");
            cvp_dut_put_float(fb_diff);
            app_var.audio_mic_cmp.talk = pow(10, (talk_diff) / 20);
            app_var.audio_mic_cmp.ff = pow(10, (ff_diff) / 20);
            app_var.audio_mic_cmp.fb = pow(10, (fb_diff) / 20);
            wlen = syscfg_write(CFG_MIC_TARGET_DIFF_CMP, (u8 *)(&app_var.audio_mic_cmp), sizeof(audio_mic_cmp_t));
            if (wlen != sizeof(audio_mic_cmp_t)) {
                printf("mic array diff cap vm write fail !!!");
                ret = CVP_DUT_ACK_VM_WRITE_ERR;
            }
            break;
#endif /*TCFG_AUDIO_MIC_ARRAY_TRIM_ENABLE*/

        /*通话算法模块开关*/
        case CVP_DUT_CVP_IOCTL:
            cvp_dut_log("CMD:CVP_DUT_CVP_IOCTL\n");
            u8 ioctl_cmd = hdl->dat[0];
            u8 toggle = hdl->dat[1];
            cvp_dut_log(" io ctl cmd %d, toggle %d", ioctl_cmd, toggle);
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_cvp_ioctl(ioctl_cmd, toggle, NULL);
            break;
        case CVP_DUT_DEC_DUT_SET:
            cvp_dut_log("CVP_DUT_DEC_DUT_SET: %d\n", hdl->dat[0]);
            cvp_dut_hdl->dec_dut_enable = hdl->dat[0];
            a2dp_dec_dut_enable(cvp_dut_hdl->dec_dut_enable);
            break;
        default:
            cvp_dut_log("CMD:UNKNOW CMD!!!\n");
            ret = CVP_DUT_ACK_ERR_UNKNOW;
        }
        if (last_mode != cvp_dut_hdl->mode) {
            reset_flag = 1;	//模式被修改，则复位通话
        }
        if (reset_flag) {
            if (bt_phone_dec_is_running()) {
                esco_enc_reset();
            }
        }
        ret = (ret == -1) ? CVP_DUT_ACK_SUCCESS : ret;
    }
    return ret;
}

//len = 参数数据长度(不包括cmd)
int cvp_dut_new_tx_packet(u8 cmd, u8 *data, int len)
{
    if (cvp_dut_hdl) {
        int packet_len = sizeof(struct cvp_dut_head_t) + len;
        struct cvp_dut_head_t *head = (struct cvp_dut_head_t *)malloc(packet_len);
        head->magic = CVP_DUT_NEW_SPP_MAGIC;
        head->packet.cmd = cmd;
        head->packet.len = len;
        head->crc = CRC16(&head->packet, packet_len - 4);
        if (len) {
            memcpy(head->packet.dat, data, len);
        }
        cvp_dut_log("tx new_dat_packet %d\n", packet_len);
        //put_buf((u8 *)head, packet_len);
        app_online_db_send(DB_PKT_TYPE_DMS, (u8 *)head, packet_len);
        free(head);
    }
    return 0;
}

int cvp_dut_new_rx_packet(u8 *dat, u8 len)
{
    u16 crc = 0;
    int ret = 0;
    struct cvp_dut_head_t *head = (struct cvp_dut_head_t *)dat;
    if (cvp_dut_hdl) {
        cvp_dut_log("rx new_dat_packet");
        //put_buf(dat, len);
        crc = CRC16(&head->packet, len - 4);
        if (head->magic == CVP_DUT_NEW_SPP_MAGIC) {
            if (head->crc == crc || head->crc == 0x1) {
                ret = cvp_dut_new_event_deal(&head->packet);
                cvp_dut_new_tx_packet(ret, NULL, 0);		//反馈收到命令
                if (ret == CVP_DUT_ACK_SUCCESS) {
                    return 0;
                }
            } else {
                cvp_dut_log("ERR head->crc %x != %x\n", head->crc, crc);
                cvp_dut_new_tx_packet(CVP_DUT_ACK_CRC_ERR, NULL, 0);		//反馈收到命令
            }
        }
    }
    return 1;
}

/*
   DEC产测状态获取
	param: music_decide 1 需判断播歌状态； 0 不需判断
*/
u8 audio_dec_dut_en_get(u8 music_decide)
{
    if (!cvp_dut_hdl) {
        return 0;
    }
    if (music_decide && !bt_media_is_running()) {
        //当前非播歌状态
        return 0;
    }
    return cvp_dut_hdl->dec_dut_enable;
}

#endif /*TCFG_AUDIO_CVP_DUT_ENABLE*/

