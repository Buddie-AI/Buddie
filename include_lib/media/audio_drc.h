#ifndef CONFIG_EFFECT_CORE_V2_ENABLE
#include "application/audio_drc.h"
#else

#ifndef _DRC_API_H_
#define _DRC_API_H_

#include "typedef.h"
#include "media/sw_drc.h"
#include "media/audio_stream.h"
#include "system/init.h"

#define CROSSOVER_EN BIT(0)   //多带分频器使能
#define MORE_BAND_EN BIT(1)   //多带分频后，再做一次全带处理使能
struct audio_drc_filter_info {
    CrossOverParam_TOOL_SET *crossover;
    wdrc_struct_TOOL_SET *wdrc;
    // struct drc_ch *pch;         //左声道系数
    // struct drc_ch *R_pch;       //右声道系数
};

typedef int (*audio_drc_filter_cb)(void *drc, struct audio_drc_filter_info *info);

struct audio_drc_param {
    u32 channels : 8;           //通道数 (channels|(L_wdrc))或(channels|(R_wdrc))
    u32 online_en : 1;          //是否支持在线调试
    u32 remain_en : 1;          //写1
    u32 stero_div : 1;          //是否左右声道 拆分的  drc效果,一般写0
    u32 reserved : 21;
    audio_drc_filter_cb cb;     //系数更新的回调函数，用户赋值
    u32 sr;                     //数据采样率
    u8 drc_name;                //在线调试是，根据drc_name区分更新系数,一般写0
    u8 out_32bit;               //是否支持32bit 的输入数据处理  1:使能  0：不使能
    u8 nband;                   //多带之后，再使能一次 全带处理
    CrossOverParam_TOOL_SET *crossover;
    wdrc_struct_TOOL_SET *wdrc;//[0]low  [1]mid  [2]high  [3]whole
    // struct drc_ch *parm;        //系数输入
};

struct audio_drc {
    CrossOverParam_TOOL_SET *crossover_inside;//
    wdrc_struct_TOOL_SET *wdrc_inside;
    // struct drc_ch sw_drc[2];    //软件drc 系数地址
    // struct drc_ch *drc_param;   //在线调试暂存软件drc系数地址,由外部赋值
    CrossOverParam_TOOL_SET *crossover;//
    wdrc_struct_TOOL_SET *wdrc;

    u32 sr;                     //采样率
    u32 channels : 8;           //通道数(channels|(L_wdrc))或(channels|(R_wdrc))
    u32 remain_flag : 1;        //输出数据支持remain
    u32 updata : 1;             //系数更标志
    u32 online_en : 1;          //是否支持在线更新系数
    u32 remain_en : 1;          //是否支持remain输出
    u32 start : 1;              //无效
    u32 run32bit : 8;           //是否使能32bit位宽数据处理1:使能  0：不使能
    u32 need_restart : 1;       //是否需要重更新系数 1:是  0：否
    u32 stero_div : 1;          //是否左右声道拆分的drc效果,1：是  0：否
    u8 other_band;
    audio_drc_filter_cb cb;     //系数更新回调
    void *hdl;                  //软件drc句柄
    void *output_priv;          //输出回调私有指针
    int (*output)(void *priv, void *data, u32 len);//输出回调函数
    u32 drc_name;               //drc标识
    struct list_head hentry;                         //
    u8 *file_path;              //离线效果文件路径


    struct audio_stream_entry entry;	// 音频流入口
};



/*----------------------------------------------------------------------------*/
/**@brief   drc打开
   @param   drc:句柄
   @param   param:drc打开传入参数
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_drc_open(struct audio_drc *drc, struct audio_drc_param *param);
/*----------------------------------------------------------------------------*/
/**@brief   drc设置输出数据回调
   @param   drc:句柄
   @param   *output:输出回调
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_drc_set_output_handle(struct audio_drc *drc, int (*output)(void *priv, void *data, u32 len), void *output_priv);
/*----------------------------------------------------------------------------*/
/**@brief   drc设置采样率
   @param   drc:句柄
   @param   sr:采样率
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_drc_set_samplerate(struct audio_drc *drc, int sr);
/*----------------------------------------------------------------------------*/
/**@brief   drc设置是否处理32bit输入数据
   @param   drc:句柄
   @param   run_32bit:1：32bit 0:16bit
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_drc_set_32bit_mode(struct audio_drc *drc, u8 run_32bit);
/*----------------------------------------------------------------------------*/
/**@brief   drc启动
   @param   drc:句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_drc_start(struct audio_drc *drc);
/*----------------------------------------------------------------------------*/
/**@brief   drc数据处理
   @param   drc:句柄
   @param   *data:输入数据地址
   @param   len:输入数据长度
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_drc_run(struct audio_drc *drc, s16 *data, u32 len);
/*----------------------------------------------------------------------------*/
/**@brief   drc关闭
   @param   drc:句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_drc_close(struct audio_drc *drc);


/*----------------------------------------------------------------------------*/
/**@brief    audio_drc_open重新封装，简化使用,该接口不接入audio_stream流处理
   @param    *parm: drc参数句柄,参数详见结构体struct audio_drc_param
   @return   eq句柄
   @note
*/
/*----------------------------------------------------------------------------*/
struct audio_drc *audio_dec_drc_open(struct audio_drc_param *parm);
/*----------------------------------------------------------------------------*/
/**@brief    audio_drc_close重新封装，简化使用,该接口不接入audio_stream流处理
   @param    drc句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_dec_drc_close(struct audio_drc *drc);


int audio_dec_drc_run(struct audio_drc *drc, s16 *data, u32 len);
void cur_crossover_set_update(u32 drc_name, CrossOverParam_TOOL_SET *crossover_parm);
void cur_drc_set_update(u32 drc_name, u8 type, void *wdrc_parm);
void cur_drc_set_bypass(u32 drc_name, u8 tar, u8 bypass);
void audio_drc_init(void);
int drc_get_filter_info(void *_drc, struct audio_drc_filter_info *info);
struct audio_drc *get_cur_drc_hdl_by_name(u32 drc_name);
#endif

#endif /*CONFIG_EFFECT_CORE_V2_ENABLE*/
