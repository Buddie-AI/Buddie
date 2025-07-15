/*************************************************************************************************/
/*!
*  \file      audio_effect_develop.c
*
*  \brief     第三方音效算法添加
*
*  Copyright (c) 2011-2023 ZhuHai Jieli Technology Co.,Ltd.
*
*/
/*************************************************************************************************/
#include "app_config.h"
#include "audio_effect_develop.h"

#if ((defined TCFG_EFFECT_DEVELOP_ENABLE) && TCFG_EFFECT_DEVELOP_ENABLE)
struct audio_effect_develop_handle {
    int sample_rate;
    u8 nch;
    u8 bit_width;
};

void *audio_effect_develop_open(int sample_rate, u8 nch, u8 bit_width)
{
    struct audio_effect_develop_handle *hdl = (struct audio_effect_develop_handle *)zalloc(sizeof(struct audio_effect_develop_handle));

    if (!hdl) {
        return NULL;
    }

    hdl->sample_rate = sample_rate;
    hdl->nch = nch;
    hdl->bit_width = bit_width;

    //TODO : 打开算法模块

    return hdl;
}

void audio_effect_develop_close(void *priv)
{
    struct audio_effect_develop_handle *hdl = (struct audio_effect_develop_handle *)priv;

    //TODO : 关闭算法模块

    if (hdl) {
        free(hdl);
    }
}

/*********************************************************************** 
 * 音效算法开发处理函数 
 * Input    :  priv - 第三方音效算法的主要私有句柄 
               data - pcm数据(16bit) 
               len  - pcm数据的byte长度 
 * Output   :  数据处理长度 
 * Notes    : 
 * History  : 
 *==============================================================*/  
int audio_effect_develop_data_handler(void *priv, void *data, int len)
{
    struct audio_effect_develop_handle *hdl = (struct audio_effect_develop_handle *)priv;

    //TODO : 音效运行

    return len;
}

#endif
