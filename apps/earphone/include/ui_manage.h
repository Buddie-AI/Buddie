#ifndef __UI_MANAGE_H_
#define __UI_MANAGE_H_

#include "typedef.h"

typedef enum {
    STATUS_NULL = 0,
    STATUS_POWERON,
    STATUS_POWEROFF,
    STATUS_POWERON_LOWPOWER,
    STATUS_BT_INIT_OK,
    STATUS_BT_CONN,
    STATUS_BT_SLAVE_CONN_MASTER,
    STATUS_BT_MASTER_CONN_ONE,
    STATUS_BT_DISCONN,
    STATUS_BT_TWS_CONN,
    STATUS_BT_TWS_DISCONN,
    STATUS_PHONE_INCOME,
    STATUS_PHONE_OUT,
    STATUS_PHONE_ACTIV,
    STATUS_CHARGE_START,
    STATUS_CHARGE_FULL,
    STATUS_CHARGE_CLOSE,
    STATUS_CHARGE_ERR,
    STATUS_LOWPOWER,
    STATUS_CHARGE_LDO5V_OFF,
    STATUS_EXIT_LOWPOWER,
    STATUS_NORMAL_POWER,
    STATUS_POWER_NULL,
} UI_STATUS;


int  ui_manage_init(void);
void ui_update_status(u8 status);
u8 get_ui_busy_status();
void led_module_enter_sniff_mode();
void led_module_exit_sniff_mode();

#endif
