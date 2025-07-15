
#ifndef _AUDIO_DEC_FILE_H_
#define _AUDIO_DEC_FILE_H_

#include "asm/includes.h"
#include "media/includes.h"
#include "media/file_decoder.h"
#include "system/includes.h"
#include "media/audio_decoder.h"
#include "app_config.h"
#include "music/music_decrypt.h"
#include "music/music_id3.h"

#include "audio_dec_eff.h"


#define FILE_DEC_REPEAT_EN			0 // 无缝循环播放

enum {
    FILE_DEC_STREAM_CLOSE = 0,
    FILE_DEC_STREAM_OPEN,
};

struct file_dec_hdl {
    //struct audio_stream *stream;	// 音频流
    struct file_decoder file_dec;	// file解码句柄
    struct audio_res_wait wait;		// 资源等待句柄
#if TCFG_USER_TWS_ENABLE
    struct file_decoder trans_dec;	// 解码句柄
    u8 *frame;
    u16 frame_len;
    u16 frame_remain_len;
    u8 trans_dec_end;
    u8 wait_tws_confirm;
    u8 tws_channel;
    u16 seqn;
    u16 tx_seqn;
    u32 wait_tws_timeout;
    void *ts_handle;
    void *syncts;
    u32 pcm_num;
    u32 mix_ch_event_params[3];
#endif
    struct audio_mixer_ch mix_ch;	// 叠加句柄

#if TCFG_EQ_ENABLE&&TCFG_MUSIC_MODE_EQ_ENABLE
    struct dec_eq_drc *eq_drc;
#endif//TCFG_MUSIC_MODE_EQ_ENABLE

    u8 remain;

    struct audio_dec_breakpoint *dec_bp; // 断点
    u32 id;					// 唯一标识符，随机值
    void *file;				// 文件句柄
    u32 pick_flag : 1;		// 挑出数据帧发送（如MP3等)。不是输出pcm，后级不能接任何音效处理等
    u32 pcm_enc_flag : 1;	// pcm压缩成数据帧发送（如WAV等）
    u32 read_err : 2;		// 读数出错 0:no err， 1:fat err,  2:disk err
    u32 wait_add : 1;	// 是否马上得到执行

#if TCFG_DEC_DECRYPT_ENABLE
    CIPHER mply_cipher;		// 解密播放
#endif
#if TCFG_DEC_ID3_V1_ENABLE
    MP3_ID3_OBJ *p_mp3_id3_v1;	// id3_v1信息
#endif
#if TCFG_DEC_ID3_V2_ENABLE
    MP3_ID3_OBJ *p_mp3_id3_v2;	// id3_v2信息
#endif

#if FILE_DEC_REPEAT_EN
    u8 repeat_num;			// 无缝循环次数
    struct fixphase_repair_obj repair_buf;	// 无缝循环句柄
#endif

    struct audio_dec_breakpoint *bp;	// 断点信息

    void *evt_priv;			// 事件回调私有参数
    void (*evt_cb)(void *, int argc, int *argv);	// 事件回调句柄

    void (*stream_handler)(void *priv, int event, struct file_dec_hdl *);	// 数据流设置回调
    void *stream_priv;						// 数据流设置回调私有句柄
};

enum {
    FILE_FROM_LOCAL,
    FILE_FROM_TWS,
};



struct file_decoder *file_dec_get_file_decoder_hdl(void);

#define file_dec_is_stop()				file_decoder_is_stop(file_dec_get_file_decoder_hdl())
#define file_dec_is_play()				file_decoder_is_play(file_dec_get_file_decoder_hdl())
#define file_dec_is_pause()				file_decoder_is_pause(file_dec_get_file_decoder_hdl())
#define file_dec_pp()				    file_decoder_pp(file_dec_get_file_decoder_hdl())
#define file_dec_FF(x)					file_decoder_FF(file_dec_get_file_decoder_hdl(),x)
#define file_dec_FR(x)					file_decoder_FR(file_dec_get_file_decoder_hdl(),x)
#define file_dec_get_breakpoint(x)		file_decoder_get_breakpoint(file_dec_get_file_decoder_hdl(),x)
#define file_dec_get_total_time()		file_decoder_get_total_time(file_dec_get_file_decoder_hdl())
#define file_dec_get_cur_time()			file_decoder_get_cur_time(file_dec_get_file_decoder_hdl())
#define file_dec_get_decoder_type()		file_decoder_get_decoder_type(file_dec_get_file_decoder_hdl())

int file_dec_create(void *priv, void (*handler)(void *, int argc, int *argv));
int file_dec_open(void *file, struct audio_dec_breakpoint *bp);
void file_dec_close();
int file_dec_restart(int id);
int file_dec_push_restart(void);
int file_dec_get_status(void);

void file_dec_set_stream_set_hdl(struct file_dec_hdl *dec,
                                 void (*stream_handler)(void *priv, int event, struct file_dec_hdl *),
                                 void *stream_priv);
void *get_file_dec_hdl();

int tws_local_media_dec_open(u8 channel, u8 *arg);

void tws_local_media_dec_close(u8 channel);

int tws_local_media_dec_state();

void send_local_media_dec_open_cmd();

int file_dec_get_source();

#endif /*TCFG_APP_MUSIC_EN*/

