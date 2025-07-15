/*****************************************************************
>file name : lib/media/cpu/br22/audio_link.c
>author :
>create time : Fri 7 Dec 2018 14:59:12 PM CST
*****************************************************************/
#include "includes.h"
#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "asm/clock.h"
#include "asm/iis.h"
#include "audio_link.h"
#include "app_config.h"
#include "update.h"
#include "audio_syncts.h"
#include "audio_iis.h"
#include "sound/sound.h"

/* #if TCFG_AUDIO_INPUT_IIS || TCFG_AUDIO_OUTPUT_IIS */
#define ALINK_TEST_ENABLE	0

#define ALINK_DEBUG_INFO
#ifdef ALINK_DEBUG_INFO
#define alink_printf  printf
#else
#define alink_printf(...)
#endif

static void alink0_info_dump();
static u32 *ALNK0_BUF_ADR[] = {
    (u32 *)(&(JL_ALNK0->ADR0)),
    (u32 *)(&(JL_ALNK0->ADR1)),
    (u32 *)(&(JL_ALNK0->ADR2)),
    (u32 *)(&(JL_ALNK0->ADR3)),
};

static u32 *ALNK0_HWPTR[] = {
    (u32 *)(&(JL_ALNK0->HWPTR0)),
    (u32 *)(&(JL_ALNK0->HWPTR1)),
    (u32 *)(&(JL_ALNK0->HWPTR2)),
    (u32 *)(&(JL_ALNK0->HWPTR3)),
};

static u32 *ALNK0_SWPTR[] = {
    (u32 *)(&(JL_ALNK0->SWPTR0)),
    (u32 *)(&(JL_ALNK0->SWPTR1)),
    (u32 *)(&(JL_ALNK0->SWPTR2)),
    (u32 *)(&(JL_ALNK0->SWPTR3)),
};

static u32 *ALNK0_SHN[] = {
    (u32 *)(&(JL_ALNK0->SHN0)),
    (u32 *)(&(JL_ALNK0->SHN1)),
    (u32 *)(&(JL_ALNK0->SHN2)),
    (u32 *)(&(JL_ALNK0->SHN3)),
};

static u32 PFI_ALNK0_DAT[] = {
    PFI_ALNK0_DAT0,
    PFI_ALNK0_DAT1,
    PFI_ALNK0_DAT2,
    PFI_ALNK0_DAT3,
};

static u32 FO_ALNK0_DAT[] = {
    FO_ALNK0_DAT0,
    FO_ALNK0_DAT1,
    FO_ALNK0_DAT2,
    FO_ALNK0_DAT3,
};


enum {
    MCLK_11M2896K = 0,
    MCLK_12M288K
};

enum {
    MCLK_EXTERNAL	= 0u,
    MCLK_SYS_CLK		,
    MCLK_OSC_CLK 		,
    MCLK_PLL_CLK		,
};

enum {
    MCLK_DIV_1		= 0u,
    MCLK_DIV_2			,
    MCLK_DIV_4			,
    MCLK_DIV_8			,
    MCLK_DIV_16			,
};

enum {
    MCLK_LRDIV_EX	= 0u,
    MCLK_LRDIV_64FS		,
    MCLK_LRDIV_128FS	,
    MCLK_LRDIV_192FS	,
    MCLK_LRDIV_256FS	,
    MCLK_LRDIV_384FS	,
    MCLK_LRDIV_512FS	,
    MCLK_LRDIV_768FS	,
};
#ifndef TCFG_IIS_SR
#define TCFG_IIS_SR  44100
#endif

extern u32 clock_get_pll_target_frequency();
u32 audio_iis_hw_rates_match(u32 sr)
{
    u8 i = 0;
    u32 pll_tar = clock_get_pll_target_frequency();
    if (pll_tar == 240) {
        u32 hw_rates[] = {ALINK_SR_48000, ALINK_SR_44100, ALINK_SR_32000, ALINK_SR_24000, ALINK_SR_22050, ALINK_SR_16000, ALINK_SR_12000, ALINK_SR_11025, ALINK_SR_8000};
        for (i = 0; i < ARRAY_SIZE(hw_rates); i++) {
            if (hw_rates[i] == sr) {
                log_i("match sr %d\n", sr);
                return sr;
            }
        }
    } else {
        u32 hw_rates[] = {ALINK_SR_44100, ALINK_SR_22050, ALINK_SR_11025};
        for (i = 0; i < ARRAY_SIZE(hw_rates); i++) {
            if (hw_rates[i] == sr) {
                log_i("match sr %d\n", sr);
                return sr;
            }
        }
    }
    log_e("not match is hw sr %d\n", sr);
    return TCFG_IIS_SR;
}


static ALINK_PARM *p_alink0_parm = NULL;

u32 alink_get_addr(void *hw_channel)
{
    struct alnk_hw_ch *hw_channel_parm = (struct alnk_hw_ch *)hw_channel;
    u32 buf_addr = 0;
    buf_addr = *ALNK0_BUF_ADR[hw_channel_parm->ch_idx];
    return buf_addr;
}

u32 alink_get_shn(void *hw_channel)
{
    struct alnk_hw_ch *hw_channel_parm = (struct alnk_hw_ch *)hw_channel;
    int shn = 0;
    shn = *ALNK0_SHN[hw_channel_parm->ch_idx];
    return shn;
}

void alink_set_shn(void *hw_channel, u32 len)
{
    struct alnk_hw_ch *hw_channel_parm = (struct alnk_hw_ch *)hw_channel;
    if (len) {
        *ALNK0_SHN[hw_channel_parm->ch_idx] = len;
    }
}

u32 alink_get_swptr(void *hw_channel)
{
    struct alnk_hw_ch *hw_channel_parm = (struct alnk_hw_ch *)hw_channel;
    int swptr = 0;
    swptr = *ALNK0_SWPTR[hw_channel_parm->ch_idx];
    return swptr;
}

void alink_set_swptr(void *hw_channel, u32 value)
{
    struct alnk_hw_ch *hw_channel_parm = (struct alnk_hw_ch *)hw_channel;
    *ALNK0_SWPTR[hw_channel_parm->ch_idx] = value;
}

u32 alink_get_hwptr(void *hw_channel)
{
    struct alnk_hw_ch *hw_channel_parm = (struct alnk_hw_ch *)hw_channel;
    int hwptr = 0;
    hwptr = *ALNK0_HWPTR[hw_channel_parm->ch_idx];
    return hwptr;
}

void alink_set_hwptr(void *hw_channel, u32 value)
{
    struct alnk_hw_ch *hw_channel_parm = (struct alnk_hw_ch *)hw_channel;
    *ALNK0_HWPTR[hw_channel_parm->ch_idx] = value;
}

void alink_set_ch_ie(void *hw_channel, u32 value)
{
    struct alnk_hw_ch *hw_channel_parm = (struct alnk_hw_ch *)hw_channel;
    ALINK_CHx_IE(hw_channel_parm->module, hw_channel_parm->ch_idx, value);
}

void alink_clr_ch_pnd(void *hw_channel)
{
    struct alnk_hw_ch *hw_channel_parm = (struct alnk_hw_ch *)hw_channel;
    ALINK_CLR_CHx_PND(hw_channel_parm->module, hw_channel_parm->ch_idx);
}

//==================================================
static void alink_io_in_init(u8 gpio)
{
    gpio_set_direction(gpio, 1);
    gpio_set_pull_down(gpio, 0);
    gpio_set_pull_up(gpio, 1);
    gpio_set_die(gpio, 1);
}

static void alink_io_out_init(u8 gpio)
{
    gpio_set_direction(gpio, 0);
    gpio_set_pull_down(gpio, 0);
    gpio_set_pull_up(gpio, 0);
    gpio_set_die(gpio, 0);
    gpio_direction_output(gpio, 1);
}

static void *alink_addr(void *hw_alink, u8 ch)
{
    ALINK_PARM *hw_alink_parm = (ALINK_PARM *)hw_alink;
    s16 *buf_addr = NULL; //can be used
    u32 buf_index = 0;

    if (hw_alink_parm->buf_mode == ALINK_BUF_CIRCLE) {
        buf_addr = (s16 *)(*ALNK0_BUF_ADR[ch] + *ALNK0_SWPTR[ch] * 4);
    } else {
        buf_addr = (s16 *)(hw_alink_parm->ch_cfg[ch].buf);
        u8 index_table[4] = {12, 13, 14, 15};
        u8 bit_index = index_table[ch];
        buf_index = (ALINK_SEL(hw_alink_parm->module, CON0) & BIT(bit_index)) ? 0 : 1;
        buf_addr = buf_addr + ((hw_alink_parm->dma_len / 2 / 2) * buf_index);
    }
    return buf_addr;
}

___interrupt
static void alink0_dma_isr(void)
{
    u16 reg;
    s16 *buf_addr = NULL ;
    u8 ch = 0;

    reg = JL_ALNK0->CON2;
    reg >>= 4;

    for (ch = 0; ch < 4; ch++) {
        if (!(reg & BIT(ch))) {
            continue;
        }
        buf_addr = (s16 *)alink_addr(p_alink0_parm, ch);
        if (p_alink0_parm->ch_cfg[ch].isr_cb) {
            if (p_alink0_parm->buf_mode == ALINK_BUF_CIRCLE) {
                p_alink0_parm->ch_cfg[ch].isr_cb(p_alink0_parm->ch_cfg[ch].private_data, buf_addr, (JL_ALNK0->PNS & 0xffff) * 4);
            } else {
                p_alink0_parm->ch_cfg[ch].isr_cb(p_alink0_parm->ch_cfg[ch].private_data, buf_addr, p_alink0_parm->dma_len / 2);
            }
        }
        ALINK_CLR_CHx_PND(p_alink0_parm->module, ch);
    }
}


static void alink_sr(void *hw_alink, u32 rate)
{
    alink_printf("ALINK_SR = %d\n", rate);
    ALINK_PARM *hw_alink_parm = (ALINK_PARM *)hw_alink;
    hw_alink_parm->sample_rate = rate;
    u8 module = hw_alink_parm->module;

    u8 pll_target_frequency = clock_get_pll_target_frequency();
    alink_printf("pll_target_frequency = %d\n", pll_target_frequency);

    switch (rate) {
    case ALINK_SR_48000:
        /*12.288Mhz 256fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(module, MCLK_DIV_1);
        ALINK_LRDIV(module, MCLK_LRDIV_256FS);
        break ;

    case ALINK_SR_44100:
        /*11.289Mhz 256fs*/
        audio_link_clock_sel(ALINK_CLOCK_11M2896K);
        ALINK_MDIV(module, MCLK_DIV_1);
        if (pll_target_frequency == 192) {
            ALINK_LRDIV(module, MCLK_LRDIV_256FS);
        } else if (pll_target_frequency == 240) {
            ALINK_LRDIV(module, MCLK_LRDIV_128FS);
        }
        break ;

    case ALINK_SR_32000:
        /*12.288Mhz 384fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(module, MCLK_DIV_1);
        ALINK_LRDIV(module, MCLK_LRDIV_384FS);
        break ;

    case ALINK_SR_24000:
        /*12.288Mhz 512fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(module, MCLK_DIV_2);
        ALINK_LRDIV(module, MCLK_LRDIV_256FS);
        break ;

    case ALINK_SR_22050:
        /*11.289Mhz 512fs*/
        audio_link_clock_sel(ALINK_CLOCK_11M2896K);
        ALINK_MDIV(module, MCLK_DIV_1);
        if (pll_target_frequency == 192) {
            ALINK_LRDIV(module, MCLK_LRDIV_512FS);
        } else if (pll_target_frequency == 240) {
            ALINK_LRDIV(module, MCLK_LRDIV_256FS);
        }
        break ;

    case ALINK_SR_16000:
        /*12.288/2Mhz 384fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(module, MCLK_DIV_1);
        ALINK_LRDIV(module, MCLK_LRDIV_768FS);
        break ;

    case ALINK_SR_12000:
        /*12.288/2Mhz 512fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(module, MCLK_DIV_4);
        ALINK_LRDIV(module, MCLK_LRDIV_256FS);
        break ;

    case ALINK_SR_11025:
        /*11.289/2Mhz 512fs*/
        audio_link_clock_sel(ALINK_CLOCK_11M2896K);
        ALINK_MDIV(module, MCLK_DIV_2);
        if (pll_target_frequency == 192) {
            ALINK_LRDIV(module, MCLK_LRDIV_512FS);
        } else if (pll_target_frequency == 240) {
            ALINK_LRDIV(module, MCLK_LRDIV_256FS);
        }
        break ;

    case ALINK_SR_8000:
        /*12.288/4Mhz 384fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(module, MCLK_DIV_4);
        ALINK_LRDIV(module, MCLK_LRDIV_384FS);
        break ;

    default:
        //44100
        /*11.289Mhz 256fs*/
        audio_link_clock_sel(ALINK_CLOCK_11M2896K);
        ALINK_MDIV(module, MCLK_DIV_1);
        ALINK_LRDIV(module, MCLK_LRDIV_256FS);
        break;
    }
    if (hw_alink_parm->role == ALINK_ROLE_SLAVE) {
        ALINK_LRDIV(module, MCLK_LRDIV_EX);
    }
}


void *alink_channel_init(void *hw_alink, ALINK_CH ch_idx, u8 dir, void *priv, void (*handle)(void *priv, void *addr, int len))
{
    if (!hw_alink) {
        return NULL;
    }

    ALINK_PARM *hw_alink_parm = (ALINK_PARM *)hw_alink;
    u8 module = hw_alink_parm->module;

    hw_alink_parm->ch_cfg[ch_idx].module = module;
    hw_alink_parm->ch_cfg[ch_idx].dir = dir;
    hw_alink_parm->ch_cfg[ch_idx].ch_idx = ch_idx;
    hw_alink_parm->ch_cfg[ch_idx].isr_cb = handle;
    hw_alink_parm->ch_cfg[ch_idx].private_data = priv;
    printf("dma len %d", hw_alink_parm->dma_len);
    hw_alink_parm->ch_cfg[ch_idx].buf = zalloc(hw_alink_parm->dma_len);

    printf(">>> alink_channel_init %x\n", hw_alink_parm->ch_cfg[ch_idx].buf);
    u32 *buf_addr;

    //===================================//
    //           ALNK CH DMA BUF         //
    //===================================//
    buf_addr = ALNK0_BUF_ADR[ch_idx];
    *buf_addr = (u32)(hw_alink_parm->ch_cfg[ch_idx].buf);

    //===================================//
    //           ALNK工作模式            //
    //===================================//
    if (hw_alink_parm->mode > ALINK_MD_IIS_RALIGN) {
        ALINK_CHx_MODE_SEL(module, ch_idx, (hw_alink_parm->mode - ALINK_MD_IIS_RALIGN));
    } else {
        ALINK_CHx_MODE_SEL(module, ch_idx, (hw_alink_parm->mode));
    }
    //===================================//
    //          ALNK CH DAT IO INIT      //
    //===================================//
    if (hw_alink_parm->ch_cfg[ch_idx].dir == ALINK_DIR_RX) {
        alink_io_in_init(hw_alink_parm->ch_cfg[ch_idx].data_io);
        gpio_set_fun_input_port(hw_alink_parm->ch_cfg[ch_idx].data_io, PFI_ALNK0_DAT[ch_idx], LOW_POWER_FREE);
    } else {
        alink_io_out_init(hw_alink_parm->ch_cfg[ch_idx].data_io);
        gpio_set_fun_output_port(hw_alink_parm->ch_cfg[ch_idx].data_io, FO_ALNK0_DAT[ch_idx], 1, 1, LOW_POWER_FREE);
    }

    ALINK_CHx_DIR_MODE(module, ch_idx, dir);
    ALINK_CHx_IE(module, ch_idx, 1);

    r_printf("alink%d, ch%d init\n", module, ch_idx);

    return &hw_alink_parm->ch_cfg[ch_idx];
}

static void alink0_info_dump()
{
    alink_printf("JL_ALNK0->CON0 = 0x%x", JL_ALNK0->CON0);
    alink_printf("JL_ALNK0->CON1 = 0x%x", JL_ALNK0->CON1);
    alink_printf("JL_ALNK0->CON2 = 0x%x", JL_ALNK0->CON2);
    alink_printf("JL_ALNK0->CON3 = 0x%x", JL_ALNK0->CON3);
    alink_printf("JL_ALNK0->LEN = 0x%x", JL_ALNK0->LEN);
    alink_printf("JL_ALNK0->ADR0 = 0x%x", JL_ALNK0->ADR0);
    alink_printf("JL_ALNK0->ADR1 = 0x%x", JL_ALNK0->ADR1);
    alink_printf("JL_ALNK0->ADR2 = 0x%x", JL_ALNK0->ADR2);
    alink_printf("JL_ALNK0->ADR3 = 0x%x", JL_ALNK0->ADR3);
    alink_printf("JL_ALNK0->PNS = 0x%x", JL_ALNK0->PNS);
    alink_printf("JL_ALNK0->HWPTR0 = 0x%x", JL_ALNK0->HWPTR0);
    alink_printf("JL_ALNK0->HWPTR1 = 0x%x", JL_ALNK0->HWPTR1);
    alink_printf("JL_ALNK0->HWPTR2 = 0x%x", JL_ALNK0->HWPTR2);
    alink_printf("JL_ALNK0->HWPTR3 = 0x%x", JL_ALNK0->HWPTR3);
    alink_printf("JL_ALNK0->SWPTR0 = 0x%x", JL_ALNK0->SWPTR0);
    alink_printf("JL_ALNK0->SWPTR1 = 0x%x", JL_ALNK0->SWPTR1);
    alink_printf("JL_ALNK0->SWPTR2 = 0x%x", JL_ALNK0->SWPTR2);
    alink_printf("JL_ALNK0->SWPTR3 = 0x%x", JL_ALNK0->SWPTR3);
    alink_printf("JL_ALNK0->SHN0 = 0x%x", JL_ALNK0->SHN0);
    alink_printf("JL_ALNK0->SHN1 = 0x%x", JL_ALNK0->SHN1);
    alink_printf("JL_ALNK0->SHN2 = 0x%x", JL_ALNK0->SHN2);
    alink_printf("JL_ALNK0->SHN3 = 0x%x", JL_ALNK0->SHN3);
}

void *alink_init(void *hw_alink)
{
    if (hw_alink == NULL) {
        return NULL;
    }
    ALINK_PARM *hw_alink_parm = (ALINK_PARM *)hw_alink;
    u8 module = hw_alink_parm->module;
    p_alink0_parm = (ALINK_PARM *)hw_alink_parm;

    ALNK_CON_RESET(module);
    ALNK_HWPTR_RESET(module);
    ALNK_SWPTR_RESET(module);
    ALNK_ADR_RESET(module);
    ALNK_SHN_RESET(module);
    ALNK_PNS_RESET(module);
    ALINK_CLR_ALL_PND(module);


    ALINK_MSRC(module, MCLK_PLL_CLK);	/*MCLK source*/

    //===================================//
    //        输出时钟配置               //
    //===================================//
    if (hw_alink_parm->role == ALINK_ROLE_MASTER) {
        //主机输出时钟
        if (hw_alink_parm->mclk_io != ALINK_CLK_OUPUT_DISABLE) {
            alink_io_out_init(hw_alink_parm->mclk_io);
            gpio_set_fun_output_port(hw_alink_parm->mclk_io, FO_ALNK0_MCLK, 1, 1, LOW_POWER_FREE);
            ALINK_MOE(module, 1);				/*MCLK output to IO*/
        }
        if ((hw_alink_parm->sclk_io != ALINK_CLK_OUPUT_DISABLE) && (p_alink0_parm->lrclk_io != ALINK_CLK_OUPUT_DISABLE)) {
            alink_io_out_init(hw_alink_parm->lrclk_io);
            gpio_set_fun_output_port(hw_alink_parm->lrclk_io, FO_ALNK0_LRCK, 1, 1, LOW_POWER_FREE);
            alink_io_out_init(hw_alink_parm->sclk_io);
            gpio_set_fun_output_port(hw_alink_parm->sclk_io, FO_ALNK0_SCLK, 1, 1, LOW_POWER_FREE);
            ALINK_SOE(module, 1);				/*SCLK/LRCK output to IO*/
        }
    } else {
        //从机输入时钟
        alink_io_in_init(hw_alink_parm->mclk_io);
        alink_io_in_init(hw_alink_parm->sclk_io);
        alink_io_in_init(hw_alink_parm->lrclk_io);
        gpio_set_fun_input_port(hw_alink_parm->mclk_io, PFI_ALNK0_MCLK, LOW_POWER_FREE);
        gpio_set_fun_input_port(hw_alink_parm->sclk_io, PFI_ALNK0_SCLK, LOW_POWER_FREE);
        gpio_set_fun_input_port(hw_alink_parm->lrclk_io, PFI_ALNK0_LRCK, LOW_POWER_FREE);
        ALINK_MOE(module, 0);				/*MCLK input to IO*/
        ALINK_SOE(module, 0);				/*SCLK/LRCK output to IO*/
    }
    //===================================//
    //        基本模式/扩展模式          //
    //===================================//
    ALINK_DSPE(module, hw_alink_parm->mode >> 2);

    //===================================//
    //         数据位宽16/32bit          //
    //===================================//
    //注意: 配置了24bit, 一定要选ALINK_FRAME_64SCLK, 因为sclk32 x 2才会有24bit;
    if (hw_alink_parm->bitwide == ALINK_LEN_24BIT) {
        ASSERT(hw_alink_parm->sclk_per_frame == ALINK_FRAME_64SCLK);
        ALINK_24BIT_MODE(module);
        //一个点需要4bytes, LR = 2, 双buf = 2
    } else {
        ALINK_16BIT_MODE(module);
        //一个点需要2bytes, LR = 2, 双buf = 2
    }
    //===================================//
    //             时钟边沿选择          //
    //===================================//
    SCLKINV(module, hw_alink_parm->clk_mode);
    //===================================//
    //            每帧数据sclk个数       //
    //===================================//
    F32_EN(module, hw_alink_parm->sclk_per_frame);
    //===================================//
    //            设置数据采样率       	 //
    //===================================//
    alink_sr(hw_alink_parm, hw_alink_parm->sample_rate);

    //===================================//
    //            设置DMA MODE       	 //
    //===================================//
    ALINK_DMA_MODE_SEL(module, hw_alink_parm->buf_mode);
    if (hw_alink_parm->buf_mode == ALINK_BUF_CIRCLE) {
        ALINK_OPNS_SET(module, hw_alink_parm->dma_len / 16);
        if (hw_alink_parm->iperiod) {
            ALINK_IPNS_SET(module, hw_alink_parm->iperiod);
        } else {
            ALINK_IPNS_SET(module, hw_alink_parm->dma_len / 8);
        }
        //一个点需要2bytes, LR = 2
        ALINK_LEN_SET(module, hw_alink_parm->dma_len / 4);
    } else {
        //一个点需要2bytes, LR = 2, 双buf = 2
        ALINK_LEN_SET(module, hw_alink_parm->dma_len / 8);
    }
    request_irq(IRQ_ALINK0_IDX, 3, alink0_dma_isr, 0);

    return hw_alink_parm;
}

void alink_channel_close(void *hw_channel)
{
    struct alnk_hw_ch *hw_channel_parm = (struct alnk_hw_ch *)hw_channel;

    ALINK_CHx_CLOSE(hw_channel_parm->module, hw_channel_parm->ch_idx);
    ALINK_CHx_IE(hw_channel_parm->module, hw_channel_parm->ch_idx, 0);

    gpio_set_pull_up(hw_channel_parm->data_io, 0);
    gpio_set_pull_down(hw_channel_parm->data_io, 0);
    gpio_set_direction(hw_channel_parm->data_io, 1);
    gpio_set_die(hw_channel_parm->data_io, 0);
    if (hw_channel_parm->buf) {
        free(hw_channel_parm->buf);
        hw_channel_parm->buf = NULL;
    }
    r_printf("alink_ch %d closed\n", hw_channel_parm->ch_idx);
}


int alink_start(void *hw_alink)
{
    ALINK_PARM *hw_alink_parm = (ALINK_PARM *)hw_alink;
    if (hw_alink_parm) {
        ALINK_EN(hw_alink_parm->module, 1);
        return 0;
    }
    return -1;
}

void alink_uninit(void *hw_alink)
{
    ALINK_PARM *hw_alink_parm = (ALINK_PARM *)hw_alink;

    if (!hw_alink_parm) {
        return;
    }

    ALINK_EN(hw_alink_parm->module, 0);
    for (int i = 0; i < 4; i++) {
        if (hw_alink_parm->ch_cfg[i].buf) {
            alink_channel_close(&hw_alink_parm->ch_cfg[i]);
        }
    }
    ALNK_CON_RESET(hw_alink_parm->module);
    hw_alink_parm = NULL;
    alink_printf("audio_link_exit OK\n");
}

static u8 iis_driver_close(void)
{
    alink_uninit(p_alink0_parm);
    return 0;
}

REGISTER_UPDATE_TARGET(iis_update_target) = {
    .name = "iis",
    .driver_close = iis_driver_close,
};

int alink_get_sr(void *hw_alink)
{
    ALINK_PARM *hw_alink_parm = (ALINK_PARM *)hw_alink;
    if (hw_alink_parm) {
        return hw_alink_parm->sample_rate;
    } else {
        return -1;
    }
}

int alink_set_sr(void *hw_alink, u32 sr)
{
    ALINK_PARM *hw_alink_parm = (ALINK_PARM *)hw_alink;

    if (!hw_alink_parm) {
        return -1;
    }

    if (hw_alink_parm->sample_rate == sr) {
        return 0;
    }

    u8 is_alink_en = (ALINK_SEL(hw_alink_parm->module, CON0) & BIT(11));

    if (is_alink_en) {
        ALINK_EN(hw_alink_parm->module, 0);
    }

    alink_sr(hw_alink_parm, sr);

    if (is_alink_en) {
        ALINK_EN(hw_alink_parm->module, 1);
    }

    return 0;
}

u32 alink_get_dma_len(void *hw_alink)
{
    ALINK_PARM *hw_alink_parm = (ALINK_PARM *)hw_alink;
    return ALINK_FIFO_LEN(hw_alink_parm->module);
}

void alink_set_tx_pns(void *hw_alink, u32 len)
{
    ALINK_PARM *hw_alink_parm = (ALINK_PARM *)hw_alink;
    ALINK_OPNS_SET(hw_alink_parm->module, len);
}

void alink_set_rx_pns(void *hw_alink, u32 len)
{
    ALINK_PARM *hw_alink_parm = (ALINK_PARM *)hw_alink;
    ALINK_IPNS_SET(hw_alink_parm->module, len);
}

int alink_get_tx_pns(void *hw_alink)
{
    ALINK_PARM *hw_alink_parm = (ALINK_PARM *)hw_alink;

    return ALINK_SEL(hw_alink_parm->module, PNS);
}

#include "audio_iis.c"
//===============================================//
//			     for APP use demo                //
//===============================================//

#if ALINK_TEST_ENABLE

short const tsin_441k[441] = {
    0x0000, 0x122d, 0x23fb, 0x350f, 0x450f, 0x53aa, 0x6092, 0x6b85, 0x744b, 0x7ab5, 0x7ea2, 0x7fff, 0x7ec3, 0x7af6, 0x74ab, 0x6c03,
    0x612a, 0x545a, 0x45d4, 0x35e3, 0x24db, 0x1314, 0x00e9, 0xeeba, 0xdce5, 0xcbc6, 0xbbb6, 0xad08, 0xa008, 0x94fa, 0x8c18, 0x858f,
    0x8181, 0x8003, 0x811d, 0x84ca, 0x8af5, 0x9380, 0x9e3e, 0xaaf7, 0xb969, 0xc94a, 0xda46, 0xec06, 0xfe2d, 0x105e, 0x223a, 0x3365,
    0x4385, 0x5246, 0x5f5d, 0x6a85, 0x7384, 0x7a2d, 0x7e5b, 0x7ffa, 0x7f01, 0x7b75, 0x7568, 0x6cfb, 0x6258, 0x55b7, 0x4759, 0x3789,
    0x2699, 0x14e1, 0x02bc, 0xf089, 0xdea7, 0xcd71, 0xbd42, 0xae6d, 0xa13f, 0x95fd, 0x8ce1, 0x861a, 0x81cb, 0x800b, 0x80e3, 0x844e,
    0x8a3c, 0x928c, 0x9d13, 0xa99c, 0xb7e6, 0xc7a5, 0xd889, 0xea39, 0xfc5a, 0x0e8f, 0x2077, 0x31b8, 0x41f6, 0x50de, 0x5e23, 0x697f,
    0x72b8, 0x799e, 0x7e0d, 0x7fee, 0x7f37, 0x7bed, 0x761f, 0x6ded, 0x6380, 0x570f, 0x48db, 0x392c, 0x2855, 0x16ad, 0x048f, 0xf259,
    0xe06b, 0xcf20, 0xbed2, 0xafd7, 0xa27c, 0x9705, 0x8db0, 0x86ab, 0x821c, 0x801a, 0x80b0, 0x83da, 0x8988, 0x919c, 0x9bee, 0xa846,
    0xb666, 0xc603, 0xd6ce, 0xe86e, 0xfa88, 0x0cbf, 0x1eb3, 0x3008, 0x4064, 0x4f73, 0x5ce4, 0x6874, 0x71e6, 0x790a, 0x7db9, 0x7fdc,
    0x7f68, 0x7c5e, 0x76d0, 0x6ed9, 0x64a3, 0x5863, 0x4a59, 0x3acc, 0x2a0f, 0x1878, 0x0661, 0xf42a, 0xe230, 0xd0d0, 0xc066, 0xb145,
    0xa3bd, 0x9813, 0x8e85, 0x8743, 0x8274, 0x8030, 0x8083, 0x836b, 0x88da, 0x90b3, 0x9acd, 0xa6f5, 0xb4ea, 0xc465, 0xd515, 0xe6a3,
    0xf8b6, 0x0aee, 0x1ced, 0x2e56, 0x3ecf, 0x4e02, 0x5ba1, 0x6764, 0x710e, 0x786f, 0x7d5e, 0x7fc3, 0x7f91, 0x7cc9, 0x777a, 0x6fc0,
    0x65c1, 0x59b3, 0x4bd3, 0x3c6a, 0x2bc7, 0x1a41, 0x0833, 0xf5fb, 0xe3f6, 0xd283, 0xc1fc, 0xb2b7, 0xa503, 0x9926, 0x8f60, 0x87e1,
    0x82d2, 0x804c, 0x805d, 0x8303, 0x8833, 0x8fcf, 0x99b2, 0xa5a8, 0xb372, 0xc2c9, 0xd35e, 0xe4da, 0xf6e4, 0x091c, 0x1b26, 0x2ca2,
    0x3d37, 0x4c8e, 0x5a58, 0x664e, 0x7031, 0x77cd, 0x7cfd, 0x7fa3, 0x7fb4, 0x7d2e, 0x781f, 0x70a0, 0x66da, 0x5afd, 0x4d49, 0x3e04,
    0x2d7d, 0x1c0a, 0x0a05, 0xf7cd, 0xe5bf, 0xd439, 0xc396, 0xb42d, 0xa64d, 0x9a3f, 0x9040, 0x8886, 0x8337, 0x806f, 0x803d, 0x82a2,
    0x8791, 0x8ef2, 0x989c, 0xa45f, 0xb1fe, 0xc131, 0xd1aa, 0xe313, 0xf512, 0x074a, 0x195d, 0x2aeb, 0x3b9b, 0x4b16, 0x590b, 0x6533,
    0x6f4d, 0x7726, 0x7c95, 0x7f7d, 0x7fd0, 0x7d8c, 0x78bd, 0x717b, 0x67ed, 0x5c43, 0x4ebb, 0x3f9a, 0x2f30, 0x1dd0, 0x0bd6, 0xf99f,
    0xe788, 0xd5f1, 0xc534, 0xb5a7, 0xa79d, 0x9b5d, 0x9127, 0x8930, 0x83a2, 0x8098, 0x8024, 0x8247, 0x86f6, 0x8e1a, 0x978c, 0xa31c,
    0xb08d, 0xbf9c, 0xcff8, 0xe14d, 0xf341, 0x0578, 0x1792, 0x2932, 0x39fd, 0x499a, 0x57ba, 0x6412, 0x6e64, 0x7678, 0x7c26, 0x7f50,
    0x7fe6, 0x7de4, 0x7955, 0x7250, 0x68fb, 0x5d84, 0x5029, 0x412e, 0x30e0, 0x1f95, 0x0da7, 0xfb71, 0xe953, 0xd7ab, 0xc6d4, 0xb725,
    0xa8f1, 0x9c80, 0x9213, 0x89e1, 0x8413, 0x80c9, 0x8012, 0x81f3, 0x8662, 0x8d48, 0x9681, 0xa1dd, 0xaf22, 0xbe0a, 0xce48, 0xdf89,
    0xf171, 0x03a6, 0x15c7, 0x2777, 0x385b, 0x481a, 0x5664, 0x62ed, 0x6d74, 0x75c4, 0x7bb2, 0x7f1d, 0x7ff5, 0x7e35, 0x79e6, 0x731f,
    0x6a03, 0x5ec1, 0x5193, 0x42be, 0x328f, 0x2159, 0x0f77, 0xfd44, 0xeb1f, 0xd967, 0xc877, 0xb8a7, 0xaa49, 0x9da8, 0x9305, 0x8a98,
    0x848b, 0x80ff, 0x8006, 0x81a5, 0x85d3, 0x8c7c, 0x957b, 0xa0a3, 0xadba, 0xbc7b, 0xcc9b, 0xddc6, 0xefa2, 0x01d3, 0x13fa, 0x25ba,
    0x36b6, 0x4697, 0x5509, 0x61c2, 0x6c80, 0x750b, 0x7b36, 0x7ee3, 0x7ffd, 0x7e7f, 0x7a71, 0x73e8, 0x6b06, 0x5ff8, 0x52f8, 0x444a,
    0x343a, 0x231b, 0x1146, 0xff17, 0xecec, 0xdb25, 0xca1d, 0xba2c, 0xaba6, 0x9ed6, 0x93fd, 0x8b55, 0x850a, 0x813d, 0x8001, 0x815e,
    0x854b, 0x8bb5, 0x947b, 0x9f6e, 0xac56, 0xbaf1, 0xcaf1, 0xdc05, 0xedd3
};

static short const tsin_16k[16] = {
    0x0000, 0x30fd, 0x5a83, 0x7641, 0x7fff, 0x7642, 0x5a82, 0x30fc, 0x0000, 0xcf04, 0xa57d, 0x89be, 0x8000, 0x89be, 0xa57e, 0xcf05,
};
static u16 tx_s_cnt = 0;
static int get_sine_data(u16 *s_cnt, s16 *data, u16 points, u8 ch)
{
    while (points--) {
        if (*s_cnt >= 441) {
            *s_cnt = 0;
        }
        *data++ = tsin_441k[*s_cnt];
        if (ch == 2) {
            *data++ = tsin_441k[*s_cnt];
        }
        (*s_cnt)++;
    }
    return 0;
}

static int get_sine_16k_data(u16 *s_cnt, s16 *data, u16 points, u8 ch)
{
    while (points--) {
        if (*s_cnt >= 16) {
            *s_cnt = 0;
        }
        *data++ = tsin_16k[*s_cnt];
        if (ch == 2) {
            *data++ = tsin_16k[*s_cnt];
        }
        (*s_cnt)++;
    }
    return 0;
}

extern struct audio_decoder_task decode_task;
void handle_tx(void *priv, void *buf, u16 len)
{
    putchar('o');
#if 1
    /* get_sine_16k_data(&tx_s_cnt, buf, len / 2 / 2, 2); */
    get_sine_data(&tx_s_cnt, buf, len / 2 / 2, 2);
    printf("tx len %x  %d", buf, len);
    /* memset(buf, 0, len); */
    alink_set_shn(&p_alink0_parm->ch_cfg[0], len / 4);
#endif
}

extern struct audio_dac_hdl dac_hdl;
void handle_rx(void *priv, void *buf, u16 len)
{
    /* putchar('r'); */
#if 1
    put_buf(buf, 32);
#else

#endif
    alink_set_shn(&p_alink0_parm->ch_cfg[1], len / 4);
}

ALINK_PARM	test_alink = {
    .module = ALINK0,
    .mclk_io = IO_PORTA_12,
    .sclk_io = IO_PORTA_13,
    .lrclk_io = IO_PORTA_14,
    .ch_cfg[0].data_io = IO_PORTA_15,
    .ch_cfg[1].data_io = IO_PORTA_11,
    /* .mode = ALINK_MD_IIS_LALIGN, */
    .mode = ALINK_MD_IIS,
    /* .role = ALINK_ROLE_SLAVE, */
    .role = ALINK_ROLE_MASTER,
    /* .clk_mode = ALINK_CLK_RAISE_UPDATE_FALL_SAMPLE, */
    .clk_mode = ALINK_CLK_FALL_UPDATE_RAISE_SAMPLE,
    .bitwide = ALINK_LEN_16BIT,
    /* .bitwide = ALINK_LEN_24BIT, */
    .sclk_per_frame = ALINK_FRAME_32SCLK,
    /* .dma_len = ALNK_BUF_POINTS_NUM * 2 * 2 , */
    .dma_len = 4 * 1024,
    /* .sample_rate = 16000, */
    .sample_rate = 44100,
    /* .sample_rate = TCFG_IIS_SAMPLE_RATE, */
    /* .buf_mode = ALINK_BUF_CIRCLE, */
    .buf_mode = ALINK_BUF_DUAL,
};



void audio_link_test(void)
{
    void *hw_alink = alink_init(&test_alink);

    void *alink_ch0 =  alink_channel_init(hw_alink, 0, ALINK_DIR_TX, NULL, handle_tx);
    /* void *alink_ch1 =  alink_channel_init(hw_alink, 1, ALINK_DIR_RX, NULL, handle_rx); */
    /* void *alink_ch2 =  alink_channel_init(hw_alink, 2, ALINK_DIR_TX, NULL, handle_tx); */
    /* void *alink_ch3 =  alink_channel_init(hw_alink, 3, ALINK_DIR_RX, NULL, handle_rx); */
    clk_set("sys", 96 * 1000000L);
    alink_start(hw_alink);


    alink0_info_dump();
    while (1) {
        /* void clr_wdt(void); */
        /*         clr_wdt(); */
        os_time_dly(2);
    }
}

#endif

