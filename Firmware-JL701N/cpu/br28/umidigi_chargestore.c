#include "asm/umidigi_chargestore.h"
#include "gpio.h"
#include "app_config.h"
#include "timer.h"

#if TCFG_UMIDIGI_BOX_ENABLE

typedef struct _UMIDIGI_VAR {
    volatile u16 timer_handle;	//消息数据获取函数的定时器句柄
    volatile u16 message;		//消息体 13bits + 3bits固定为0
    void (*app_umidigi_chargetore_message_deal)(u16 message);
} UMIDIGI_VAR;

#define __this 	(&umidigi_var)
static UMIDIGI_VAR umidigi_var;

void umidigi_chargestore_message_callback(void (*app_umidigi_chargetore_message_deal)(u16 message))
{
    __this->app_umidigi_chargetore_message_deal = app_umidigi_chargetore_message_deal;
}


/*对收到的message数据包进行奇校验 message的BIT(3)为crc_odd位*/
static void crc_odd_check(u16 message)
{
    u8 one_num = 0, crc_odd = 0;

    /* 判断起始位为0和结束位为0 */
    if (message & (BIT(0) | BIT(14))) {
        r_printf("start bit or stop bit is not zero\n");
        return;
    }

    /* 计算cmd和data一共有几个1 */
    for (u8 i = 2; i < 14; i++) {
        if (message & BIT(i)) {
            one_num ++;
        }
    }

    /* r_printf("one_num = %d\n", one_num); */

    /*奇校验计算 crc_odd位于message的BIT(1)*/
    if (one_num % 2) {
        crc_odd = 0x0;
    } else {
        crc_odd = 0x2;
    }

    /* r_printf("crc_odd = %d\n", crc_odd); */
    /* r_printf("(message & BIT(1)) = %d\n", (message & BIT(1))); */

    /*比对计算出的crc_odd值与实际收到的crc_odd值*/
    if ((message & BIT(1)) == crc_odd) {
        if (__this->app_umidigi_chargetore_message_deal) {
            __this->app_umidigi_chargetore_message_deal(message);
        }
    } else {
        r_printf("message crc check error, received an invalid message!\n");
    }
}

/*通过port口电平状态收集充电舱发送的数据*/
static void get_port_status(void *priv)
{
    static u8 keep_cnt = 0;
    static u8 change_cnt = 0;
    static u8 timeout_cnt = 0;
    static u8 pre_status = 0;
    static u8 message_index = 0;
    u8 port_status = gpio_read(TCFG_CHARGESTORE_PORT);

    //第一步:判断数据是否超时
    timeout_cnt++;
    if (timeout_cnt >= (TIMER2CMESSAGE * 16 / 2)) {
        goto __end_exit;
    }

    //第二步:判断IO是否翻转,及状态切换滤波
    keep_cnt++;
    if (port_status == pre_status) {
        change_cnt = 0;
        return;
    } else {
        change_cnt++;
        if (change_cnt <= 2) {
            return;
        }
    }

    //第三步:判断采集时间是否在范围内,并更新n bit数据
    while (keep_cnt) {
        if (keep_cnt >= 5) {
            if (pre_status) {
                __this->message |= BIT(14 - message_index);
            }
            if (keep_cnt >= 10) {
                keep_cnt -= 10;
            } else {
                keep_cnt = 0;
            }
            message_index++;
        } else {
            break;
        }
    }
    pre_status = port_status;
    timeout_cnt = 0;
    keep_cnt = 0;
    change_cnt = 0;
    if (message_index < 15) {
        return;
    }

    //第四步:校验数据
    r_printf("message = %04x\n", __this->message);
    crc_odd_check(__this->message);
    __this->message = 0;

__end_exit:
    keep_cnt = 0;
    change_cnt = 0;
    timeout_cnt = 0;
    pre_status = 0;
    message_index = 0;
    usr_timer_del(__this->timer_handle);
    __this->timer_handle = 0;
}

/*ldo下降沿唤醒 collect message*/
void ldo_port_wakeup_to_cmessage(void)
{
    /*只有通讯口的下降沿唤醒会触发定时器 即数据的起始位*/
    if (!__this->timer_handle) {
        __this->message = 0;
        __this->timer_handle = usr_timer_add(NULL, get_port_status, TIMER2CMESSAGE / 10, 1);
    }
}

#endif

