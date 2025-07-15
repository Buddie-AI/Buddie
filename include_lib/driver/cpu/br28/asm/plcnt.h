#ifndef _PLCNT_DRV_H_
#define _PLCNT_DRV_H_


#define PLCNT_KEY_CH_MAX		3

struct touch_key_port {
    u16 press_delta;    //按下判决的阈值
    u8 port; 			//触摸按键IO
    u8 key_value; 		//按键返回值
};

struct touch_key_platform_data {
    u8 num; 	        //触摸按键个数
    const struct touch_key_port *port_list;
};


/* =========== pclcnt API ============= */
//plcnt 初始化
int plcnt_init(void *_data);

//获取plcnt按键状态
u8 get_plcnt_value(void);


#endif  /* _PLCNT_DRV_H_ */


