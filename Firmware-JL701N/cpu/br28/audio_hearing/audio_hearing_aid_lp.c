/*
 ****************************************************************
 *							Hearing Aid Low Power
 *							检测数据能量的函数
 * File  : audi_hearing_aid_lp.c
 * By    :
 * Notes :
 ****************************************************************
 */

#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "app_main.h"
#include "audio_config.h"
#include "app_action.h"
#include "audio_hearing_aid_lp.h"

extern struct audio_dac_hdl dac_hdl;
extern struct audio_adc_hdl adc_hdl;

#ifndef TCFG_AUDIO_DHA_MIC_SAMPLE_RATE
#define TCFG_AUDIO_DHA_MIC_SAMPLE_RATE  32000
#endif

//*********************************************************************************//
//           mic能量检测处理配置             //
//*********************************************************************************//
//是否开启mic 能量检测
#define MIC_ENERGY_DETECT  1

//判断多少ms没有能量就进入低功耗
#define MIC_SNIFF_TIME    2 * 60 * 1000
//#define MIC_SNIFF_TIME    5 * 1000

//进入低功耗后每隔多少ms计算一次能量
#define MIC_CHECK_INTER   1 * 1000

//低功耗状态下能量检测计算多少ms的能量
#define MIC_ENERGY_TIME    20

//进入低功耗的能量值,小于此能量值进入低功耗
#define enter_sniff_energy  50

//退出低功耗的能量值,大于退出低功耗  //从低功耗出来时adc 能量值会比较高，需要比较高的阈值
#define exit_sniff_energy   170

//跳过刚上电的多少次中断数据
#define adc_isr_ignore_cnt 8

//是否打开mic 能量的调试打印
#define ENERGY_DEBUG_EN 0

static  u8 enter_sleep = 0;

#if ENERGY_DEBUG_EN
static u32 debug_size = 4096 * 2;
/* static s16 debug_buf[4096 * 2]; */
static u32 debug_p = 0;
static u32 zero_cnt = 0;
static u32 no_zero_cnt = 0;
static u8  debug_flag = 0;
#endif

struct hearing_aid_energy_detect {
    u32 exit_sniff_points;  //一次检测的点数
    u32 abs_total_0; //mic0 的能量
    u32 abs_total_1; //mic1 的能量
    u32 points_count; //点数
    u32 energy_detect_flag;//是否开始计算能量的标志
    u32 enter_sniff_points; //多少个点的能量低于阈值进低功耗
    u32 exit_sniff_timer;   //检测能量值定时器
    u16 dac_start_timer;   //检测能量值定时器
    u16 dac_stop_timer;   //检测能量值定时器
    u8  dac_start_flag;     //dac 开启或关闭的标志位
    u8 isr_ignore_cnt;    //跳过刚上电的多少次中断数据
    u8 adc_ch_num;   //打开多少个mic通道
    struct adc_mic_ch mic_ch;
    s16 *adc_dma_buf;
    struct audio_adc_output_hdl adc_output;
    u8 dac_open;
    void (*hw_open)(void);
    void (*hw_close)(void);
};

static struct hearing_aid_energy_detect  *mic_aid = NULL;

void  exit_sniff_start(void)   //定时能量检测,能量检测的时候mic能不能正常工作
{
#if ENERGY_DEBUG_EN
    printf("enter low_power_deal.c %d\n", __LINE__);
#endif

    mic_aid->energy_detect_flag = 1; //开始能量计算
    mic_aid->isr_ignore_cnt = adc_isr_ignore_cnt;

#if 1 //重新打开mic
    extern void hearing_aid_energy_demo_open(u8 mic_idx, u8 gain, u16 sr, u8 mic_2_dac);
    hearing_aid_energy_demo_open(TCFG_AD2DA_LOW_LATENCY_MIC_CHANNEL, 12, TCFG_AUDIO_DHA_MIC_SAMPLE_RATE, 0);
    /* audio_adc_mic_aid_open(mic_aid->mic_idx, mic_aid->gain, mic_aid->sr, 0); */
#else
    SFR(JL_ADDA->DAA_CON0, 16,  1,  1);     // VBG_EN_11v
    SFR(JL_ADDA->ADA_CON0, 17,  1,  0);     // CTADCA_S2_RSTB
    os_time_dly(50);
    SFR(JL_ADDA->ADA_CON0, 17,  1,  1);     // CTADCA_S2_RSTB
    SFR(JL_ADDA->DAA_CON0, 12,  1,  1);     //AUDIO_IBIAS_EN_11V
#endif

    enter_sleep = 0; //退出低功耗
    sys_timer_del(mic_aid->exit_sniff_timer);
}


void  dac_start_timer(void)   //启动dac
{
#if ENERGY_DEBUG_EN
    printf("enter low_power_deal.c%d\n", __LINE__);
#endif

    extern void hearing_aid_energy_demo_close(void);
    hearing_aid_energy_demo_close();

    printf("enter low_power_deal.c%d\n", __LINE__);

    if (mic_aid->hw_open) {
        mic_aid->hw_open();
    }
    /* sound_pcm_dev_start(NULL, 16000, 10); */
    enter_sleep = 0; //退出低功耗
    /* mic_aid->adc_2_dac = 1; */
    mic_aid->dac_start_flag = 1 ;
    sys_timer_del(mic_aid->dac_start_timer);
}


void  dac_stop_timer(void)   //关闭dac
{
#if ENERGY_DEBUG_EN
    printf("enter low_power_deal.c%d\n", __LINE__);
#endif


    extern void hearing_aid_energy_demo_close(void);
    hearing_aid_energy_demo_close();

    if (mic_aid->hw_close) {

        printf("enter low_power_deal.c%d,%p\n", __LINE__, mic_aid->hw_close);
        mic_aid->hw_close();
    }
    /* audio_adc_del_output_handler(&adc_hdl, &mic_aid->adc_output); */
    /* audio_adc_mic_close(&mic_aid->mic_ch); */
    mic_aid->adc_ch_num = 0; //mic_aid 被关了
    //将变量清0，开始新一轮计算
    mic_aid->abs_total_0 = 0;
    mic_aid->abs_total_1 = 0;
    mic_aid->points_count = 0;
    mic_aid->energy_detect_flag = 0;

    enter_sleep = 1; //进入低功耗
    /* mic_aid->adc_2_dac = 0; */
    mic_aid->dac_start_flag = 0 ;
    sys_timer_del(mic_aid->dac_stop_timer);

    printf("enter low_power_deal.c%d\n", __LINE__);
}


void audio_hearing_aid_lp_open(u8 ch_num, void (*hw_open)(void), void (*hw_close)(void)) //对多少个通道做数据处理
{
    if (!mic_aid) {
        printf("enter low_power_deal.c%d\n", __LINE__);
        mic_aid = zalloc(sizeof(struct hearing_aid_energy_detect));
    }

#if 0
    else {
        /* memset(mic_aid,0,sizeof(struct hearing_aid_energy_detect)); */
    }
#endif
    mic_aid->exit_sniff_points = MIC_ENERGY_TIME * (TCFG_AUDIO_DHA_MIC_SAMPLE_RATE / 1000); //进入低功耗后一次检测的点数
    mic_aid->enter_sniff_points = MIC_SNIFF_TIME * (TCFG_AUDIO_DHA_MIC_SAMPLE_RATE / 1000); //多少个点的能量低于阈值进低功耗
    mic_aid->adc_ch_num = ch_num;
    mic_aid-> hw_open = hw_open;     //正常工作打开硬件的函数(关mic，关dac)
    mic_aid-> hw_close = hw_close;  //进入低功耗关硬件的函数(开mic，开dac)
    mic_aid->abs_total_0 = 0;
    mic_aid->abs_total_1 = 0;
    mic_aid->points_count = 0;
    enter_sleep = 0;

}

void audio_hearing_aid_lp_close(void)
{
    if (mic_aid) {
        free(mic_aid);
        mic_aid = NULL;
    }
}

u8 audio_hearing_aid_lp_flag(void)
{
    return  enter_sleep;
}

void audio_hearing_aid_lp_detect(void *priv, s16 *data, int len)
{
#if MIC_ENERGY_DETECT
    u32  points = len / 2;
    s16 *pdata = data;
    u32 i = 0;
    s32 total_0 = 0;
    s32 total_1 = 0;
    s16 diff = 0;
    u32 target_points = 0;
    u32 target_energy = 0;

    if (enter_sleep == 0) { //低功耗期间也会进中断，不能计算能量值
        if (mic_aid->isr_ignore_cnt > 0) {
            mic_aid->isr_ignore_cnt--;
            return;
        }			//跳过刚开始的一次中断数据

        for (i = 0; i < points; i++) {
            total_0 += *pdata;
            pdata++;
            if (mic_aid->adc_ch_num == 2) {
                total_1 += *pdata;
                pdata++;
            }
        }

        total_0 /= (len / 2 / mic_aid->adc_ch_num); //直流量
        total_1 /= (len / 2 / mic_aid->adc_ch_num);
        // printf(">> total_0 %d total_1 %d\n", total_0, total_1);
        pdata = data;

        if (mic_aid->energy_detect_flag) { //由定时器启动的能量计算
            target_points = mic_aid->exit_sniff_points;  //多少时间检测
            target_energy = exit_sniff_energy;
        } else {
            target_points = mic_aid-> enter_sniff_points; //正常工作检测多久
            target_energy = enter_sniff_energy;
        }

        for (i = 0; i < points; i++) {
#if ENERGY_DEBUG_EN
#if 0
            if (debug_p < debug_size) {
                debug_buf[debug_p++] = *pdata ;
                if (*pdata == 0) {
                    zero_cnt++;
                } else {
                    no_zero_cnt++;
                }
            }
#endif
#endif

            diff = *pdata - total_0;
            if (diff > 0) {
                mic_aid->abs_total_0 += diff;
            } else {
                mic_aid->abs_total_0 -= diff;
            }
            pdata++;
            if (mic_aid->adc_ch_num == 2) {
                diff = *pdata - total_1;
                if (diff > 0) {
                    mic_aid->abs_total_1 += diff;
                } else {
                    mic_aid->abs_total_1 -= diff;
                }
                pdata++;
            }
            mic_aid -> points_count++;


#if ENERGY_DEBUG_EN
            /* printf("enter audio_adc_aid.c@@@@@@@@@@@@@@@@@@@@@@@@@@%d,%d,%d,%d\n",__LINE__,target_points,enter_sleep,mic_aid->points_count); */
#endif


            if (mic_aid->points_count == target_points) {

#if ENERGY_DEBUG_EN
                printf("enter low_power_deal.c %d,%d,%d,%d,%d\n", __LINE__, mic_aid->abs_total_0, mic_aid->abs_total_1, mic_aid->points_count, target_points);
#endif
                mic_aid->abs_total_0 /= target_points;
                mic_aid->abs_total_1 /= target_points;

#if ENERGY_DEBUG_EN
                printf("enter low_power_deal.c %d,%d,%d,%d\n", __LINE__, mic_aid->abs_total_0, mic_aid->abs_total_1, target_energy);
#endif

#if ENERGY_DEBUG_EN
                extern void audio_adda_dump(void); //打印所有的dac,adc寄存器
                extern void audio_gain_dump();
                audio_adda_dump();
                audio_gain_dump();
#endif

                if ((mic_aid->abs_total_0 < target_energy) && (mic_aid->abs_total_1 < target_energy)) { //




                    mic_aid->dac_stop_timer = sys_timer_add(NULL, dac_stop_timer, 1);
                    mic_aid->exit_sniff_timer = sys_timer_add(NULL, exit_sniff_start, MIC_CHECK_INTER);	//退出低功耗计算能量值
#if ENERGY_DEBUG_EN
                    debug_p = 0;
                    zero_cnt = no_zero_cnt = 0;
                    /* memset(debug_buf, 0, debug_size * 2); */
                    printf("enter low_power_deal.c %d,%d,%d\n", __LINE__, mic_aid->abs_total_0, mic_aid->abs_total_1);
#endif
                    break;
                } else {
                    //将变量清0，开始新一轮计算
                    mic_aid->abs_total_0 = 0;
                    mic_aid->abs_total_1 = 0;
                    mic_aid->points_count = 0;
                    mic_aid->energy_detect_flag = 0;
                    enter_sleep = 0;

#if ENERGY_DEBUG_EN
                    debug_p = 0;
                    /* memset(debug_buf, 0, debug_size * 2); */
                    zero_cnt = no_zero_cnt = 0;
#endif

                    if (!audio_dac_is_working(&dac_hdl)) {

                        mic_aid->dac_start_timer = sys_timer_add(NULL, dac_start_timer, 1);
                    }
                }

            } //end of if  点数等于 enter_sniff_points
        }//endf of for
    }  //end of  低功耗期间也会进中断，不能计算能量值

    if (mic_aid->dac_open == 0) {
        return;
    }

#endif

}





