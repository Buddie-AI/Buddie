/*********************************************************************************************
    *   Filename        : bt_profile_config.h

    *   Description     : 

    *   Author          : Tongai 

    *   Email           : laotongai@zh-jieli.com

    *   Last modifiled  : 2020-07-01 16:31

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef BT_PROFILE_H
#define BT_PROFILE_H


#define BT_BTSTACK_CLASSIC                   BIT(0)
#define BT_BTSTACK_LE_ADV                    BIT(1)
#define BT_BTSTACK_LE                        BIT(2)

extern const int config_stack_modules;
#define STACK_MODULES_IS_SUPPORT(x)         (config_stack_modules & (x))



extern u8 app_bredr_pool[];
extern u8 app_le_pool[];
extern u8 app_l2cap_pool[];
extern u8 app_bredr_profile[];

extern u16 get_bredr_pool_len(void);
extern u16 get_le_pool_len(void);
extern u16 get_l2cap_stack_len(void);
extern u16 get_profile_pool_len(void);


#endif
