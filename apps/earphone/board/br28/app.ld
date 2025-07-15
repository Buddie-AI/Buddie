#include "app_config.h"

SECTIONS
{
    .text : ALIGN(4)
    {
        gsensor_dev_begin = .;
        KEEP(*(.gsensor_dev))
        gsensor_dev_end = .;

        fm_dev_begin = .;
        KEEP(*(.fm_dev))
        fm_dev_end = .;

        fm_emitter_dev_begin = .;
        KEEP(*(.fm_emitter_dev))
        fm_emitter_dev_end = .;

        storage_device_begin = .;
        KEEP(*(.storage_device))
        storage_device_end = .;

        imusensor_dev_begin = .;
        KEEP(*(.imusensor_dev))
        imusensor_dev_end = .;

#if TCFG_APP_PC_EN
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

		. = ALIGN(32);
    }

	.data ALIGN(32):
	{
	} > ram0

	.bss ALIGN(32):
    {
    } > ram0

	.data_code ALIGN(32):
	{
	} > ram0

}
