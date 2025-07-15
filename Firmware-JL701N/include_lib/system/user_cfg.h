#ifndef __USER_CFG_H__
#define __USER_CFG_H__

#include "typedef.h"
#include "app_config.h"

#define LOCAL_NAME_LEN	32	/*BD_NAME_LEN_MAX*/

//bt bin结构
typedef struct __BT_CONFIG {
    u8 edr_name[LOCAL_NAME_LEN];       //经典蓝牙名
    u8 mac_addr[6];                    //蓝牙MAC地址
    u8 rf_power;                       //发射功率
    u8 dac_analog_gain;                //通话DAC模拟增益
    u8 mic_analog_gain;                //通话MIC增益
    u16 tws_device_indicate;         /*设置对箱搜索标识，inquiry时候用,搜索到相应的标识才允许连接*/
    u8 tws_local_addr[6];
    u8 ble_name[LOCAL_NAME_LEN];       //ble蓝牙名
    u8 ble_mac_addr[6];                //ble蓝牙MAC地址
    u8 ble_rf_power;                   //ble发射功率
} _GNU_PACKED_  BT_CONFIG;

//audio bin结构
typedef struct __AUDIO_CONFIG {
    u8 sw;
    u8 max_sys_vol;         //最大系统音量
    u8 default_vol;         //开机默认音量
    u8 tone_vol;            //提示音音量
} _GNU_PACKED_ AUDIO_CONFIG;

//status bin结构体
typedef struct __STATUS {
    u8 charge_start;    //开始充电
    u8 charge_full;     //充电完成
    u8 power_on;        //开机
    u8 power_off;       //关机
    u8 lowpower;        //低电
    u8 max_vol;         //最大音量
    u8 phone_in;        //来电
    u8 phone_out;       //去电
    u8 phone_activ;     //通话中
    u8 bt_init_ok;      //蓝牙初始化完成
    u8 bt_connect_ok;   //蓝牙连接成功
    u8 bt_disconnect;   //蓝牙断开
    u8 tws_connect_ok;   	//TWS连接成功
    u8 tws_disconnect;   	//TWS蓝牙断开
} _GNU_PACKED_ STATUS;

typedef struct __STATUS_CONFIG {
    u8 sw;
    STATUS led;    //led status
    STATUS tone;   //tone status
} _GNU_PACKED_ STATUS_CONFIG;

//charge bin结构
typedef struct __CHARGE_CONFIG {
    u8 sw;                  //开关
    u8 poweron_en;          //支持开机充电
    u8 full_v;              //充满电压
    u8 full_c;              //充满电流
    u8 charge_c;            //充电电流
} _GNU_PACKED_  CHARGE_CONFIG;

//key
typedef struct __KEY_OP {
    u8 short_msg;     //短按消息
    u8 long_msg;      //长按消息
    u8 hold_msg;      //hold 消息
    u8 up_msg;        //抬键消息
    u8 double_msg;    //双击消息
    u8 triple_msg;    //三击消息
} _GNU_PACKED_ KEY_OP;

//mic type
typedef struct __MIC_TYPE_CONFIG {
    u8 type;     //0:不省电容模式        1:省电容模式
    //1:16K 2:7.5K 3:5.1K 4:6.8K 5:4.7K 6:3.5K 7:2.9K  8:3K  9:2.5K 10:2.1K 11:1.9K  12:2K  13:1.8K 14:1.6K  15:1.5K 16:1K 31:0.6K
    u8 pull_up;
    //00:2.3v 01:2.5v 10:2.7v 11:3.0v
    u8 ldo_lev;
} _GNU_PACKED_ MIC_TYPE_CONFIG;



//自动关机时间配置
typedef struct __AUTO_OFF_TIME_CONFIG {
    u8 auto_off_time;
} _GNU_PACKED_ AUTO_OFF_TIME_CONFIG;

//低电压提示配置
typedef struct __AUTO_LOWPOWER_V_CONFIG {
    u16 warning_tone_v;
    u16 poweroff_tone_v;
} _GNU_PACKED_ AUTO_LOWPOWER_V_CONFIG;

//LRC配置
typedef struct __LRC_CONFIG {
    u16 lrc_ws_inc;
    u16 lrc_ws_init;
    u16 btosc_ws_inc;
    u16 btosc_ws_init;
    u8 lrc_change_mode;
} _GNU_PACKED_ LRC_CONFIG;

void cfg_file_parse(u8 idx);
const u8 *bt_get_mac_addr();
void bt_get_tws_local_addr(u8 *addr);

STATUS *get_led_config(void);
STATUS *get_tone_config(void);
void get_random_number(u8 *ptr, u8 len);
extern void bt_get_vm_mac_addr(u8 *addr);
extern u8 get_max_sys_vol(void);
extern const char *bt_get_local_name();
extern u16 bt_get_tws_device_indicate(u8 *tws_device_indicate);
const char *bt_get_local_name();
extern void bt_update_mac_addr(u8 *addr);
extern void bt_set_local_name(char *name, u8 len);
extern void bt_reset_and_get_mac_addr(u8 *addr);
extern void bt_set_pair_code_en(u8 en);

#endif
