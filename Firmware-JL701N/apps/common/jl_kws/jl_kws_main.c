#include "jl_kws_common.h"
#include "jl_kws_api.h"

#if TCFG_KWS_VOICE_RECOGNITION_ENABLE

//==========================================================//
// 					  KWS 语音识别                          //
//==========================================================//
struct kws_speech_recognition {
    u8 task_init;
    u8 kws_state;
    u8 kws_task_state;
};

static struct kws_speech_recognition jl_kws = {0};
//==============================================//
//              KWS CPU频率配置                 //
//==============================================//
#define KWS_BT_CALL_SYS_FREQUENCE_HZ 		(48 * 1000000)

#define __this 			(&jl_kws)

enum KWS_TASK_MSG {
    KWS_SPEECH_RECOGNITION_RUN = 1,
    KWS_SPEECH_RECOGNITION_STOP,
    KWS_SPEECH_RECOGNITION_CLOSE,
};

enum KWS_STATE {
    KWS_STATE_IDLE = 0,
    KWS_STATE_INIT,
    KWS_STATE_RUN,
    KWS_STATE_STOP,
    KWS_STATE_CLOSE,
};

enum KWS_TASK_STATE {
    KWS_TASK_STATE_IDLE = 0,
    KWS_TASK_STATE_INIT,
    KWS_TASK_STATE_RUN,
    KWS_TASK_STATE_STOP,
    KWS_TASK_STATE_CLOSE,
};

//=======================================================//
//                  jl_kws_main                          //
//=======================================================//
//=========== 线程名称
#define THIS_TASK_NAME 		"kws"

static int kws_speech_recognition_run(void)
{
    void *rbuf = 0;
    u32 rbuf_len = 0;
    u32 audio_data_len = 0;
    int ret = JL_KWS_ERR_NONE;
    int event = KWS_VOICE_EVENT_NONE;

    kws_info("%s", __func__);

    ret = jl_kws_audio_start();
    if (ret != JL_KWS_ERR_NONE) {
        return ret;
    }

    ret = jl_kws_algo_start();
    if (ret != JL_KWS_ERR_NONE) {
        return ret;
    }

    __this->kws_task_state = KWS_TASK_STATE_RUN;

    while (1) {
        if (__this->kws_state != KWS_STATE_RUN) {
            break;
        }
        rbuf = jl_kws_algo_get_frame_buf(&rbuf_len);

        if (rbuf == NULL) {
            ret = JL_KWS_ERR_ALGO_NO_FRAME_BUF;
            break;
        }

        audio_data_len = jl_kws_audio_get_data(rbuf, rbuf_len);

        if (audio_data_len == rbuf_len) {
            /* kws_putchar('r'); */
            event = jl_kws_algo_detect_run(rbuf, rbuf_len);
            if (event != KWS_VOICE_EVENT_NONE) {
                jl_kws_event_state_update(event);
            }
        }
    }

    return ret;
}


static int jl_kws_speech_recognition_init(void)
{
    kws_info("%s", __func__);
    //1.算法初始化
    int ret = 0;
    ret = jl_kws_algo_init();
    if (ret != JL_KWS_ERR_NONE) {
        return ret;
    }

    //2.Audio MIC初始化
    ret = jl_kws_audio_init();
    if (ret != JL_KWS_ERR_NONE) {
        return ret;
    }

    //3. event初始化
    ret = jl_kws_event_init();
    if (ret != JL_KWS_ERR_NONE) {
        return ret;
    }

    return JL_KWS_ERR_NONE;
}

static int kws_speech_recognition_stop(void)
{
    kws_info("%s", __func__);

    if (__this->task_init) {

        jl_kws_algo_stop();

        jl_kws_audio_stop();

        jl_kws_event_stop();

        __this->kws_task_state = KWS_TASK_STATE_STOP;
    }

    return JL_KWS_ERR_NONE;
}


static void kws_speech_recognition_close(void)
{
    kws_info("%s", __func__);

    if (__this->task_init) {

        jl_kws_algo_close();

        jl_kws_audio_close();

        jl_kws_event_close();

        __this->kws_task_state = KWS_TASK_STATE_CLOSE;
        //task_kill(THIS_TASK_NAME);
    }

    return;
}


static void kws_speech_recognition_task(void *param)
{
    int msg[16];
    int ret = 0;

    kws_info("%s", __func__);

    if (param) {
        os_sem_post((OS_SEM *)(param)); //wait task ready sem
    }

    ret = jl_kws_speech_recognition_init();
    kws_debug("ret = %d", ret);
    if (ret != JL_KWS_ERR_NONE) {
        kws_speech_recognition_close();
        __this->task_init = 0;
        task_kill(THIS_TASK_NAME);
    }

    __this->kws_state = KWS_STATE_INIT;
    __this->kws_task_state = KWS_TASK_STATE_INIT;

    while (1) {
        ret = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (ret == OS_TASKQ) {
            switch (msg[1]) {
            case KWS_SPEECH_RECOGNITION_RUN:
                kws_speech_recognition_run();
                break;
            case KWS_SPEECH_RECOGNITION_STOP:
                kws_speech_recognition_stop();
                break;
            case KWS_SPEECH_RECOGNITION_CLOSE:
                kws_speech_recognition_close();
                os_sem_post((OS_SEM *)msg[2]);
                break;
            default:
                break;
            }
        }
    }
}

static u8 kws_idle_query(void)
{
    return !(__this->kws_task_state == KWS_TASK_STATE_RUN);
}

REGISTER_LP_TARGET(kws_lp_target) = {
    .name = "kws",
    .is_idle = kws_idle_query,
};

//==========================================================//
// 				   JL_KWS MAIN API                          //
//==========================================================//
int jl_kws_speech_recognition_open(void)
{
    kws_info("%s", __func__);
    OS_SEM ready_sem;

    if (__this->task_init == 0) {
        __this->task_init = 1;
        os_sem_create(&ready_sem, 0);
        task_create(kws_speech_recognition_task, (void *)&ready_sem, THIS_TASK_NAME);
        //wait task ready
        os_sem_pend(&ready_sem, 20);
    }

    return 0;
}

int jl_kws_speech_recognition_start(void)
{
    int ret;
    kws_info("%s", __func__);
    if (__this->task_init) {
        if (__this->kws_state == KWS_STATE_RUN) {
            return 0;
        }
        __this->kws_state = KWS_STATE_RUN;
        ret = os_taskq_post_msg(THIS_TASK_NAME, 1, KWS_SPEECH_RECOGNITION_RUN);
    }

    return 0;
}

void jl_kws_speech_recognition_stop(void)
{
    int ret;
    kws_info("%s", __func__);
    if (__this->task_init) {
        if (__this->kws_state == KWS_STATE_STOP) {
            return;
        }
        __this->kws_state = KWS_STATE_STOP;
        ret = os_taskq_post_msg(THIS_TASK_NAME, 1, KWS_SPEECH_RECOGNITION_STOP);
    }
}

void jl_kws_speech_recognition_close(void)
{
    kws_info("%s", __func__);
    OS_SEM del_sem;
    if (__this->task_init) {
        if (__this->kws_state == KWS_STATE_CLOSE) {
            return;
        }
        __this->kws_state = KWS_STATE_CLOSE;
        os_sem_create(&del_sem, 0);
        os_taskq_post_msg(THIS_TASK_NAME, 2, KWS_SPEECH_RECOGNITION_CLOSE, (void *)&del_sem);
        os_sem_pend(&del_sem, 0xffff);
        task_kill(THIS_TASK_NAME);
        __this->task_init = 0;
    }
}


//==========================================================//
// 					   测试代码                             //
//==========================================================//
void jl_kws_main_user_demo(void)
{
    //1.打开jl_kws模块
    jl_kws_speech_recognition_open();

    //2.在某时刻开始识别
    os_time_dly(1000);
    jl_kws_speech_recognition_start();

    //3.在某时刻之后停止识别
    os_time_dly(1500);
    jl_kws_speech_recognition_stop();

    //4.在某时刻之后重新开启识别
    os_time_dly(2000);
    jl_kws_speech_recognition_start();

    //5.在某时刻之后关闭jl_kws模块
    os_time_dly(2500);
    jl_kws_speech_recognition_close();
}

#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */
