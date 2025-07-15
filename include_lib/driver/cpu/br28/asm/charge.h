#ifndef _CHARGE_H_
#define _CHARGE_H_

#include "typedef.h"
#include "device.h"

/*------充满电电压选择 4.041V-4.534V-------*/
//低压压电池配置0~15
#define CHARGE_FULL_V_4041		0
#define CHARGE_FULL_V_4061		1
#define CHARGE_FULL_V_4081		2
#define CHARGE_FULL_V_4101		3
#define CHARGE_FULL_V_4119		4
#define CHARGE_FULL_V_4139		5
#define CHARGE_FULL_V_4159		6
#define CHARGE_FULL_V_4179		7
#define CHARGE_FULL_V_4199		8
#define CHARGE_FULL_V_4219		9
#define CHARGE_FULL_V_4238		10
#define CHARGE_FULL_V_4258		11
#define CHARGE_FULL_V_4278		12
#define CHARGE_FULL_V_4298		13
#define CHARGE_FULL_V_4318		14
#define CHARGE_FULL_V_4338		15
//高压电池配置16~31
#define CHARGE_FULL_V_4237		16
#define CHARGE_FULL_V_4257		17
#define CHARGE_FULL_V_4275		18
#define CHARGE_FULL_V_4295		19
#define CHARGE_FULL_V_4315		20
#define CHARGE_FULL_V_4335		21
#define CHARGE_FULL_V_4354		22
#define CHARGE_FULL_V_4374		23
#define CHARGE_FULL_V_4394		24
#define CHARGE_FULL_V_4414		25
#define CHARGE_FULL_V_4434		26
#define CHARGE_FULL_V_4454		27
#define CHARGE_FULL_V_4474		28
#define CHARGE_FULL_V_4494		29
#define CHARGE_FULL_V_4514		30
#define CHARGE_FULL_V_4534		31
#define CHARGE_FULL_V_MAX       32
/*****************************************/

/*充满电电流选择 2mA-30mA*/
#define CHARGE_FULL_mA_2		0
#define CHARGE_FULL_mA_5		1
#define CHARGE_FULL_mA_7	 	2
#define CHARGE_FULL_mA_10		3
#define CHARGE_FULL_mA_15		4
#define CHARGE_FULL_mA_20		5
#define CHARGE_FULL_mA_25		6
#define CHARGE_FULL_mA_30		7

/*
 	充电电流选择
	恒流：20-220mA
*/
#define CHARGE_mA_15			0
#define CHARGE_mA_20			1
#define CHARGE_mA_25			2
#define CHARGE_mA_30			3
#define CHARGE_mA_35			4
#define CHARGE_mA_40			5
#define CHARGE_mA_50			6
#define CHARGE_mA_60			7
#define CHARGE_mA_80			8
#define CHARGE_mA_100			9
#define CHARGE_mA_120			10
#define CHARGE_mA_140			11
#define CHARGE_mA_160			12
#define CHARGE_mA_200			13
#define CHARGE_mA_250			14
#define CHARGE_mA_300			15
#define CHARGE_mA_1P5           (BIT(4)|CHARGE_mA_15)
#define CHARGE_mA_2             (BIT(4)|CHARGE_mA_20)
#define CHARGE_mA_2P5           (BIT(4)|CHARGE_mA_25)
#define CHARGE_mA_3             (BIT(4)|CHARGE_mA_30)
#define CHARGE_mA_3P5           (BIT(4)|CHARGE_mA_35)
#define CHARGE_mA_4             (BIT(4)|CHARGE_mA_40)
#define CHARGE_mA_5             (BIT(4)|CHARGE_mA_50)
#define CHARGE_mA_6             (BIT(4)|CHARGE_mA_60)
#define CHARGE_mA_8             (BIT(4)|CHARGE_mA_80)
#define CHARGE_mA_10            (BIT(4)|CHARGE_mA_100)
#define CHARGE_mA_12            (BIT(4)|CHARGE_mA_120)
#define CHARGE_mA_14            (BIT(4)|CHARGE_mA_140)
#define CHARGE_mA_16            (BIT(4)|CHARGE_mA_160)

/*
 	充电口下拉选择
	电阻 50k ~ 200k
*/
#define CHARGE_PULLDOWN_50K     0
#define CHARGE_PULLDOWN_100K    1
#define CHARGE_PULLDOWN_150K    2
#define CHARGE_PULLDOWN_200K    3


#define CHARGE_CCVOL_V			300		//涓流充电向恒流充电的转换点

#define DEVICE_EVENT_FROM_CHARGE	(('C' << 24) | ('H' << 16) | ('G' << 8) | '\0')

struct charge_platform_data {
    u8 charge_en;	        //内置充电使能
    u8 charge_poweron_en;	//开机充电使能
    u8 charge_full_V;	    //充满电电压大小
    u8 charge_full_mA;	    //充满电电流大小
    u8 charge_mA;	        //恒流充电电流大小
    u8 charge_trickle_mA;   //涓流充电电流大小
    u8 ldo5v_pulldown_en;   //下拉使能位
    u8 ldo5v_pulldown_lvl;	//ldo5v的下拉电阻配置项,若充电舱需要更大的负载才能检测到插入时，请将该变量置为对应阻值
    u8 ldo5v_pulldown_keep; //下拉电阻在softoff时是否保持,ldo5v_pulldown_en=1时有效
    u16 ldo5v_off_filter;	//ldo5v拔出过滤值，过滤时间 = (filter*2 + 20)ms,ldoin<0.6V且时间大于过滤时间才认为拔出,对于充满直接从5V掉到0V的充电仓，该值必须设置成0，对于充满由5V先掉到0V之后再升压到xV的充电仓，需要根据实际情况设置该值大小
    u16 ldo5v_on_filter;    //ldo5v>vbat插入过滤值,电压的过滤时间 = (filter*2)ms
    u16 ldo5v_keep_filter;  //1V<ldo5v<vbat维持电压过滤值,过滤时间= (filter*2)ms
    u16 charge_full_filter; //充满过滤值,连续检测充满信号恒为1才认为充满,过滤时间 = (filter*2)ms
};

#define CHARGE_PLATFORM_DATA_BEGIN(data) \
		struct charge_platform_data data  = {

#define CHARGE_PLATFORM_DATA_END()  \
};


enum {
    CHARGE_FULL_33V = 0,	//充满标记位
    TERMA_33V,				//模拟测试信号
    VBGOK_33V,				//模拟测试信号
    CICHARGE_33V,			//涓流转恒流信号
};

enum {
    CHARGE_EVENT_CHARGE_START,
    CHARGE_EVENT_CHARGE_CLOSE,
    CHARGE_EVENT_CHARGE_FULL,
    CHARGE_EVENT_LDO5V_KEEP,
    CHARGE_EVENT_LDO5V_IN,
    CHARGE_EVENT_LDO5V_OFF,
    CHARGE_EVENT_USB_CHARGE_IN,
    CHARGE_EVENT_USB_CHARGE_OFF,
};


extern void set_charge_event_flag(u8 flag);
extern void set_charge_online_flag(u8 flag);
extern void set_charge_event_flag(u8 flag);
extern u8 get_charge_online_flag(void);
extern u8 get_charge_poweron_en(void);
extern void set_charge_poweron_en(u32 onOff);
extern void charge_start(void);
extern void charge_close(void);
extern u8 get_charge_mA_config(void);
extern void set_charge_mA(u8 charge_mA);
extern u8 get_ldo5v_pulldown_en(void);
extern u8 get_ldo5v_pulldown_res(void);
extern u8 get_ldo5v_online_hw(void);
extern u8 get_lvcmp_det(void);
extern void charge_check_and_set_pinr(u8 mode);
extern u16 get_charge_full_value(void);
extern void charge_module_stop(void);
extern void charge_module_restart(void);
extern void ldoin_wakeup_isr(void);
extern void charge_wakeup_isr(void);
extern int charge_init(const struct dev_node *node, void *arg);
extern const struct device_operations charge_dev_ops;
extern void charge_set_ldo5v_detect_stop(u8 stop);

#endif    //_CHARGE_H_
