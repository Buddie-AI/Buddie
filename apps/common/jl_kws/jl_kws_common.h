#ifndef _JL_KWS_COMMON_H_
#define _JL_KWS_COMMON_H_

#include "includes.h"
#include "app_config.h"

// #ifdef SUPPORT_MS_EXTENSIONS
// #pragma bss_seg(	".jl_kws_bss")
// #pragma data_seg(	".jl_kws_data")
// #pragma const_seg(	".jl_kws_const")
// #pragma code_seg(	".jl_kws_code")
// #endif


#define kws_info 		printf

#define KWS_DEBUG_ENABLE
#ifdef KWS_DEBUG_ENABLE
#define kws_debug 		printf
#define kws_putchar 	putchar
#else
#define kws_debug(...)
#define kws_putchar(...)
#endif /* #define KWS_DEBUG_ENABLE */


enum JL_KWS_ERR {
    JL_KWS_ERR_NONE = 0,
    JL_KWS_ERR_AUDIO_MIC_NO_BUF = -400,
    JL_KWS_ERR_AUDIO_INIT_STATE_ERR,
    JL_KWS_ERR_AUDIO_MIC_STATE_ERR,

    JL_KWS_ERR_ALGO_NO_BUF = -300,
    JL_KWS_ERR_ALGO_NO_FRAME_BUF,
};

enum KWS_VOICE_EVENT {
    KWS_VOICE_EVENT_NONE = 0,
    KWS_VOICE_EVENT_YES = 2,
    KWS_VOICE_EVENT_NO,
};

//========================================//
//            jl_kws_algo API             //
//========================================//
extern int jl_kws_algo_init(void);
extern int jl_kws_algo_detect_run(u8 *buf, u32 len);
extern void *jl_kws_algo_get_frame_buf(u32 *buf_len);
extern int jl_kws_algo_start(void);
extern void jl_kws_algo_close(void);
extern void jl_kws_algo_stop(void);

//========================================//
//            jl_kws_audio API            //
//========================================//
extern int jl_kws_audio_init(void);
extern int jl_kws_audio_start(void);
extern void jl_kws_audio_stop(void);
extern void jl_kws_audio_close(void);
extern int jl_kws_audio_get_data(void *buf, u32 len);

//========================================//
//            jl_kws_event API            //
//========================================//
extern int jl_kws_event_init(void);
extern void jl_kws_event_stop(void);
extern void jl_kws_event_close(void);
extern void jl_kws_event_state_update(u8 voice_event);

#endif /* #ifndef _JL_KWS_COMMON_H_ */
