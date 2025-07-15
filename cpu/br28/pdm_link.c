#include "includes.h"
#include "asm/includes.h"
#include "asm/gpio.h"
#include "asm/dac.h"
#include "media/includes.h"
#include "pdm_link.h"

#define PLNK_DEBUG_ENABLE
#ifdef PLNK_DEBUG_ENABLE
//#define PLNK_LOG(fmt, ...) 	printf("[PLNK] "fmt, ##__VA_ARGS__)
#define PLNK_LOG 		y_printf
#else
#define PLNK_LOG(...)
#endif


#define PLNK_TEST_ENABLE	0


static audio_plnk_t *pdm_hdl = NULL;
___interrupt
static void audio_plnk_isr(void)
{
    JL_PORTA->OUT ^= BIT(12);
    u8 buf_flag;
    s16 *buf = pdm_hdl->buf;
    u16 len = pdm_hdl->buf_len;
    s16 *data_buf;
    if (JL_PLNK->CON & BIT(15)) {
        PLNK_PND_CLR();
        if (JL_PLNK->CON & BIT(9)) {
            buf_flag = 0;    //buf0 available
        } else {
            buf_flag = 1;    //buf1 available
        }
        data_buf = (s16 *)buf;
        data_buf += buf_flag * len;
    }
    if (pdm_hdl && pdm_hdl->output) {
        pdm_hdl->output(data_buf, len);
    }
}

static void plnk_info_dump()
{
    PLNK_LOG("PLNK_CON = 0x%x", JL_PLNK->CON);
    PLNK_LOG("CLK_CON = 0x%x", JL_CLOCK->CLK_CON0);
    PLNK_LOG("PLNK_CON1 = 0x%x", JL_PLNK->CON1);
    PLNK_LOG("PLNK_LEN = 0x%x", JL_PLNK->LEN);
    PLNK_LOG("PLNK_ADR = 0x%x", JL_PLNK->ADR);
    PLNK_LOG("PLNK_SMR = %d", JL_PLNK->SMR);
    PLNK_LOG("PLNK_DOR = %d", JL_PLNK->DOR);
}

static void audio_plnk_sr_set(u16 sr)
{
    PLNK_LOG("PLNK_SR = %d\n", sr);
    JL_PLNK->DOR = PLNK_SCLK / sr - 1;
    u8 div = PLNK_CLK / PLNK_SCLK - 1;
    SFR(JL_ASS->CLK_CON, 2, 5, div);
}

static void audio_plnk_sclk_io_init(u8 port)
{
    r_printf("port === %d\n", port);
    gpio_set_fun_output_port(port, FO_PLNK_SCLK, 0, 1, LOW_POWER_FREE);
    gpio_set_direction(port, 0);
    gpio_set_die(port, 0);
    gpio_direction_output(port, 1);
}


static void audio_plnk_ch_io_init(u8 ch_index, u8 port)
{
    gpio_set_pull_down(port, 0);
    gpio_set_pull_up(port, 1);
    gpio_set_direction(port, 1);
    gpio_set_die(port, 1);
    if (ch_index) {
        gpio_set_fun_input_port(port, PFI_PLNK_DAT1, LOW_POWER_FREE);
    } else {
        gpio_set_fun_input_port(port, PFI_PLNK_DAT0, LOW_POWER_FREE);
    }
}


int audio_plnk_open(audio_plnk_t *hdl)
{
    if (hdl == NULL) {
        return -EINVAL;
    }

    pdm_hdl = hdl;
    PLNK_LOG("audio_plnk_open,ch=%d,sr=%d", pdm_hdl->ch_num, pdm_hdl->sr);
    PLNK_CON_RESET();
    JL_PLNK->ADR = (u32)pdm_hdl->buf;
    JL_PLNK->LEN = pdm_hdl->buf_len;
    /*
     *PLNK_CLK -> SMR -> SCLK -> DOR ->SR
     *1MHz <= SCLK <= 4MHz
     *sclk = PLINK_CLK / (SMR + 1)
     *5 <= SMR <= 47
     *PLNK_DOR
     *Version(A/B):0~4096
     *Version(C):1~2048
     */
    SFR(JL_PLNK->CON, 1, 1, 0);	//DFDLY1_SEL(0:M=1 1:M=2)
    SFR(JL_PLNK->CON, 5, 1, 0);	//DFDLY1_SEL(0:M=1 1:M=2)
    audio_plnk_sr_set(pdm_hdl->sr);
    audio_plnk_sclk_io_init(pdm_hdl->sclk_io);
    audio_plnk_ch_io_init(0, pdm_hdl->ch0_io);
    /* audio_plnk_ch_io_init(1, pdm_hdl->ch1_io); */
    SFR(JL_PLNK->CON1, 26, 1, 0);	//DFDLY1_SEL(0:M=1 1:M=2)
    SFR(JL_PLNK->CON1, 10, 1, 0); 	//DFDLY0_SEL(0:M=1 1:M=2)

    SFR(JL_PLNK->CON1, 19, 5, 10);	//SHIFT1_SEL[23:19]
    SFR(JL_PLNK->CON1, 3, 5, 10); 	//SHIFT0_SEL[7:3]

    SFR(JL_PLNK->CON1, 16, 3, 3);	//SCALE1_SEL
    SFR(JL_PLNK->CON1, 0, 3, 3);	//SCALE0_SEL

    SFR(JL_PLNK->CON1, 24, 2, 0);	//ORDER1_SEL
    SFR(JL_PLNK->CON1, 8, 2, 0);	//ORDER0_SEL

    SFR(JL_PLNK->CON1, 27, 1, 0);	//SHDIR1_SEL
    SFR(JL_PLNK->CON1, 11, 1, 0);	//SHDIR0_SEL

    SFR(JL_PLNK->CON1, 28, 2, 0);
    SFR(JL_PLNK->CON1, 12, 2, 0);

    SFR(JL_PLNK->CON1, 30, 1, 0);	//CIC1_EN
    SFR(JL_PLNK->CON1, 14, 1, 0);	//CIC0_EN


    if (PLNK_CH_EN == PLNK_CH0_EN) {
        PLNK_CH0_MD_SET(pdm_hdl->ch0_mode);
    } else if (PLNK_CH_EN == PLNK_CH1_EN) {
        PLNK_CH1_MD_SET(pdm_hdl->ch1_mode);
    } else {
        PLNK_CH0_MD_SET(pdm_hdl->ch0_mode);
        PLNK_CH1_MD_SET(pdm_hdl->ch1_mode);
    }
    PLNK_PND_CLR();
    request_irq(IRQ_PDM_LINK_IDX, 1, audio_plnk_isr, 0);
    return 0;
}

int audio_plnk_start(audio_plnk_t *hdl)
{
    JL_PLNK->CON |= BIT(8); //IE
    if (hdl->ch_num == 1) {
        SFR(JL_PLNK->CON, 0, 1, 1);  //CH0EN
        SFR(JL_PLNK->CON, 4, 1, 0);  //CH1 DIS
    } else {
        SFR(JL_PLNK->CON, 0, 1, 1);  //CH0EN
        SFR(JL_PLNK->CON, 4, 1, 1);  //CH1EN
    }
    plnk_info_dump();
    return 0;
}

int audio_plnk_close(void)
{
    JL_PLNK->CON = 0;
    pdm_hdl = NULL;
    return 0;
}

#if PLNK_TEST_ENABLE

extern struct audio_dac_hdl dac_hdl;

static short const sin0_16k[16] = {
    0x0000, 0x30fd, 0x5a83, 0x7641, 0x7fff, 0x7642, 0x5a82, 0x30fc, 0x0000, 0xcf04, 0xa57d, 0x89be, 0x8000, 0x89be, 0xa57e, 0xcf05,
};

static u16 tx1_s_cnt = 0;
static int get_sine0_data(u16 *s_cnt, s16 *data, u16 points, u8 ch)
{
    while (points--) {
        if (*s_cnt >= 16) {
            *s_cnt = 0;
        }
        *data++ = sin0_16k[*s_cnt];
        if (ch == 2) {
            *data++ = sin0_16k[*s_cnt];
        }
        (*s_cnt)++;
    }
    return 0;
}

static void handle_output(void *buf, u16 len)
{
    /* get_sine0_data(&tx1_s_cnt, buf, 256, 1); */
    s16 *paddr = (s16 *)buf;
    u32 len0 = 256 * 2;
    u32 rlen = 256 * 2;
    while (len0) {
        rlen = audio_dac_write(&dac_hdl, paddr, len0);
        /* printf("rlen = %d\n", rlen); */
        if (rlen != len0) {
            printf("rlen = %d, len0 = %d\n", rlen, len0);
        }
        len0 -= rlen;
        paddr += rlen / 2;
        if (rlen == 0) {
        }
    }
}

audio_plnk_t	test_plink = {
    .sclk_io = IO_PORTA_04,
    .ch_num	= 1,
    .ch0_io = IO_PORTA_03,
    .ch0_mode = CH0MD_CH0_SCLK_RISING_EDGE,
    .sr = 16000,
    .buf_len = 256,
};

void audio_plnk_test(void)
{
    y_printf("pdm link demo\n");

    test_plink.output = handle_output;
    test_plink.buf = zalloc(test_plink.buf_len * test_plink.ch_num * 2 * 2);
    ASSERT(test_plink.buf);
    /* JL_WL_AUD->CON0 |= BIT(0); */

    audio_plnk_open(&test_plink);
    audio_plnk_start(&test_plink);

#if 1
    extern void app_audio_state_switch(u8 state, s16 max_volume);
    app_audio_state_switch(1, 16);
    audio_dac_set_volume(&dac_hdl, 16);
    audio_dac_set_sample_rate(&dac_hdl, 16000);
    audio_dac_start(&dac_hdl);
    SFR(JL_ASS->CLK_CON, 0, 2, 3);
#endif

    while (1);

}

#endif
