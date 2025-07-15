SECTIONS
{
    .data : ALIGN(4)
    {
        . = ALIGN(4);
        audio_sync_data_begin = .;
            *(.audio_sync_data)
        audio_sync_data_end = .;
        . = ALIGN(4);

        *(.sbc_data)
        *(.msbc_data)
        *(.cvsd_data)
        *(.aac_data)
        *(.ldac_decoder_data)
        *(.sbc_eng_data)
        *(.bt_audioplc_data)
        *(.lc3_decoder_data)
        *(.lc3_dec_data)
        *(.wtgv2_data)
        *(.wtgv2dec_data)
		*(.opus_encoder_data)
		*(.speex_encoder_data)

        *(.anc_data)
        *(.anc_user_data)
        *(.anc_box_data)
        *(.anc_btspp_data)
	    *(.audio_adc_data)
        *(.audio_cfifo_data)

        *(.sms_data)
        . = ALIGN(4);
        *(.pSOUND360TD_cal_data)
        . = ALIGN(4);
        *(.jlsp_data)
        *(.jlsp_aec_data)
        *(.jlsp_nlp_data)
        *(.jlsp_dns_data)
        *(.jlsp_enc_data)
        *(.jlsp_prep_data)
        *(.jlsp_wn_data)
        *(.jlsp_tri_data)
        *(.jlsp_agc_data)
        . = ALIGN(4);
#if AUDIO_EFFECTS_GAIN_AT_RAM
		*(.audio_gain_code)
		*(.audio_gain_const)
#endif/*AUDIO_EFFECTS_GAIN_AT_RAM*/
    } > ram0

    .bss (NOLOAD) :ALIGN(4)
    {
        . = ALIGN(4);
        audio_sync_bss_begin = .;
        *(.audio_sync_bss)
        audio_sync_bss_end = .;
        . = ALIGN(4);

        *(.cvsd_bss)
        *(.aac_bss)
        *(.ldac_decoder_bss)
        *(.sbc_eng_bss)
        *(.bt_audioplc_bss)
        *(.lc3_decoder_bss)
        *(.lc3_dec_bss)
        *(.wtgv2_bss)
        *(.wtgv2dec_bss)
		*(.speex_encoder_bss)
		*(.opus_encoder_bss)

        *(.anc_bss)
        *(.anc_user_bss)
        *(.anc_box_bss)
        *(.anc_btspp_bss)
	    *(.audio_adc_bss)
        *(.audio_cfifo_bss)

		*(.audio_dec_bss)
		*(.audio_buf)
        *(.src_filt)
        *(.src_dma)

        *(.jlsp_bss)
        *(.jlsp_aec_bss)
        *(.jlsp_nlp_bss)
        *(.jlsp_dns_bss)
        *(.jlsp_enc_bss)
        *(.jlsp_prep_bss)
        *(.jlsp_wn_bss)
        *(.jlsp_tri_bss)
        *(.jlsp_agc_bss)
        . = ALIGN(4);
        *(.pSOUND360TD_cal_bss)

    } > ram0

    .text : ALIGN(4)
    {
        media_text_start = .;

		. = ALIGN(4);
        *(.stream_code)
        *(.howlings_phf_code)
        *(.notchhowling_sparse_code)
        *(.notchhowling_code)
        *(.howlings_phf_sparse_code)
        *(.notchhowling_const)
        *(.howlings_phf_const)

		. = ALIGN(4);
        *(.llns.text)
        *(.llns.text.const)

		. = ALIGN(4);
		*(.aec_code)
		*(.aec_const)

		. = ALIGN(4);
		*(.res_code)
		*(.res_const)

		. = ALIGN(4);
		*(.ns_code)
		*(.ns_const)
        *(.bark_const)

        . = ALIGN(4);
        *(.jlsp_code)
        . = ALIGN(4);
        *(.jlsp_const)
        . = ALIGN(4);
        *(.dns_16k_data)
        . = ALIGN(4);
        *(.dns_8k_data)
        . = ALIGN(4);
        *(.jlsp_dns_code)
        *(.jlsp_dns_const)
        *(.dns_param_data_wind)
        *(.dns_common_data_wind)

        *(.agc_code)
        *(.dms_code)
        *(.dms_sparse_code)

		*(.nlp_code)
		*(.nlp_const)
		*(.der_code)
		*(.der_const)
		*(.qmf_code)
		*(.qmf_const)
		*(.fft_code)
		*(.fft_const)
        *(.drc_sparse_code)
        *(.drc_code)
        *(.drc_const)
#if (AUDIO_EFFECTS_GAIN_AT_RAM == 0)
		*(.audio_gain_code)
		*(.audio_gain_const)
#endif
		*(.ns_sparse_code )
		*(.aec_sparse_code)
		*(.nlp_sparse_code)
		*(.der_sparse_code)
		*(.qmf_sparse_code)
		*(.bt_audioplc_code)
		*(.bt_audioplc_sparse_code)
		*(.bt_audioplc_const)
		. = ALIGN(4);
	    *(.lf_audioplc_code)
		*(.lf_audioplc_sparse_code)
		*(.lf_audioplc_const)

		. = ALIGN(4);
        *(.pcm_code)
        *(.pcm_const)
		/* *(.cvsd_code) */
		/* *(.cvsd_const) */
		/* *(.g729_code) */
		/* *(.g729_const) */
		*(.wtg_dec_code)
		*(.wtg_dec_sparse_code)
		*(.wtg_dec_const)
		*(.wtgv2_code)
		*(.wtgv2_const)
		*(.wtgv2dec_code)
		*(.wtgv2dec_const)
		*(.wtgv2dec_str)
		*(.wtg_decv2_sparse_code)
 		*(.speex_dec_code)
		*(.speex_dec_const)
		*(.speex_const)
		*(.speex_code)
		*(.speex_encoder_code)
		*(.speex_encoder_const)
	    *(.opus_code)
	    *(.opus_const)
		*(.opus_encoder_code)
		*(.opus_encoder_const)
		*(.mp3_code)
		*(.mp3_const)
		*(.mp3_dec_sparse_code)
		*(.mp3_dec_sparse_const)
		*(.mp3_dec_code)
		*(.mp3_dec_const)
        *(.bfilt_code)
		*(.msbc_code)
		*(.msbc_const)
		*(.mty_code)
		*(.mp3tsy_dec_code)
		*(.mp3tsy_dec_sparse_code)
		*(.mp3tsy_dec_const)
		*(.sbc_code)
		*(.sbc_const)
		*(.sine_code)
		*(.sine_const)
        *(.wav_code)
        *(.wav_const)
		*(.wav_dec_code)
		*(.wav_dec_const)
		*(.wav_dec_sparse_code)
		*(.wav_dec_sparse_const)
		*(.wma_code)
		*(.wma_const)
		*(.wma_dec_code)
		*(.wma_dec_sparse_code)
		*(.wma_dec_const)
        *(.aac_code)
        *(.aac_const)
        *(.ldac_decoder_code)
        *(.ldac_decoder_const)
        *(.bt_ldac_dec_code)
        *(.bt_ldac_dec_const)
		*(.amr_code)
		*(.amr_const)
		*(.amr_dec_code)
		*(.amr_dec_const)
		*(.amr_dec_sparse_code)
		*(.ape_code)
		*(.ape_const)
		*(.ape_dec_code)
		*(.ape_dec_sparse_code)
		*(.ape_dec_const)
		*(.dts_code)
		*(.dts_const)
		*(.dts_dec_sparse_code)
		*(.dts_dec_code)
		*(.dts_dec_const)
		*(.dts_dec_ff_const)
		*(.flac_code)
		*(.flac_const)
		*(.flac_dec_code)
		*(.flac_dec_const)
		*(.flac_dec_sparse_code)
		*(.audio_decoder_code)
		*(.audio_decoder_const)
        *(.audio_track_const)
        *(.audio_track_code)

		/* *(.cvsd_code) */
		/* *(.cvsd_const) */
		*(.m4a_code)
		*(.m4a_const)
		*(.m4a_dec_code)
		*(.m4a_dec_sparse_code)
		*(.m4a_dec_sparse_const)
		*(.bt_aac_dec_core_sparse_const)
		*(.bt_aac_dec_core_sparse_code)
		*(.bt_aac_dec_core_code)
		*(.bt_aac_dec_eng_code)
		*(.bt_aac_dec_eng_const)
		*(.alac_code)
		*(.alac_const)
		*(.alac_dec_code)
		*(.media_device_code)
		*(.media_device_const)
		*(.audio_encoder_code)
		*(.audio_encoder_const)
		*(.mixer_code)
		*(.mixer_const)
		*(.dec_server_code)
		*(.dec_server_const)
		*(.rec_server_code)
		*(.rec_server_const)
		*(.auto_mute_code)
		*(.auto_mute_const)
		*(.plc_code)
		*(.plc_const)
		*(.wireless_sync_code)
		*(.wireless_sync_const)
		*(.sbc_eng_code)
		*(.sbc_eng_const)

    	anc_code_begin = .;
		*(.anc_user_const)
		*(.anc_user_code)
		*(.anc_const)
		*(.anc_code)
		*(.anc_core_const)
		*(.anc_core_code)
		*(.anc_box_const)
		*(.anc_box_code)
		*(.anc_btspp_const)
		*(.anc_btspp_code)
    	anc_code_end = .;
		anc_code_size = anc_code_end - anc_code_begin;

		*(.audio_codec_code)
	    *(.audio_adc_code)
	    *(.audio_adc_const)
	    *(.audio_cfifo_code)
	    *(.audio_cfifo_const)

		*(.bt_compressor_sparse_const)
		*(.bt_compressor_sparse_code)
		*(.compressor_sparse_code)
		*(.compressor_sparse_const)

		*(.bt_limiter_sparse_const)
		*(.bt_limiter_sparse_code)
		*(.limiter_sparse_code)
		*(.limiter_sparse_const)

		*(.bt_crossOver_sparse_const)
		*(.bt_crossOver_sparse_code)
		*(.crossOver_sparse_code)
		*(.crossOver_sparse_const)

		*(.lib_pitchshift_code)
		*(.lib_pitchshift_const)

		. = ALIGN(4);
    	_audio_decoder_begin = .;
    	PROVIDE(audio_decoder_begin = .);
    	    *(.audio_decoder)
    	_audio_decoder_end = .;
    	PROVIDE(audio_decoder_end = .);

    	_audio_encoder_begin = .;
    	PROVIDE(audio_encoder_begin = .);
    	    *(.audio_encoder)
    	_audio_encoder_end = .;
    	PROVIDE(audio_encoder_end = .);

    	_audio_package_begin = .;
    	PROVIDE(audio_package_begin = .);
    	    *(.audio_package)
    	_audio_package_end = .;
    	PROVIDE(audio_package_end = .);

    	_audio_dev_begin = .;
    	PROVIDE(audio_dev_begin = .);
    	    *(.audio_device)
    	_audio_dev_end = .;
    	PROVIDE(audio_dev_end = .);

    	_audio_hwaccel_begin = .;
    	PROVIDE(audio_hwaccel_begin = .);
    	    *(.audio_hwaccel)
    	_audio_hwaccel_end = .;
    	PROVIDE(audio_hwaccel_end = .);

		. = ALIGN(4);
    	media_code_begin = .;
    	    *(.media.*.text)
		. = ALIGN(4);

		*(.compressor_code)
		*(.compressor_const)
		*(.bt_compressor_const)
		*(.bt_compressor_code)

		*(.crossOver_code)
		*(.crossOver_const)
		*(.bt_crossOver_const)
		*(.bt_crossOver_code)

		*(.limiter_code)
		*(.limiter_const)
		*(.bt_limiter_const)
		*(.bt_limiter_code)

        *(.lc3_decoder_const)
        *(.lc3_decoder_code)
        *(.lc3_dec_const)
        *(.lc3_dec_code)

		*(.audio_vbass_code)
		*(.audio_vbass_const)
		*(.vbss_code)
		*(.vbss_const)
        *(.vbss_sparse_code)
		*(.vbss_sparse_const)


        . = ALIGN(4);
        *(.pSOUND360TD_cal_sparse_code)

		. = ALIGN(4);
        *(.g729_code)
        *(.g729_const)
		*(.cvsd_code)
		*(.cvsd_const)

		*(.media.audio*)
		*(.media.mixer*)
		*(.media.media*)

		*( .lib_wtg_dec_code )
		*( .lib_wtg_dec_const)

		*(.audio_resample_code)
		*(.audio_resample_const)

        *(.resample_fastcal_sparse_code)
        *(.resample_fastcal_code)
        *(.resample_fastcal_const)

    	audio_sync_code_begin = .;
        *(.audio_sync_code)
    	audio_sync_code_end = .;

		. = ALIGN(4);

    	media_code_end = .;
		. = ALIGN(4);
		media_code_size = media_code_end - media_code_begin;

		. = ALIGN(4);
        media_text_end = .;
    } > code0


	.data_code ALIGN(32):
	{
	  	 *(.media.aec.text)
	} > ram0
}

