#include "jl_kws_common.h"

#if TCFG_KWS_VOICE_RECOGNITION_ENABLE

//===================================================//
//                    算法接口函数                   //
//===================================================//
#define JL_KWS_PARAM_FILE 	SDFILE_RES_ROOT_PATH"jl_kws.cfg"
#define KWS_FRAME_LEN 		320
void *JL_kws_init(void *ptr, void *fp1, int online_cmvn, float confidence_yes, float confidence_no, int smooth_yes, int smooth_no);
extern int jl_kws_get_need_buf_len(void);
//每帧固定320 bytes, 16k采样率, 10ms
extern int jl_detect_kws(char *data, int len, void *jl_kws1);
extern void JL_kws_free(void *_jl_det_kws);

#define ALIGN(a, b) \
	({ \
	 	int m = (u32)(a) & ((b)-1); \
		int rets = (u32)(a) + (m?((b)-m):0);	 \
		rets;\
	})


//===================================================//
//             IO 口DEBUG(测量算法时间)              //
//===================================================//
#define     KWS_IO_DEBUG_0(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}
#define     KWS_IO_DEBUG_1(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}

/*
 *Yes/No词条识别阈值:
 *(1)越大越难识别，相应的，词条识别匹配度就越高，即越不容易误触发
 *(2)越小越容易识别，相应的，词条识别匹配度就越低，即越容易误触发
 */
#define KWS_YES_THR		0.6f
#define KWS_NO_THR		0.6f

struct jl_kws_algo {
    void *kws_handle;
    void *kws_algo_buf;
    u32 *frame_buf;
    void *param_ptr;
};

static struct jl_kws_algo __kws_algo = {0};

static void kws_algo_local_init(void *buf, void *param_ptr)
{
    //===============================//
    //       	 算法初始化          //
    //===============================//
    void *buf_align = (void *)ALIGN(buf, 4);
    kws_info("file ptr: 0x%x, buf_align: 0x%x", (u32)param_ptr, (u32)buf_align);
    memset(buf, 0, jl_kws_get_need_buf_len());
    int cmn_online = 1;
    //int online_cmvn = 1;
    int smooth_yes = 25;
    int smooth_no = 30;
    float confidence_yes = KWS_YES_THR;
    float confidence_no = KWS_NO_THR;
    __kws_algo.kws_handle = JL_kws_init(buf_align, param_ptr, cmn_online, confidence_yes, confidence_no, smooth_yes, smooth_no);
}

int jl_kws_algo_init(void)
{
    FILE *fp = NULL;
    struct vfs_attr param_attr = {0};
    u32 param_len = 0;
    u8 *param_ptr = NULL;
    u32 need_buf_len = 0 ;
    void *buf_ptr = NULL;
    int ret = 0;

    //===============================//
    //          打开参数文件         //
    //===============================//
    fp = fopen(JL_KWS_PARAM_FILE, "r");
    if (!fp) {
        return -ENOENT;
    }

    fget_attrs(fp, &param_attr);
    param_len = param_attr.fsize;
    __kws_algo.param_ptr = (void *)param_attr.sclust; //cpu access addr

    fclose(fp);
    //===============================//
    //       申请算法需要的buf       //
    //===============================//
    need_buf_len = jl_kws_get_need_buf_len();
    buf_ptr = zalloc(need_buf_len);
    kws_debug("need_buf_len = 0x%x", need_buf_len);
    if (!buf_ptr) {
        return JL_KWS_ERR_ALGO_NO_BUF;
    }

    __kws_algo.kws_algo_buf = buf_ptr;

    buf_ptr = zalloc(KWS_FRAME_LEN);
    kws_debug("KWS_FRAME_LEN = 0x%x", KWS_FRAME_LEN);
    if (!buf_ptr) {
        free(__kws_algo.kws_algo_buf);
        __kws_algo.kws_algo_buf = NULL;
        return JL_KWS_ERR_ALGO_NO_BUF;
    }
    __kws_algo.frame_buf = buf_ptr;

    kws_algo_local_init(__kws_algo.kws_algo_buf, __kws_algo.param_ptr);

    return JL_KWS_ERR_NONE;
}


int jl_kws_algo_detect_run(u8 *buf, u32 len)
{
    int ret = 0;
    /* local_irq_disable(); */
    /* KWS_IO_DEBUG_1(A,4); */
    ret = jl_detect_kws((char *)buf, len, __kws_algo.kws_handle);
    /* KWS_IO_DEBUG_0(A,4); */
    /* local_irq_enable(); */

    if ((ret != KWS_VOICE_EVENT_YES) &&
        (ret != KWS_VOICE_EVENT_NO)) {
        ret = KWS_VOICE_EVENT_NONE;
    }

    return ret;
}

void *jl_kws_algo_get_frame_buf(u32 *buf_len)
{
    if (__kws_algo.frame_buf) {
        *buf_len = KWS_FRAME_LEN;
        return __kws_algo.frame_buf;
    }

    return NULL;
}

int jl_kws_algo_start(void)
{
    return JL_KWS_ERR_NONE;
}

void jl_kws_algo_stop(void)
{
    return;
}


void jl_kws_algo_close(void)
{
    kws_info("%s", __func__);

    if (__kws_algo.kws_handle) {
        JL_kws_free(__kws_algo.kws_handle);
        __kws_algo.kws_handle = NULL;
    }

    if (__kws_algo.kws_algo_buf) {
        free(__kws_algo.kws_algo_buf);
        __kws_algo.kws_algo_buf = NULL;
    }

    if (__kws_algo.frame_buf) {
        free(__kws_algo.frame_buf);
        __kws_algo.frame_buf = NULL;
    }
}

#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */
