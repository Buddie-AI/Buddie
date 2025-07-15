
	OVERLAY : AT(0x200000) SUBALIGN(4)
    {
		.overlay_aec
		{
			aec_code_begin  = . ;
			*(.text._*)
			*(.data._*)
			*(.aec_code)
			*(.aec_const)
			*(.res_code)
			*(.res_const)
			*(.ns_code)
			*(.ns_const)
            *(.bark_const)
			*(.fft_code)
			*(.fft_const)
            *(.agc_code)
            *(.dms_code)
			*(.dms_const)
            *(.dms_sparse_code)
			aec_code_end = . ;
			aec_code_size = aec_code_end - aec_code_begin ;

			*(.msbc_enc)
			*(.cvsd_codec)
			*(.aec_bss)
			*(.aec_data)
			*(.res_data)
			*(.ns_data)
            *(.dns_common_data)
            *(.dns_param_data_single)
            *(.dns_param_data_dual)
            *(.jlsp_nlp_code)
            *(.jlsp_nlp_const)
            *(.jlsp_aec_code)
            *(.jlsp_aec_const)
            *(.jlsp_prep_code)
            *(.jlsp_prep_const)
            *(.jlsp_enc_code)
            *(.jlsp_enc_const)
            *(.jlsp_wn_code)
            *(.jlsp_wn_const)
            *(.jlsp_tri_code)
            *(.jlsp_tri_const)
            *(.jlsp_agc_code)
            *(.jlsp_agc_const)
			*(.res_bss)
			*(.ns_bss)
			*(.aec_mem)
		}
		.overlay_aac
		{
#if !TCFG_APP_PC_EN
	        aac_dec_code_begin = .;
	        *(.bt_aac_dec_code)
	        *(.bt_aac_dec_sparse_code)
	        aac_dec_code_end = .;
	        aac_dec_code_size  = aac_dec_code_end - aac_dec_code_begin ;

	        . = ALIGN(4);
	        bt_aac_dec_const_begin = .;
	        *(.bt_aac_dec_const)
	        *(.bt_aac_dec_sparse_const)
	        bt_aac_dec_const_end = .;
	        bt_aac_dec_const_size = bt_aac_dec_const_end -  bt_aac_dec_const_begin ;

	        *(.bt_aac_dec_data)
	        *(.bt_aac_dec_bss)

	        . = ALIGN(4);
	        *(.aac_mem)
	        *(.aac_ctrl_mem)
	        /* 		. += 0x5fe8 ; */
	        /*		. += 0xef88 ; */
#endif
		}
        /*
        .overlay_lc3
        {
            lc3_dec_code_begin = .;
            *(.lc3_dec_code)
            lc3_dec_code_end = .;
            lc3_dec_code_size = lc3_dec_code_end - lc3_dec_code_begin;

            . = ALIGN(4);
            lc3_dec_const_begin = .;
            *(.lc3_dec_const)
            lc3_dec_const_end = .;
            lc3_dec_const_size = lc3_dec_const_end - lc3_dec_const_begin;
            *(.lc3_dec_data)
            *(.lc3_dec_bss)
        }
        */

		.overlay_mp3
		{
			*(.mp3_mem)
			*(.mp3_ctrl_mem)
			*(.mp3pick_mem)
			*(.mp3pick_ctrl_mem)
			*(.dec2tws_mem)
		}
		.overlay_wma
		{
			*(.wma_mem)
			*(.wma_ctrl_mem)
			*(.wmapick_mem)
			*(.wmapick_ctrl_mem)
		}
		.overlay_wav
		{
			*(.wav_mem)
			*(.wav_ctrl_mem)
		}
		.overlay_ape
        {
            *(.ape_mem)
            *(.ape_ctrl_mem)
		}
		.overlay_flac
        {
            *(.flac_mem)
            *(.flac_ctrl_mem)
		}
		.overlay_m4a
        {
            *(.m4a_mem)
            *(.m4a_ctrl_mem)
		}
		.overlay_amr
        {
            *(.amr_mem)
            *(.amr_ctrl_mem)
		}
		.overlay_dts
        {
            *(.dts_mem)
            *(.dts_ctrl_mem)
		}
		.overlay_fm
		{
			*(.fm_mem)
		}
        .overlay_pc
		{
            *(.usb_audio_play_dma)
            *(.usb_audio_rec_dma)
            *(.uac_rx)
            *(.mass_storage)

            *(.usb_ep0)
            *(.usb_msd_dma)
            *(.usb_hid_dma)
            *(.usb_iso_dma)
            *(.usb_cdc_dma)
            *(.uac_var)
            *(.usb_config_var)
            *(.cdc_var)
		}

    } > ram0


