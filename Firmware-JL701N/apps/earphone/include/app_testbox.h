#ifndef _APP_TESTBOX_H_
#define _APP_TESTBOX_H_

#include "typedef.h"
#include "system/event.h"

extern void testbox_set_bt_init_ok(u8 flag);
extern u8 testbox_get_status(void);
extern void testbox_clear_status(void);
extern u8 testbox_get_ex_enter_dut_flag(void);
extern u8 testbox_get_ex_enter_storage_mode_flag(void);
extern u8 testbox_get_connect_status(void);
extern void testbox_clear_connect_status(void);
extern u8 testbox_get_keep_tws_conn_flag(void);
extern int app_testbox_event_handler(struct testbox_event *testbox_dev);


#endif    //_APP_TESTBOX_H_
