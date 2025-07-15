#ifndef __ADV_ANC_VOICE_KEY_H__
#define __ADV_ANC_VOICE_KEY_H__

#include "le_rcsp_adv_module.h"

void deal_anc_voice_key_setting(u8 *anc_key_setting, u8 write_vm, u8 tws_sync);
void get_anc_voice_key_mode(u8 *anc_voice_mode);
void set_anc_voice_key_mode(u8 *anc_voice_mode);

int update_anc_voice_key_opt(void);

#endif
