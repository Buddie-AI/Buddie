#ifndef __ADV_HEARING_AID_SETTING_H__
#define __ADV_HEARING_AID_SETTING_H__

#include "le_rcsp_adv_module.h"
#include "audio_hearing_aid.h"

// 是否使用三方的ble/edr数据传输
#define	ASSISTED_HEARING_CUSTOM_TRASNDATA		0

void adv_hearing_aid_init(void);

void set_hearing_aid_setting(u8 *hear_aid_setting);
void get_hearing_aid_setting(u8 *hear_aid_setting);
void deal_hearing_aid_setting(u8 opCode, u8 opCode_SN, u8 *hear_aid_setting, u8 write_en, u8 type_sync);
void get_dha_fitting_info(u8 *dha_fitting_info);

// 主动推数给app
u32 hearing_aid_rcsp_notify(u8 *data, u16 data_len);
// app回复函数
u32 hearing_aid_rcsp_response(u8 *data, u16 data_len);

// 第三方调用函数
int app_third_party_hearing_aid_handle(u8 *data, u16 len);
int app_third_party_hearing_aid_resp(u8 *data, u16 len);

#endif
