#ifndef _PHONE_MESSAGE_
#define _PHONE_MESSAGE_

#include "application/audio_dec_app.h"
#include "btstack/avctp_user.h"
#include "classic/tws_api.h"
#include "app_config.h"
#include "app_main.h"

#define PHONE_MESSAGE_TWS_ENABLE       	0

#if (TCFG_PHONE_MESSAGE_ENABLE)
//////////////////////////////////////////////////////////////////////////////

#if TCFG_USER_TWS_ENABLE
#undef PHONE_MESSAGE_TWS_ENABLE
#define PHONE_MESSAGE_TWS_ENABLE       	1	// 通话留言TWS使能
#endif

#define PHONE_MESSAGE_ENC_USE_TASK		(1)	// 使用单独线程存储录音数据

#define PHONE_MESSAGE_DEC_GET_INFO_ONLY_ENC		0	// 仅在播录音过程中可以获取信息
#define PHONE_MESSAGE_DEC_GET_INFO_LIMIT_TIME	(10*1000)	// 在限制时间内获取有效

#define PHONE_MESSAGE_TWS_BUF_LEN		(4*1024) // tws转发buf
#define PHONE_MESSAGE_TWS_PKT_LEN		(512)	// 每一包最大长度

#define PHONE_MESSAGE_USE_BUF_LEN		(1*1024) // 留言推提示音和录音buf

// 通话留言按键响应类型
#define CONFIG_PHONE_MESSAGE_KEY_ALL			1 // 都可以响应
#define CONFIG_PHONE_MESSAGE_KEY_LEFT			2 // 仅左耳可以响应
#define CONFIG_PHONE_MESSAGE_KEY_RIGHT			3 // 仅右耳可以响应
#define CONFIG_PHONE_MESSAGE_ENC_KEY_TYPE		CONFIG_PHONE_MESSAGE_KEY_ALL//CONFIG_PHONE_MESSAGE_KEY_RIGHT
#define CONFIG_PHONE_MESSAGE_PLAY_KEY_TYPE		CONFIG_PHONE_MESSAGE_KEY_ALL


#define PHONE_MESSAGE_ENC_HEAD			SDFILE_RES_ROOT_PATH"tone/msg_head.*"
#define PHONE_MESSAGE_ENC_TAIL			SDFILE_RES_ROOT_PATH"tone/msg_tail.*"

#define PHONE_MESSAGE_DEC_HAVE			SDFILE_RES_ROOT_PATH"tone/msg_have.*"
#define PHONE_MESSAGE_DEC_NONE			SDFILE_RES_ROOT_PATH"tone/msg_none.*"

enum {
    PHONE_MESSAGE_EVENT_NULL = 0,
    PHONE_MESSAGE_EVENT_HEAD_TONE_END, 	// head提示音播放结束
    PHONE_MESSAGE_EVENT_TIMEOUT, 	// 超时
    PHONE_MESSAGE_EVENT_ENERGY,		// 能量检测结束
    PHONE_MESSAGE_EVENT_WRITE_FILE_ERROR,	// 写文件失败
    PHONE_MESSAGE_EVENT_STOP,		// 留言已结束
};

enum {
    PHONE_MESSAGE_STATUS_IDLE = 0,
    PHONE_MESSAGE_STATUS_PLAY_HEAD,	// 正在推送起始提示音
    PHONE_MESSAGE_STATUS_ENC,		// 正在录音
    PHONE_MESSAGE_STATUS_PLAY_TAIL,	// 正在推送结束提示音
    PHONE_MESSAGE_STATUS_WAIT_STOP,	// 等待结束
};

enum {
    PHONE_MESSAGE_USE_BUF_TYPE_NULL = 0,
    PHONE_MESSAGE_USE_BUF_TYPE_OUT,	// buf用于推送提示音
    PHONE_MESSAGE_USE_BUF_TYPE_ENC,	// buf用于录音
};

enum {
    PHONE_MESSAGE_DEC_STATUS_IDLE = 0,
    PHONE_MESSAGE_DEC_STATUS_PLAY_HAVE,	// 播放 有留言 提示音
    PHONE_MESSAGE_DEC_STATUS_PLAY_NONE,	// 播放 没有留言 提示音
    PHONE_MESSAGE_DEC_STATUS_PLAY_ENC,	// 播放留言内容
    PHONE_MESSAGE_DEC_STATUS_WAIT_STOP,	// 等待结束
};

struct phone_message_hdl {
    u32 status : 4;		// 留言状态
    volatile u32 init_ok : 1;	// 已经初始化
        u32 time_check : 1;		// 留言超时使能
        u32 energy_check : 1;	// 留言能量检测使能
        u32 energy_flag : 1;	// 标记全程是否有能量检测达标
        u32 min_time_check : 1;	// 时间太短，删除
        u32 enc_limit_energy : 1;	// 整个过程中能量都不达标，删除
        u32 esco_dec_mute : 1;	// 近端静音
        u32 tws : 1;			// 1:tws模式；0:普通模式，本地播放
        void *head_file_hdl;	// 留言起始提示音
        void *tail_file_hdl;	// 留言结束提示音
        struct audio_dec_app_hdl *dec;	// 解码句柄
        u16 mic_sr;		// mic采样率
        u8  mic_ch;		// mic通道数
        u8  pkt_len;	// 帧长
        u8  phone_num[12];	// 电话号码
        u16 sample_rate;	// 留言数据采样率
        u32 coding_type;	// 留言数据类型
        u32 out_need_points;	// mic输出点数统计
        u32 out_cur_points;		// 当前输出点数
        u32 out_mute_points;	// 静音点数
        cbuffer_t out_cbuf;		// 输出循环buf
#if PHONE_MESSAGE_ENC_USE_TASK
        volatile u8 enc_task_stop;	// 留言任务stop
        u8 enc_task_init_ok : 1;	// 留言任务初始化完成
        u8 enc_write_err : 1;		// 留言任务写出错
        u16 enc_lost;				// 留言写数据丢失统计
        OS_SEM enc_task_sem;		// 留言任务信号量
        cbuffer_t enc_cbuf;			// 留言写数循环buf
#endif
        u8 *use_buf;		// 使用buf
        u8  use_buf_type;	// 使用buf类型
        u32 energy_limit;	// 能量阀值
        u16 energy_max;		// 连续最大能量不达标次数
        u16 energy_cnt;		// 能量不达标计数
        unsigned long max_time;		// 留言最大时间限制
        unsigned long min_time;		// 留言最小时间限制
        unsigned long pause_time;	// 临时停了多长时间
        unsigned long start_limit_time;	// 起始提示音启动限制时间
        void *evt_priv;		// 事件回调私有句柄
        int (*evt_cb)(void *priv, int event, int *param);	// 事件回调
    };

    struct phone_message_dec_hdl {
    u32 status : 4;		// 播放状态
    u32 del_enc : 1;	// 是否删除文件
    u32 limit_time_check : 1;	// 获取留言信息时间限制使能
    u32 tws : 1;			// 1:tws模式；0:普通模式，本地播放
    u32 tws_master : 1;		// 主机。留言所在的机器
    u32 tws_file_end : 1;	// 留言文件已经读完。并不意味着tws已经发送完
    u32 tws_no_put_close : 1;	// 不推送close消息
    void *file_hdl;			// 文件句柄
    struct audio_dec_app_hdl *dec;	// 解码句柄
    char *name_mic_have;	// 有留言 提示音
    char *name_mic_none;	// 没有留言 提示音
    u8 pkt_len;			// 留言数据帧长
    u8 phone_num[12];	// 留言电话号码
    u16 sample_rate;	// 留言数据采样率
    u32 coding_type;	// 留言数据类型
    unsigned long limit_time;	// 获取留言信息时间限制
    u32 create_time;	// 当前解码创建时的时间

#if PHONE_MESSAGE_TWS_ENABLE
    void *tws_buf;		// tws buf
    u8 *tws_pkt_data;	// tws数据起始位置
    int tws_pkt_len;	// tws数据总长
    int tws_pkt_ptr;	// tws数据已经使用长度
#endif
};

//////////////////////////////////////////////////////////////////////////////
extern u8  phone_message_have;
extern u8  phone_message_idx;
extern struct phone_message_hdl *phone_message;
extern struct phone_message_dec_hdl *phone_message_dec;

//////////////////////////////////////////////////////////////////////////////
// call
int phone_message_call_open(void);
void phone_message_close(void);
struct phone_message_hdl *phone_message_create(void);
int phone_message_mic_write(s16 *data, int len); // 负数：非电话留言
int phone_message_output_read(s16 *data, int len); // 负数：非电话留言
int phone_message_enc_write(u8 *data, int len); // 负数：非电话留言
void phone_message_enc_file_del(void);

//////////////////////////////////////////////////////////////////////////////
// play
void phone_message_dec_close(void);
struct phone_message_dec_hdl *phone_massage_dec_create(void);
int phone_massage_dec(struct phone_message_dec_hdl *dec, u8 have);
int phone_message_dec_open(u8 del_enc);

//////////////////////////////////////////////////////////////////////////////
// api
void phone_message_call_api_close(void);
int phone_message_call_api_open(void);
int phone_message_call_api_start(void);
int phone_message_call_api_stop(void);
int phone_message_call_api_set_info(u32 coding_type, u16 sr, u8 *num);
int phone_message_call_api_get_info(u32 *p_coding_type, u16 *p_sr, u8 *num, u8 num_len);
int phone_message_call_api_is_work(void);
int phone_message_call_api_esco_out_data(s16 *data, int len);

void phone_message_play_api_close(void);
int phone_message_play_api_open(u8 del_enc);
int phone_message_play_api_get_info(u32 *p_coding_type, u16 *p_sr, u8 *num, u8 num_len, u8 always);
int phone_message_play_api_is_work(void);

//////////////////////////////////////////////////////////////////////////////
// tws
int phone_message_tws_call_open(void);

int phone_message_tws_play_open(u8 have, u8 del_enc);
void phone_message_tws_play_close(void);

void phone_message_tws_enc_del(void);

int phone_message_tws_event_deal(struct bt_event *evt);

// tws dec
int phone_message_tws_dec_init(struct phone_message_dec_hdl *play);
void phone_message_tws_dec_release(struct phone_message_dec_hdl *play);
int phone_message_tws_dec_trans_read(struct phone_message_dec_hdl *play, void *buf, u32 len);
int phone_message_tws_dec_probe(struct phone_message_dec_hdl *play);
void phone_message_tws_dec_output_data(struct phone_message_dec_hdl *play, void *data, int len);

#endif /*(TCFG_PHONE_MESSAGE_ENABLE)*/

#endif /*_PHONE_MESSAGE_*/

