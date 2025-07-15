/*****************************************************************
>file name : kws_event.h
>author : lichao
>create time : Fri 05 Nov 2021 10:11:40 AM CST
*****************************************************************/
#ifndef _AUDIO_KWS_EVENT_
#define _AUDIO_KWS_EVENT_

enum audio_kws_event {
    KWS_EVENT_NULL = 0,

    /*Hey xxx系列关键词*/
    KWS_EVENT_HEY_KEYWORD,

    /*杰理唤醒词*/
    KWS_EVENT_XIAOJIE,

    /*百度 -- 小度小度等系列命令词消息*/
    KWS_EVENT_XIAODU,       /*小度小度*/

    /*音乐关键词*/
    KWS_EVENT_PLAY_MUSIC,   /*播放音乐*/
    KWS_EVENT_STOP_MUSIC,   /*停止播放*/
    KWS_EVENT_PAUSE_MUSIC,  /*暂停播放*/
    KWS_EVENT_VOLUME_UP,    /*增大音量*/
    KWS_EVENT_VOLUME_DOWN,  /*减小音量*/
    KWS_EVENT_PREV_SONG,    /*上一首*/
    KWS_EVENT_NEXT_SONG,    /*下一首*/

    /*通话关键词*/
    KWS_EVENT_CALL_ACTIVE,  /*接听电话*/
    KWS_EVENT_CALL_HANGUP,  /*挂断电话*/

    /*ANC系列关键词词*/
    KWS_EVENT_ANC_ON,       /*打开降噪*/
    KWS_EVENT_ANC_OFF,      /*关闭降噪*/
    KWS_EVENT_TRANSARENT_ON,/*打开通透*/

    /*TODO*/

    KWS_EVENT_MAX,
};

#endif
