
#include "application/audio_dec_app.h"
#include "app_config.h"
#include "audio_config.h"
#include "app_main.h"


//////////////////////////////////////////////////////////////////////////////
extern struct audio_decoder_task decode_task;
extern struct audio_mixer mixer;
extern struct audio_dac_hdl dac_hdl;
#if AUDIO_DAC_MULTI_CHANNEL_ENABLE
extern struct audio_dac_channel default_dac;
#endif

extern const int audio_dec_app_mix_en;

extern u32 audio_output_channel_num(void);

//////////////////////////////////////////////////////////////////////////////
struct audio_dec_app_audio_state_hdl {
    struct audio_mixer 			*p_mixer;
    u32 dec_mix : 1;	// 1:叠加模式
    u32 flag;
};

//////////////////////////////////////////////////////////////////////////////
const struct audio_dec_format_hdl decode_format_list[] = {
    {"wtg", AUDIO_CODING_G729},
    {"msbc", AUDIO_CODING_MSBC},
    {"msb", AUDIO_CODING_MSBC},
    {"sbc", AUDIO_CODING_SBC},
    {"mty", AUDIO_CODING_MTY},
    {"aac", AUDIO_CODING_AAC},
    {"mp3", AUDIO_CODING_MP3},
    {"wma", AUDIO_CODING_WMA},
    {"wav", AUDIO_CODING_WAV},
#if (defined(TCFG_DEC_MIDI_ENABLE) && TCFG_DEC_MIDI_ENABLE)
    //midi 文件播放，需要对应音色文件配合
    {"midi", AUDIO_CODING_MIDI},
    {"mid", AUDIO_CODING_MIDI},
#endif //TCFG_DEC_MIDI_ENABLE

#if TCFG_DEC_WTGV2_ENABLE
    {"wts", AUDIO_CODING_WTGV2},
#endif
    {"speex", AUDIO_CODING_SPEEX},
    {"opus", AUDIO_CODING_OPUS},
    {0, 0},
};

#if defined(CONFIG_MEDIA_DEVELOP_ENABLE)
static struct audio_stream_entry *audio_dec_app_entries[2] = {NULL};
#endif

//////////////////////////////////////////////////////////////////////////////

/*----------------------------------------------------------------------------*/
/**@brief    解码创建参数初始化
   @param    *dec: 解码句柄
   @return   0-正常
   @note     弱函数重定义
*/
/*----------------------------------------------------------------------------*/
int audio_dec_app_create_param_init(struct audio_dec_app_hdl *dec)
{
    dec->p_decode_task = &decode_task;
#if defined(CONFIG_MEDIA_DEVELOP_ENABLE)
    if (!audio_dec_app_mix_en) {
#if AUDIO_DAC_MULTI_CHANNEL_ENABLE
        audio_dec_app_entries[0] = &default_dac.entry;
#else
        audio_dec_app_entries[0] = &dac_hdl.entry;
#endif
        dec->entries = audio_dec_app_entries;
    } else
#endif
    {
        dec->p_mixer = &mixer;
    }
#if defined(CONFIG_MEDIA_DEVELOP_ENABLE)
    u8 dac_connect_mode =  app_audio_output_mode_get();
    if (dac_connect_mode == DAC_OUTPUT_FRONT_LR_REAR_LR) {
        dec->out_ch_num = 4;
    } else {
        dec->out_ch_num = audio_output_channel_num();
    }
#else
    u8 dac_connect_mode =  audio_dac_get_channel(&dac_hdl);
    switch (dac_connect_mode) {
    /* case DAC_OUTPUT_DUAL_LR_DIFF: */
    case DAC_OUTPUT_LR:
        dec->out_ch_num = 2;
        break;
    /* case DAC_OUTPUT_FRONT_LR_REAR_LR: */
    /* dec->out_ch_num = 4; */
    /* break; */
    default :
        dec->out_ch_num = 1;
        break;
    }
#endif
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    文件解码创建参数初始化
   @param    *file_dec: 文件解码句柄
   @return   0-正常
   @note     弱函数重定义
*/
/*----------------------------------------------------------------------------*/
int audio_dec_file_app_create_param_init(struct audio_dec_file_app_hdl *file_dec)
{
    file_dec->format = (struct audio_dec_format_hdl *)decode_format_list;
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    解码输出状态设置
   @param    *dec: 解码句柄
   @param    flag: 解码标签
   @return   0-正常
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_dec_app_audio_state_switch(struct audio_dec_app_hdl *dec, u32 flag)
{
    u8 need_set_audio = 1;
    /* if ((dec->dec_mix) && (audio_mixer_get_ch_num(dec->p_mixer) > 1)) { */
    /* need_set_audio = 0; */
    /* } */
    if (app_audio_get_state() == APP_AUDIO_STATE_IDLE) {
        need_set_audio = 1;
    }
    if (need_set_audio) {
        if (flag == AUDIO_DEC_FILE_FLAG_AUDIO_STATE_MUSIC) {
            app_audio_state_switch(APP_AUDIO_STATE_MUSIC, get_max_sys_vol());
        } else {
#ifdef TCFG_WTONT_ONCE_VOL
            extern u8 get_tone_once_vol(void);
            app_audio_state_switch(APP_AUDIO_STATE_WTONE, get_tone_once_vol());
#else
            app_audio_state_switch(APP_AUDIO_STATE_WTONE, get_tone_vol());
#endif
        }
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    解码输出状态退出
   @param    *p_aud_state: 输出状态
   @return   0-正常
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_dec_app_audio_state_exit(struct audio_dec_app_audio_state_hdl *p_aud_state)
{
    u8 need_set_audio = 1;
    /* if ((p_aud_state->dec_mix) && (audio_mixer_get_ch_num(p_aud_state->p_mixer) > 1)) { */
    /* need_set_audio = 0; */
    /* } */
    /* if (app_audio_get_state() == APP_AUDIO_STATE_IDLE) { */
    /* need_set_audio = 1; */
    /* } */
    if (need_set_audio) {
        if (p_aud_state->flag == AUDIO_DEC_FILE_FLAG_AUDIO_STATE_MUSIC) {
            app_audio_state_exit(APP_AUDIO_STATE_MUSIC);
        } else {
            app_audio_state_exit(APP_AUDIO_STATE_WTONE);
        }
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    文件解码初始化完成
   @param    *file_dec: 文件解码句柄
   @return   0-正常
   @note     弱函数重定义
*/
/*----------------------------------------------------------------------------*/
int audio_dec_file_app_init_ok(struct audio_dec_file_app_hdl *file_dec)
{
    audio_dec_app_audio_state_switch(file_dec->dec, file_dec->flag & AUDIO_DEC_FILE_FLAG_AUDIO_STATE_MASK);
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    文件解码结束
   @param    *file_dec: 文件解码句柄
   @return   0-正常
   @note     弱函数重定义
*/
/*----------------------------------------------------------------------------*/
int audio_dec_file_app_play_end(struct audio_dec_file_app_hdl *file_dec)
{
    struct audio_dec_app_audio_state_hdl aud_state = {0};
    aud_state.p_mixer = file_dec->dec->p_mixer;
    aud_state.dec_mix = file_dec->dec->dec_mix;
    aud_state.flag = file_dec->flag & AUDIO_DEC_FILE_FLAG_AUDIO_STATE_MASK;
    audio_dec_file_app_close(file_dec);
    audio_dec_app_audio_state_exit(&aud_state);
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    正弦波解码初始化完成
   @param    *sine_dec: 正弦波解码句柄
   @return   0-正常
   @note     弱函数重定义
*/
/*----------------------------------------------------------------------------*/
int audio_dec_sine_app_init_ok(struct audio_dec_sine_app_hdl *sine_dec)
{
    audio_dec_app_audio_state_switch(sine_dec->dec, sine_dec->flag & AUDIO_DEC_FILE_FLAG_AUDIO_STATE_MASK);
    return 0;
}
/*----------------------------------------------------------------------------*/
/**@brief    正弦波解码结束
   @param    *sine_dec: 正弦波解码句柄
   @return   0-正常
   @note     弱函数重定义
*/
/*----------------------------------------------------------------------------*/
int audio_dec_sine_app_play_end(struct audio_dec_sine_app_hdl *sine_dec)
{
    struct audio_dec_app_audio_state_hdl aud_state = {0};
    aud_state.p_mixer = sine_dec->dec->p_mixer;
    aud_state.dec_mix = sine_dec->dec->dec_mix;
    aud_state.flag = sine_dec->flag & AUDIO_DEC_FILE_FLAG_AUDIO_STATE_MASK;
    audio_dec_sine_app_close(sine_dec);
    audio_dec_app_audio_state_exit(&aud_state);
    return 0;
}

#if (!defined(CONFIG_MEDIA_DEVELOP_ENABLE))
void audio_dec_app_output_sr_set(struct audio_dec_app_hdl *dec)
{
    /* #if defined(CONFIG_CPU_BR23) */
    /* extern u32 audio_output_rate(int input_rate); */
    /* dec->src_out_sr = audio_output_rate(dec->src_out_sr); */
    /* #endif */
    if (dec->src_out_sr == 0) {
        dec->src_out_sr = audio_dac_get_sample_rate(&dac_hdl);
        if (dec->src_out_sr == 0) {
            dec->src_out_sr = 16000;
            log_w("src out is zero \n");
        }
    }
    if (dec->sample_rate == 0) {
        dec->sample_rate = dec->src_out_sr;
    }
}
#endif


//////////////////////////////////////////////////////////////////////////////
// test
#if 0
#include "tone_player.h"

void audio_dec_file_test(void)
{
    struct audio_dec_file_app_hdl *hdl;
    hdl = audio_dec_file_app_create(TONE_POWER_ON, 1);
    if (hdl) {
        audio_dec_file_app_open(hdl);
    }
    os_time_dly(2);
    hdl = audio_dec_file_app_create(TONE_POWER_OFF, 1);
    if (hdl) {
        audio_dec_file_app_open(hdl);
    }
    os_time_dly(300);
}

static const struct audio_sin_param sine_test[] = {
    /*{0, 1000, 0, 100},*/
    {200 << 9, 4000, 0, 100},
};
static const struct audio_sin_param sine_test1[] = {
    {450 << 9, 24960, 1, 16.667 * 512},
    {0, 16000, 0, 100},
};
void audio_dec_sine_test(void)
{
    struct audio_dec_sine_app_hdl *hdl;
    /* hdl = audio_dec_sine_app_create(SDFILE_RES_ROOT_PATH"tone/vol_max.sin", 1); */
    hdl = audio_dec_sine_app_create_by_parm(sine_test1, ARRAY_SIZE(sine_test1), 1);
    if (hdl) {
        audio_dec_sine_app_open(hdl);
    }
    os_time_dly(2);
    hdl = audio_dec_sine_app_create_by_parm(sine_test, ARRAY_SIZE(sine_test), 1);
    if (hdl) {
        audio_dec_sine_app_open(hdl);
    }
    /* os_time_dly(300); */
}

void audio_dec_usb_file_test(void)
{
    tone_play_stop();
    clk_set("sys", 192 * 1000000L);

    struct audio_dec_file_app_hdl *hdl;
    /* hdl = audio_dec_file_app_create("storage/udisk/C/1.mp3", 1); */
    hdl = audio_dec_file_app_create("storage/udisk/C/1.wav", 1);
    if (hdl) {
        audio_dec_file_app_open(hdl);
    }
    os_time_dly(2);
    /* hdl = audio_dec_file_app_create("storage/udisk/C/2.mp3", 1); */
    hdl = audio_dec_file_app_create("storage/udisk/C/2.wav", 1);
    if (hdl) {
        audio_dec_file_app_open(hdl);
    }
    os_time_dly(300);
}

#endif /*test*/


