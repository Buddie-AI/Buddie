// *INDENT-OFF*
#include "app_config.h"

/* =================  BR28 SDK memory ========================
 _______________ ___ 0x1A0000(176K)
|   isr base    |
|_______________|___ _IRQ_MEM_ADDR(size = 0x100)
|rom export ram |
|_______________|
|    update     |
|_______________|___ RAM_LIMIT_H
|     HEAP      |
|_______________|___ data_code_pc_limit_H
| audio overlay |
|_______________|
|   data_code   |
|_______________|___ data_code_pc_limit_L
|     bss       |
|_______________|
|     data      |
|_______________|
|   irq_stack   |
|_______________|
|   boot info   |
|_______________|
|     TLB       |
|_______________|0 RAM_LIMIT_L

 =========================================================== */

#include "maskrom_stubs.ld"

EXTERN(
_start
#include "sdk_used_list.c"
);

UPDATA_SIZE     = 0x80;
UPDATA_BEG      = _MASK_MEM_BEGIN - UPDATA_SIZE;
UPDATA_BREDR_BASE_BEG = 0x1A0000;

RAM_LIMIT_L     = 0x100000;
RAM_LIMIT_H     = UPDATA_BEG;
PHY_RAM_SIZE    = RAM_LIMIT_H - RAM_LIMIT_L;

//from mask export
ISR_BASE       = _IRQ_MEM_ADDR;
ROM_RAM_SIZE   = _MASK_MEM_SIZE;
ROM_RAM_BEG    = _MASK_MEM_BEGIN;

#ifdef TCFG_LOWPOWER_RAM_SIZE
RAM0_BEG 		= RAM_LIMIT_L + (128K * TCFG_LOWPOWER_RAM_SIZE);
#else
RAM0_BEG 		= RAM_LIMIT_L;
#endif
RAM0_END 		= RAM_LIMIT_H;
RAM0_SIZE 		= RAM0_END - RAM0_BEG;

RAM1_BEG 		= RAM_LIMIT_L;
RAM1_END 		= RAM0_BEG;
RAM1_SIZE 		= RAM1_END - RAM1_BEG;

#ifdef CONFIG_NEW_CFG_TOOL_ENABLE
CODE_BEG        = 0x6000100;
#else
CODE_BEG        = 0x6000120;
#endif

//============ About EQ coeff RAM ================
//internal EQ priv ram
EQ_PRIV_COEFF_BASE  =0x1A0600;
EQ_PRIV_SECTION_NUM = 10;
EQ_PRIV_COEFF_END   = EQ_PRIV_COEFF_BASE + 4 * EQ_PRIV_SECTION_NUM * (5+3)*2;
EQ_SECTION_NUM = EQ_SECTION_MAX;

//=============== About BT RAM ===================
//CONFIG_BT_RX_BUFF_SIZE = (1024 * 18);

MEMORY
{
	code0(rx)    	  : ORIGIN = CODE_BEG,  LENGTH = CONFIG_FLASH_SIZE
	ram0(rwx)         : ORIGIN = RAM0_BEG,  LENGTH = RAM0_SIZE

    //ram1 - 用于volatile-heap
	//ram1(rwx)         : ORIGIN = RAM1_BEG,  LENGTH = RAM1_SIZE

	psr_ram(rwx)        : ORIGIN = 0x2000000,  LENGTH = 8*1024*1024
}


ENTRY(_start)

SECTIONS
{
    . = ORIGIN(code0);
    .text ALIGN(4):
    {
        PROVIDE(text_rodata_begin = .);

        *(.startup.text)

		*(.text)

		. = ALIGN(4);
	    update_target_begin = .;
	    PROVIDE(update_target_begin = .);
	    KEEP(*(.update_target))
	    update_target_end = .;
	    PROVIDE(update_target_end = .);
		. = ALIGN(4);

		*(.LOG_TAG_CONST*)
        *(.rodata*)

		. = ALIGN(4); // must at tail, make rom_code size align 4
        PROVIDE(text_rodata_end = .);

        clock_critical_handler_begin = .;
        KEEP(*(.clock_critical_txt))
        clock_critical_handler_end = .;

        chargestore_handler_begin = .;
        KEEP(*(.chargestore_callback_txt))
        chargestore_handler_end = .;

		/********maskrom arithmetic ****/
        *(.opcore_table_maskrom)
        *(.bfilt_table_maskroom)
        *(.opcore_maskrom)
        *(.bfilt_code)
        *(.bfilt_const)
		/********maskrom arithmetic end****/

		. = ALIGN(4);
        #include "ui/ui/ui.ld"
		. = ALIGN(4);
		__VERSION_BEGIN = .;
        KEEP(*(.sys.version))
        __VERSION_END = .;

        *(.noop_version)
		. = ALIGN(4);

		. = ALIGN(4);
		tool_interface_begin = .;
		KEEP(*(.tool_interface))
		tool_interface_end = .;
        . = ALIGN(4);
        cmd_interface_begin = .;
        KEEP(*(.eff_cmd))
        cmd_interface_end = .;
		. = ALIGN(4);


		. = ALIGN(32);
		m_code_addr = . ;
		*(.m.code*)
		*(.movable.code*)
			m_code_size = ABSOLUTE(. - m_code_addr) ;
		. = ALIGN(32);
	  } > code0

    . = ORIGIN(ram0);
	//TLB 起始需要16K 对齐；
    .mmu_tlb ALIGN(0x10000):
    {
        *(.mmu_tlb_segment);
    } > ram0

	.boot_info ALIGN(32):
	{
		*(.boot_info)
        . = ALIGN(32);
	} > ram0

	.irq_stack ALIGN(32):
    {
        _cpu0_sstack_begin = .;
        *(.cpu0_stack)
        _cpu0_sstack_end = .;
		. = ALIGN(4);

        _cpu1_sstack_begin = .;
        *(.cpu1_stack)
        _cpu1_sstack_end = .;
		. = ALIGN(4);

    } > ram0

	.data ALIGN(32):
	{
		//cpu start
        . = ALIGN(4);
        *(.data_magic)

        . = ALIGN(4);
		#include "media/cpu/br28/audio_lib_data.ld"

		. = ALIGN(4);
        *(.data*)

		. = ALIGN(4);
	} > ram0

	.bss ALIGN(32):
    {
        . = ALIGN(4);
        *(.bss)
        . = ALIGN(4);
        *(.volatile_ram)

		*(.btstack_pool)

        *(.mem_heap)
		*(.memp_memory_x)

        . = ALIGN(4);
		*(.non_volatile_ram)

        . = ALIGN(32);

        *(.usb_h_dma)
        *(.usb_ep0)

#if TCFG_USB_CDC_BACKGROUND_RUN
        *(.usb_cdc_dma)
        *(.usb_config_var)
        *(.cdc_var)
#endif

        *(.usb_audio_play_dma)
        *(.usb_audio_rec_dma)
        *(.uac_rx)
        *(.mass_storage)

        *(.usb_msd_dma)
        *(.usb_hid_dma)
        *(.usb_iso_dma)
        *(.usb_cdc_dma)
        *(.uac_var)
        *(.usb_config_var)
        *(.cdc_var)
        . = ALIGN(32);

    } > ram0

	.data_code ALIGN(32):
	{
		data_code_pc_limit_begin = .;
		*(.flushinv_icache)
        *(.cache)
        *(.os_critical_code)
        *(.volatile_ram_code)
        *(.chargebox_code)
        *(.fat_data_code)
        *(.fat_data_code_ex)

        *(.ui_ram)
        *(.math_fast_funtion_code)
        . = ALIGN(4);
        *(.pSOUND360TD_cal_const)
        *(.pSOUND360TD_cal_code)
        . = ALIGN(4);
        *(.SpatialAudio.text.const)
        *(.SpatialAudio.text)
        *(.SpatialAudio.text.cache.L1)

		. = ALIGN(4);
        _SPI_CODE_START = . ;
        *(.spi_code)
		. = ALIGN(4);
        _SPI_CODE_END = . ;
		. = ALIGN(4);

	} > ram0

	.moveable_slot ALIGN(4):
	{
	    *(movable.slot.*)

	} >ram0

	__report_overlay_begin = .;
    #include "app_overlay.ld"

	data_code_pc_limit_end = .;
	__report_overlay_end = .;

	_HEAP_BEGIN = . ;
	_HEAP_END = RAM0_END;


    . = ORIGIN(psr_ram);
	.psr_data_code ALIGN(32):
	{
	    *(.psram_data_code)
		. = ALIGN(4);
	} > psr_ram

	.psr_bss_code ALIGN(32):
	{
		. = ALIGN(4);
	} > psr_ram
}

#include "app.ld"
#include "update/update.ld"
#include "btstack/btstack_lib.ld"
#include "system/port/br28/system_lib.ld"
#include "btctrler/port/br28/btctler_lib.ld"
#include "driver/cpu/br28/driver_lib.ld"
#include "media/cpu/br28/audio_lib.ld"
//================== Section Info Export ====================//
text_begin  = ADDR(.text);
text_size   = SIZEOF(.text);
text_end    = text_begin + text_size;

bss_begin = ADDR(.bss);
bss_size  = SIZEOF(.bss);
bss_end    = bss_begin + bss_size;

data_addr = ADDR(.data);
data_begin = text_begin + text_size;
data_size =  SIZEOF(.data);

moveable_slot_addr = ADDR(.moveable_slot);
moveable_slot_begin = data_begin + data_size;
moveable_slot_size =  SIZEOF(.moveable_slot);

data_code_addr = ADDR(.data_code);
data_code_begin = moveable_slot_begin + moveable_slot_size;
data_code_size =  SIZEOF(.data_code);


//================ OVERLAY Code Info Export ==================//
aec_addr = ADDR(.overlay_aec);
aec_begin = data_code_begin + data_code_size;
aec_size =  SIZEOF(.overlay_aec);

aac_addr = ADDR(.overlay_aac);
aac_begin = aec_begin + aec_size;
aac_size =  SIZEOF(.overlay_aac);

psr_data_code_addr = ADDR(.psr_data_code);
psr_data_code_begin = aac_begin + aac_size;
psr_data_code_size =  SIZEOF(.psr_data_code);

/*
lc3_addr = ADDR(.overlay_lc3);
lc3_begin = aac_begin + aac_size;
lc3_size = SIZEOF(.overlay_lc3);
*/

/* moveable_addr = ADDR(.overlay_moveable) ; */
/* moveable_size = SIZEOF(.overlay_moveable) ; */
//===================== HEAP Info Export =====================//
ASSERT(_HEAP_BEGIN > bss_begin,"_HEAP_BEGIN < bss_begin");
ASSERT(_HEAP_BEGIN > data_addr,"_HEAP_BEGIN < data_addr");
ASSERT(_HEAP_BEGIN > data_code_addr,"_HEAP_BEGIN < data_code_addr");
/* ASSERT(_HEAP_BEGIN > moveable_slot_addr,"_HEAP_BEGIN < moveable_slot_addr"); */
/* ASSERT(_HEAP_BEGIN > __report_overlay_begin,"_HEAP_BEGIN < __report_overlay_begin"); */

PROVIDE(HEAP_BEGIN = _HEAP_BEGIN);
PROVIDE(HEAP_END = _HEAP_END);
_MALLOC_SIZE = _HEAP_END - _HEAP_BEGIN;
PROVIDE(MALLOC_SIZE = _HEAP_END - _HEAP_BEGIN);

ASSERT(MALLOC_SIZE  >= 0x8000, "heap space too small !")

//============================================================//
//=== report section info begin:
//============================================================//
report_text_beign = ADDR(.text);
report_text_size = SIZEOF(.text);
report_text_end = report_text_beign + report_text_size;

report_mmu_tlb_begin = ADDR(.mmu_tlb);
report_mmu_tlb_size = SIZEOF(.mmu_tlb);
report_mmu_tlb_end = report_mmu_tlb_begin + report_mmu_tlb_size;

report_boot_info_begin = ADDR(.boot_info);
report_boot_info_size = SIZEOF(.boot_info);
report_boot_info_end = report_boot_info_begin + report_boot_info_size;

report_irq_stack_begin = ADDR(.irq_stack);
report_irq_stack_size = SIZEOF(.irq_stack);
report_irq_stack_end = report_irq_stack_begin + report_irq_stack_size;

report_data_begin = ADDR(.data);
report_data_size = SIZEOF(.data);
report_data_end = report_data_begin + report_data_size;

report_bss_begin = ADDR(.bss);
report_bss_size = SIZEOF(.bss);
report_bss_end = report_bss_begin + report_bss_size;

report_data_code_begin = ADDR(.data_code);
report_data_code_size = SIZEOF(.data_code);
report_data_code_end = report_data_code_begin + report_data_code_size;

report_overlay_begin = __report_overlay_begin;
report_overlay_size = __report_overlay_end - __report_overlay_begin;
report_overlay_end = __report_overlay_end;

report_heap_beign = _HEAP_BEGIN;
report_heap_size = _HEAP_END - _HEAP_BEGIN;
report_heap_end = _HEAP_END;

BR28_PHY_RAM_SIZE = PHY_RAM_SIZE;
BR28_SDK_RAM_SIZE = report_mmu_tlb_size + \
					report_boot_info_size + \
					report_irq_stack_size + \
					report_data_size + \
					report_bss_size + \
					report_overlay_size + \
					report_data_code_size + \
					report_heap_size;
//============================================================//
//=== report section info end
//============================================================//

