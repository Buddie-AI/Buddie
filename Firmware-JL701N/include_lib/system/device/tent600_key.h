#ifndef DEVICE_TENT600_KEY_H
#define DEVICE_TENT600_KEY_H
#include "typedef.h"
#include "asm/adc_api.h"


struct tent600_key_platform_data {
    u8 enalbe;
    u8 tent600_key_pin;
    u8 extern_up_en;
    u32 ad_channel;
};


int tent600_key_init(const struct tent600_key_platform_data *tent600_key_data);
u8 tent600_get_key_value(void);

#endif
