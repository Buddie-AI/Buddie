#ifndef __POWER_MANAGE_H_
#define __POWER_MANAGE_H_

#include "generic/typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DEVICE_EVENT_POWER_SHUTDOWN = 0x10,
    DEVICE_EVENT_POWER_STARTUP,
    DEVICE_EVENT_POWER_PERCENT,
    DEVICE_EVENT_POWER_CHARGER_IN,
    DEVICE_EVENT_POWER_CHARGER_OUT
};

#define PWR_SCAN_TIMES 	   3

#define PWR_DELAY_INFINITE      0xffffffff

#define PWR_WKUP_PORT           "wkup_port"
#define PWR_WKUP_ALARM          "wkup_alarm"
#define PWR_WKUP_PWR_ON         "wkup_pwr_on"
#define PWR_WKUP_ABNORMAL       "wkup_abnormal"
#define PWR_WKUP_SHORT_KEY      "wkup_short_key"

struct sys_power_hal_ops {
    void (*init)(void);
    void (*poweroff)(void *arg);
    int (*wakeup_check)(char *reason, int max_len);
    int (*port_wakeup_config)(const char *port, int enable);
    int (*alarm_wakeup_config)(u32 sec, int enable);
    int (*get_battery_voltage)(void);
    int (*get_battery_percent)(void);
    int (*charger_online)(void);
};

extern const struct sys_power_hal_ops sys_power_hal_ops_begin[];
extern const struct sys_power_hal_ops sys_power_hal_ops_end[];

#define REGISTER_SYS_POWER_HAL_OPS(ops) \
    static const struct sys_power_hal_ops ops sec(.sys_power_hal_ops)


void sys_power_early_init();
/*
 * @brief 断电关机，不释放资源
 */
void sys_power_poweroff(void *arg);
/*
 * @brief 软关机，触发DEVICE_EVENT_POWER_SHUTDOWN事件，app捕获事件释放资源再调用sys_power_poweroff()
 */
void sys_power_shutdown();

int sys_power_set_port_wakeup(const char *port, int enable);

int sys_power_set_alarm_wakeup(u32 sec, int enable);

const char *sys_power_get_wakeup_reason();

void sys_power_clr_wakeup_reason(const char *str);

int sys_power_get_battery_voltage();

int sys_power_get_battery_persent();

int sys_power_is_charging();

int sys_power_charger_online(void);
/*
 * @brief 倒计时自动关机
 * @parm dly_secs 延时关机时间，赋值0为永不关机
 * @return none
 */
void sys_power_auto_shutdown_start(u32 dly_secs);
void sys_power_auto_shutdown_pause();
void sys_power_auto_shutdown_resume();
void sys_power_auto_shutdown_clear();
void sys_power_auto_shutdown_stop();


int sys_power_low_voltage(u32 voltage);

/*
 * @brief 低电延时关机
 * @parm p_low_percent 低电电量百分比
 * @parm dly_secs 延时关机时间，赋值0为立即关机，赋值PWR_DELAY_INFINITE为永不关机
 * @return none
 */
void sys_power_low_voltage_shutdown(u32 voltage, u32 dly_secs);
/*
 * @brief 插拔延时关机
 * @parm dly_secs 延时关机时间，赋值0为立即关机，赋值PWR_DELAY_INFINITE为永不关机
 * @return none
 */
void sys_power_charger_off_shutdown(u32 dly_secs);


#ifdef __cplusplus
}
#endif
#endif
