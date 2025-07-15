#include "key_driver.h"
#include "iokey.h"
#include "gpio.h"
#include "system/event.h"
#include "app_config.h"
#include "asm/clock.h"

#if TCFG_IOKEY_ENABLE

static const struct iokey_platform_data *__this = NULL;

u8 io_get_key_value(void);

#if MOUSE_KEY_SCAN_MODE
struct key_driver_para iokey_scan_para = {
    .scan_time 	  	  = 5,				//按键扫描频率, 单位: ms
    .last_key 		  = NO_KEY,  		//上一次get_value按键值, 初始化为NO_KEY;
    .filter_time  	  = 2,				//按键消抖延时;
    .long_time 		  = 5,  			//按键判定长按数量
    .hold_time 		  = (5 + 0),   	//按键判定HOLD数量
    .click_delay_time = 20,				//按键被抬起后等待连击延时数量
    .key_type		  = KEY_DRIVER_TYPE_IO,
    .get_value 		  = io_get_key_value,
};
#else
//按键驱动扫描参数列表
struct key_driver_para iokey_scan_para = {
    .scan_time 	  	  = 10,				//按键扫描频率, 单位: ms
    .last_key 		  = NO_KEY,  		//上一次get_value按键值, 初始化为NO_KEY;
    .filter_time  	  = 4,				//按键消抖延时;
    .long_time 		  = 75,  			//按键判定长按数量
    .hold_time 		  = (75 + 15),  	//按键判定HOLD数量
    .click_delay_time = 20,				//按键被抬起后等待连击延时数量
    .key_type		  = KEY_DRIVER_TYPE_IO,
    .get_value 		  = io_get_key_value,
};
#endif

#define MARK_BIT_VALUE(b, v) 		do {if ((v & (~BIT(7))) < 7) b |= BIT(v & (~BIT(7)));} while(0)

static void key_io_pull_down_input(u8 key_io)
{
    gpio_direction_input(key_io);
    gpio_set_pull_down(key_io, 1);
    gpio_set_pull_up(key_io, 0);
    gpio_set_die(key_io, 1);
}


static void key_io_pull_up_input(u8 key_io)
{
    gpio_direction_input(key_io);
    gpio_set_pull_down(key_io, 0);
    gpio_set_pull_up(key_io, 1);
    gpio_set_die(key_io, 1);
}

static void key_io_output_high(u8 key_io)
{
    gpio_set_pull_down(key_io, 0);
    gpio_set_pull_up(key_io, 0);
    gpio_direction_output(key_io, 1);
}

static void key_io_output_low(u8 key_io)
{
    gpio_set_pull_down(key_io, 0);
    gpio_set_pull_up(key_io, 0);
    gpio_direction_output(key_io, 0);
}

static int get_io_key_value(u8 key_io)
{
    return gpio_read(key_io);
}


static void key_io_reset(void)
{
    int i;

    for (i = 0; i < __this->num; i++) {
        switch (__this->port[i].connect_way) {
        case ONE_PORT_TO_HIGH:
            key_io_pull_down_input(__this->port[i].key_type.one_io.port);
            break;

        case ONE_PORT_TO_LOW:
#if (TCFG_IO_MULTIPLEX_WITH_SD == ENABLE)
            if (TCFG_MULTIPLEX_PORT != __this->port[i].key_type.one_io.port) {
                key_io_pull_up_input(__this->port[i].key_type.one_io.port);
            }
#else

            key_io_pull_up_input(__this->port[i].key_type.one_io.port);
#endif
            break;

        case DOUBLE_PORT_TO_IO:
            break;

        default:
            ASSERT(0, "IO KEY CONNECT ERR!!!");
            break;
        }
    }
}


#if MULT_KEY_ENABLE
extern const struct key_remap_data iokey_remap_data;
static u8 iokey_value_remap(u8 bit_mark)
{
    for (int i = 0; i < iokey_remap_data.remap_num; i++) {
        if (iokey_remap_data.table[i].bit_value == bit_mark) {
            return iokey_remap_data.table[i].remap_value;
        }
    }

    return NO_KEY;
}
#endif


#if TCFG_IO_MULTIPLEX_WITH_SD == ENABLE
static u8 mult_key_value = 1;
static void udelay(u32 usec)
{
    JL_TIMER0->CON = BIT(14);
    JL_TIMER0->CNT = 0;
    JL_TIMER0->PRD = clk_get("lsb") / 1000000L  * usec;
    JL_TIMER0->CON = BIT(0); //lsb clk
    while ((JL_TIMER0->CON & BIT(15)) == 0);
    JL_TIMER0->CON = BIT(14);
}
extern u8 sd_io_suspend(u8 sdx, u8 sd_io);
extern u8 sd_io_resume(u8 sdx, u8 sd_io);
void sd_mult_io_detect(void *arg)
{
    static u32 cnt = 0;
    if (sd_io_suspend(1, 1) == 0) {
        gpio_set_direction(TCFG_MULTIPLEX_PORT, 0);
        gpio_write(TCFG_MULTIPLEX_PORT, 0);
        udelay(10);
        gpio_set_die(TCFG_MULTIPLEX_PORT, 1);
        gpio_set_pull_down(TCFG_MULTIPLEX_PORT, 0);
        gpio_set_pull_up(TCFG_MULTIPLEX_PORT, 1);
        gpio_set_direction(TCFG_MULTIPLEX_PORT, 1);
        udelay(10);
        mult_key_value = gpio_read(TCFG_MULTIPLEX_PORT);
        sd_io_resume(1, 1);
    }
}
#endif

__attribute__((weak)) u8 iokey_filter_hook(u8 io_state)
{
    return 0;
}

u8 io_get_key_value(void)
{
    int i;

    u8 press_value = 0;
    u8 read_value = 0;
    u8 read_io;
    u8 write_io;
    u8 connect_way;
    u8 ret_value = NO_KEY;
    u8 bit_mark = 0;

    if (!__this->enable) {
        return NO_KEY;
    }

    //先扫描单IO接按键方式
    for (i = 0; i < __this->num; i++) {
        connect_way = __this->port[i].connect_way;

        if (connect_way == ONE_PORT_TO_HIGH) {
            press_value = 1;
        } else if (connect_way == ONE_PORT_TO_LOW) {
            press_value = 0;
        } else {
            continue;
        }

        read_io = __this->port[i].key_type.one_io.port;

#if (TCFG_IO_MULTIPLEX_WITH_SD == ENABLE)
        if (read_io == TCFG_MULTIPLEX_PORT) {
            read_value = mult_key_value;
        } else {
            read_value = get_io_key_value(read_io);
        }
#else
        read_value = get_io_key_value(read_io);
#endif
        if (iokey_filter_hook(read_value)) {
#ifdef TCFG_IOKEY_TIME_REDEFINE
            extern struct key_driver_para iokey_scan_user_para;
            iokey_scan_user_para.filter_cnt = 0;
            iokey_scan_user_para.press_cnt = 0;
            iokey_scan_user_para.click_cnt = 0;
            iokey_scan_user_para.click_delay_cnt = 0;
            iokey_scan_user_para.last_key = NO_KEY;
#else
            iokey_scan_para.filter_cnt = 0;
            iokey_scan_para.press_cnt = 0;
            iokey_scan_para.click_cnt = 0;
            iokey_scan_para.click_delay_cnt = 0;
            iokey_scan_para.last_key = NO_KEY;
#endif
            return NO_KEY;
        }
        if (read_value == press_value) {
            ret_value = __this->port[i].key_value;
#if MULT_KEY_ENABLE
            MARK_BIT_VALUE(bit_mark, ret_value);  //标记被按下的按键
#else
            goto _iokey_get_value_end;
#endif
        }
    }

    //再扫描两个IO接按键方式, in_port: 上拉输入, out_port: 输出低
    for (i = 0; i < __this->num; i++) {
        connect_way = __this->port[i].connect_way;
        if (connect_way == DOUBLE_PORT_TO_IO) {//标准双io
            press_value = 0;
            read_io = __this->port[i].key_type.two_io.in_port;
            key_io_output_low(__this->port[i].key_type.two_io.out_port);  //输出低
            key_io_pull_up_input(read_io); 	//上拉
            read_value = get_io_key_value(read_io);
            key_io_reset(); //按键初始化为单IO检测状态
            if (read_value == press_value) {
                ret_value = __this->port[i].key_value;
#if MULT_KEY_ENABLE
                MARK_BIT_VALUE(bit_mark, ret_value);	//标记被按下的按键
#else
                goto _iokey_get_value_end;
#endif
            }
        }
    }

#if MULT_KEY_ENABLE
    bit_mark = iokey_value_remap(bit_mark);  //组合按键重新映射按键值
    ret_value = (bit_mark != NO_KEY) ? bit_mark : ret_value;
#endif

_iokey_get_value_end:
    return ret_value;
}



int iokey_init(const struct iokey_platform_data *iokey_data)
{
    int i;

    __this = iokey_data;
    if (__this == NULL) {
        return -EINVAL;
    }
    if (!__this->enable) {
        return KEY_NOT_SUPPORT;
    }

    key_io_reset();

    return 0;
}

#endif  /* #if TCFG_IOKEY_ENABLE */

