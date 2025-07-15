
#ifndef _AUDIO_PITCH_API_H_
#define _AUDIO_PITCH_API_H_
#include "pitchshifter/pitchshifter_api.h"
// #include "mono2stereo/reverb_mono2stero_api.h"


#define PITCHSHIFT_USE_AUDIO_STREAM   0 //是否使用audio_sream流节点
#if PITCHSHIFT_USE_AUDIO_STREAM
#include "media/audio_stream.h"
#endif

typedef struct _s_pitch_hdl {
    PITCHSHIFT_FUNC_API *ops;
    u8 *databuf;
    PITCH_SHIFT_PARM parm;
#if PITCHSHIFT_USE_AUDIO_STREAM
    struct audio_stream_entry entry;	// 音频流入口
    int out_len;
    int process_len;
#endif
    u8 run_en;
    u8 update;
} s_pitch_hdl;
/*
 * 获取变声模块默认参数；open时不传默认参数会使用内部默认参数
 * 仅用于获取初值。实时参数存放在open的返回句柄parm中
 */
PITCH_SHIFT_PARM *get_pitch_parm(void);
/*
 * 变声模块打开
 */
s_pitch_hdl *open_pitch(PITCH_SHIFT_PARM *param);
/*
 * 变声模块关闭
 */
void close_pitch(s_pitch_hdl *picth_hdl);
/*
 * 变声模块参数更新
 */
void update_picth_parm(s_pitch_hdl *pitch_hdl, PITCH_SHIFT_PARM *p_pitch_parm);
/*
 * 变声模块数据处理
 */
void pitch_run(s_pitch_hdl *picth_hdl, s16 *indata, s16 *outdata, int len, u8 ch_num);
/*
 * 变声模块暂停处理
 */
void pause_pitch(s_pitch_hdl *pitch_hdl, u8 run_mark);
#endif

