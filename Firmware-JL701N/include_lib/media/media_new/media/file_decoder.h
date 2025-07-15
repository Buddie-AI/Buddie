#ifndef FILE_DECODER_H
#define FILE_DECODER_H


#include "media/includes.h"

enum {
    FILE_DEC_STATUS_STOP = 0,	// 解码停止
    FILE_DEC_STATUS_PLAY,		// 正在解码
    FILE_DEC_STATUS_PAUSE,		// 解码暂停
    FILE_DEC_STATUS_WAIT_PAUSE,
    FILE_DEC_STATUS_WAIT_PLAY,
    FILE_DEC_STATUS_PAUSE_SUCCESS,
};


struct file_decoder {
    u8 ch_num;			    // 声道数
    u8 output_ch_num;	    // 输出声道数
    u8 status;		        // 解码状态
    u8 dec_no_out_sound;	// 解码不直接输出声音（用于TWS转发）
    u8 tmp_pause;	        // 被其他解码打断，临时暂停
    u32 wait_pause;
    u8 remain;
    u16 resume_tmr_id;
    u16 sample_rate;
    u32 coding_type;
    u32 dec_total_time;	    // 总共能播放多长时间
    u32 dec_cur_time;	    // 当前播放时间
    u32 once_out_cnt;	    // 解码一次的输出统计
    enum audio_channel ch_type;		// 输出类型
    struct audio_decoder decoder;	// 解码句柄
    int (*output_handler)(struct file_decoder *dec, s16 *data, int len);
    void *tws_sync;
    s16 fade_step;
    s16 fade_value;
};

int file_decoder_open(struct file_decoder *dec,
                      const struct audio_dec_input *input,
                      struct audio_decoder_task *decode_task,
                      struct audio_dec_breakpoint *bp,
                      u8 pick);

int frame_decoder_open(struct file_decoder *dec,
                       const struct audio_dec_input *input,
                       struct audio_decoder_task *decode_task);

void file_decoder_close(struct file_decoder *dec);

void file_decoder_set_output_channel(struct file_decoder *dec);

void file_decoder_set_event_handler(struct file_decoder *dec,
                                    void (*handler)(struct audio_decoder *, int, int *), u32 maigc);

bool file_decoder_is_stop(struct file_decoder *file_dec);
bool file_decoder_is_play(struct file_decoder *file_dec);
bool file_decoder_is_pause(struct file_decoder *file_dec);
int file_decoder_pp(struct file_decoder *file_dec);
int file_decoder_FF(struct file_decoder *file_dec, int step);
int file_decoder_FR(struct file_decoder *file_dec, int step);
int file_decoder_get_breakpoint(struct file_decoder *file_dec, struct audio_dec_breakpoint *bp);
int file_decoder_get_total_time(struct file_decoder *file_dec);
int file_decoder_get_cur_time(struct file_decoder *file_dec);
int file_decoder_get_decoder_type(struct file_decoder *file_dec);
void file_decoder_set_tws_sync_enable(struct file_decoder *dec, void *sync);
void file_decoder_mark_tws_sync_data(struct file_decoder *dec, u16 seqn);
void file_decoder_tws_sync_restart(struct file_decoder *dec);

#endif /*FILE_DECODER_H*/

