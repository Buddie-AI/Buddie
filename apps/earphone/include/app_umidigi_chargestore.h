#ifndef _APP_UMIDIGI_CHARGESTORE_H_
#define _APP_UMIDIGI_CHARGESTORE_H_

#include "typedef.h"
#include "system/event.h"
#include "board_config.h"

/*******************************UMIDIGI充电舱数据包格式*****************************/
/*
 *	|<--1bit-->||<-------4bits----->||<--------------8bits------------>||<--1bit-->||<--1bits-->|
 *	|<-start-->||<-------CMD------->||<--------------DATA------------->|| crc_odd  ||<--stop--->|
 *	|    14    || 13 | 12 | 11 | 10	|| 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2   ||    1     ||     0     |
 */

/*充电舱发送的命令列表*/
#define CMD_RESERVED_0 				0x00	//reserved
#define CMD_FACTORY_RESET 			0x01	//恢复出厂设置
#define CMD_RESERVED_2 				0x02	//reserved
#define CMD_OPEN_CASE 				0x03	//充电舱开盖
#define CMD_CLOSE_CASE 				0x04	//充电舱关盖
#define CMD_ENTER_PAIRING_MODE 		0x06	//进入配对模式
#define CMD_DUT_MODE_CMD 			0x08	//进入DUT模式
#define CMD_USB_STATE 				0x0A	//USB状态
#define CMD_RESERVED_B 				0x0B	//reserved(RTK internal use)
#define CMD_SEND_CASE_BATTERY 		0x0D	//发送充电舱电量

/*message中的CMD与DATA位置，用于从一个完整的数据包中截取CMD和DATA*/
/*CMD_IN_MESSAGE  = 0xf000*/
#define CMD_IN_MESSAGE 		(BIT(13) | BIT(12) | BIT(11) | BIT(10))
/*DATA_IN_MESSAGE = 0x0ff0*/
#define DATA_IN_MESSAGE 	(BIT(9) | BIT(8) | BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2))

void app_umidigi_chargetore_message_deal(u16 message);

extern u8 umidigi_chargestore_get_power_level(void);			//获取充电舱电池电量
extern u8 umidigi_chargestore_get_power_status(void);			//获取充电舱充电状态
extern u8 umidigi_chargestore_get_cover_status(void);			//获取充电舱合盖或开盖状态
extern u8 umidigi_chargestore_get_earphone_online(void);  		//获取合盖状态时耳机在仓数量
extern void umidigi_chargestore_set_bt_init_ok(u8 flag);		//获取蓝牙初始化成功标志位
extern u8 umidigi_chargestore_check_going_to_poweroff(void);	//获取允许poweroff标志位
extern void umidigi_chargestore_set_phone_disconnect(void);
extern void umidigi_chargestore_set_phone_connect(void);
extern void umidigi_chargestore_set_sibling_chg_lev(u8 chg_lev);//设置对耳同步的充电舱电量
extern void umidigi_chargestore_set_power_level(u8 power);		//设置充电舱电池电量
extern int umidigi_chargestore_sync_chg_level(void);

extern int app_umidigi_chargestore_event_handler(struct umidigi_chargestore_event *umidigi_chargestore_dev);

#endif

