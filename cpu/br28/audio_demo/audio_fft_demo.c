/*
 ****************************************************************************
 *							Audio FFT Demo
 *
 *Description	: Audio FFT使用范例
 *Notes			:
 *(1)本demo为开发测试范例，请不要修改该demo， 如有需求，请自行复制再修改
 *(2)FFT输入数据位宽
 *----------------------------------------------------------------------
 *	FFT模式		FFT点数			最大输入位宽
 *----------------------------------------------------------------------
 *	复数		64				[21:0]
 *				128				[20:0]
 *				256				[19:0]
 *				512				[18:0]
 *				1024			[17:0]
 *----------------------------------------------------------------------
 *	实数		64				[20:0]
 *				128				[18:0]
 *				256				[17:0]
 *				512				[18:0]
 *				1024			[16:0]
 *----------------------------------------------------------------------
 *(3)IFFT输入位宽：[29:0]
 ****************************************************************************
 */

#include "audio_demo.h"
#include "hw_fft.h"

// For 128 point Real_FFT test
void hw_fft_demo_real()
{

    int tmpbuf[130];  // 130 = (128/2+1)*2
    unsigned int fft_config;

    printf("********* test start  **************\n");

    for (int i = 0; i < 128; i++) {
        tmpbuf[i] = i + 1;
        printf("tmpbuf[%d]: %d \n", i, tmpbuf[i]);
    }

    // Do 128 point FFT
    fft_config = hw_fft_config(128, 7, 1, 0, 1);

    hw_fft_run(fft_config, tmpbuf, tmpbuf);

    for (int i = 0; i < 130; i++) {
        printf("tmpbuf_[%d]: %d \n", i, tmpbuf[i]);
    }

    // Do 128 point IFFT
    fft_config = hw_fft_config(128, 7, 1, 1, 1);

    hw_fft_run(fft_config, tmpbuf, tmpbuf);

    for (int i = 0; i < 128; i++) {
        printf("tmpbuf_[%d]: %d \n", i, tmpbuf[i]);
    }
}

// For 128 point Complex_FFT test
void hw_fft_demo_complex()
{

    int tmpbuf[280];    // 280 = (128/2+1)*2*2
    unsigned int fft_config;

    printf("********* test start  **************\n");

    for (int i = 0; i < 128; i++) {
        tmpbuf[2 * i] = i + 1;
        tmpbuf[2 * i + 1] = i + 1;
        printf("tmpbuf[%d]: %d \n", 2 * i, tmpbuf[2 * i]);
        printf("tmpbuf[%d]: %d \n", 2 * i + 1, tmpbuf[2 * i + 1]);
    }

    // Do 128 point FFT

    fft_config = hw_fft_config(128, 7, 1, 0, 0);

    hw_fft_run(fft_config, tmpbuf, tmpbuf);

    for (int i = 0; i < 128; i++) {
        printf("tmpbuf[%d]: %d \n", 2 * i, tmpbuf[2 * i]);
        printf("tmpbuf[%d]: %d \n", 2 * i + 1, tmpbuf[2 * i + 1]);
    }

    // Do 128 point IFFT
    fft_config = hw_fft_config(128, 7, 1, 1, 0);

    hw_fft_run(fft_config, tmpbuf, tmpbuf);

    for (int i = 0; i < 128; i++) {
        printf("tmpbuf[%d]: %d \n", 2 * i, tmpbuf[2 * i]);
        printf("tmpbuf[%d]: %d \n", 2 * i + 1, tmpbuf[2 * i + 1]);
    }
}

