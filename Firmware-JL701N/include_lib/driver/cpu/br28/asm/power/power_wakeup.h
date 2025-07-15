#ifndef __POWER_WAKEUP_H__
#define __POWER_WAKEUP_H__

enum WAKEUP_REASON {
    //WAKEUP
    PWR_WK_REASON_PLUSE_CNT_OVERFLOW,   	    //pcnt唤醒复位
    PWR_WK_REASON_P11,   					    //P11唤醒复位
    PWR_WK_REASON_LPCTMU,   				    //触摸唤醒复位
    PWR_WK_REASON_PORT_EDGE,				    //数字io输入边沿唤醒复位
    PWR_WK_REASON_ANA_EDGE,					    //模拟io输入边沿唤醒复位
    PWR_WK_REASON_VDDIO_LVD,				    //vddio lvd唤醒复位
    PWR_WK_REASON_EDGE_INDEX0,				    //p33 index0  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX1,				    //p33 index1  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX2,			    	//p33 index2  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX3,				    //p33 index3  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX4,     	            //p33 index4  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX5,          	    //p33 index5  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX6,          	    //p33 index6  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX7,          	    //p33 index7  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX8,          	    //p33 index8  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX9,          	    //p33 index9  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX10,         	    //p33 index10 io唤醒复位
    PWR_WK_REASON_EDGE_INDEX11,         	    //p33 index11 io唤醒复位
    PWR_ANA_WK_REASON_FALLINIG_EDGE_LDOIN, 		//LDO5V上升沿唤醒复位
    PWR_ANA_WK_REASON_RISING_EDGE_LDOIN,        //LDO5V下降沿唤醒复位
    PWR_ANA_WK_REASON_FALLING_EDGE_VBATCH,      //VBATCH上升降沿唤醒复位
    PWR_ANA_WK_REASON_RISING_EDGE_VBATCH,       //VBATCH下降沿唤醒复位
    PWR_RTC_WK_REASON_ALM, 					    //RTC闹钟唤醒复位
    PWR_RTC_WK_REASON_256HZ, 					//RTC 256Hz时基唤醒复位
    PWR_RTC_WK_REASON_64HZ, 					//RTC 64Hz时基唤醒复位
    PWR_RTC_WK_REASON_2HZ, 						//RTC 2Hz时基唤醒复位
    PWR_RTC_WK_REASON_1HZ, 						//RTC 1Hz时基唤醒复位
};

//=========================唤醒参数配置==================================

struct port_wakeup {
    u8 iomap;      			  //唤醒io
    u8 pullup_down_enable;    //上下拉是否使能
    POWER_WKUP_EDGE edge;     //唤醒边沿条件
    POWER_WKUP_FLT filter;	  //滤波参数，数字io输入没有滤波可配制
};

struct wakeup_param {
    //数字io输入
    const struct port_wakeup *port[MAX_WAKEUP_PORT];
    //模拟io输入
    const struct port_wakeup *aport[MAX_WAKEUP_ANA_PORT];
};

//=========================唤醒接口==================================
void power_wakeup_index_enable(u8 index, u8 enable);
void power_wakeup_gpio_enable(u8 gpio, u8 enable);
void power_wakeup_gpio_edge(u8 gpio, POWER_WKUP_EDGE edge);

void power_awakeup_index_enable(u8 index, u8 enable);
void power_awakeup_gpio_enable(u8 gpio, u8 enable);
void power_awakeup_gpio_edge(u8 gpio, POWER_WKUP_EDGE edge);

void power_wakeup_set_callback(void (*wakeup_callback)(u8 index, u8 gpio));
void power_awakeup_set_callback(void (*wakeup_callback)(u8 index, u8 gpio, POWER_WKUP_EDGE edge));

void port_edge_wkup_set_callback_by_index(u8 index, void (*wakeup_callback)(u8 index, u8 gpio));
void aport_edge_wkup_set_callback_by_index(u8 index, void (*wakeup_callback)(u8 index, u8 gpio, POWER_WKUP_EDGE edge));

void power_wakeup_init(const struct wakeup_param *param);

u8 is_wakeup_source(enum WAKEUP_REASON index);

void power_wakeup_reason_dump();

u8 is_ldo5v_wakeup(void);

#endif
