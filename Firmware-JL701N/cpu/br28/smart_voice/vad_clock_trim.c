/*****************************************************************
>file name : vco_clock_trim.c
>author : IC lishanliao
>create time : Mon 22 Nov 2021 02:44:34 PM CST
>Decription : 通过vco时钟的trim，使vco clock接近29MHz，
              这样adc采样对应电压更准确
*****************************************************************/
#include "vad_mic.h"
#include "voice_mic_data.h"

static u8 vco_clock_calibrated = 0;
#define VAD_EN_10V_1_(x)              SFR(P11_LPVAD->VAD_ACON0, 29, 1, x)
#define VAD_ACM_10V_4_(x)             SFR(P11_LPVAD->VAD_ACON0,  0, 4, x)
#define VAD_LDO_IS_10V_2_(x)          SFR(P11_LPVAD->VAD_ACON1, 0,  2, x)
#define VAD_LDO_VS_10V_3_(x)          SFR(P11_LPVAD->VAD_ACON1, 2,  3, x)
#define VAD_BUF_EN_10V_1_(x)          SFR(P11_LPVAD->VAD_ACON0, 17, 1, x)
#define VAD_BUF_IS_10V_2_(x)          SFR(P11_LPVAD->VAD_ACON0, 18, 2, x)
#define VAD_PGA_EN_10V_1_(x)          SFR(P11_LPVAD->VAD_ACON1, 22, 1, x)
#define VAD_PGA_IS_10V_2_(x)          SFR(P11_LPVAD->VAD_ACON1, 27, 2, x)
#define VAD_PGA_GS_10V_4_(x)          SFR(P11_LPVAD->VAD_ACON1, 23, 4, x)
#define VAD_BUF2INN_EN_10V_1_(x)      SFR(P11_LPVAD->VAD_ACON0, 16, 1, x)
#define VAD_ADC_EN_10V_1_(x)          SFR(P11_LPVAD->VAD_ACON0,  4, 1, x)
#define VAD_ADC_RIN_10V_3_(x)         SFR(P11_LPVAD->VAD_ACON0,  9, 3, x)
#define VAD_ADC_RBS_10V_3_(x)         SFR(P11_LPVAD->VAD_ACON0,  6, 3, x)
#define VAD_ADC_TEN_10V_1_(x)         SFR(P11_LPVAD->VAD_ACON0, 12, 1, x)
#define VAD_VCON_TOE_10V_1_(x)        SFR(P11_LPVAD->VAD_CON  , 27, 1, x)
#define VAD_VCOP_TOE_10V_1_(x)        SFR(P11_LPVAD->VAD_CON  , 28, 1, x)
#define VAD_IREF1U_TOE_10V_1_(x)      SFR(P11_LPVAD->VAD_ACON1, 30, 1, x)
void vco_clock_test_mode_setup(void)
{
    P11_LPVAD->VAD_ACON0 = 0;
    P11_LPVAD->VAD_ACON1 = 0;
    P11_LPVAD->VAD_CON = 0;
    SFR(JL_ADDA->ADDA_CON1,  1,  1,  1);//enable vad analog to  ANA pad

    VAD_EN_10V_1_(1)    ;
    VAD_ACM_10V_4_(7)    ;
    VAD_LDO_VS_10V_3_(3)    ;
    VAD_LDO_IS_10V_2_(2)    ;
    VAD_BUF_EN_10V_1_(1)    ;
    VAD_BUF_IS_10V_2_(2)    ;
    VAD_PGA_EN_10V_1_(1)    ;
    VAD_PGA_IS_10V_2_(2)    ;
    VAD_PGA_GS_10V_4_(4)    ;
    VAD_BUF2INN_EN_10V_1_(1);
    VAD_ADC_EN_10V_1_(1)    ;
    VAD_ADC_RIN_10V_3_(5)   ;
    VAD_ADC_RBS_10V_3_(5)   ;
    VAD_VCON_TOE_10V_1_(1)  ;
    VAD_VCOP_TOE_10V_1_(1)  ;
    VAD_IREF1U_TOE_10V_1_(1);
}

/*
void vco_clk2io_test(void)
{
    //vad_vcon0->PA6
    //vad_vcop0->PA7
    SFR(JL_CLOCK->CLK_CON0, 24, 4, 15); //clk_out2_sel sel vad_vcon0
    SFR(JL_CLOCK->CLK_CON0, 28, 4, 15); //clk_out2_sel sel vad_vcop0
    SFR(JL_IOMC->OCH_CON0, 0, 5, 16); //
    SFR(JL_IOMC->OCH_CON0, 5, 9, 17);

    JL_OMAP->PA6_OUT = (0 << 0) | //input enable
                       (1 << 1) | //output enable
                       (0 << 2); //output signal select

    JL_OMAP->PA7_OUT = (0 << 0) | //input enable
                       (1 << 1) | //output enable
                       (1 << 2); //output signal select
    JL_PORTA->DIR &= ~BIT(6);
    JL_PORTA->DIR &= ~BIT(7);
    printf("============== start VCO_CLK_IO test =================");
}
*/

void delay1us(u16 i)
{
    int num = clk_get("sys") / 1000000 * i;
    while (num--) {
        asm("nop");
    }
}

#define   GPC_FD_MUX_3_(x)    SFR(JL_CLOCK->CLK_CON2,28,4,x)
//osc clk mux 0:btosc24m 1:btosc48m 2:std24m
//            3:rtc_osc  4:lrc_clk  5:pat_clk
#define   OSC_CLK_MUX_3_(x)   SFR(JL_CLOCK->CLK_CON0,1,3,x)

//0:lsb_clk 1:osc_clk 2:cap_mux_clk 3:clk_mux
//4:gpc_fd(9:vad_vcon 10:vad_vcop)  5:ring_clk
//6:pll_d1p0_mo 7:irflt_in
#define   GPCNT_CSS_3_(x)   SFR(JL_GPCNT->CON,1,3,x)//count source select
#define   GPCNT_GSS_3_(x)   SFR(JL_GPCNT->CON,12,3,x)//gate source select

#define   GPCNT_GTS_4_(x)   SFR(JL_GPCNT->CON,8,4,x)//gate time select 32*2^n
#define   GPCNT_EN_1_(x)    SFR(JL_GPCNT->CON,0,1,x)
#define   CLR_PND_1_(x)     SFR(JL_GPCNT->CON,6,1,x)
u32 get_gpcnt(u8 sel_p_n, u8 gs_clk, u8 cs_clk, u8 prd)
{
    u32 gpcnt_n = 0;

    /*printf("start get cnt\r\n");*/
    GPCNT_EN_1_(0);
    if (sel_p_n == 0) {
        GPC_FD_MUX_3_(9);//9:vad_vcon  10:vad_vcop
    } else {
        GPC_FD_MUX_3_(10);//9:vad_vcon  10:vad_vcop
    }
    OSC_CLK_MUX_3_(gs_clk);

    GPCNT_CSS_3_(cs_clk);
    GPCNT_GSS_3_(1);
    GPCNT_GTS_4_(prd);//32*2^n
    GPCNT_EN_1_(1);
    CLR_PND_1_(1);
    delay1us(1000);

    asm("csync");
    asm("csync");
    asm("csync");
    while (!(JL_GPCNT->CON >> 7 & 0x01));
    asm("csync");
    asm("csync");
    asm("csync");

    gpcnt_n = JL_GPCNT->NUM;
    return  gpcnt_n;

}

#define FCNT_N 10
u32 get_avr_gpcnt(void)
{
    u32 f_cnt = 0, i = 0, fc_num = 0, f_max = 0, f_min = 1000000;

    // delay_1000ms();
    for (i = 0; i < FCNT_N; i++) {
        //osc clk mux 0:btosc24m 1:btosc48m 2:std24m
        //            3:rtc_osc  4:lrc_clk  5:pat_clk
        //  32*2^(7)=4096

        f_cnt = get_gpcnt(1, 2, 4, 7); //32*2^n
        //f_cnt = get_gpcnt(1,0,4,7); //32*2^n
        //printf("===========get vco clk cnt=%d \r\r\n\n",f_cnt);

        fc_num = f_cnt + fc_num;
        if (f_max < f_cnt) {
            f_max = f_cnt;
        }
        if (f_min > f_cnt) {
            f_min = f_cnt;
        }

        wdt_clear();
    }
    //printf("===========get vco f_min clk cnt=%d \r\r\n\n",f_min);
    //printf("===========get vco f_max clk cnt=%d \r\r\n\n",f_max);
    f_cnt = (fc_num - f_max - f_min) / (FCNT_N - 2);
    return f_cnt;
}

#define VCO_TRIM_CNT 8
#define VCO_TRIM_TAR 4949   // 24/32.99= 32*2^7/VCO_TRIM_TAR
void vad_vco_clk_trim(void)
{
    u32 gp_cnt[VCO_TRIM_CNT], gp_min, gp_ind;
    u8 i;

    os_time_dly(80); //500ms delay

    for (i = 0; i < VCO_TRIM_CNT; i++) {
        VAD_ADC_RIN_10V_3_(i)   ;
        VAD_ADC_RBS_10V_3_(i)   ;
        gp_cnt[i] = get_avr_gpcnt();
        printf("get vco clk average cnt[%d]=%d\n", i, gp_cnt[i]);
    }

    for (i = 0; i < VCO_TRIM_CNT; i++) {
        if (gp_cnt[i] > VCO_TRIM_TAR) {
            gp_cnt[i] = gp_cnt[i] - VCO_TRIM_TAR;
        } else {
            gp_cnt[i] = VCO_TRIM_TAR - gp_cnt[i];
        }
        if (i == 0) {
            gp_min = gp_cnt[i];
            gp_ind = i;
        } else if (gp_min > gp_cnt[i]) {
            gp_min = gp_cnt[i];
            gp_ind = i;
        }
        wdt_clear();
    }
    printf("set vco clk index=%d,res=%d\n", gp_ind, gp_min);

    /*VAD_ADC_RIN_10V_3_(gp_ind)   ;*/
    /*VAD_ADC_RBS_10V_3_(gp_ind)   ;*/
    struct vad_mic_platform_data *data = (struct vad_mic_platform_data *)(VAD_AVAD_CONFIG_BEGIN + sizeof(struct avad_config));
    data->mic_data.adc_rin = gp_ind;
    data->mic_data.adc_rbs = gp_ind;

}

void audio_vad_clock_trim(void)
{
    if (vco_clock_calibrated) {
        return;
    }
    vco_clock_test_mode_setup();
    /*vco_clk2io_test();*/
    vad_vco_clk_trim();
    vco_clock_calibrated = 1;
}
