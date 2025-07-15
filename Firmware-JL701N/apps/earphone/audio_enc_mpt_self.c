
/***********************************************************

 *			Audio Enc Mass production test self
 * File  : audio_enc_mpt_self.c
 * note  : 自研ENC产测 声学频响测试

************************************************************/

#include "hw_fft.h"
#include "app_config.h"
#include "audio_enc_mpt_self.h"
#include "generic/list.h"
#include "system/task.h"

#if 0
#define encmpt_log(x, ...)  printf("[enc_mpt]" x " ", ## __VA_ARGS__)
#else
#define encmpt_log(...)
#endif/*log_en*/

#if AUDIO_ENC_MPT_SELF_ENABLE

void anc_HpEst_psd(s16 *input0, float *out0_sum, int len, int *cnt);
void anc_HpEst_psd_out(float *out0_sum, int flen, int cnt);

//输出数据类型
typedef float ENC_OUT_TYPE;
/* typedef short ENC_OUT_TYPE; */

typedef struct {
    struct list_head entry;
    u16 id;					//对应数据通道id
    u16 len;				//长度
    int cnt;				//计算次数
    s16 *in;				//输入地址
    ENC_OUT_TYPE *out;		//输出地址
} enc_freres_bulk;

typedef struct {
    u16 version;
    u16 cnt;
    u8 dat[0];	//小心double访问非对齐异常
} enc_freres_file_t;

typedef struct {
    u32 id;
    u32 offset;
    u32 len;
} enc_freres_file_head_t;

typedef struct {
    volatile u8 state;				//状态
    u8 run_busy;
    u8 inbuf_busy;
    u16 ch;					//目标通道
    /* enc_freres_bulk bulk[5];//链表，最长支持5段 */
    enc_freres_bulk *bulk;//链表，动态申请不定长
    u8 *file_data;			//输出文件
    int file_len;			//输出文件长度
    int in_packet_len;
    struct list_head head;
} audio_fre_respone_t;

static audio_fre_respone_t *hdl;

#if 0	//输入数据连续性验证
u8 *data1;
u8 *data2;
u8 *data3;
#define ENC_CHECK_LEN 8192
static u8 ainit = 0;
static u16 offset1 = 0;
static u16 offset2 = 0;
static u16 offset3 = 0;
void audio_enc_mpt_inbuf_check(u16 id, s16 *buf, int len)
{
    if (!ainit) {
        ainit = 1;
        data1 = zalloc(ENC_CHECK_LEN);
        data2 = zalloc(ENC_CHECK_LEN);
        data3 = zalloc(ENC_CHECK_LEN);
    }
    if (id & AUDIO_ENC_MPT_FF_MIC) {
        if ((offset1 + len) > ENC_CHECK_LEN) {
            offset1 = 0;
        }
        memcpy(data1 + offset1, (u8 *)buf, len);
        offset1 += len;
    }
    if (id & AUDIO_ENC_MPT_TALK_MIC) {
        if ((offset2 + len) > ENC_CHECK_LEN) {
            offset2 = 0;
        }
        memcpy(data2 + offset2, (u8 *)buf, len);
        offset2 += len;
    }
    if (id & AUDIO_ENC_MPT_CVP_OUT) {
        if ((offset3 + len) > ENC_CHECK_LEN) {
            offset3 = 0;
        }
        memcpy(data3 + offset3, (u8 *)buf, len);
        offset3 += len;
    }
}

void audio_enc_mpt_inbuf_printf(void)
{
    int i = 0;
    s16 *dat1 = (s16 *)data1;
    s16 *dat2 = (s16 *)data2;
    s16 *dat3 = (s16 *)data3;
    encmpt_log("ff");
    for (i = 0; i < ENC_CHECK_LEN / 2; i++) {
        printf("%d\n", dat1[i]);
    }

    encmpt_log("TALK");
    for (i = 0; i < ENC_CHECK_LEN / 2; i++) {
        printf("%d\n", dat2[i]);
    }

    encmpt_log("cvp");
    for (i = 0; i < ENC_CHECK_LEN / 2; i++) {
        printf("%d\n", dat3[i]);
    }
}
#endif/*默认关闭*/

//id = 0 计算链表所有满足长度的节点;
static void audio_enc_mpt_fre_response_process(u16 id)
{
    /* return; */
    enc_freres_bulk *bulk;
    if (hdl->state != ENC_FRE_RES_STATE_RUN) {
        return;
    }

    hdl->run_busy = 1;
    list_for_each_entry(bulk, &hdl->head, entry) {
        //原始数据指针、目标输出指针、计算长度
        if ((!id) || id == bulk->id) {
            if (bulk->len >= hdl->in_packet_len) {
                anc_HpEst_psd(bulk->in, bulk->out, AUDIO_ENC_MPT_FRERES_POINT, &bulk->cnt);
                bulk->len -= (hdl->in_packet_len >> 1);
                /* encmpt_log("process id 0X%x len %d\n", bulk->id, bulk->len); */
                if (bulk->len) {	//overlap
                    memcpy(bulk->in, bulk->in + (hdl->in_packet_len >> 2), bulk->len);
                }
            }
        }
    }
    hdl->run_busy = 0;
}

static void audio_enc_mpt_fre_resonpse_task(void *p)
{
    int res;
    int msg[16];
    u32 pend_timeout = portMAX_DELAY;
    encmpt_log(">>>audio_fre_res_task<<<\n");
    while (1) {
        res = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (res == OS_TASKQ) {
            switch (msg[1]) {
            case ENC_FRE_RESPONE_MSG_RUN:
                audio_enc_mpt_fre_response_process((u16)msg[2]);
                break;
            }
        } else {
            encmpt_log("res:%d,%d", res, msg[1]);
        }
    }

}

//初始化链表以及的通道内容
static void audio_enc_mpt_fre_response_bulk_init(enc_freres_file_t *file, u8 cnt, u16 id)
{
    u8 *data_home;
    enc_freres_file_head_t *file_head = file->dat;

    /* if (cnt > 4) { */
    /* encmpt_log("ERR:Audio enc Fre bulk full!!!\n"); */
    /* return; */
    /* } */

    data_home = file->dat + (file->cnt * 12);

    //file初始化head
    file_head[cnt].id = id;
    //len(byte)
    file_head[cnt].len = AUDIO_ENC_MPT_FRERES_POINT / 2 * sizeof(ENC_OUT_TYPE);
    if (cnt) {
        file_head[cnt].offset = file_head[cnt - 1].len + file_head[cnt - 1].offset;
    } else {
        file_head[cnt].offset = 0;
    }

    hdl->bulk[cnt].id = id;
    hdl->bulk[cnt].out = (ENC_OUT_TYPE *)(data_home + file_head[cnt].offset);
    hdl->bulk[cnt].in = (s16 *)malloc(AUDIO_ENC_MPT_FRERES_POINT * sizeof(short));
    encmpt_log("hdl %x file %x, out %x, data_home %x, inbuf_len %d\n", (int)hdl->file_data, (int)file, \
               (int)hdl->bulk[cnt].out, (int)data_home, AUDIO_ENC_MPT_FRERES_POINT * sizeof(short));
    //len(byte)
    hdl->bulk[cnt].len = 0;
    /* hdl->bulk[cnt].len = AUDIO_ENC_MPT_FRERES_POINT * sizeof(short); */
    list_add_tail(&hdl->bulk[cnt].entry, &hdl->head);
}

//文件内存申请，结构初始化
static enc_freres_file_t *audio_enc_mpt_fre_response_file_init(int cnt)
{
    enc_freres_file_t *file;
    //4byte(vesion + cnt) + cnt * (12byte (id + offset + len) + outdata)
    //以目标file文件组成结构申请空间
    //data => 复数取平方和, 因此POINT/2
    hdl->file_len = 4 + cnt * (12 + AUDIO_ENC_MPT_FRERES_POINT / 2 * sizeof(ENC_OUT_TYPE));

#if 0
    //评估内存是否满足当前需求
    extern size_t xPortGetPhysiceMemorySize(void);
    int cur_mem_size = xPortGetPhysiceMemorySize();
    if (cur_mem_size < hdl->file_len + 2000) {	//至少余量多2000byte 才可使用该功能
        printf("ERR!!!MIC_FFT debug_buf:%lu + 2000,free_mem:%d\n", hdl->file_len, cur_mem_size);
        return NULL;
    }
#endif
    encmpt_log("file init len %d\n", hdl->file_len);
    hdl->file_data = zalloc(hdl->file_len);
    file = (enc_freres_file_t *) hdl->file_data;
    file->cnt = cnt;
    return file;
}

//音频测试频响计算启动, ch 对应目标的通道
void audio_enc_mpt_fre_response_start(u16 ch)
{
    enc_freres_file_t *file;
    int cnt = 0;
    int shift_det = 0;
    u16 det_ch = ch;
    int det_cnt = 0;
    encmpt_log("%s, ch 0x%x\n", __func__, ch);
    if (!ch) {
        return;
    }
    if (hdl) {
        //支持重入
        hdl->state = ENC_FRE_RES_STATE_START;
        audio_enc_mpt_fre_response_release();
        encmpt_log("enc_fre start again\n");
    }
#if AUDIO_ENC_MPT_FRERES_ASYNC
    task_create(audio_enc_mpt_fre_resonpse_task, NULL, "enc_mpt_self");
#endif/*AUDIO_ENC_MPT_FRERES_ASYNC*/

    hdl = zalloc(sizeof(audio_fre_respone_t));
    hdl->ch = ch;
    hdl->in_packet_len = AUDIO_ENC_MPT_FRERES_POINT * sizeof(short);

    while (det_ch) {
        if (det_ch & BIT(0)) {
            det_cnt++;
        }
        det_ch >>= 1;
    }
    file = audio_enc_mpt_fre_response_file_init(det_cnt);
    if (!file) {
        free(hdl);
        return;
    }

    //链表初始化
    INIT_LIST_HEAD(&hdl->head);

    //申请链表空间
    hdl->bulk = zalloc(sizeof(enc_freres_bulk) * det_cnt);
    encmpt_log("det_cnt %d, list bulk size %d\n", det_cnt, sizeof(enc_freres_bulk) * det_cnt);

    while (ch) {
        if (ch & BIT(0)) {
            audio_enc_mpt_fre_response_bulk_init(file, cnt, BIT(shift_det));
            encmpt_log("list init %x\n", BIT(shift_det));
            cnt++;
        }
        shift_det++;
        ch >>= 1;
    }
    hdl->state = ENC_FRE_RES_STATE_RUN;
}


//更新目标通道输入buf、len
void audio_enc_mpt_fre_response_inbuf(u16 id, s16 *buf, int len)
{
    //len(byte)
    if (!hdl) {
        goto __err1;
        /* return; */
    }
    if (hdl->state != ENC_FRE_RES_STATE_RUN) {
        goto __err1;
        /* return; */
    }
    enc_freres_bulk *bulk;
    hdl->inbuf_busy = 1;
    list_for_each_entry(bulk, &hdl->head, entry) {
        if (id & bulk->id) {
            /* bulk->id = id; */
            if ((bulk->len + len) <= hdl->in_packet_len) {
                /* encmpt_log("inbuf id 0X%x, len %d, %d\n", id, bulk->len, len); */
                memcpy(((u8 *)bulk->in) + bulk->len, (u8 *)buf, len);
                bulk->len += len;
            } else if (bulk->len < hdl->in_packet_len) {
                encmpt_log("id 0X%x, inbuf soon be full\n", id);
                memcpy(bulk->in + (bulk->len >> 1), buf, hdl->in_packet_len - bulk->len);
                bulk->len = hdl->in_packet_len;
            } else {
                encmpt_log("id 0X%x, inbuf full\n", id);
            }
#if AUDIO_ENC_MPT_FRERES_ASYNC
            if (bulk->len >= hdl->in_packet_len) {
                audio_enc_mpt_fre_response_post_run(bulk->id);
            }
#endif/*AUDIO_ENC_MPT_FRERES_ASYNC*/

        }
    }
    hdl->inbuf_busy = 0;

    return;
__err1:
    /* encmpt_log("inbuf err\n"); */
    int i = 0;
    while (id) {
        if (id & BIT(0)) {
            putchar('0' + i);
        }
        id >>= 1;
        i++;
    }
}

//音频测试频响计算运行
void audio_enc_mpt_fre_response_post_run(u16 id)
{
    if (!hdl) {
        return;
    }
    if (hdl->state != ENC_FRE_RES_STATE_RUN) {
        return;
    }
#if AUDIO_ENC_MPT_FRERES_ASYNC
    os_taskq_post_msg("enc_mpt_self", 2, ENC_FRE_RESPONE_MSG_RUN, id);
#else
    audio_enc_mpt_fre_response_process(0);
#endif/*AUDIO_ENC_MPT_FRERES_ASYNC*/
}

//音频测试频响计算停止
void audio_enc_mpt_fre_response_stop(void)
{
    if (!hdl) {
        return;
    }
    encmpt_log("%s \n", __func__);
    enc_freres_bulk *bulk;
    hdl->state = ENC_FRE_RES_STATE_STOP;
    while (hdl->inbuf_busy || hdl->run_busy) {
        os_time_dly(1);
    };
    list_for_each_entry(bulk, &hdl->head, entry) {
        if (bulk->cnt) {
            //数据只有实部，因此 point >> 1
            anc_HpEst_psd_out(bulk->out, AUDIO_ENC_MPT_FRERES_POINT >> 1, bulk->cnt);
            /* encmpt_log("cnt %d \n", bulk->cnt); */
        }
    }
    encmpt_log("%s ok\n", __func__);
}

//工具获取数据文件
int audio_enc_mpt_fre_response_file_get(u8 **buf)
{
    if (!hdl) {
        return 0;
    }
    encmpt_log("%s, len %d\n", __func__, hdl->file_len);
    *buf = hdl->file_data;
    /* audio_enc_mpt_inbuf_printf(); */
    /* put_buf(hdl->file_data + 40, hdl->file_len - 40); */
#if 0//debug
    enc_freres_bulk *bulk;
    int point = AUDIO_ENC_MPT_FRERES_POINT / 2;
    list_for_each_entry(bulk, &hdl->head, entry) {
        encmpt_log("bulk id 0X%x\n", bulk->id);
        for (int i = 0; i < point; i++) {
            printf("%d\n", (int)(bulk->out[i] * 10000));
        }
    }
#endif
    return hdl->file_len;
}

//数据获取结束，释放内存
void audio_enc_mpt_fre_response_release(void)
{
    if (!hdl) {
        return;
    }
    //mic_close
    encmpt_log("%s\n", __func__);
    while (hdl->inbuf_busy || hdl->run_busy) {
        os_time_dly(1);
    };
#if AUDIO_ENC_MPT_FRERES_ASYNC
    task_kill("enc_mpt_self");
#endif/*AUDIO_ENC_MPT_FRERES_ASYNC*/
    enc_freres_bulk *bulk;
    list_for_each_entry(bulk, &hdl->head, entry) {
        free(bulk->in);
    }
    free(hdl->file_data);
    free(hdl->bulk);
    free(hdl);
    hdl = NULL;
    encmpt_log("%s ok\n", __func__);
}




#endif/*AUDIO_ENC_MPT_SELF_ENABLE*/
