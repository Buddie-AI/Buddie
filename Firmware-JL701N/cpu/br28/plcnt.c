#include "includes.h"
#include "asm/plcnt.h"
#include "asm/gpio.h"


static u8 port_index_mapping_talbe[PLCNT_KEY_CH_MAX] = {0};
static u32 touch_pre_value[PLCNT_KEY_CH_MAX] = {0};
static u32 touch_normal_value[PLCNT_KEY_CH_MAX] = {0};

#define TOUCH_VAL_CALIBRATE_CYCLE   (200)//周期大约2秒
static u32 touch_calibrate_cnt[PLCNT_KEY_CH_MAX] = {0};
static u32 touch_calibrate_tmp_value[PLCNT_KEY_CH_MAX] = {0};

static u8 touch_state = 0;


static const struct touch_key_platform_data *user_data = NULL;

static JL_PORT_FLASH_TypeDef *PLCNT_IO_PORTx = NULL;
static u8 PLCNT_IO_xx;

void plcnt_io_out(struct touch_key_port *key)
{
    if (key->port > IO_MAX_NUM) {
        return;
    } else if ((key->port / 16) == 0) {
        PLCNT_IO_PORTx = JL_PORTA;
    } else if ((key->port / 16) == 1) {
        PLCNT_IO_PORTx = JL_PORTB;
    }
    PLCNT_IO_xx = key->port % 16;
    PLCNT_IO_PORTx->DIE |=  BIT(PLCNT_IO_xx);
    PLCNT_IO_PORTx->PU  &= ~BIT(PLCNT_IO_xx);
    PLCNT_IO_PORTx->PD  |=  BIT(PLCNT_IO_xx);
    PLCNT_IO_PORTx->OUT |=  BIT(PLCNT_IO_xx);
    PLCNT_IO_PORTx->DIR &= ~BIT(PLCNT_IO_xx);
}

void plcnt_io_in(struct touch_key_port *key)
{
    if (key->port > IO_MAX_NUM) {
        return;
    } else if ((key->port / 16) == 0) {
        PLCNT_IO_PORTx = JL_PORTA;
    } else if ((key->port / 16) == 1) {
        PLCNT_IO_PORTx = JL_PORTB;
    }
    PLCNT_IO_xx = key->port % 16;
    PLCNT_IO_PORTx->DIE |=  BIT(PLCNT_IO_xx);
    PLCNT_IO_PORTx->PU  &= ~BIT(PLCNT_IO_xx);
    PLCNT_IO_PORTx->PD  |=  BIT(PLCNT_IO_xx);
    PLCNT_IO_PORTx->OUT |=  BIT(PLCNT_IO_xx);
    PLCNT_IO_PORTx->DIR |=  BIT(PLCNT_IO_xx);
}

void plcnt_iomc(struct touch_key_port *key)
{
    //放电计数引脚选择intputchannel2
    gpio_ich_sel_iutput_signal(key->port, INPUT_CH_SIGNAL_CAP, INPUT_CH_TYPE_GP_ICH);
    /*gpio_set_fun_input_port(key->port, PFI_GP_ICH6);*/
    /*SFR(JL_IOMAP->CON1, 8, 4, 6);*/
}

static u32 key_or = 0;
void plcnt_flt(u32 cur_val, u32 ch)
{
    /* printf("ch%d : %d\n", ch, cur_val); */
    //简单滤波
    if (touch_pre_value[ch] == 0) {
        touch_pre_value[ch] = cur_val;
    } else if (cur_val >= touch_pre_value[ch]) {
        touch_pre_value[ch] = touch_pre_value[ch] + (u32)((cur_val - touch_pre_value[ch]) * 0.2f);
    } else {
        touch_pre_value[ch] = touch_pre_value[ch] - (u32)((touch_pre_value[ch] - cur_val) * 0.2f);
    }
    //处理滤波之后的值
    if (touch_pre_value[ch] > (touch_normal_value[ch] + user_data->port_list[ch].press_delta)) {
        key_or |= BIT(ch);
        touch_calibrate_cnt[ch] = 0;
    } else {
        key_or &= ~BIT(ch);
        touch_calibrate_cnt[ch] ++;
    }
    //定期标定常态下的基准值
    if (touch_calibrate_cnt[ch] > TOUCH_VAL_CALIBRATE_CYCLE) {
        touch_normal_value[ch] = touch_calibrate_tmp_value[ch] / 10;
        touch_calibrate_tmp_value[ch] = 0;
        touch_calibrate_cnt[ch] = 0;
    } else if (touch_calibrate_cnt[ch] >= (TOUCH_VAL_CALIBRATE_CYCLE / 2)) {
        if (touch_calibrate_cnt[ch] < ((TOUCH_VAL_CALIBRATE_CYCLE / 2) + 10)) {
            touch_calibrate_tmp_value[ch] += touch_pre_value[ch];
        }
    } else {
        touch_calibrate_tmp_value[ch] = 0;
    }
}

void scan_capkey_async(void *priv)
{
    if (!user_data) {
        return;
    }
    static u8 cur_ch = 0;
    static u32 new_val = 0;
    static u32 old_val = 0;

    u8 pre_ch = cur_ch - 1;
    u8 net_ch = cur_ch + 1;
    if (cur_ch == 0) {
        pre_ch = user_data->num - 1;
    }
    if (net_ch >= user_data->num) {
        net_ch = 0;
    }

    new_val = JL_PCNT->VAL;
    u32 delta_val = 0;
    if (new_val >= old_val) {
        delta_val = new_val - old_val;
    } else {
        delta_val = (u32) - new_val + old_val;
    }
    plcnt_flt(delta_val, pre_ch);
    old_val = JL_PCNT->VAL;
    plcnt_iomc(&(user_data->port_list[cur_ch]));
    plcnt_io_in(&(user_data->port_list[cur_ch]));
    if (net_ch != cur_ch) {
        plcnt_io_out(&(user_data->port_list[net_ch]));
    }
    cur_ch = net_ch;
}

void scan_capkey(void *priv)
{
    if (!user_data) {
        return;
    }
    for (u8 ch = 0; ch < user_data->num; ch ++) {
        plcnt_io_out(&(user_data->port_list[ch]));
        plcnt_iomc(&(user_data->port_list[ch]));
        u32 tmp_val, start_val, end_val, delta_val = 0;
        for (u8 i = 0; i < 5; i ++) {
            delay(100);
            start_val = JL_PCNT->VAL;
            PLCNT_IO_PORTx->DIR |=  BIT(PLCNT_IO_xx);
            __asm__ volatile("csync");
            while (PLCNT_IO_PORTx->IN & BIT(PLCNT_IO_xx));
            end_val = JL_PCNT->VAL;
            PLCNT_IO_PORTx->DIR &= ~BIT(PLCNT_IO_xx);
            if (end_val > start_val) {
                tmp_val = end_val - start_val;
            } else {
                tmp_val = (u32) - start_val + end_val;
            }
            delta_val += tmp_val;
        }
        delta_val = delta_val / 5;
        /* printf("%d: %d\n", ch, delta_val); */
        plcnt_flt(delta_val, ch);
    }
}



/*
  @brief   引脚放电计数模块初始化
*/
int plcnt_init(void *_data)
{
    if (_data == NULL) {
        return -1;
    }
    user_data = (const struct touch_key_platform_data *)_data;

    if (user_data->num > PLCNT_KEY_CH_MAX) {
        return -1;
    }

    for (u8 ch = 0; ch < user_data->num; ch ++) {
        touch_normal_value[ch] = 0 - (2 * user_data->port_list[ch].press_delta);
    }

    for (u8 ch = 0; ch < user_data->num; ch ++) {
        plcnt_iomc(&(user_data->port_list[ch]));
        plcnt_io_out(&(user_data->port_list[ch]));
    }

    JL_PCNT->CON = 0;
    JL_PCNT->CON |= (0b11 << 2);      //选择PLL240M为时钟源
    JL_PCNT->CON |= BIT(1);           //引脚放电计数使能

    /* sys_s_hi_timer_add(NULL, scan_capkey_async, 2); //2ms */
    sys_s_hi_timer_add(NULL, scan_capkey, 10); //10ms

    return 0;
}

static u8 key_filter(u8 key)
{
    static u8 used_key = 0xff;
    static u8 old_key;
    static u8 key_counter;

    if (old_key != key) {
        key_counter = 0;
        old_key = key;
    } else {
        key_counter++;
        if (key_counter == 3) {
            used_key = key;
        }
    }
    return used_key;
}

u8 get_plcnt_value(void)
{
    /* printf("%d %d %d\n", touch_pre_value[0], touch_pre_value[1], touch_pre_value[2]); */
    static u8 pre_i = 0;
    u8 key_num = 0xff;
    if (key_or) {
        if (key_or & BIT(pre_i)) {
            key_num = pre_i;
        } else {
            for (u8 ch = 0; ch < user_data->num; ch ++) {
                if (key_or & BIT(ch)) {
                    key_num = ch;
                    pre_i = ch;
                    break;
                }
            }
        }
        /* printf("key_num : %d\n", key_num); */
    }
    return  key_filter(key_num);
}

