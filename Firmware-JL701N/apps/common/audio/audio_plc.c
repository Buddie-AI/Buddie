/*
 ************************************************************
 *
 *
 *
 ************************************************************
 */
#include "system/includes.h"
#include "PLC.h"
#include "os/os_api.h"
#include "app_config.h"
#include "audio_config.h"

#if TCFG_ESCO_PLC

//#define AUDIO_PLC_LOG_ENABLE
#ifdef AUDIO_PLC_LOG_ENABLE
#define PLC_LOG		y_printf
#else
#define PLC_LOG(...)
#endif/*AUDIO_PLC_LOG_ENABLE*/

enum {
    PLC_STA_CLOSE = 0,
    PLC_STA_OPEN,
    PLC_STA_RUN,
};


#define PLC_FRAME_LEN	120
typedef struct {
    u8 state;
    u8 repair;
    u8 wideband;
    OS_MUTEX mutex;
    s16 *run_buf;
} audio_plc_t;
static audio_plc_t plc;

int audio_plc_open(u16 sr)
{
    PLC_LOG("audio_plc_open:%d\n", sr);
    memset((u8 *)&plc, 0, sizeof(audio_plc_t));
    plc.run_buf	= malloc(PLC_query()); /*buf_size:1040*/
    PLC_LOG("PLC_buf:%x,size:%d\n", plc.run_buf, PLC_query());
    if (plc.run_buf) {
        s8 err = PLC_init(plc.run_buf);
        if (err) {
            PLC_LOG("PLC_init err:%d", err);
            free(plc.run_buf);
            return -EINVAL;
        }
        os_mutex_create(&plc.mutex);
        if (sr == 16000) {
            plc.wideband = 1;
        }
        plc.state = PLC_STA_OPEN;
    }
    PLC_LOG("audio_plc_open succ\n");
    return 0;
}

void audio_plc_run(s16 *data, u16 len, u8 repair)
{
    u16 repair_point, tmp_point;
    s16 *p_in, *p_out;
    p_in    = data;
    p_out   = data;
    tmp_point = len / 2;
    u8 repair_flag = 0;

    os_mutex_pend(&plc.mutex, 0);
    if (plc.state == PLC_STA_CLOSE) {
        os_mutex_post(&plc.mutex);
        return;
    }

#if 0	//debug
    static u16 repair_cnt = 0;
    if (repair) {
        repair_cnt++;
        y_printf("[E%d]", repair_cnt);
    } else {
        repair_cnt = 0;
    }
    //printf("[%d]",point);
#endif/*debug*/

    repair_flag = repair;
    if (plc.wideband) {
        /*
         *msbc plc deal
         *如果上一帧是错误，则当前帧也要修复
         */
        if (plc.repair) {
            repair_flag = 1;
        }
        plc.repair = repair;
    }

    while (tmp_point) {
        repair_point = (tmp_point > PLC_FRAME_LEN) ? PLC_FRAME_LEN : tmp_point;
        tmp_point = tmp_point - repair_point;
        PLC_run(p_in, p_out, repair_point, repair_flag);
        p_in  += repair_point;
        p_out += repair_point;
    }
    os_mutex_post(&plc.mutex);
}

int audio_plc_close(void)
{
    PLC_LOG("audio_plc_close\n");
    os_mutex_pend(&plc.mutex, 0);
    plc.state = PLC_STA_CLOSE;
    if (plc.run_buf) {
        free(plc.run_buf);
        plc.run_buf = NULL;
    }
    os_mutex_post(&plc.mutex);
    PLC_LOG("audio_plc_close succ\n");
    return 0;
}
#else
int audio_plc_open(void)
{
    return 0;
}

void audio_plc_run(s16 *dat, u16 len, u8 repair_flag)
{
}

int audio_plc_close()
{
    return 0;
}
#endif/*TCFG_ESCO_PLC*/
