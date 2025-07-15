/*********************************************************************************************
    *   Filename        : btcontroller_modules.h

    *   Description     : Lto 优化Macro 定义

    *   Author          : Bingquan

    *   Email           : bingquan_cai@zh-jieli.com

    *   Last modifiled  : 2018-12-19 16:38

    *   Copyright:(c)JIELI  2011-2017  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef _BTCONTROLLER_MODE_H_
#define _BTCONTROLLER_MODE_H_

/*
ble 测试串口默认使用usb口，代码要确保usb口没有其他地方使用
开启测试模式，uart0 key 等一些功能默认关闭，请看特殊情况
自行处理

1、提供实验室测试rf bqb的时候配 BT_BQB

2、提供实验室测试rf fcc的时候配 BT_FCC

3、如果要定频测试配BT_FRE  频点:2402 , 发射功率最大

4、性能测试配BT_PER,使用仪器直接连接测试即可,测试完毕后
需要复位或者上电开机才会恢复正常流程，不复位或上电的话只支持链接一个设备

量产测试性能可直接配BT_NORMAL  然后通过某个外部操作来
调用 void bredr_set_dut_enble(u8 en,u8 phone )
en 1 :使能 bredr dut 测试然后就可以使用仪器链接测试
phone: 1 可以被手机连接，0  不可以被手机连接上

如果样机通过按键等操作进入dut测试调用bredr_set_dut_enble使能可以被仪器链接，
同时调用下面函数，关闭耳机快速链接，开启可发现可链接
    tws_cancle_all_noconn() ;
    user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_ENABLE, 0, NULL);
    user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);


5、可以调用 void bt_fix_fre_api() 函数实现经典蓝牙定频测试，频点2402，发射功率最大，
调用后不可恢复之前状态，只是用来量产测试，测试完需要复位或重新上电开机！

6、可以调用 void ble_fix_fre_api()函数实现ble定频测试,发射功率最大,

*/

#define BT_NORMAL      0x01
#define BT_BQB         0x02
#define BT_FCC         0x04
#define BT_FRE         0x10
#define BT_PER         0x20


#define CONFIG_BT_MODE             BT_NORMAL

#endif
