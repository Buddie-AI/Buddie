#ifndef _CTMU_DRV_H_
#define _CTMU_DRV_H_

#include "typedef.h"

#define CTMU_KEY_CH_MAX		3

typedef struct _CTMU_KEY_VAR {
    s32 touch_release_buf[CTMU_KEY_CH_MAX]; 		//按键释放值滤波器buffer
    u16 touch_cnt_buf[CTMU_KEY_CH_MAX];			//按键计数值滤波器buffer
    s16 FLT1CFG1;					//滤波器1配置参数1
    s16 FLT1CFG2;					//滤波器1配置参数2, 等于(-RELEASECFG0)<<FLT1CFG0
    s16 PRESSCFG;					//按下判决门限
    s16 RELEASECFG0;				//释放判决门限0
    s16 RELEASECFG1;				//释放判决门限1
    s8  FLT0CFG;					//滤波器0配置参数(0/1/2/3)
    s8  FLT1CFG0;					//滤波器1配置参数0
    u16 touch_key_state;			//按键状态标志，随时可能被中断改写，按键处理程序需要将此标志复制出来再行处理
    u8  touch_init_cnt[CTMU_KEY_CH_MAX];				//初始化计数器，非0时进行初始化
} sCTMU_KEY_VAR;


struct ctmu_key_port {
    u8 port; 			//触摸按键IO
    u8 key_value; 		//按键返回值
};

struct ctmu_touch_key_platform_data {
    u8 num; 	//触摸按键个数
    s16 press_cfg;  	//按下判决门限
    s16 release_cfg0; 	//释放判决门限0
    s16 release_cfg1; 	//释放判决门限1
    const struct ctmu_key_port *port_list;
};

/* =========== ctmu API ============= */
//ctmu 初始化
int ctmu_init(void *_data);

//获取plcnt按键状态
u8 get_ctmu_value(void);


#endif /* #ifndef _CTMU_DRV_H_ */

