
#ifndef _AUDIO_DEC_APP_H_
#define _AUDIO_DEC_APP_H_

#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"

//////////////////////////////////////////////////////////////////////////////
// 标签
#define AUDIO_DEC_APP_MASK			(('A' << 24) | ('D' << 16) | ('P' << 8) | ('M' << 8))

//////////////////////////////////////////////////////////////////////////////
// 解码状态
enum audio_dec_app_status {
    AUDIO_DEC_APP_STATUS_STOP = 0,
    AUDIO_DEC_APP_STATUS_PLAY,
    AUDIO_DEC_APP_STATUS_PAUSE,
};

// 解码事件
enum audio_dec_app_event {
    AUDIO_DEC_APP_EVENT_NULL = 0,
    /*
     * Description: 解码预处理
     * Arguments  : None.
     * Return	  : 0-正常，非0-挂起
     * Note(s)    : (struct audio_dec_app_hdl*)hdl->handler->dec_probe中调用
     *              返回0后正常执行后续的audio_dec_app_probe_evt_cb_after()弱函数
     */
    AUDIO_DEC_APP_EVENT_DEC_PROBE,
    /*
     * Description: 解码输出
     * Arguments  : int parm[2]; param[0]:data, param[1]:len
     * Return	  : 不判断
     * Note(s)    : (struct audio_dec_app_hdl*)hdl->decoder.entry.data_handler中调用
     *              仅用于查看更改内容，不改变数据长度，后续正常执行audio_stream_run()
     */
    AUDIO_DEC_APP_EVENT_DEC_OUTPUT,
    /*
     * Description: 解码后处理
     * Arguments  : None.
     * Return	  : 同.dec_post接口返回
     * Note(s)    : (struct audio_dec_app_hdl*)hdl->handler->dec_post中调用
     */
    AUDIO_DEC_APP_EVENT_DEC_POST,
    /*
     * Description: 解码停止
     * Arguments  : None.
     * Return	  : 同.dec_stop接口返回
     * Note(s)    : (struct audio_dec_app_hdl*)hdl->handler->dec_stop中调用
     */
    AUDIO_DEC_APP_EVENT_DEC_STOP,
    /*
     * Description: 解码初始化完成
     * Arguments  : None.
     * Return	  : 不判断
     * Note(s)    : 解码start()函数中audio_decoder_start()函数前调用
     */
    AUDIO_DEC_APP_EVENT_START_INIT_OK,
    /*
     * Description: 解码播放正常
     * Arguments  : None.
     * Return	  : 不判断
     * Note(s)    : 解码start()函数中audio_decoder_start()函数后调用
     */
    AUDIO_DEC_APP_EVENT_START_OK,
    /*
     * Description: 解码播放失败
     * Arguments  : None.
     * Return	  : 不判断
     * Note(s)    : 解码start()函数中解码失败后调用
     */
    AUDIO_DEC_APP_EVENT_START_ERR,
    /*
     * Description: 解码关闭
     * Arguments  : None.
     * Return	  : 不判断
     * Note(s)    : audio_dec_app_close()函数中各功能关闭后，audio_stream_close()之前调用
     */
    AUDIO_DEC_APP_EVENT_DEC_CLOSE,
    /*
     * Description: 解码结束消息
     * Arguments  : None.
     * Return	  : 不判断
     * Note(s)    :
     */
    AUDIO_DEC_APP_EVENT_PLAY_END,
};

struct audio_dec_app_file_hdl {
    int (*read)(void *hdl, void *buf, u32 len);
    int (*seek)(void *hdl, int offset, int orig);
    int (*len)(void *hdl);
};

struct audio_dec_app_hdl {
    u32 mask;	// 固定为AUDIO_DEC_APP_MASK
    u32 id;		// 唯一标识符，随机值
    struct list_head list_entry;	// 链表
    struct audio_decoder decoder;
    struct audio_res_wait wait;
    struct audio_mixer_ch mix_ch;
    enum audio_channel ch_type;
    volatile enum audio_dec_app_status status;
    u32 ch_num : 4;		// channel通道数
    u32 out_ch_num : 4;	// 输出声道数
    u32 tmp_pause : 1;
    u32 dec_mix : 1;	// 1:叠加模式
    u32 remain : 1;
    u32 frame_type : 1;	// 1:frame格式；0:file格式
    u32 close_by_res_put : 1; //被打断就自动close

    u32 mix_ext;	// mixer ext out mask

    u16 frame_pkt_len;	// frame格式帧长
    u16 frame_data_len;	// frame格式当前数据长度
    u8 *frame_buf;		// frame格式帧buf

    u32 dec_type;		// 指定解码格式，start后为实际解码格式
    u16 sample_rate;	// 指定采样率，start后为实际解码采样率

    u16 src_out_sr;		// 指定输出采样，不指定则和解码采样相同
    struct audio_src_handle *hw_src;

    struct audio_decoder_task 	*p_decode_task;
    struct audio_mixer 			*p_mixer;


    int (*out_put)(void *, s16 *data, int len); // 解码输出回调。不设置，则输出到mixer
    void *out_priv;

    int (*evt_cb)(void *, enum audio_dec_app_event event, int *param); // 事件回调
    void *evt_priv;

    struct audio_dec_input dec_input;
    struct audio_dec_input *input;		// 指定使用input接口
    struct audio_dec_handler *handler;	// 指定使用handler接口

    void *file_hdl;
    struct audio_dec_app_file_hdl *file;

    void *app_hdl;	// 指向上一级句柄
};

// 解码的种类。比如用该值来区分音量等
#define AUDIO_DEC_FILE_FLAG_AUDIO_STATE_TONE		(0)
#define AUDIO_DEC_FILE_FLAG_AUDIO_STATE_MUSIC		(1)
#define AUDIO_DEC_FILE_FLAG_AUDIO_STATE_MASK		(0x000000ff)

/*
 * 通过文件后缀名匹配解码类型
 */
struct audio_dec_format_hdl {
    const char *fmt;	// 后缀名
    u32 coding_type;	// 解码类型
};

/*
 * 文件解码
 */
struct audio_dec_file_app_hdl {
    struct audio_dec_app_hdl *dec;
    struct audio_dec_format_hdl	*format;
    u32   flag;		// 标签，可用于设置audio通道等
    void *file_hdl;
    void *priv;		// 私有参数
};

/*
 * 正弦波参数头部
 */
struct audio_sine_param_head {
    u16 repeat_time;
    u8  set_cnt;
    u8  cur_cnt;
};

/*
 * 正弦波参数
 */
struct audio_sin_param {
    //int idx_increment;
    int freq;
    int points;
    int win;
    int decay;
};

/*
 * 正弦波解码
 */
#define AUDIO_DEC_SINE_APP_NUM_MAX		8
struct audio_dec_sine_app_hdl {
    struct audio_dec_app_hdl *dec;
    void *sin_maker;
    struct audio_sin_param sin_parm[AUDIO_DEC_SINE_APP_NUM_MAX];
    struct audio_sin_param *sin_src;
    u8	  sin_num;
    u8	  sin_repeat;
    u16   sin_default_sr;
    u32   sin_volume;
    u32   flag;		// 标签，可用于设置audio通道等
    void *file_hdl;
    void *priv;
};

//////////////////////////////////////////////////////////////////////////////
/*
*********************************************************************
*                  Audio Decoder App Get Format By Name
* Description: 通过名字判断解码类型
* Arguments  : *name	名字
*              *format	解码类型映射
* Return	 : 解码类型
* Note(s)    : 查询不到返回AUDIO_CODING_UNKNOW
*********************************************************************
*/
u32 audio_dec_app_get_format_by_name(char *name, struct audio_dec_format_hdl *format);

/*
*********************************************************************
*                  Audio Decoder App Create
* Description: 创建一个dec_app解码
* Arguments  : *priv	事件回调私有参数
*              *evt_cb	事件回调接口
*              mix		1-叠加播放，0-抢占播放
* Return	 : dec_app句柄
* Note(s)    : mix==1默认参数：dec->wait.protect = 1;
*              mix==0默认参数：dec->wait.preemption = 1;
*********************************************************************
*/
struct audio_dec_app_hdl *audio_dec_app_create(void *priv, int (*evt_cb)(void *, enum audio_dec_app_event event, int *param), u8 mix);

/*
*********************************************************************
*                  Audio Decoder App Open
* Description: 打开dec_app解码
* Arguments  : *dec		dec_app句柄
* Return	 : true		成功
* Note(s)    : None.
*********************************************************************
*/
int audio_dec_app_open(struct audio_dec_app_hdl *dec);

/*
*********************************************************************
*                  Audio Decoder App Close
* Description: 关闭dec_app解码
* Arguments  : *dec		dec_app句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_dec_app_close(struct audio_dec_app_hdl *dec);

/*
*********************************************************************
*                  Audio Decoder App Start
* Description: 开始解码
* Arguments  : *dec		解码句柄
* Return	 : 0		成功
* Note(s)    : None.
*********************************************************************
*/
int audio_dec_app_start(struct audio_dec_app_hdl *dec);


/*
*********************************************************************
*                  Audio Decoder App Set File Info
* Description: 设置dec_app文件信息
* Arguments  : *dec		dec_app句柄
*              *file_hdl	文件句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_dec_app_set_file_info(struct audio_dec_app_hdl *dec, void *file_hdl);

/*
*********************************************************************
*                  Audio Decoder App Set Frame Info
* Description: 设置dec_app流数据信息
* Arguments  : *dec		dec_app句柄
*              pkt_len	帧长
*              coding_type	流数据类型
* Return	 : None.
* Note(s)    : 会申请一块pkt_len长度的空间做缓存
*********************************************************************
*/
void audio_dec_app_set_frame_info(struct audio_dec_app_hdl *dec, u16 pkt_len, u32 coding_type);

/*
*********************************************************************
*                  Audio Decoder App Pause/Play
* Description: dec_app解码暂停播放
* Arguments  : *dec		dec_app句柄
* Return	 : true		成功
* Note(s)    : None.
*********************************************************************
*/
int audio_dec_app_pp(struct audio_dec_app_hdl *dec);

/*
*********************************************************************
*                  Audio Decoder App Get Status
* Description: 获取dec_app解码状态
* Arguments  : *dec		dec_app句柄
* Return	 : enum audio_dec_app_status
* Note(s)    : 错误时返回 负数
*********************************************************************
*/
int audio_dec_app_get_status(struct audio_dec_app_hdl *dec);

/*
*********************************************************************
*                  Audio Decoder App Check Handler Live
* Description: 检测解码句柄是否存在
* Arguments  : *dec		dec_app句柄
* Return	 : true		句柄存在
* Note(s)    : None.
*********************************************************************
*/
int audio_dec_app_check_hdl(struct audio_dec_app_hdl *dec);

/*
*********************************************************************
*                  Audio Decoder App File Decoder Create
* Description: 创建一个文件解码
* Arguments  : *name	文件名
*              mix		1-叠加播放，0-抢断播放
* Return	 : 文件解码句柄
* Note(s)    : 默认开启被打断就自动close功能，hdl->dec->close_by_res_put=1;
*              默认开启可抢断同优先级解码，hdl->dec->wait.snatch_same_prio=1;
*              其他默认配置见audio_dec_app_create()函数说明
*********************************************************************
*/
struct audio_dec_file_app_hdl *audio_dec_file_app_create(char *name, u8 mix);

/*
*********************************************************************
*                  Audio Decoder App File Decoder Open
* Description: 打开文件解码
* Arguments  : *file_dec	文件解码句柄
* Return	 : true		成功
* Note(s)    : None.
*********************************************************************
*/
int audio_dec_file_app_open(struct audio_dec_file_app_hdl *file_dec);

/*
*********************************************************************
*                  Audio Decoder App File Decoder close
* Description: 关闭文件解码
* Arguments  : *file_dec	文件解码句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_dec_file_app_close(struct audio_dec_file_app_hdl *file_dec);

/*
*********************************************************************
*                  Audio Decoder App Sine File Decoder Create
* Description: 创建一个正弦波文件解码
* Arguments  : *name	文件名
*              mix		1-叠加播放，0-抢断播放
* Return	 : 正弦波解码句柄
* Note(s)    : 默认开启被打断就自动close功能，hdl->dec->close_by_res_put=1;
*              默认开启可抢断同优先级解码，hdl->dec->wait.snatch_same_prio=1;
*              其他默认配置见audio_dec_app_create()函数说明
*********************************************************************
*/
struct audio_dec_sine_app_hdl *audio_dec_sine_app_create(char *name, u8 mix);

/*
*********************************************************************
*                  Audio Decoder App Sine Param Decoder Create
* Description: 创建一个正弦波数组解码
* Arguments  : *sin		数组地址
*              sin_num	数组数量
*              mix		1-叠加播放，0-抢断播放
* Return	 : 正弦波解码句柄
* Note(s)    : 默认开启被打断就自动close功能，hdl->dec->close_by_res_put=1;
*              默认开启可抢断同优先级解码，hdl->dec->wait.snatch_same_prio=1;
*              其他默认配置见audio_dec_app_create()函数说明
*********************************************************************
*/
struct audio_dec_sine_app_hdl *audio_dec_sine_app_create_by_parm(struct audio_sin_param *sin, u8 sin_num, u8 mix);

/*
*********************************************************************
*                  Audio Decoder App Sine Decoder Open
* Description: 打开正弦波解码
* Arguments  : *sine_dec	正弦波解码句柄
* Return	 : true		成功
* Note(s)    : None.
*********************************************************************
*/
int audio_dec_sine_app_open(struct audio_dec_sine_app_hdl *sine_dec);

/*
*********************************************************************
*                  Audio Decoder App Sine Decoder Close
* Description: 关闭正弦波解码
* Arguments  : *sine_dec	正弦波解码句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_dec_sine_app_close(struct audio_dec_sine_app_hdl *sine_dec);

/*
*********************************************************************
*                  Audio Decoder App Sine Probe Deal
* Description: 正弦波解码参数预处理
* Arguments  : *sine_dec	正弦波解码句柄
* Return	 : None.
* Note(s)    : 正弦波解码需要先预处理一下参数
*********************************************************************
*/
void audio_dec_sine_app_probe(struct audio_dec_sine_app_hdl *sine_dec);

#endif /*_AUDIO_DEC_APP_H_*/

