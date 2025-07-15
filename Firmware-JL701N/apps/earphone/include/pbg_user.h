#ifndef PBG_USER_H_
#define PBG_USER_H_

#include "typedef.h"
#include "key_event_deal.h"

enum {
    PBG_POS_IN_EAR = 0, //入耳
    PBG_POS_OUT_BOX,    //出仓
    PBG_POS_IN_BOX,     //在仓
    PBG_POS_NOT_EXIST,  //不在线
    PBG_POS_KEEP_NOW = 0x0f,  //维持，不改变
    PBG_POS_MAX,  //
};


#define BD_ADDR_LEN 6
typedef uint8_t bd_addr_t[BD_ADDR_LEN];

void pbg_user_set_tws_state(u8 conn_flag);
void pbg_user_recieve_sync_info(u8 *sync_info);
void pbg_user_mic_fixed_deal(u8 mode);
void pbg_user_event_deal(struct pbg_event *evt);
bool pbg_user_key_vaild(u8 *key_msg, struct sys_event *event);
void pbg_user_ear_pos_sync(u8 left, u8 right);
void pbg_user_battery_level_sync(u8 *dev_bat);
int  pbg_user_is_connected(void);

#endif


