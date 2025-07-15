
#ifndef _DRC_API_H_
#define _DRC_API_H_

#include "typedef.h"
#include "sw_drc.h"

struct audio_drc_filter_info {
    struct drc_ch *pch;         //左声道系数
    struct drc_ch *R_pch;       //右声道系数
};

#if (defined(EQ_CORE_V1)|| defined(EQ_CORE_V2))
typedef int (*audio_drc_filter_cb)(void *drc, struct audio_drc_filter_info *info);
#else
typedef int (*audio_drc_filter_cb)(struct audio_drc_filter_info *info);
#endif

struct audio_drc_param {
    u32 channels : 8;           //通道数 (channels|(L_wdrc))或(channels|(R_wdrc))
    u32 online_en : 1;          //是否支持在线调试
    u32 remain_en : 1;          //写1
    u32 stero_div : 1;          //是否左右声道 拆分的  drc效果,一般写0
    u32 reserved : 21;
    audio_drc_filter_cb cb;     //系数更新的回调函数，用户赋值
    u8 drc_name;                //在线调试是，根据drc_name区分更新系数,一般写0
#if (defined(EQ_CORE_V1)|| defined(EQ_CORE_V2))
    u8 out_32bit;               //是否支持32bit 的输入数据处理  1:使能  0：不使能
    u32 sr;                     //数据采样率
#endif
};

struct audio_drc {
    struct drc_ch sw_drc[2];    //软件drc 系数地址
    u32 sr;                    //采样率
    u32 channels : 8;           //通道数(channels|(L_wdrc))或(channels|(R_wdrc))
    u32 remain_flag : 1;        //输出数据支持remain
    u32 updata : 1;             //系数更标志
    u32 online_en : 1;          //是否支持在线更新系数
    u32 remain_en : 1;          //是否支持remain输出
    u32 start : 1;              //无效
    u32 run32bit : 8;           //是否使能32bit位宽数据处理1:使能  0：不使能
    u32 need_restart : 1;       //是否需要重更新系数 1:是  0：否
    u32 stero_div : 1;          //是否左右声道拆分的drc效果,1：是  0：否
    audio_drc_filter_cb cb;     //系数更新回调
    void *hdl;                  //软件drc句柄
    void *output_priv;          //输出回调私有指针
    int (*output)(void *priv, void *data, u32 len);//输出回调函数
    u32 drc_name;               //drc标识
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
/**@brief    audio_drc_open重新封装，简化使用
   @param    *parm: drc参数句柄,参数详见结构体struct audio_drc_param
   @return   eq句柄
   @note
*/
/*----------------------------------------------------------------------------*/
struct audio_drc *audio_dec_drc_open(struct audio_drc_param *parm);

/*----------------------------------------------------------------------------*/
/**@brief    audio_drc_close重新封装，简化使用
   @param    drc句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_dec_drc_close(struct audio_drc *drc);


#if (defined(EQ_CORE_V1)|| defined(EQ_CORE_V2))
#else
void drc_app_run_check(struct audio_drc *drc);
int drc_get_filter_info(struct audio_drc_filter_info *info);
#endif

#endif

