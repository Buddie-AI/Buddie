#ifndef DEVICE_ADKEY_H
#define DEVICE_ADKEY_H

#include "typedef.h"
#include "asm/adc_api.h"


#define ADKEY_MAX_NUM 10

struct adkey_platform_data {
    u8 enable;
    u8 adkey_pin;
    u8 extern_up_en;                //是否用外部上拉，1：用外部上拉， 0：用内部上拉10K
    u32 ad_channel;
    u16 ad_value[ADKEY_MAX_NUM];
    u8  key_value[ADKEY_MAX_NUM];
};

struct adkey_rtcvdd_platform_data {
    u8 enable;
    u8 adkey_pin;
    u8  adkey_num;
    u32 ad_channel;
    u32 extern_up_res_value;                //是否用外部上拉，1：用外部上拉， 0：用内部上拉10K
    u16 res_value[ADKEY_MAX_NUM]; 	//电阻值, 从 [大 --> 小] 配置
    u8  key_value[ADKEY_MAX_NUM];
};

//ADKEY API:
extern int adkey_init(const struct adkey_platform_data *adkey_data);
extern u8 ad_get_key_value(void);

//RTCVDD ADKEY API:
extern int adkey_rtcvdd_init(const struct adkey_rtcvdd_platform_data *rtcvdd_adkey_data);
extern u8 adkey_rtcvdd_get_key_value(void);

#endif

