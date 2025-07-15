#ifndef  __EFUSE_H__
#define  __EFUSE_H__


void   efuse_init();
u16 get_chip_id();
u16 get_vbat_trim();
u16 get_vbat_trim_435();
u16 get_vbg_trim();
u8 get_sysdvdd_trim();
u32 get_chip_version();
/* struct lrc_config_t {                */
/*     u16 lrc_ws_inc;			//from uboot   */
/*     u16 lrc_ws_init;		//from uboot   */
/*     u16 btosc_ws_inc;		//from uboot  */
/*     u16 btosc_ws_init;		//from uboot */
/*     u8 lrc_change_mode;	//from uboot */
/* };                                   */
u16 get_lrc_ws_inc();			//from uboot
u16 get_lrc_ws_init();		//from uboot
u16 get_btosc_ws_inc();		//from uboot
u16 get_btosc_ws_init();		//from uboot
u8 get_lrc_change_mode();	//from uboot
u8 get_wvdd_level_trim();

u32 get_boot_flag();
void set_boot_flag(u32 flag);

u8 get_btvbg_xosc_trim();

u8 get_vad_vbg_trim();

#endif  /*EFUSE_H*/
