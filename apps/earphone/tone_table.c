#include "tone_player.h"
#include "system/includes.h"


#ifdef CONFIG_MEDIA_NEW_ENABLE



static const char *const tone_index[] = {
    TONE_NUM_0,
    TONE_NUM_1,
    TONE_NUM_2,
    TONE_NUM_3,
    TONE_NUM_4,
    TONE_NUM_5,
    TONE_NUM_6,
    TONE_NUM_7,
    TONE_NUM_8,
    TONE_NUM_9,
    TONE_BT_MODE,
    TONE_BT_CONN,
    TONE_BT_DISCONN,
    TONE_TWS_CONN,
    TONE_TWS_DISCONN,
    TONE_LOW_POWER,
    TONE_POWER_OFF,
    TONE_POWER_ON,
    TONE_RING,
    TONE_MAX_VOL,
    TONE_NORMAL,
    TONE_ANC_OFF,
    TONE_ANC_ON,
    TONE_TRANSPARENCY,
    TONE_ANC_ADAPTIVE,
    TONE_ANC_ADAPTIVE_COEFF,
    TONE_ANC_NORMAL_COEFF,
};

/*
 * 参数配置:
 * freq : 实际频率 * 512
 * points : 正弦波点数
 * win : 正弦窗
 * decay : 衰减系数(百分比), 正弦窗模式下为频率设置：频率*512
 *
 */
static const struct sin_param sine_16k_normal[] __BANK_TONE = {
    /*{0, 1000, 0, 100},*/
    {1000 << 9, 4000, 0, 100},
};

static const struct sin_param sine_key_tone[] __BANK_TONE = {
    /*sine_make.c/fade_points = 0效果更加*/
    {2800 << 9, 1400, 1, 60 << 9},
};

static const struct sin_param sine_low_latency_in[] __BANK_TONE = {
    /*{0, 1000, 0, 100},*/
    {400 << 9, 4000, 0, 100},
};

static const struct sin_param sine_low_latency_out[] __BANK_TONE = {
    /*{0, 1000, 0, 100},*/
    {600 << 9, 8000, 0, 100},
};

static const struct sin_param sine_tws_disconnect_16k[] __BANK_TONE = {
    /*
    {390 << 9, 4026, SINE_TOTAL_VOLUME / 4026},
    {262 << 9, 8000, SINE_TOTAL_VOLUME / 8000},
    */
    {262 << 9, 4026, 0, 100},
    {390 << 9, 8000, 0, 100},
};

static const struct sin_param sine_tws_connect_16k[] __BANK_TONE = {
    /*
    {262 << 9, 4026, SINE_TOTAL_VOLUME / 4026},
    {390 << 9, 8000, SINE_TOTAL_VOLUME / 8000},
    */
    {262.298 * 512, 8358, 0, 100},
};

static const struct sin_param sine_low_power[] __BANK_TONE = {
    {424 << 9, 3613, 0, 100},
    {319 << 9, 3623, 0, 100},
};

static const struct sin_param sine_ring[] __BANK_TONE = {
    {450 << 9, 24960, 1, 16.667 * 512},
    {0, 16000, 0, 100},
};

static const struct sin_param sine_tws_max_volume[] __BANK_TONE = {
    {210 << 9, 2539, 0, 100},
    {260 << 9, 7619, 0, 100},
    {400 << 9, 2539, 0, 100},
};

static const struct sin_param sine_anc_adaptive_1[] __BANK_TONE = {
    {330 << 9, 2539, 0, 100},	//低 mi
    {392 << 9, 7619, 0, 100},	//低 so
};

static const struct sin_param sine_anc_adaptive_2[] __BANK_TONE = {
    {392 << 9, 2539, 0, 100},	//低 so
    {330 << 9, 7619, 0, 100},	//低 mi
};


__BANK_TONE_ENTRY
static const struct sin_param *get_sine_param_by_index(u8 index, u8 *num)
{
    const struct sin_param *param_data;

    switch (index) {
    case SINE_WTONE_NORAML:
        param_data = sine_16k_normal;
        *num = ARRAY_SIZE(sine_16k_normal);
        break;
    case SINE_WTONE_TWS_CONNECT:
        param_data = sine_tws_connect_16k;
        *num = ARRAY_SIZE(sine_tws_connect_16k);
        break;
    case SINE_WTONE_TWS_DISCONNECT:
        param_data = sine_tws_disconnect_16k;
        *num = ARRAY_SIZE(sine_tws_disconnect_16k);
        break;
    case SINE_WTONE_LOW_POWER:
        param_data = sine_low_power;
        *num = ARRAY_SIZE(sine_low_power);
        break;
    case SINE_WTONE_RING:
        param_data = sine_ring;
        *num = ARRAY_SIZE(sine_ring);
        break;
    case SINE_WTONE_MAX_VOLUME:
        param_data = sine_tws_max_volume;
        *num = ARRAY_SIZE(sine_tws_max_volume);
        break;
    case SINE_WTONE_LOW_LATENRY_IN:
        param_data = sine_low_latency_in;
        *num = ARRAY_SIZE(sine_low_latency_in);
        break;
    case SINE_WTONE_LOW_LATENRY_OUT:
        param_data = sine_low_latency_out;
        *num = ARRAY_SIZE(sine_low_latency_out);
        break;
    case SINE_WTONE_ANC_ADAPTIVE_1:
        param_data = sine_anc_adaptive_1;
        *num = ARRAY_SIZE(sine_anc_adaptive_1);
        break;
    case SINE_WTONE_ANC_ADAPTIVE_2:
        param_data = sine_anc_adaptive_2;
        *num = ARRAY_SIZE(sine_anc_adaptive_2);
        break;

    default:
        return NULL;
    }

    return param_data;
}

/*
 *index:提示音索引
 *preemption:抢断标志
 */
__BANK_TONE_ENTRY
int tone_play_index(u8 index, u8 preemption)
{
    printf("tone_play_index:%d,preemption:%d", index, preemption);
    if (index >= IDEX_TONE_NONE) {
        return 0;
    }
    return tone_play(tone_index[index], preemption);
}
/*
 *@brief:提示音比较，确认目标提示音和正在播放的提示音是否一致
 *@return: 0 	匹配
 *		   非0	不匹配或者当前没有提示音播放
 *@note:通过提示音索引比较
 */
int tone_name_cmp_by_index(u8 index)
{
    if (index >= IDEX_TONE_NONE) {
        return 0;
    }
    return tone_name_compare(tone_index[index]);
}

__BANK_TONE_ENTRY
int tone_play_index_no_tws(u8 index, u8 preemption)
{
    printf("tone_play_index no tws:%d,preemption:%d", index, preemption);
    if (index >= IDEX_TONE_NONE) {
        return 0;
    }
    return tone_play_no_tws(tone_index[index], preemption);
}

/* __BANK_TONE_ENTRY */
int tone_play_index_with_callback(u8 index, u8 preemption, void (*user_evt_handler)(void *priv), void *priv)
{
    printf("tone_play_index:%d,preemption:%d", index, preemption);
    if (index >= IDEX_TONE_NONE) {
        return 0;
    }
    return tone_play_with_callback(tone_index[index], preemption, user_evt_handler, priv);
}



__BANK_INIT_ENTRY
int tone_table_init()
{
    tone_play_set_sine_param_handler(get_sine_param_by_index);
    return 0;
}
__initcall(tone_table_init);

#endif /*CONFIG_MEDIA_NEW_ENABLE*/

