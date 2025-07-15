#ifndef __LP_TOUCH_KEY_TOOL__
#define __LP_TOUCH_KEY_TOOL__

#include "typedef.h"


enum {
    BT_KEY_CH_RES_MSG,
    BT_EARTCH_RES_MSG,
    BT_EVENT_HW_MSG,
    BT_EVENT_SW_MSG,
    BT_EVENT_VDDIO,
};

//tws
void lpctmu_tws_send_event_data(int msg, int type);
void lpctmu_tws_send_res_data(int data1, int data2, int data3, int data4, int data5, int type);

//spp
int lp_touch_key_online_debug_init(void);
int lp_touch_key_online_debug_send(u8 ch, u16 val);
int lp_touch_key_online_debug_key_event_handle(u8 ch_index, struct sys_event *event);

//testbox
u8 lp_touch_key_testbox_remote_test(u8 ch, u8 event);
void lp_touch_key_testbox_inear_trim(u8 flag);

#endif

