#include "system/includes.h"
#include "media/includes.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "user_cfg.h"
#include "audio_config.h"
#include "audio_dvol.h"
#include "audio_anc.h"
#include "audio_output_dac.h"
#include "asm/math_fast_function.h"

#define LOG_TAG             "[APP_AUDIO]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define DEFAULT_DIGTAL_VOLUME   16384

typedef short unaligned_u16 __attribute__((aligned(1)));
struct app_audio_config {
    u8 state;
    u8 prev_state;
    u8 mute_when_vol_zero;
    volatile u8 fade_gain_l;
    volatile u8 fade_gain_r;
    volatile s16 fade_dgain_l;
    volatile s16 fade_dgain_r;
    volatile s16 fade_dgain_step_l;
    volatile s16 fade_dgain_step_r;
    volatile int fade_timer;
    s16 digital_volume;
    u8 analog_volume_l;
    u8 analog_volume_r;
    atomic_t ref;
    s16 max_volume[APP_AUDIO_STATE_WTONE + 1];
    u8 sys_cvol_max;
    u8 call_cvol_max;
    u16 sys_hw_dvol_max;	//系统最大硬件数字音量(非通话模式)
    u16 call_hw_dvol_max;	//通话模式最大硬件数字音量
    unaligned_u16 *sys_cvol;
    unaligned_u16 *call_cvol;
};
static const char *audio_state[] = {
    "idle",
    "music",
    "call",
    "tone",
    "err",
};

static struct app_audio_config app_audio_cfg = {0};

#define __this      (&app_audio_cfg)
extern struct audio_dac_hdl dac_hdl;
extern struct dac_platform_data dac_data;

/*
 *************************************************************
 *
 *	audio volume fade
 *
 *************************************************************
 */

static void audio_fade_timer(void *priv)
{
    u8 gain_l = dac_hdl.vol_l;
    u8 gain_r = dac_hdl.vol_r;
    //printf("<fade:%d-%d-%d-%d>", gain_l, gain_r, __this->fade_gain_l, __this->fade_gain_r);
    local_irq_disable();
    if ((gain_l == __this->fade_gain_l) && (gain_r == __this->fade_gain_r)) {
        usr_timer_del(__this->fade_timer);
        __this->fade_timer = 0;
        /*音量为0的时候mute住*/
        audio_dac_set_L_digital_vol(&dac_hdl, gain_l ? __this->digital_volume : 0);
        audio_dac_set_R_digital_vol(&dac_hdl, gain_r ? __this->digital_volume : 0);
        if ((gain_l == 0) && (gain_r == 0)) {
            if (__this->mute_when_vol_zero) {
                __this->mute_when_vol_zero = 0;
                audio_dac_mute(&dac_hdl, 1);
            }
        }
        local_irq_enable();
        /*
         *淡入淡出结束，也把当前的模拟音量设置一下，防止因为淡入淡出的音量和保存音量的变量一致，
         *而寄存器已经被清0的情况
         */
        audio_dac_set_analog_vol(&dac_hdl, gain_l);
        /* log_info("dac_fade_end, VOL : 0x%x 0x%x\n", JL_ADDA->DAA_CON1, JL_AUDIO->DAC_VL0); */
        return;
    }
    if (gain_l > __this->fade_gain_l) {
        gain_l--;
    } else if (gain_l < __this->fade_gain_l) {
        gain_l++;
    }

    if (gain_r > __this->fade_gain_r) {
        gain_r--;
    } else if (gain_r < __this->fade_gain_r) {
        gain_r++;
    }
    audio_dac_set_analog_vol(&dac_hdl, gain_l);
    local_irq_enable();
}

static int audio_fade_timer_add(u8 gain_l, u8 gain_r)
{
    /* r_printf("dac_fade_begin:0x%x,gain_l:%d,gain_r:%d\n", __this->fade_timer, gain_l, gain_r); */
    local_irq_disable();
    __this->fade_gain_l = gain_l;
    __this->fade_gain_r = gain_r;
    if (__this->fade_timer == 0) {
        __this->fade_timer = usr_timer_add((void *)0, audio_fade_timer, 2, 1);
        /* y_printf("fade_timer:0x%x", __this->fade_timer); */
    }
    local_irq_enable();

    return 0;
}


#if (SYS_VOL_TYPE == VOL_TYPE_AD)

#define DGAIN_SET_MAX_STEP (300)
#define DGAIN_SET_MIN_STEP (30)

static unsigned short combined_vol_list[17][2] = {
    { 0,     0}, //0: None
    { 0,   412}, // 1:-50.00 db
    { 0,   604}, // 2:-46.67 db
    { 0,   887}, // 3:-43.33 db
    { 0,  1301}, // 4:-40.00 db
    { 0,  1910}, // 5:-36.67 db
    { 0,  2804}, // 6:-33.33 db
    { 0,  4115}, // 7:-30.00 db
    { 0,  6041}, // 8:-26.67 db
    { 0,  8867}, // 9:-23.33 db
    { 0, 13014}, // 10:-20.00 db
    { 1,  9574}, // 11:-16.67 db
    { 1, 14052}, // 12:-13.33 db
    { 2, 10338}, // 13:-10.00 db
    { 2, 15174}, // 14:-6.67 db
    { 3, 11162}, // 15:-3.33 db
    { 3, 16384}, // 16:0.00 db
};
static unsigned short call_combined_vol_list[16][2] = {
    { 0,     0}, //0: None
    { 0,   412}, // 1:-50.00 db
    { 0,   604}, // 2:-46.67 db
    { 0,   887}, // 3:-43.33 db
    { 0,  1301}, // 4:-40.00 db
    { 0,  1910}, // 5:-36.67 db
    { 0,  2804}, // 6:-33.33 db
    { 0,  4115}, // 7:-30.00 db
    { 0,  6041}, // 8:-26.67 db
    { 0,  8867}, // 9:-23.33 db
    { 0, 13014}, // 10:-20.00 db
    { 1,  9574}, // 11:-16.67 db
    { 1, 14052}, // 12:-13.33 db
    { 2, 10338}, // 13:-10.00 db
    { 2, 15174}, // 14:-6.67 db
    { 3, 11162}, // 15:-3.33 db
};

void audio_combined_vol_init(u8 cfg_en)
{
    u16 sys_cvol_len = 0;
    u16 call_cvol_len = 0;
    u8 *sys_cvol  = NULL;
    u8 *call_cvol  = NULL;
    unaligned_u16 *cvol;

    __this->sys_cvol_max = ARRAY_SIZE(combined_vol_list) - 1;
    __this->sys_cvol = combined_vol_list;
    __this->call_cvol_max = ARRAY_SIZE(call_combined_vol_list) - 1;
    __this->call_cvol = call_combined_vol_list;

    if (cfg_en) {
        sys_cvol  = syscfg_ptr_read(CFG_COMBINE_SYS_VOL_ID, &sys_cvol_len);
        //ASSERT(((u32)sys_cvol & BIT(0)) == 0, "sys_cvol addr unalignd(2):%x\n", sys_cvol);
        if (sys_cvol && sys_cvol_len) {
            __this->sys_cvol = sys_cvol;
            __this->sys_cvol_max = sys_cvol_len / 4 - 1;
            //y_printf("read sys_combine_vol succ:%x,len:%d",__this->sys_cvol,sys_cvol_len);
            /* cvol = __this->sys_cvol;
            for(int i = 0,j = 0;i < (sys_cvol_len / 2);j++) {
            	printf("sys_vol %d: %d, %d\n",j,*cvol++,*cvol++);
            	i += 2;
            } */
        } else {
            r_printf("read sys_cvol false:%x,%x\n", sys_cvol, sys_cvol_len);
        }

        call_cvol  = syscfg_ptr_read(CFG_COMBINE_CALL_VOL_ID, &call_cvol_len);
        //ASSERT(((u32)call_cvol & BIT(0)) == 0, "call_cvol addr unalignd(2):%d\n", call_cvol);
        if (call_cvol && call_cvol_len) {
            __this->call_cvol = (unaligned_u16 *)call_cvol;
            __this->call_cvol_max = call_cvol_len / 4 - 1;
            //y_printf("read call_combine_vol succ:%x,len:%d",__this->call_cvol,call_cvol_len);
            /* cvol = __this->call_cvol;
            for(int i = 0,j = 0;i < call_cvol_len / 2;j++) {
            	printf("call_vol %d: %d, %d\n",j,*cvol++,*cvol++);
            	i += 2;
            } */
        } else {
            r_printf("read call_combine_vol false:%x,%x\n", call_cvol, call_cvol_len);
        }
    }

    log_info("sys_cvol_max:%d,call_cvol_max:%d\n", __this->sys_cvol_max, __this->call_cvol_max);
}

static int audio_combined_fade_timer_add(u8 gain_l, u8 gain_r)
{
    u8  gain_max;
    u8  target_again_l = 0;
    u8  target_again_r = 0;
    u16 target_dgain_l = 0;
    u16 target_dgain_r = 0;

    if (__this->state == APP_AUDIO_STATE_CALL) {
        gain_max = __this->call_cvol_max;
        gain_l = (gain_l > gain_max) ? gain_max : gain_l;
        gain_r = (gain_r > gain_max) ? gain_max : gain_r;
        target_again_l = *(&__this->call_cvol[gain_l * 2]);
        target_again_r = *(&__this->call_cvol[gain_r * 2]);
        target_dgain_l = *(&__this->call_cvol[gain_l * 2 + 1]);
        target_dgain_r = *(&__this->call_cvol[gain_r * 2 + 1]);
    } else {
        gain_max = __this->sys_cvol_max;
        gain_l = (gain_l > gain_max) ? gain_max : gain_l;
        gain_r = (gain_r > gain_max) ? gain_max : gain_r;
        target_again_l = *(&__this->sys_cvol[gain_l * 2]);
        target_again_r = *(&__this->sys_cvol[gain_r * 2]);
        target_dgain_l = *(&__this->sys_cvol[gain_l * 2 + 1]);
        target_dgain_r = *(&__this->sys_cvol[gain_r * 2 + 1]);
    }
#if 0//TCFG_AUDIO_ANC_ENABLE
    target_again_l = anc_dac_gain_get(ANC_DAC_CH_L);
    target_again_r = anc_dac_gain_get(ANC_DAC_CH_R);
#endif

    printf("[l]v:%d,Av:%d,Dv:%d", gain_l, target_again_l, target_dgain_l);
    //y_printf("[r]v:%d,Av:%d,Dv:%d", gain_r, target_again_r, target_dgain_r);
    /* log_info("dac_com_fade_begin:0x%x\n", __this->fade_timer); */

    local_irq_disable();

    __this->fade_gain_l  = target_again_l;
    __this->fade_gain_r  = target_again_r;
    __this->fade_dgain_l = target_dgain_l;
    __this->fade_dgain_r = target_dgain_r;

    if (__this->fade_timer == 0) {
        /* __this->fade_timer = usr_timer_add((void *)0, audio_combined_fade_timer, 2, 1); */
        /* log_info("combined_fade_timer:0x%x", __this->fade_timer); */
    }

    audio_dac_vol_set(TYPE_DAC_AGAIN, 0x3, __this->fade_gain_l, 1);
    audio_dac_vol_set(TYPE_DAC_DGAIN, 0x3, __this->fade_dgain_l, 1);

    local_irq_enable();

    return 0;
}

#endif/*SYS_VOL_TYPE == VOL_TYPE_AD*/

#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL_HW)
#define DVOL_HW_LEVEL_MAX	31	/*注意:总共是(DVOL_HW_LEVEL_MAX + 1)级*/
u16 hw_dig_vol_table[DVOL_HW_LEVEL_MAX + 1] = {
    0	, //0
    93	, //1
    111	, //2
    132	, //3
    158	, //4
    189	, //5
    226	, //6
    270	, //7
    323	, //8
    386	, //9
    462	, //10
    552	, //11
    660	, //12
    789	, //13
    943	, //14
    1127, //15
    1347, //16
    1610, //17
    1925, //18
    2301, //19
    2751, //20
    3288, //21
    3930, //22
    4698, //23
    5616, //24
    6713, //25
    8025, //26
    9592, //27
    10222,//28
    14200,//29
    16000,//30
    16384 //31
};

//float dB_Convert_Mag(float x)
/* #define DIG_VOL_MAX_VALUE       (0.0f)  // 数字音量最大值(单位:dB) */
/* #define DIG_VOL_STEP            (1.5f)  // 逐级递减差值(单位:dB) */
void audio_hw_digital_vol_init(u8 cfg_en)
{
    float dB_value = DIG_VOL_MAX_VALUE;
#if (TCFG_AUDIO_ANC_ENABLE)
    dB_value = (dB_value > ANC_MODE_DIG_VOL_LIMIT) ? ANC_MODE_DIG_VOL_LIMIT : dB_value;
#endif/*TCFG_AUDIO_ANC_ENABLE*/
#if 0
    int vol = get_max_sys_vol();
    float temp = 0;
    //printf("hw digital volume list:\n");
    while (vol) {
        temp = 16384.0f * dB_Convert_Mag(dB_value);
        hw_dig_vol_table[vol] = (u16)temp;
        //printf("[%d] dB:%.1f %.1f %d\n", vol, dB_value, temp, hw_dig_vol_table[vol]);
        dB_value -= DIG_VOL_STEP;
        vol--;
    }
    hw_dig_vol_table[0] = 0;
#else
    app_var.aec_dac_gain = (app_var.aec_dac_gain > BT_CALL_VOL_LEAVE_MAX) ? BT_CALL_VOL_LEAVE_MAX : app_var.aec_dac_gain;
    __this->sys_hw_dvol_max = (u16)(16384.f * dB_Convert_Mag(dB_value));
    float call_hw_dvol_max_dB = DIG_VOL_MAX_VALUE + (BT_CALL_VOL_LEAVE_MAX - app_var.aec_dac_gain) * BT_CALL_VOL_STEP;
    printf("aec_dac_gain:%d,call_hw_dvol_max_dB:%.1f\n", app_var.aec_dac_gain, call_hw_dvol_max_dB);
    __this->call_hw_dvol_max = (u16)(16384.f * dB_Convert_Mag(call_hw_dvol_max_dB));
    printf("sys_hw_dvol_max:%d,call_hw_dvol_max:%d\n", __this->sys_hw_dvol_max, __this->call_hw_dvol_max);
#endif
}

#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL_HW*/


#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)

#define DVOL_SW_LEVEL_MAX	31	/*注意:总共是(DVOL_SW_LEVEL_MAX + 1)级*/
u16 sw_dig_vol_table[DVOL_SW_LEVEL_MAX + 1] = {0};

void audio_sw_digital_vol_init(u8 cfg_en)
{
    printf("audio_sw_digital_vol_init,user-defined[%d]\n", SW_DIG_VOL_TAB_USER_DEFINED);
    float dB_value = DIG_VOL_MAX_VALUE;
#if (TCFG_AUDIO_ANC_ENABLE)
    dB_value = (dB_value > ANC_MODE_DIG_VOL_LIMIT) ? ANC_MODE_DIG_VOL_LIMIT : dB_value;
#endif/*TCFG_AUDIO_ANC_ENABLE*/

#if SW_DIG_VOL_TAB_USER_DEFINED
    int vol = get_max_sys_vol();
    float temp = 0;
    //printf("sw digital volume list:\n");
    while (vol) {
        temp = 16384.0f * dB_Convert_Mag(dB_value);
        sw_dig_vol_table[vol] = (u16)temp;
        //printf("[%d] dB:%.1f %.1f %d\n", vol, dB_value, temp, sw_dig_vol_table[vol]);
        dB_value -= DIG_VOL_STEP;
        vol--;
    }
    sw_dig_vol_table[0] = 0;
    __this->sys_hw_dvol_max = DEFAULT_DIGITAL_VOLUME;
    audio_digital_vol_init(sw_dig_vol_table, get_max_sys_vol());
#else
    __this->sys_hw_dvol_max = (u16)(16384.0f * dB_Convert_Mag(dB_value));
    audio_digital_vol_init(NULL, 0);
#endif/*SW_DIG_VOL_TAB_USER_DEFINED*/

    app_var.aec_dac_gain = (app_var.aec_dac_gain > BT_CALL_VOL_LEAVE_MAX) ? BT_CALL_VOL_LEAVE_MAX : app_var.aec_dac_gain;
    __this->call_hw_dvol_max = (u16)(__this->sys_hw_dvol_max * dB_Convert_Mag((BT_CALL_VOL_LEAVE_MAX - app_var.aec_dac_gain) * BT_CALL_VOL_STEP));
    printf("sys_hw_dvol_max:%d,call_hw_dvol_max:%d\n", __this->sys_hw_dvol_max, __this->call_hw_dvol_max);
}
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/

void audio_volume_list_init(u8 cfg_en)
{
#if (SYS_VOL_TYPE == VOL_TYPE_AD)
    audio_combined_vol_init(cfg_en);
#elif (SYS_VOL_TYPE == VOL_TYPE_DIGITAL_HW)
    audio_hw_digital_vol_init(cfg_en);
#elif (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    audio_sw_digital_vol_init(cfg_en);
#endif/*SYS_VOL_TYPE*/
}

static void set_audio_device_volume(u8 type, s16 vol)
{
    audio_dac_set_analog_vol(&dac_hdl, vol);
}

static int get_audio_device_volume(u8 vol_type)
{
    return 0;
}

void volume_up_down_direct(s8 value)
{

}

/*
*********************************************************************
*          			Audio Volume Fade
* Description: 音量淡入淡出
* Arguments  : left_vol
*			   right_vol
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_fade_in_fade_out(u8 left_vol, u8 right_vol)
{
    /*根据audio state切换的时候设置的最大音量,限制淡入淡出的最大音量*/
    u8 max_vol_l = __this->max_volume[__this->state];
    u8 max_vol_r = max_vol_l;
    /*printf("[fade]state:%s,max_volume:%d,cur:%d,%d", audio_state[__this->state], max_vol_l, left_vol, left_vol);*/
    /*淡入淡出音量限制*/
    u8 left_gain = left_vol > max_vol_l ? max_vol_l : left_vol;
    u8 right_gain = right_vol > max_vol_r ? max_vol_r : right_vol;

    /*数字音量*/
#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    u8 dvol_idx = 0; //记录音量通道供数字音量控制使用
    s8 volume = right_gain;
    switch (__this->state) {
    case APP_AUDIO_STATE_IDLE:
    case APP_AUDIO_STATE_MUSIC:
        dvol_idx = MUSIC_DVOL;
        break;
    case APP_AUDIO_STATE_CALL:
        dvol_idx = CALL_DVOL;
        break;
    case APP_AUDIO_STATE_WTONE:
        dvol_idx = TONE_DVOL;
        break;
    default:
        break;
    }

    if (volume > __this->max_volume[__this->state]) {
        volume = __this->max_volume[__this->state];
    }
    /*printf("set_vol[%s]:=%d\n", audio_state[__this->state], volume);*/

    /*按照配置限制通话时候spk最大音量*/
    if (__this->state == APP_AUDIO_STATE_CALL) {
        __this->digital_volume = __this->call_hw_dvol_max;
    } else {
        __this->digital_volume = __this->sys_hw_dvol_max;
    }
    /*printf("[SW_DVOL]Gain:%d,AVOL:%d,DVOL:%d\n", left_gain, __this->analog_volume_l, __this->digital_volume);*/
    audio_dac_vol_set(TYPE_DAC_AGAIN, 0x3, __this->analog_volume_l, 1);
    audio_dac_vol_set(TYPE_DAC_DGAIN, 0x3, __this->digital_volume, 1);
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/

    /*模拟数字联合音量*/
#if (SYS_VOL_TYPE == VOL_TYPE_AD)
#if TCFG_CALL_USE_DIGITAL_VOLUME
    if (__this->state == APP_AUDIO_STATE_CALL) {
        audio_fade_timer_add(left_gain, right_gain);
    } else {
        audio_combined_fade_timer_add(left_gain, right_gain);
    }
#else
    audio_combined_fade_timer_add(left_gain, right_gain);
#endif/*TCFG_CALL_USE_DIGITAL_VOLUME*/
#endif/*SYS_VOL_TYPE == VOL_TYPE_AD*/

#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL_HW)
    /* u8 dvol_hw_level = 0; */
    /* dvol_hw_level = left_gain * DVOL_HW_LEVEL_MAX / get_max_sys_vol(); */
    /* __this->digital_volume = hw_dig_vol_table[dvol_hw_level]; */
#if 0
    __this->digital_volume = hw_dig_vol_table[left_gain];
    /* audio_fade_timer_add(__this->analog_volume_l, __this->analog_volume_r); */
#else
    float dvol_db = 0;
    if (left_gain) {
        if (__this->state == APP_AUDIO_STATE_CALL) {
            dvol_db = (BT_CALL_VOL_LEAVE_MAX - left_gain) * BT_CALL_VOL_STEP;
            __this->digital_volume = (s16)(__this->call_hw_dvol_max * dB_Convert_Mag(dvol_db));
        } else {
            dvol_db = (BT_MUSIC_VOL_LEAVE_MAX - left_gain) * BT_MUSIC_VOL_STEP;
            __this->digital_volume = (s16)(__this->sys_hw_dvol_max * dB_Convert_Mag(dvol_db));
        }
    } else {
        __this->digital_volume = 0;
    }
#endif
    printf("[HW_DVOL]Gain:%d,AVOL:%d,DVOL:%d\n", left_gain, __this->analog_volume_l, __this->digital_volume);
    audio_dac_vol_set(TYPE_DAC_AGAIN, 0x3, __this->analog_volume_l, 1);
    audio_dac_vol_set(TYPE_DAC_DGAIN, 0x3, __this->digital_volume, 1);
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL_HW*/
}

/*
*********************************************************************
*          			Audio Volume Set
* Description: 音量设置
* Arguments  : state	目标声音通道
*			   volume	目标音量值
*			   fade		是否淡入淡出
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void app_audio_set_volume(u8 state, s8 volume, u8 fade)
{
    u8 dvol_idx = 0; //记录音量通道供数字音量控制使用
    switch (state) {
    case APP_AUDIO_STATE_IDLE:
    case APP_AUDIO_STATE_MUSIC:
        app_var.music_volume = volume;
        dvol_idx = MUSIC_DVOL;
        break;
    case APP_AUDIO_STATE_CALL:
        app_var.call_volume = volume;
        dvol_idx = CALL_DVOL;
#if TCFG_CALL_USE_DIGITAL_VOLUME
        audio_digital_vol_set(volume);
        return;
#endif/*TCFG_CALL_USE_DIGITAL_VOLUME*/
        break;
    case APP_AUDIO_STATE_WTONE:
        app_var.wtone_volume = volume;
        dvol_idx = TONE_DVOL;
        break;
    default:
        return;
    }
    printf("set_vol[%s]:%s=%d\n", audio_state[__this->state], audio_state[state], volume);

#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    if (volume > __this->max_volume[state]) {
        volume = __this->max_volume[state];
    }
    audio_digital_vol_set(dvol_idx, volume);
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/

    if (state == __this->state) {
        audio_dac_set_volume(&dac_hdl, volume);
        audio_fade_in_fade_out(volume, volume);
    }
}

/*
*********************************************************************
*                  Audio Volume Get
* Description: 音量获取
* Arguments  : state	要获取音量值的音量状态
* Return	 : 返回指定状态对应得音量值
* Note(s)    : None.
*********************************************************************
*/
s8 app_audio_get_volume(u8 state)
{
    s8 volume = 0;
    switch (state) {
    case APP_AUDIO_STATE_IDLE:
    case APP_AUDIO_STATE_MUSIC:
        volume = app_var.music_volume;
        break;
    case APP_AUDIO_STATE_CALL:
        volume = app_var.call_volume;
        break;
    case APP_AUDIO_STATE_WTONE:
        volume = app_var.wtone_volume;
        if (!volume) {
            volume = app_var.music_volume;
        }
        break;
    case APP_AUDIO_CURRENT_STATE:
        volume = app_audio_get_volume(__this->state);
        break;
    default:
        break;
    }

    return volume;
}


/*
*********************************************************************
*                  Audio Mute
* Description: 静音控制
* Arguments  : value Mute操作
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
static const char *audio_mute_string[] = {
    "mute_default",
    "unmute_default",
    "mute_L",
    "unmute_L",
    "mute_R",
    "unmute_R",
};

#define AUDIO_MUTE_FADE			1
#define AUDIO_UMMUTE_FADE		1
void app_audio_mute(u8 value)
{
    u8 volume = 0;
    printf("audio_mute:%s", audio_mute_string[value]);
    switch (value) {
    case AUDIO_MUTE_DEFAULT:
#if AUDIO_MUTE_FADE
        audio_fade_in_fade_out(0, 0);
        __this->mute_when_vol_zero = 1;
#else
        audio_dac_set_analog_vol(&dac_hdl, 0);
        audio_dac_mute(&dac_hdl, 1);
#endif/*AUDIO_MUTE_FADE*/
        break;
    case AUDIO_UNMUTE_DEFAULT:
#if AUDIO_UMMUTE_FADE
        audio_dac_mute(&dac_hdl, 0);
        volume = app_audio_get_volume(APP_AUDIO_CURRENT_STATE);
        audio_fade_in_fade_out(volume, volume);
#else
        audio_dac_mute(&dac_hdl, 0);
        volume = app_audio_get_volume(APP_AUDIO_CURRENT_STATE);
        audio_dac_set_analog_vol(&dac_hdl, volume);
#endif/*AUDIO_UMMUTE_FADE*/
        break;
    }
}

/*
*********************************************************************
*                  Audio Volume Up
* Description: 增加当前音量通道的音量
* Arguments  : value	要增加的音量值
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void app_audio_volume_up(u8 value)
{
    s16 volume = 0;
    switch (__this->state) {
    case APP_AUDIO_STATE_IDLE:
    case APP_AUDIO_STATE_MUSIC:
        app_var.music_volume += value;
        if (app_var.music_volume > get_max_sys_vol()) {
            app_var.music_volume = get_max_sys_vol();
        }
        volume = app_var.music_volume;
        break;
    case APP_AUDIO_STATE_CALL:
        app_var.call_volume += value;
#if TCFG_CALL_USE_DIGITAL_VOLUME
        audio_digital_vol_set(volume);
        return;
#endif
        if (app_var.call_volume > app_var.aec_dac_gain) {
            app_var.call_volume = app_var.aec_dac_gain;
        }
        volume = app_var.call_volume;

        break;
    case APP_AUDIO_STATE_WTONE:
        app_var.wtone_volume += value;
        if (app_var.wtone_volume > get_max_sys_vol()) {
            app_var.wtone_volume = get_max_sys_vol();
        }
        volume = app_var.wtone_volume;
        break;
    default:
        return;
    }

    app_audio_set_volume(__this->state, volume, 1);
}

/*
*********************************************************************
*                  Audio Volume Down
* Description: 减少当前音量通道的音量
* Arguments  : value	要减少的音量值
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void app_audio_volume_down(u8 value)
{
    s16 volume = 0;
    switch (__this->state) {
    case APP_AUDIO_STATE_IDLE:
    case APP_AUDIO_STATE_MUSIC:
        app_var.music_volume -= value;
        if (app_var.music_volume < 0) {
            app_var.music_volume = 0;
        }
        volume = app_var.music_volume;
        break;
    case APP_AUDIO_STATE_CALL:
        app_var.call_volume -= value;
        if (app_var.call_volume < 0) {
            app_var.call_volume = 0;
        }
        volume = app_var.call_volume;
#if TCFG_CALL_USE_DIGITAL_VOLUME
        audio_digital_vol_set(volume);
        return;
#endif
        break;
    case APP_AUDIO_STATE_WTONE:
        app_var.wtone_volume -= value;
        if (app_var.wtone_volume < 0) {
            app_var.wtone_volume = 0;
        }
        volume = app_var.wtone_volume;
        break;
    default:
        return;
    }

    app_audio_set_volume(__this->state, volume, 1);
}

/*level:0~15*/
static const u16 phone_call_dig_vol_tab[] = {
    0,	//0
    111,//1
    161,//2
    234,//3
    338,//4
    490,//5
    708,//6
    1024,//7
    1481,//8
    2142,//9
    3098,//10
    4479,//11
    6477,//12
    9366,//13
    14955,//14
    16384 //15
};

/*
*********************************************************************
*                  Audio State Switch
* Description: 切换声音状态
* Arguments  : state
*			   max_volume
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void app_audio_state_switch(u8 state, s16 max_volume)
{
    r_printf("audio state old:%s,new:%s,vol:%d\n", audio_state[__this->state], audio_state[state], max_volume);
    printf(">>>>>>>>>>>>>> audio state old:%s,new:%s,vol:%d\n", audio_state[__this->state], audio_state[state], max_volume);

    u8 analog_vol = MAX_ANA_VOL;
#if TCFG_AUDIO_ANC_ENABLE
    analog_vol = anc_dac_gain_get(ANC_DAC_CH_L);	//获取ANC设置的DAC模拟增益
#endif/*TCFG_AUDIO_ANC_ENABLE*/
    __this->prev_state = __this->state;
    __this->state = state;
#if TCFG_CALL_USE_DIGITAL_VOLUME
    if (__this->state == APP_AUDIO_STATE_CALL) {
        audio_digital_vol_open(max_volume, max_volume, 4);
        /*调数字音量的时候，模拟音量定最大*/
        audio_dac_set_analog_vol(&dac_hdl, max_volume);
    }
#endif
    /*限制最大音量*/
    __this->max_volume[state] = max_volume;

#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL_HW)
    __this->analog_volume_l = analog_vol;
    __this->analog_volume_r = analog_vol;
#else
    __this->digital_volume = DEFAULT_DIGITAL_VOLUME;
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL_HW*/

#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    __this->analog_volume_l = MAX_ANA_VOL;
    __this->analog_volume_r = MAX_ANA_VOL;
#if 0//TCFG_AUDIO_ANC_ENABLE
    __this->analog_volume_l = anc_dac_gain_get(ANC_DAC_CH_L);
    __this->analog_volume_r = anc_dac_gain_get(ANC_DAC_CH_R);
    //g_printf("anc mode,vol_l:%d,vol_r:%d", __this->analog_volume_l, __this->analog_volume_r);
#endif/*TCFG_AUDIO_ANC_ENABLE*/
#else
    if (__this->state == APP_AUDIO_STATE_CALL) {
#if TCFG_CALL_USE_DIGITAL_VOLUME
        u8 call_volume_level_max = ARRAY_SIZE(phone_call_dig_vol_tab) - 1;
        app_var.call_volume = (app_var.call_volume > call_volume_level_max) ? call_volume_level_max : app_var.call_volume;
        __this->digital_volume = phone_call_dig_vol_tab[app_var.call_volume];
        printf("call_volume:%d,digital_volume:%d", app_var.call_volume, __this->digital_volume);
        dac_digital_vol_open();
        dac_digital_vol_tab_register(phone_call_dig_vol_tab, ARRAY_SIZE(phone_call_dig_vol_tab));
        /*调数字音量的时候，模拟音量定最大*/
        audio_dac_set_analog_vol(&dac_hdl, max_volume);
#elif (SYS_VOL_TYPE == VOL_TYPE_AD)
        /*通话联合音量调节的时候，最大音量为15，和手机的等级一致*/
        __this->max_volume[state] = BT_CALL_VOL_LEAVE_MAX;
#endif/*TCFG_CALL_USE_DIGITAL_VOLUME*/
    }
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/

    app_audio_set_volume(state, app_audio_get_volume(state), 1);
}

/*
*********************************************************************
*                  Audio State Exit
* Description: 退出当前的声音状态
* Arguments  : state	要退出的声音状态
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void app_audio_state_exit(u8 state)
{
#if TCFG_CALL_USE_DIGITAL_VOLUME
    if (__this->state == APP_AUDIO_STATE_CALL) {
        audio_digital_vol_close();
    }
#endif/*TCFG_CALL_USE_DIGITAL_VOLUME*/
    if (state == __this->state) {
        __this->state = __this->prev_state;
        __this->prev_state = APP_AUDIO_STATE_IDLE;
    } else if (state == __this->prev_state) {
        __this->prev_state = APP_AUDIO_STATE_IDLE;
    }
}

/*
*********************************************************************
*                  Audio Set Max Volume
* Description: 设置最大音量
* Arguments  : state		要设置最大音量的声音状态
*			   max_volume	最大音量
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void app_audio_set_max_volume(u8 state, s16 max_volume)
{
    __this->max_volume[state] = max_volume;
}

/*
*********************************************************************
*                  Audio State Get
* Description: 获取当前声音状态
* Arguments  : None.
* Return	 : 返回当前的声音状态
* Note(s)    : None.
*********************************************************************
*/
u8 app_audio_get_state(void)
{
    return __this->state;
}

/*
*********************************************************************
*                  Audio Volume_Max Get
* Description: 获取当前声音通道的最大音量
* Arguments  : None.
* Return	 : 返回当前的声音通道最大音量
* Note(s)    : None.
*********************************************************************
*/
s16 app_audio_get_max_volume(void)
{
    if (__this->state == APP_AUDIO_STATE_IDLE) {
        return get_max_sys_vol();
    }
#if TCFG_CALL_USE_DIGITAL_VOLUME
    if (__this->state == APP_AUDIO_STATE_CALL) {
        return (ARRAY_SIZE(phone_call_dig_vol_tab) - 1);
    }
#endif/*TCFG_CALL_USE_DIGITAL_VOLUME*/
    return __this->max_volume[__this->state];
}

void app_audio_set_mix_volume(u8 front_volume, u8 back_volume)
{
    /*set_audio_device_volume(AUDIO_MIX_FRONT_VOL, front_volume);
    set_audio_device_volume(AUDIO_MIX_BACK_VOL, back_volume);*/
}
#if 0

void audio_vol_test()
{
    app_set_sys_vol(10, 10);
    log_info("sys vol %d %d\n", get_audio_device_volume(AUDIO_SYS_VOL) >> 16, get_audio_device_volume(AUDIO_SYS_VOL) & 0xffff);
    log_info("ana vol %d %d\n", get_audio_device_volume(AUDIO_ANA_VOL) >> 16, get_audio_device_volume(AUDIO_ANA_VOL) & 0xffff);
    log_info("dig vol %d %d\n", get_audio_device_volume(AUDIO_DIG_VOL) >> 16, get_audio_device_volume(AUDIO_DIG_VOL) & 0xffff);
    log_info("max vol %d %d\n", get_audio_device_volume(AUDIO_MAX_VOL) >> 16, get_audio_device_volume(AUDIO_MAX_VOL) & 0xffff);

    app_set_max_vol(30);
    app_set_ana_vol(25, 24);
    app_set_dig_vol(90, 90);

    log_info("sys vol %d %d\n", get_audio_device_volume(AUDIO_SYS_VOL) >> 16, get_audio_device_volume(AUDIO_SYS_VOL) & 0xffff);
    log_info("ana vol %d %d\n", get_audio_device_volume(AUDIO_ANA_VOL) >> 16, get_audio_device_volume(AUDIO_ANA_VOL) & 0xffff);
    log_info("dig vol %d %d\n", get_audio_device_volume(AUDIO_DIG_VOL) >> 16, get_audio_device_volume(AUDIO_DIG_VOL) & 0xffff);
    log_info("max vol %d %d\n", get_audio_device_volume(AUDIO_MAX_VOL) >> 16, get_audio_device_volume(AUDIO_MAX_VOL) & 0xffff);
}
#endif

void dac_power_on(void)
{
    log_info(">>>dac_power_on:%d", __this->ref.counter);
#if TCFG_AUDIO_DAC_ENABLE
    if (atomic_inc_return(&__this->ref) == 1) {
        audio_dac_open(&dac_hdl);
    }
#endif // #if TCFG_AUDIO_DAC_ENABLE
}

void dac_power_off(void)
{
#if TCFG_AUDIO_DAC_ENABLE
    /*log_info(">>>dac_power_off:%d", __this->ref.counter);*/
    if (atomic_read(&__this->ref) != 0 && atomic_dec_return(&__this->ref)) {
        return;
    }
#if 0
    app_audio_mute(AUDIO_MUTE_DEFAULT);
    if (dac_hdl.vol_l || dac_hdl.vol_r) {
        u8 fade_time = dac_hdl.vol_l * 2 / 10 + 1;
        os_time_dly(fade_time);
        printf("fade_time:%d ms", fade_time);
    }
#endif
    audio_dac_close(&dac_hdl);
#endif // #if TCFG_AUDIO_DAC_ENABLE
}

#if 0
/*
 *写到dac buf的数据接口
 */
void audio_write_data_hook(void *data, u32 len)
{

}
#endif

/*
 *dac快速校准
 */
//#define DAC_TRIM_FAST_EN
#ifdef DAC_TRIM_FAST_EN
u8 dac_trim_fast_en()
{
    return 1;
}
#endif/*DAC_TRIM_FAST_EN*/

/*
 *自定义dac上电延时时间，具体延时多久应通过示波器测量
 */
#if 1
void dac_power_on_delay()
{
#if TCFG_MC_BIAS_AUTO_ADJUST
    void mic_capless_auto_adjust_init();
    mic_capless_auto_adjust_init();
#endif
    os_time_dly(50);
}
#endif

#define TRIM_VALUE_LR_ERR_MAX           (600)   // 距离参考值的差值限制
#define abs(x) ((x)>0?(x):-(x))
int audio_dac_trim_value_check(struct audio_dac_trim *dac_trim)
{
    printf("audio_dac_trim_value_check %d %d\n", dac_trim->left, dac_trim->right);
    s16 reference = 0;
    if (TCFG_AUDIO_DAC_CONNECT_MODE != DAC_OUTPUT_MONO_R) {
        if (abs(dac_trim->left - reference) > TRIM_VALUE_LR_ERR_MAX) {
            log_error("dac trim channel l err\n");
            return -1;
        }
    }
    if (TCFG_AUDIO_DAC_CONNECT_MODE != DAC_OUTPUT_MONO_L) {
        if (abs(dac_trim->right - reference) > TRIM_VALUE_LR_ERR_MAX) {
            log_error("dac trim channel r err\n");
            return -1;
        }
    }

    return 0;
}

/*
 *capless模式一开始不要的数据包数量
 */
u16 get_ladc_capless_dump_num(void)
{
    return 10;
}

extern struct adc_platform_data adc_data;
int read_mic_type_config(void)
{
    MIC_TYPE_CONFIG mic_type;
    int ret = syscfg_read(CFG_MIC_TYPE_ID, &mic_type, sizeof(MIC_TYPE_CONFIG));
    if (ret > 0) {
        log_info_hexdump(&mic_type, sizeof(MIC_TYPE_CONFIG));
        adc_data.mic_capless = mic_type.type;
        adc_data.mic_bias_res = mic_type.pull_up;
        adc_data.mic_ldo_vsel = mic_type.ldo_lev;
        return 0;
    }
    return -1;
}


