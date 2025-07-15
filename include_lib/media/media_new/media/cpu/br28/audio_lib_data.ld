
	. = ALIGN(4);
	audio_sync_data_begin = .;
        *(.audio_sync_data)
	audio_sync_data_end = .;
	. = ALIGN(4);

    *(.sbc_data)
    *(.msbc_data)
    *(.cvsd_data)
    *(.aac_data)
	*(.sbc_eng_data)
	*(.bt_audioplc_data)
    *(.lc3_decoder_data)
    *(.lc3_dec_data)
    *(.wtgv2_data)
    *(.wtgv2dec_data)

    *(.anc_data)
	*(.anc_core_data)
    *(.anc_user_data)
    *(.anc_box_data)
    *(.anc_btspp_data)

	*(.sms_data)
	*(.audio_track_data)
	*(.audio_adc_data)
