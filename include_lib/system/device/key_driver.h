#ifndef SYS_KEY_DRIVER_H
#define SYS_KEY_DRIVER_H

#include "typedef.h"



typedef enum __KEY_DRIVER_TYPE {
    KEY_DRIVER_TYPE_IO = 0x0,
    KEY_DRIVER_TYPE_AD,
    KEY_DRIVER_TYPE_RTCVDD_AD,
    KEY_DRIVER_TYPE_IR,
    KEY_DRIVER_TYPE_TOUCH,
    KEY_DRIVER_TYPE_CTMU_TOUCH,
    KEY_DRIVER_TYPE_RDEC,
    KEY_DRIVER_TYPE_SLIDEKEY,
    KEY_DRIVER_TYPE_SOFTKEY,
    KEY_DRIVER_TYPE_BRIGHTNESS,
    KEY_DRIVER_TYPE_VOICE,

    KEY_DRIVER_TYPE_MAX,
} KEY_DRIVER_TYPE;



#define NO_KEY 		0xff


#define KEY_NOT_SUPPORT  0x01


struct key_driver_para {
    const u32 scan_time;	//按键扫描频率, 单位ms
    u8 last_key;  			//上一次get_value按键值
//== 用于消抖类参数
    u8 filter_value; 		//用于按键消抖
    u8 filter_cnt;  		//用于按键消抖时的累加值
    const u8 filter_time;	//当filter_cnt累加到base_cnt值时, 消抖有效
//== 用于判定长按和HOLD事件参数
    const u8 long_time;  	//按键判定长按数量
    const u8 hold_time;  	//按键判定HOLD数量
    u8 press_cnt;  		 	//与long_time和hold_time对比, 判断long_event和hold_event
//== 用于判定连击事件参数
    u8 click_cnt;  			//单击次数
    u8 click_delay_cnt;  	//按键被抬起后等待连击事件延时计数
    const u8 click_delay_time;	////按键被抬起后等待连击事件延时数量
    u8 notify_value;  		//在延时的待发送按键值
    u8 key_type;
    u8(*get_value)(void);
};

//组合按键映射按键值
struct key_remap {
    u8 bit_value;
    u8 remap_value;
};

struct key_remap_data {
    u8 remap_num;
    const struct key_remap *table;
};

// key_driver API:
extern int key_driver_init(void);



#endif


