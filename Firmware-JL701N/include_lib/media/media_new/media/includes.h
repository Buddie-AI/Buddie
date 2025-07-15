/*****************************************************************
>file name : include_lib/media/includes.h
>author : lichao
>create time : Mon 26 Nov 2018 07:47:14 PM CST
*****************************************************************/
#ifndef __MEDIA_INCLUDES_H__
#define __MEDIA_INCLUDES_H__

#ifdef CONFIG_AUDIO_ONCHIP

#include "media/audio_decoder.h"
#include "media/audio_encoder.h"
#include "media/mixer.h"
#include "media/automute.h"
#include "media/audio_stream.h"

#include "asm/cpu_includes.h"
#include "application/eq_func_define.h"
/*
#include "asm/dac.h"
#include "asm/audio_adc.h"
#if (defined CONFIG_CPU_BR26 || \
     defined CONFIG_CPU_BR23 || \
     defined CONFIG_CPU_BR21 || \
     defined CONFIG_CPU_BR25 || \
	 defined CONFIG_CPU_BR30)
#include "asm/audio_src.h"
#endif
*/

/*encoder init*/
extern int msbc_encoder_init();
extern int cvsd_encoder_init();
extern int opus_encoder_preinit();
extern int speex_encoder_preinit();

/*decoder init*/
extern int pcm_decoder_enable();
extern int cvsd_decoder_init();
extern int msbc_decoder_init();
extern int g729_decoder_init();
extern int sbc_decoder_init();
extern int mp3_decoder_init();
extern int wma_decoder_init();
extern int wav_decoder_init();
extern int mty_decoder_init();
extern int flac_decoder_init();
extern int ape_decoder_init();
extern int m4a_decoder_init();
extern int amr_decoder_init();
extern int dts_decoder_init();
extern int mp3pick_decoder_init();
extern int wmapick_decoder_init();
extern int aac_decoder_init();

#if defined CONFIG_CPU_BR23 || \
    defined CONFIG_CPU_BR25
#include "asm/audio_spdif.h"
#endif

#endif/*CONFIG_AUDIO_ONCHIP*/
#endif
