#include "includes.h"
#include "asm/power/p11.h"
#include "asm/power/p33.h"
#include "asm/lp_touch_key_tool.h"
#include "asm/lp_touch_key_alog.h"
#include "device/key_driver.h"
#include "key_event_deal.h"
#include "app_config.h"
#include "asm/lp_touch_key_api.h"
#include "asm/charge.h"

#define LOG_TAG_CONST       LP_KEY
#define LOG_TAG             "[LP_KEY]"
/* #define LOG_ERROR_ENABLE */
/* #define LOG_DEBUG_ENABLE */
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"


#if TCFG_LP_TOUCH_KEY_ENABLE

#define CTMU_CHECK_LONG_CLICK_BY_RES    1

/*************************************************************************************************************************
  										lp_touch_key driver
*************************************************************************************************************************/

#define LP_TOUCH_KEY_TIMER_MAGIC_NUM 		0xFFFF

struct ctmu_key _ctmu_key = {
    .slide_dir = TOUCH_KEY_EVENT_MAX,
    .click_cnt = {0, 0, 0, 0, 0},
    .last_key = {CTMU_KEY_NULL, CTMU_KEY_NULL, CTMU_KEY_NULL, CTMU_KEY_NULL, CTMU_KEY_NULL},
    .short_timer = {LP_TOUCH_KEY_TIMER_MAGIC_NUM, LP_TOUCH_KEY_TIMER_MAGIC_NUM, LP_TOUCH_KEY_TIMER_MAGIC_NUM, LP_TOUCH_KEY_TIMER_MAGIC_NUM, LP_TOUCH_KEY_TIMER_MAGIC_NUM},
};

#define __this      (&_ctmu_key)

static volatile u8 is_lpkey_active = 0;


extern void lp_touch_key_send_cmd(enum CTMU_M2P_CMD cmd);

//======================================================//
// 内置触摸灵敏度表, 由内置触摸灵敏度调试工具生成 		//
// 请将该表替换sdk中lp_touch_key.c中同名的参数表        //
//======================================================//
const static struct ch_adjust_table ch_sensitivity_table[] = {
    /*  cfg0 		cfg1 		cfg2 */
//ch0  PB0
    {  109,   131,  4318}, // level 0
    {  109,   131,  3982}, // level 1
    {  109,   131,  3646}, // level 2
    {  109,   131,  3310}, // level 3
    {  109,   131,  2974}, // level 4
    {  109,   131,  2638}, // level 5
    {  109,   131,  2303}, // level 6
    {  109,   131,  1967}, // level 7
    {  109,   131,  1631}, // level 8
    {  109,   131,  1295}, // level 9
//ch1  PB1
    {   10,    15,   126}, // level 0
    {   10,    15,   117}, // level 1
    {   10,    15,   107}, // level 2
    {   10,    15,    97}, // level 3
    {   10,    15,    87}, // level 4
    {   10,    15,    77}, // level 5
    {   10,    15,    67}, // level 6
    {   10,    15,    57}, // level 7
    {   10,    15,    47}, // level 8
    {   10,    15,    38}, // level 9
//ch2  PB2
    {   10,    15,   152}, // level 0
    {   10,    15,   140}, // level 1
    {   10,    15,   128}, // level 2
    {   10,    15,   116}, // level 3
    {   10,    15,   104}, // level 4
    {   10,    15,    92}, // level 5
    {   10,    15,    81}, // level 6
    {   10,    15,    69}, // level 7
    {   10,    15,    57}, // level 8
    {   10,    15,    45}, // level 9
//ch3  PB4
    {   10,    15,   152}, // level 0
    {   10,    15,   140}, // level 1
    {   10,    15,   128}, // level 2
    {   10,    15,   116}, // level 3
    {   10,    15,   104}, // level 4
    {   10,    15,    92}, // level 5
    {   10,    15,    81}, // level 6
    {   10,    15,    69}, // level 7
    {   10,    15,    57}, // level 8
    {   10,    15,    45}, // level 9
    //ch4  PB5
    {  109,   131,  4318}, // level 0
    {  109,   131,  3982}, // level 1
    {  109,   131,  3646}, // level 2
    {  109,   131,  3310}, // level 3
    {  109,   131,  2974}, // level 4
    {  109,   131,  2638}, // level 5
    {  109,   131,  2303}, // level 6
    {  109,   131,  1967}, // level 7
    {  109,   131,  1631}, // level 8
    {  109,   131,  1295}, // level 9
};


#define LPCTMU_VH_LEVEL     3 // 上限电压档位
#define LPCTMU_VL_LEVEL     0 // 下限电压档位
#define LPCTMU_CUR_LEVEL    7 // 充放电电流档位

static const u8 lpctmu_ana_vh_table[4] = {
    LPCTMU_VH_065V,
    LPCTMU_VH_070V,
    LPCTMU_VH_075V,
    LPCTMU_VH_080V,
};
static const u8 lpctmu_ana_vl_table[4] = {
    LPCTMU_VL_020V,
    LPCTMU_VL_025V,
    LPCTMU_VL_030V,
    LPCTMU_VL_035V,
};
static const u8 lpctmu_ana_cur_table[8] = {
    LPCTMU_ISEL_036UA,
    LPCTMU_ISEL_072UA,
    LPCTMU_ISEL_108UA,
    LPCTMU_ISEL_144UA,
    LPCTMU_ISEL_180UA,
    LPCTMU_ISEL_216UA,
    LPCTMU_ISEL_252UA,
    LPCTMU_ISEL_288UA
};

u8 get_lpctmu_ana_level(void)
{
    return ((lpctmu_ana_vh_table[LPCTMU_VH_LEVEL]) | \
            (lpctmu_ana_vl_table[LPCTMU_VL_LEVEL]) | \
            (lpctmu_ana_cur_table[LPCTMU_CUR_LEVEL]));
}


struct lp_touch_key_alog_cfg {
    u16 ready_flag;
    u16 range;
    s32 sigma;
};
static struct lp_touch_key_alog_cfg alog_cfg[5];
static u32 alog_sta_cnt[5];
static u8 ch_range_sensity[5] = {7, 7, 7, 7, 7};
static u16 save_alog_cfg_timeout[5] = {0, 0, 0, 0, 0};
static u16 ctmu_res_scan_time_add = 0;
#define TOUCH_RANGE_MIN     50
#define TOUCH_RANGE_MAX     500

#define CTMU_RES_BUF_SIZE   15
static u16 ctmu_res_buf[5][CTMU_RES_BUF_SIZE];
static u16 ctmu_res_buf_in[5] = {0, 0, 0, 0, 0};
static u16 falling_res_avg[5] = {0, 0, 0, 0, 0};
static u16 long_event_res_avg[5] = {0, 0, 0, 0, 0};

static const u8 _ch_priv[5] = {0, 1, 2, 3, 4};

int __attribute__((weak)) lp_touch_key_event_remap(struct sys_event *e)
{
    return true;
}

static void __ctmu_notify_key_event(struct sys_event *event, u8 ch)
{

#if CFG_DISABLE_KEY_EVENT
    return;
#endif

    event->type = SYS_KEY_EVENT;
    event->u.key.type = KEY_DRIVER_TYPE_CTMU_TOUCH; 	//区分按键类型
    event->arg  = (void *)DEVICE_EVENT_FROM_KEY;

    if (__this->key_ch_msg_lock) {//锁住期间不能发消息
        return;
    }

#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
    if (lp_touch_key_online_debug_key_event_handle(ch, event)) {
        return;
    }
#endif /* #if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE */

    if (lp_touch_key_event_remap(event)) {
        sys_event_notify(event);
    }
}

__attribute__((weak)) u8 remap_ctmu_short_click_event(u8 ch, u8 click_cnt, u8 event)
{
    return event;
}


#if (TCFG_LP_SLIDE_KEY_ENABLE && (!TCFG_LP_SLIDE_KEY_SHORT_DISTANCE))

u8 __attribute__((weak)) ctmu_slide_direction_handle(u8 cur_ch, u8 pre_ch)
{
    if ((cur_ch == 0xff) || (pre_ch == 0xff)) {
        return TOUCH_KEY_EVENT_MAX;
    }
    u8 dir_case = (pre_ch << 4) | (cur_ch & 0xf);
    log_debug("ctmu_touch_key_slide_direction = 0x%x  :  %x -> %x\n", dir_case, pre_ch, cur_ch);
    switch (dir_case) {
    case 0x12:      //方向 1 -> 2
    case 0x23:      //方向 2 -> 3
    case 0x13:      //方向 1 -> 3
        return TOUCH_KEY_EVENT_SLIDE_UP;
        break;
    case 0x32:      //方向 3 -> 2
    case 0x21:      //方向 2 -> 1
    case 0x31:      //方向 3 -> 1
        return TOUCH_KEY_EVENT_SLIDE_DOWN;
        break;
    }
    return TOUCH_KEY_EVENT_MAX;
}

#endif

static void __ctmu_short_click_time_out_handle(void *priv)
{
    u8 ch = *((u8 *)priv);
    struct sys_event e;
    switch (__this->click_cnt[ch]) {
    case 0:
        return;
        break;
    case 1:
        e.u.key.event = KEY_EVENT_CLICK;
        break;
    case 2:
        e.u.key.event = KEY_EVENT_DOUBLE_CLICK;
        break;
    case 3:
        e.u.key.event = KEY_EVENT_TRIPLE_CLICK;
        break;
    case 4:
        e.u.key.event = KEY_EVENT_FOURTH_CLICK;
        break;
    case 5:
        e.u.key.event = KEY_EVENT_FIRTH_CLICK;
        break;
    default:
        e.u.key.event = KEY_EVENT_USER;
        break;
    }

    e.u.key.event = remap_ctmu_short_click_event(ch, __this->click_cnt[ch], e.u.key.event);
    e.u.key.value = __this->config->ch[ch].key_value;

#if (TCFG_LP_SLIDE_KEY_ENABLE && (!TCFG_LP_SLIDE_KEY_SHORT_DISTANCE))
    if (__this->slide_dir < TOUCH_KEY_EVENT_MAX) {
        e.u.key.event = __this->slide_dir;
        e.u.key.value = __this->config->slide_mode_key_value;
        __this->slide_dir = ctmu_slide_direction_handle(0xff, 0xff);
    }
#endif

    log_debug("notify key%d short event, cnt: %d", ch, __this->click_cnt[ch]);
    __ctmu_notify_key_event(&e, ch);

    __this->short_timer[ch] = LP_TOUCH_KEY_TIMER_MAGIC_NUM;
    __this->last_key[ch] = CTMU_KEY_NULL;
    __this->click_cnt[ch] = 0;
}

static void ctmu_short_click_handle(u8 ch)
{
    static u8 pre_ch = 0xff;
    __this->slide_dir = TOUCH_KEY_EVENT_MAX;
    __this->last_key[ch] = CTMU_KEY_SHORT_CLICK;
    if (__this->short_timer[ch] == LP_TOUCH_KEY_TIMER_MAGIC_NUM) {

#if (TCFG_LP_SLIDE_KEY_ENABLE && (!TCFG_LP_SLIDE_KEY_SHORT_DISTANCE))
        if (__this->config->slide_mode_en) {
            if ((pre_ch != 0xff) && (pre_ch != ch) && (__this->click_cnt[pre_ch] == 1)) {
                usr_timeout_del(__this->short_timer[pre_ch]);
                __this->short_timer[pre_ch] = LP_TOUCH_KEY_TIMER_MAGIC_NUM;
                __this->click_cnt[pre_ch] = 0;
                __this->last_key[pre_ch] = CTMU_KEY_NULL;
                __this->slide_dir = ctmu_slide_direction_handle(ch, pre_ch);
            }
            pre_ch = ch;
        }
#endif
        __this->click_cnt[ch] = 1;
#if (TCFG_LP_SLIDE_KEY_ENABLE && TCFG_LP_SLIDE_KEY_SHORT_DISTANCE)
        __this->short_timer[ch] = usr_timeout_add((void *)&_ch_priv[ch], __ctmu_short_click_time_out_handle, CTMU_SHORT_CLICK_DELAY_TIME +  150, 1);
#else
        __this->short_timer[ch] = usr_timeout_add((void *)&_ch_priv[ch], __ctmu_short_click_time_out_handle, CTMU_SHORT_CLICK_DELAY_TIME, 1);
#endif
    } else {
        __this->click_cnt[ch]++;
        usr_timer_modify(__this->short_timer[ch], CTMU_SHORT_CLICK_DELAY_TIME);
    }
}

static void ctmu_raise_click_handle(u8 ch)
{
    struct sys_event e = {0};
    if (__this->last_key[ch] >= CTMU_KEY_LONG_CLICK) {
        e.u.key.event = KEY_EVENT_UP;
        e.u.key.value = __this->config->ch[ch].key_value;
        __ctmu_notify_key_event(&e, ch);

        __this->last_key[ch] = CTMU_KEY_NULL;
        log_debug("notify key HOLD UP event");
    } else {
        ctmu_short_click_handle(ch);
    }
}

static void ctmu_long_click_handle(u8 ch)
{
    __this->last_key[ch] = CTMU_KEY_LONG_CLICK;

    struct sys_event e;
    e.u.key.event = KEY_EVENT_LONG;
    e.u.key.value = __this->config->ch[ch].key_value;

    __ctmu_notify_key_event(&e, ch);
}

static void ctmu_hold_click_handle(u8 ch)
{
    __this->last_key[ch] = CTMU_KEY_HOLD_CLICK;

    struct sys_event e;
    e.u.key.event = KEY_EVENT_HOLD;
    e.u.key.value = __this->config->ch[ch].key_value;

    __ctmu_notify_key_event(&e, ch);
}


#if TCFG_LP_EARTCH_KEY_ENABLE

__attribute__((weak))
void ear_lptouch_update_state(u8 state)
{
    return;
}

static void __ctmu_ear_in_timeout_handle(void *priv)
{
#if CFG_DISABLE_INEAR_EVENT
    return;
#endif
    if (__this->config->ch[__this->config->eartch_ch].key_value == 0xFF) {
        //使用外部自定义流程
        if (__this->eartch_last_state == EAR_IN) {
            ear_lptouch_update_state(0);
        } else {
            ear_lptouch_update_state(1);
        }
        return;
    }

#if TCFG_EARTCH_EVENT_HANDLE_ENABLE
    u8 state;
    if (__this->eartch_last_state == EAR_IN) {
        state = EARTCH_STATE_IN;
    } else if (__this->eartch_last_state == EAR_OUT) {
        state = EARTCH_STATE_OUT;
    } else {
        return;
    }
    eartch_state_update(state);
#endif /* #if TCFG_EARTCH_EVENT_HANDLE_ENABLE */
}

static void __ctmu_key_unlock_timeout_handle(void *priv)
{
    __this->key_ch_msg_lock = false;
    __this->key_ch_msg_lock_timer = 0;
}

static void ctmu_eartch_event_handle(u8 eartch_state)
{
    __this->eartch_last_state = eartch_state;

#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
    struct sys_event event;
    if (eartch_state == EAR_IN) {
        event.u.key.event = 10;
    } else if (eartch_state == EAR_OUT) {
        event.u.key.event = 11;
    }
    if (lp_touch_key_online_debug_key_event_handle(__this->config->eartch_ch, &event)) {
        return;
    }
#endif /* #if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE */

    __this->key_ch_msg_lock = true;

    if (__this->key_ch_msg_lock_timer == 0) {
        __this->key_ch_msg_lock_timer = sys_hi_timeout_add(NULL, __ctmu_key_unlock_timeout_handle, ERATCH_KEY_MSG_LOCK_TIME);
    } else {
        sys_hi_timer_modify(__this->key_ch_msg_lock_timer, ERATCH_KEY_MSG_LOCK_TIME);
    }
    __ctmu_ear_in_timeout_handle(NULL);
}

#endif

/*************************************************************************************************************************
  										lp_touch_key driver
*************************************************************************************************************************/
#if CTMU_CHECK_LONG_CLICK_BY_RES

static void lp_touch_key_ctmu_res_buf_clear(u8 ch)
{
    ctmu_res_buf_in[ch] = 0;
    for (u8 i = 0; i < CTMU_RES_BUF_SIZE; i ++) {
        ctmu_res_buf[ch][i] = 0;
    }
}

static void lp_touch_key_ctmu_res_all_buf_clear(void)
{
    for (u8 ch = 0; ch < LP_CTMU_CHANNEL_SIZE; ch ++) {
        lp_touch_key_ctmu_res_buf_clear(ch);
    }
}

static u32 lp_touch_key_ctmu_res_buf_avg(u8 ch)
{
#if !TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
    u32 res_sum = 0;
    u8 j = 0;
    u8 cnt = ctmu_res_buf_in[ch];
    for (u8 i = 0; i < (CTMU_RES_BUF_SIZE - 5); i ++) {
        if (ctmu_res_buf[ch][cnt]) {
            res_sum += ctmu_res_buf[ch][cnt];
            j ++;
        } else {
            return 0;
        }
        cnt ++;
        if (cnt >= CTMU_RES_BUF_SIZE) {
            cnt = 0;
        }
    }
    if (res_sum) {
        return (res_sum / j);
    }
#endif
    return 0;
}

static u8 lp_touch_key_check_long_click_by_ctmu_res(u8 ch)
{
#if !TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
    long_event_res_avg[ch] = lp_touch_key_ctmu_res_buf_avg(ch);
    log_debug("long_event_res_avg: %d\n", long_event_res_avg[ch]);
    if ((falling_res_avg[ch] < 2000)  || \
        (falling_res_avg[ch] > 20000) || \
        (long_event_res_avg[ch] < 2000) || \
        (long_event_res_avg[ch] > 20000)) {
    } else {
        u16 cfg2 = (M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2H + ch * 8) << 8) | M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2L + ch * 8);
        u16 diff = (falling_res_avg[ch] > long_event_res_avg[ch]) ? (falling_res_avg[ch] - long_event_res_avg[ch]) : (long_event_res_avg[ch] - falling_res_avg[ch]);
        if (diff < (cfg2 / 2)) {
            log_debug("long event return ! diff: %d  <  cfg2/2: %d\n", diff, (cfg2 / 2));

            //复位算法
            lp_touch_key_send_cmd(CTMU_M2P_RESET_ALGO);
            return 1;
        }
    }
#endif
    return 0;
}

#endif

u8 lp_touch_key_alog_range_display(u8 *display_buf)
{
    if (__this->init == 0) {
        return 0;
    }
    u8 tmp_buf[32], i = 0;
    memset(tmp_buf, 0, sizeof(tmp_buf));
    for (u8 ch = 0; ch < LP_CTMU_CHANNEL_SIZE; ch ++) {
        if (__this->config->ch[ch].enable) {
            u16 range = alog_cfg[ch].range % 1000;
            tmp_buf[0 + i * 4] = (range / 100) + '0';
            tmp_buf[1 + i * 4] = ((range % 100) / 10) + '0';
            tmp_buf[2 + i * 4] = (range % 10) + '0';
            tmp_buf[3 + i * 4] = ' ';
            i ++;
        }
    }
    if (i) {
        display_buf[0] = i * 4 + 1;
        display_buf[1] = 0x01;
        memcpy((u8 *)&display_buf[2], tmp_buf, i * 4);
        printf_buf(display_buf, i * 4 + 2);
        return (display_buf[0] + 1);
    }
    return 0;
}

#if !TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE

void lp_touch_key_save_alog_cfg(void *priv)
{
    u8 ch = *((u8 *)priv);
    int ret = syscfg_write(VM_LP_TOUCH_KEY0_ALOG_CFG + ch, (void *)&alog_cfg[ch], sizeof(struct lp_touch_key_alog_cfg));
    if (ret != sizeof(struct lp_touch_key_alog_cfg)) {
        log_info("write vm alog cfg ready flag error !\n");
    }
    save_alog_cfg_timeout[ch] = 0;
}

void lp_touch_key_ctmu_cfg2_check_update(u8 ch, u16 cfg2_new)
{
    u16 cfg0 = (M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0H + ch * 8) << 8) | M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0L + ch * 8);
    u16 cfg2_old = (M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2H + ch * 8) << 8) | M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2L + ch * 8);
    if ((cfg2_old != cfg2_new) && (cfg2_new > (3 * cfg0))) {
        printf("ctmu ch%d cfg2_old = %d  cfg2_new = %d\n", ch, cfg2_old, cfg2_new);
        M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2L + ch * 8) = cfg2_new & 0xff;
        M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2H + ch * 8) = (cfg2_new >> 8) & 0xff;
        u16 cfg1 = (M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG1H + ch * 8) << 8) | (M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG1L + ch * 8));
        M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0L + ch * 8) = cfg1 & 0xff;
        M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0H + ch * 8) = (cfg1 >> 8) & 0xff;
    }
}

static void lp_touch_key_ctmu_res_scan(void *priv)
{
    for (u8 ch = 0; ch < LP_CTMU_CHANNEL_SIZE; ch ++) {
        if (__this->config->ch[ch].enable) {
            if (__this->config->eartch_en && (__this->config->eartch_ch == ch)) {
            } else {
                u16 ctmu_res0;
                u16 ctmu_res1;
__read_res:
                ctmu_res0 = (P2M_MESSAGE_ACCESS(P2M_MASSAGE_CTMU_CH0_H_RES + ch * 2) << 8) | P2M_MESSAGE_ACCESS(P2M_MASSAGE_CTMU_CH0_L_RES + ch * 2);
                delay(100);
                ctmu_res1 = (P2M_MESSAGE_ACCESS(P2M_MASSAGE_CTMU_CH0_H_RES + ch * 2) << 8) | P2M_MESSAGE_ACCESS(P2M_MASSAGE_CTMU_CH0_L_RES + ch * 2);
                if (ctmu_res0 != ctmu_res1) {
                    goto __read_res;
                }
#if 1

                if ((ctmu_res0 < 2000) || (ctmu_res0 > 20000)) {
                    continue;
                }

#if CTMU_CHECK_LONG_CLICK_BY_RES
                ctmu_res_buf[ch][ctmu_res_buf_in[ch]] = ctmu_res0;
                ctmu_res_buf_in[ch] ++;
                if (ctmu_res_buf_in[ch] >= CTMU_RES_BUF_SIZE) {
                    ctmu_res_buf_in[ch] = 0;
                }
#endif

                if (alog_cfg[ch].ready_flag == 0) {
                    continue;
                }
                if (alog_sta_cnt[ch] < 50) {
                    alog_sta_cnt[ch] ++;
                    continue;
                }
                TouchAlgo_Update(ch, ctmu_res0);

                u8 range_valid = 0;
                u16 touch_range = TouchAlgo_GetRange(ch, (u8 *)&range_valid);
                s32 touch_sigma = TouchAlgo_GetSigma(ch);
                /* printf("ch%d res:%d range:%d val:%d sigma:%d\n", ch, ctmu_res0, touch_range, range_valid, touch_sigma); */
                if ((range_valid) && (touch_range > TOUCH_RANGE_MIN) && (touch_range < TOUCH_RANGE_MAX)) {
                    if (touch_range != alog_cfg[ch].range) {
                        u16 cfg2_new = touch_range * (10 - ch_range_sensity[ch]) / 10;
                        lp_touch_key_ctmu_cfg2_check_update(ch, cfg2_new);
                        alog_cfg[ch].range = touch_range;
                        alog_cfg[ch].sigma = touch_sigma;
                        if (save_alog_cfg_timeout[ch] == 0) {
                            save_alog_cfg_timeout[ch] = sys_timeout_add((void *)&_ch_priv[ch], lp_touch_key_save_alog_cfg, 1);
                        }
                    }
                } else if ((range_valid) && (touch_range >= TOUCH_RANGE_MAX)) {
                    TouchAlgo_SetRange(ch, alog_cfg[ch].range);
                }
#endif
            }
        }
    }
}

void lp_touch_key_alog_ready_flag_check_and_set(void)
{
    for (u8 ch = 0; ch < LP_CTMU_CHANNEL_SIZE; ch ++) {
        if (__this->config->ch[ch].enable) {
            if (__this->config->eartch_en && (__this->config->eartch_ch == ch)) {
            } else {
                alog_cfg[ch].ready_flag = 0;
                syscfg_read(VM_LP_TOUCH_KEY0_ALOG_CFG + ch, (void *)&alog_cfg[ch], sizeof(struct lp_touch_key_alog_cfg));
                if (alog_cfg[ch].ready_flag == 0) {
                    alog_cfg[ch].ready_flag = 1;
                    lp_touch_key_save_alog_cfg((void *)&ch);
                }
            }
        }
    }
}

#endif


static void ctmu_port_init(u8 port)
{
    gpio_set_die(port, 0);
    gpio_set_dieh(port, 0);
    gpio_set_direction(port, 1);
    gpio_set_pull_down(port, 0);
    gpio_set_pull_up(port, 0);
}

u8 p11_wakeup_query(void)
{
    return is_wakeup_source(PWR_WK_REASON_P11);
};

void lp_touch_key_send_cmd(enum CTMU_M2P_CMD cmd)
{
    M2P_CTMU_CMD = cmd;
    P11_M2P_INT_SET = BIT(M2P_CTMU_INDEX);
}

#define IO_RESET_PORTB_01 			18
void lp_touch_key_init(const struct lp_touch_key_platform_data *config)
{
    log_info("%s >>>>", __func__);

    ASSERT(config && (__this->init == 0));
    __this->config = config;

    u8 pinr_io;
    if (P33_CON_GET(P3_PINR_CON) & BIT(0)) {
        pinr_io = P33_CON_GET(P3_PORT_SEL0);
        if (pinr_io == IO_RESET_PORTB_01) {
            P33_CON_SET(P3_PINR_CON, 0, 8, 0);
            P33_CON_SET(P3_PINR_CON1, 0, 8, 0x16);
            P33_CON_SET(P3_PINR_CON1, 0, 1, 1);
            log_info("reset pin change: old: %d, P3_PINR_CON1 = 0x%x", pinr_io, P33_CON_GET(P3_PINR_CON1));
        }
    }

    u8 ch_enable = 0;
    u8 ch_wakeup_en = 0;
    u8 ch_debug = 0;
    for (u8 ch = 0; ch < LP_CTMU_CHANNEL_SIZE; ch ++) {
        if (__this->config->ch[ch].enable) {
            ch_enable |= BIT(ch);
            if (__this->config->ch[ch].wakeup_enable) {
                ch_wakeup_en |= BIT(ch);
            }
            if (CFG_CHx_DEBUG_ENABLE & BIT(ch)) {
                ch_debug |= BIT(ch);
            }
        }
    }
    if (p11_wakeup_query() == 0 || get_charge_online_flag() || (!(ch_wakeup_en)) || (is_ldo5v_wakeup())) {
        for (u32 m2p_addr = 0x18; m2p_addr < 0x56; m2p_addr ++) {
            M2P_MESSAGE_ACCESS(m2p_addr) = 0;
        }
        for (u32 p2m_addr = 0x10; p2m_addr < 0x24; p2m_addr ++) {
            P2M_MESSAGE_ACCESS(p2m_addr) = 0;
        }
    }

    M2P_CTMU_MSG = 0;
    M2P_CTMU_CH_CFG = 0;
    M2P_CTMU_TIME_BASE = CTMU_SAMPLE_RATE_PRD;

    M2P_CTMU_LONG_TIMEL = (CTMU_LONG_KEY_DELAY_TIME & 0xFF);
    M2P_CTMU_LONG_TIMEH = ((CTMU_LONG_KEY_DELAY_TIME >> 8) & 0xFF);
    M2P_CTMU_HOLD_TIMEL = (CTMU_HOLD_CLICK_DELAY_TIME & 0xFF);
    M2P_CTMU_HOLD_TIMEH = ((CTMU_HOLD_CLICK_DELAY_TIME >> 8) & 0xFF);

    M2P_CTMU_SOFTOFF_LONG_TIMEL = (CFG_M2P_CTMU_SOFTOFF_LONG_TIME & 0xFF);
    M2P_CTMU_SOFTOFF_LONG_TIMEH = ((CFG_M2P_CTMU_SOFTOFF_LONG_TIME >> 8) & 0xFF);

#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
    M2P_CTMU_LONG_PRESS_RESET_TIME_VALUE_L = 0;
    M2P_CTMU_LONG_PRESS_RESET_TIME_VALUE_H = 0;
#else
    M2P_CTMU_LONG_PRESS_RESET_TIME_VALUE_L = (CTMU_RESET_TIME_CONFIG & 0xFF);
    M2P_CTMU_LONG_PRESS_RESET_TIME_VALUE_H = ((CTMU_RESET_TIME_CONFIG  >> 8) & 0xFF);
#endif

    log_info("M2P_CTMU_LONG_TIMEL = 0x%x", M2P_CTMU_LONG_TIMEL);
    log_info("M2P_CTMU_LONG_TIMEH = 0x%x", M2P_CTMU_LONG_TIMEH);
    log_info("M2P_CTMU_HOLD_TIMEL = 0x%x", M2P_CTMU_HOLD_TIMEL);
    log_info("M2P_CTMU_HOLD_TIMEH = 0x%x", M2P_CTMU_HOLD_TIMEH);

    log_info("M2P_CTMU_SOFTOFF_LONG_TIMEL = 0x%x", M2P_CTMU_SOFTOFF_LONG_TIMEL);
    log_info("M2P_CTMU_SOFTOFF_LONG_TIMEH = 0x%x", M2P_CTMU_SOFTOFF_LONG_TIMEH);

    log_info("M2P_CTMU_LONG_PRESS_RESET_TIME_VALUE_L = 0x%x", M2P_CTMU_LONG_PRESS_RESET_TIME_VALUE_L);
    log_info("M2P_CTMU_LONG_PRESS_RESET_TIME_VALUE_H = 0x%x", M2P_CTMU_LONG_PRESS_RESET_TIME_VALUE_H);

    M2P_CTMU_CH_ENABLE = ch_enable;
    M2P_CTMU_CH_WAKEUP_EN = ch_wakeup_en;
    M2P_CTMU_CH_DEBUG = ch_debug;

    for (u8 ch = 0; ch < LP_CTMU_CHANNEL_SIZE; ch ++) {
        if (__this->config->ch[ch].enable) {
            u8 ch_sensity = __this->config->ch[ch].sensitivity;
            M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0L + ch * 8) = ((ch_sensitivity_table[ch_sensity + ch * 10].cfg0) & 0xFF);
            M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0H + ch * 8) = (((ch_sensitivity_table[ch_sensity + ch * 10].cfg0) >> 8) & 0xFF);
            M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG1L + ch * 8) = ((ch_sensitivity_table[ch_sensity + ch * 10].cfg1) & 0xFF);
            M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG1H + ch * 8) = (((ch_sensitivity_table[ch_sensity + ch * 10].cfg1) >> 8) & 0xFF);
            M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2L + ch * 8) = ((ch_sensitivity_table[ch_sensity + ch * 10].cfg2) & 0xFF);
            M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2H + ch * 8) = (((ch_sensitivity_table[ch_sensity + ch * 10].cfg2) >> 8) & 0xFF);

            log_info("M2P_CTMU_CH%d_CFG0L = 0x%x", ch, M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0L + ch * 8));
            log_info("M2P_CTMU_CH%d_CFG0H = 0x%x", ch, M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0H + ch * 8));
            log_info("M2P_CTMU_CH%d_CFG1L = 0x%x", ch, M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG1L + ch * 8));
            log_info("M2P_CTMU_CH%d_CFG1H = 0x%x", ch, M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG1H + ch * 8));
            log_info("M2P_CTMU_CH%d_CFG2L = 0x%x", ch, M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2L + ch * 8));
            log_info("M2P_CTMU_CH%d_CFG2H = 0x%x", ch, M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2H + ch * 8));

            ctmu_port_init(__this->config->ch[ch].port);
#if TCFG_LP_EARTCH_KEY_ENABLE
            if (__this->config->eartch_en && (__this->config->eartch_ch == ch)) {
                M2P_CTMU_CH_CFG |= BIT(1);
                M2P_CTMU_EARTCH_CH = (__this->config->eartch_ref_ch << 4) | (__this->config->eartch_ch & 0xf);
#if (TCFG_EARTCH_EVENT_HANDLE_ENABLE && TCFG_LP_EARTCH_KEY_ENABLE)
                extern int eartch_event_deal_init(void);
                eartch_event_deal_init();
#endif
                u16 trim_value;
                int ret = syscfg_read(LP_KEY_EARTCH_TRIM_VALUE, &trim_value, sizeof(trim_value));
                __this->eartch_trim_flag = 0;
                __this->eartch_inear_ok = 0;
                __this->eartch_trim_value = 0;
                if (ret > 0) {
                    __this->eartch_inear_ok = 1;
                    __this->eartch_trim_value = trim_value;
                    log_info("eartch_trim_value = %d", __this->eartch_trim_value);
                    M2P_CTMU_EARTCH_TRIM_VALUE_L = (__this->eartch_trim_value & 0xFF);
                    M2P_CTMU_EARTCH_TRIM_VALUE_H = ((__this->eartch_trim_value >> 8) & 0xFF);
                } else {
                    //没有trim的情况下用不了
                    M2P_CTMU_EARTCH_TRIM_VALUE_L = (10000 & 0xFF);
                    M2P_CTMU_EARTCH_TRIM_VALUE_H = ((10000 >> 8) & 0xFF);
                }
                //软件触摸灵敏度调试
                M2P_CTMU_INEAR_VALUE_L = __this->config->eartch_soft_inear_val & 0xFF;
                M2P_CTMU_INEAR_VALUE_H = __this->config->eartch_soft_inear_val >> 8;
                M2P_CTMU_OUTEAR_VALUE_L = __this->config->eartch_soft_outear_val & 0xFF;
                M2P_CTMU_OUTEAR_VALUE_H = __this->config->eartch_soft_outear_val >> 8;
            } else
#endif
            {
#if !TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
                TouchAlgo_Init(ch, TOUCH_RANGE_MIN, TOUCH_RANGE_MAX);
                alog_sta_cnt[ch] = 0;
                ch_range_sensity[ch] = ch_sensity;
                log_info("read vm alog cfg\n");
                int ret = syscfg_read(VM_LP_TOUCH_KEY0_ALOG_CFG + ch, (void *)&alog_cfg[ch], sizeof(struct lp_touch_key_alog_cfg));
                if ((ret == (sizeof(struct lp_touch_key_alog_cfg))) && (alog_cfg[ch].range > TOUCH_RANGE_MIN) && (alog_cfg[ch].range < TOUCH_RANGE_MAX)) {
                    log_info("vm read ch%d alog ready:%d sigma:%d range:%d\n", ch, alog_cfg[ch].ready_flag, alog_cfg[ch].sigma, alog_cfg[ch].range);
                    u16 cfg0 = (M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0H + ch * 8) << 8) | (M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0L + ch * 8));
                    M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG1L + ch * 8) = (cfg0 + 5) & 0xff;
                    M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG1H + ch * 8) = ((cfg0 + 5) >> 8) & 0xff;
                    u16 cfg2_new = alog_cfg[ch].range * (10 - ch_range_sensity[ch]) / 10;
                    M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2L + ch * 8) = cfg2_new & 0xff;
                    M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2H + ch * 8) = (cfg2_new >> 8) & 0xff;
                    TouchAlgo_SetSigma(ch, alog_cfg[ch].sigma);
                    TouchAlgo_SetRange(ch, alog_cfg[ch].range);
                }

#endif
            }
        }
    }

#if !TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE

#if 1   //触摸算法要尽可能的在装完整机之后开始跑，而这里是耳机生产流程中获取它第一次在舱内开机的时候，在vm标记一个变量，从此开始跑算法。

    if (get_charge_online_flag()) {
        log_info("check charge online!\n");
        lp_touch_key_alog_ready_flag_check_and_set();//如果有其他好的位置，请将该函数移到那个位置执行。
    }

#endif

    if (ctmu_res_scan_time_add == 0) {
        ctmu_res_scan_time_add = usr_timer_add(NULL, lp_touch_key_ctmu_res_scan, 20, 0);
    }

#if CTMU_CHECK_LONG_CLICK_BY_RES
    lp_touch_key_ctmu_res_all_buf_clear();
#endif

#endif

    //CTMU 初始化命令
    if (p11_wakeup_query() == 0 || get_charge_online_flag() || (!(ch_wakeup_en)) || (is_ldo5v_wakeup())) {
        log_info("lp touch init by %d_%d_%d_%d\n", p11_wakeup_query(), get_charge_online_flag(), (!(ch_wakeup_en)), is_ldo5v_wakeup());
        LPCTMU_ANA0_CONFIG(get_lpctmu_ana_level());
        P2M_CTMU_WKUP_MSG &= (~(P2M_MESSAGE_INIT_MODE_FLAG));
    } else {
        P2M_CTMU_WKUP_MSG |= (P2M_MESSAGE_INIT_MODE_FLAG);
        log_info("p11 wakeup, lp touch key continue work");
    }
    __this->softoff_mode = LP_TOUCH_SOFTOFF_MODE_ADVANCE;

    log_info("M2P_CTMU_CH_ENABLE = 0x%x", M2P_CTMU_CH_ENABLE);
    log_info("M2P_CTMU_CH_WAKEUP_EN = 0x%x", M2P_CTMU_CH_WAKEUP_EN);
    log_info("M2P_CTMU_CH_CFG = 0x%x", M2P_CTMU_CH_CFG);
    log_info("M2P_CTMU_EARTCH_CH = 0x%x", M2P_CTMU_EARTCH_CH);

    lp_touch_key_send_cmd(CTMU_M2P_INIT);

#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
    lp_touch_key_online_debug_init();
#endif /* #if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE */

    __this->init = 1;
}


#if (TCFG_LP_SLIDE_KEY_ENABLE && TCFG_LP_SLIDE_KEY_SHORT_DISTANCE)


/******************************双通道滑动触摸识别的基础原理***********************************

1.单击，但如果超过设定的长按时间，那就为长按
chx  __________________________________________
chx  ___________                     __________
                |_______>180________|


2.单击，但如果超过设定的长按时间，那就为长按
chx  ___________                     __________
                |_______>180________|
chx  _______ <30                     <30 ______
            |___________________________|


3.单击，但如果超过设定的长按时间，那就为长按
chx  ___________                     __________
                |_______>180________|
chx  _______ <30                       >30   __
            |_______________________________|


4.单击，但如果超过设定的长按时间，那就为长按
chx  ___________                     __________
                |_______>180________|
chx  ___   >30                       <30 ______
        |_______________________________|


5.单击，但如果超过设定的长按时间，那就为长按
chx  ___________                     __________
                |_______>180________|
chx  ___   >30                         >30   __
        |___________________________________|


6.单击，但如果超过设定的长按时间，那就为长按
chx  ___________                         ______
                |_______________________|
chx  _______ <30        >180         _<30______
            |_______________________|


7.根据前后操作，有可能是滑动，也有可能是连击中的后单击，也有可能是无效操作
chx  ___________                 __________
                |_______________|
chx  _______ <30    <180     _<30__________
            |_______________|


8.滑动，t < 设定的长按的时间
chx  ___________                 __________
                |_______________|
chx  ___   >30       t       _<30__________
        |___________________|


9.滑动，t < 设定的长按的时间
chx  ___________                     ______
                |___________________|
chx  ___   >30       t       __>30_________
        |___________________|


10.滑动，t < 设定的长按的时间
chx  ___________                     ______
                |___________________|
chx  _______ <30     t       __>30_________
            |_______________|

*****************************************************************************/

enum falling_order_type {
    FALLING_NULL,
    FALLING_FIRST,
    FALLING_SECOND,
};
enum raising_order_type {
    RAISING_NULL,
    RAISING_FIRST,
    RAISING_SECOND,
};

enum time_interval_type {
    LONG_TIME,
    SHORT_TIME,
};

enum slide_key_type {
    SHORT_CLICK = 1,
    DOUBLE_CLICK,
    TRIPLE_CLICK,
    FOURTH_CLICK,
    FIRTH_CLICK,
    LONG_CLICK = 8,
    LONG_HOLD_CLICK = 9,
    LONG_UP_CLICK = 10,
    SLIDE_UP = 0x12,
    SLIDE_DOWN = 0x21,
};

static u8 falling_order[2] = {FALLING_NULL, FALLING_NULL};
static u8 raising_order[2] = {RAISING_NULL, RAISING_NULL};
static u8 falling_time_interval = 0;
static u8 raising_time_interval = 0;
static u8 last_key_type = 0;

#define FALLING_TIMEOUT_TIME    30              //两个按下边沿的时间间隔的阈值
static int falling_timeout_add = 0;
static void falling_timeout_handle(void *priv)
{
    falling_timeout_add = 0;
}

#define RAISING_TIMEOUT_TIME    30              //两个抬起边沿的时间间隔的阈值
static int raising_timeout_add = 0;
static void raising_timeout_handle(void *priv)
{
    raising_timeout_add = 0;
}

#define CLICK_IDLE_TIMEOUT_TIME 500             //如果该时间内，没有单击，那么该时间之后的第一次单击，识别为首次单击
static int click_idle_timeout_add = 0;
static void click_idle_timeout_handle(void *priv)
{
    click_idle_timeout_add = 0;
}

#define FIRST_SHORT_CLICK_TIMEOUT_TIME  190     //只针对首次单击，要满足按够那么长时间。连击时，后面的单击没有该时间要求
static int first_short_click_timeout_add = 0;
static void first_short_click_timeout_handle(void *priv)
{
    first_short_click_timeout_add = 0;
}

void __attribute__((weak)) send_keytone_event(void)
{
    //可以在此处发送按键音的消息
    /* struct sys_event e; */
    /* e.u.key.event = 5; */
    /* e.u.key.value = 3; */
    /* __ctmu_notify_key_event(&e, 0); */
}

#define SEND_KEYTONE_TIMEOUT_TIME   250         //长按时的按键音，在按下后的多长时间要响
static u8 send_keytone_flag = 0;
static int send_keytone_timeout_add = 0;
static void send_keytone_timeout_handle(void *priv)
{
    send_keytone_event();
    send_keytone_timeout_add = 0;
    send_keytone_flag = 1;
}

#define SLIDE_CLICK_TIMEOUT_TIME    500         //识别为滑动之后，该时间内，如果有case7.也要识别为滑动
#define FAST_SLIDE_CNT_MAX          4           //一来就连续那么多次的case7.，就会识别为一次滑动。
static u8 fast_slide_cnt = 0;
static u8 fast_slide_type = 0;
static int slide_click_timeout_add = 0;
void slide_click_timeout_handle(void *priv)
{
    slide_click_timeout_add = 0;
}

static void check_channel_falling_info(u8 ch)
{
    falling_order[ch] = FALLING_FIRST;
    if (falling_order[!ch] == FALLING_FIRST) {
        falling_order[ch] = FALLING_SECOND;
    }
    send_keytone_flag = 0;
    if (send_keytone_timeout_add == 0) {
        send_keytone_timeout_add = sys_hi_timeout_add(NULL, send_keytone_timeout_handle, SEND_KEYTONE_TIMEOUT_TIME);
    } else {
        sys_hi_timer_modify(send_keytone_timeout_add, SEND_KEYTONE_TIMEOUT_TIME);
    }
    if ((last_key_type != SHORT_CLICK) || (click_idle_timeout_add == 0)) {
        if (first_short_click_timeout_add == 0) {
            first_short_click_timeout_add = sys_hi_timeout_add(NULL, first_short_click_timeout_handle, FIRST_SHORT_CLICK_TIMEOUT_TIME);
        } else {
            sys_hi_timer_modify(first_short_click_timeout_add, FIRST_SHORT_CLICK_TIMEOUT_TIME);
        }
    }
    if (falling_order[ch] == FALLING_FIRST) {
        if (falling_timeout_add == 0) {
            falling_timeout_add = sys_hi_timeout_add(NULL, falling_timeout_handle, FALLING_TIMEOUT_TIME);
        } else {
            sys_hi_timer_modify(falling_timeout_add, FALLING_TIMEOUT_TIME);
        }
        falling_time_interval = SHORT_TIME;
    } else if (falling_order[ch] == FALLING_SECOND) {
        if (falling_timeout_add) {
            sys_hi_timeout_del(falling_timeout_add);
            falling_timeout_add = 0;
            falling_time_interval = SHORT_TIME;
        } else {
            falling_time_interval = LONG_TIME;
        }
    }
}

static u8 check_channel_raising_info_and_key_type(u8 ch)
{
    u8 key_type = 0;
    fast_slide_type = 0;
    if (falling_order[ch] == FALLING_NULL) {
        return key_type;
    }
    raising_order[ch] = RAISING_FIRST;
    if (raising_order[!ch] == RAISING_FIRST) {
        raising_order[ch] = RAISING_SECOND;
    }
    if (falling_order[ch] > falling_order[!ch]) {
        if (raising_order[ch] == RAISING_FIRST) {
            key_type = SHORT_CLICK;             //case 1~5.
        } else {
            if (falling_time_interval == SHORT_TIME) {
                if (raising_timeout_add) {
                    sys_hi_timeout_del(raising_timeout_add);
                    raising_timeout_add = 0;
                    key_type = SHORT_CLICK;     //case 6~7.
                    if (ch == 0) {
                        fast_slide_type = SLIDE_DOWN;
                    } else {
                        fast_slide_type = SLIDE_UP;
                    }
                } else if (ch == 0) {
                    key_type = SLIDE_DOWN;      //case 10.
                } else {
                    key_type = SLIDE_UP;        //case 10.
                }
            } else if (ch == 0) {
                key_type = SLIDE_DOWN;          //caee 8~9.
            } else {
                key_type = SLIDE_UP;            //case 8~9.
            }
        }
    } else {
        if (raising_order[ch] == RAISING_FIRST) {
            if (falling_time_interval == SHORT_TIME) {
                if (raising_timeout_add == 0) {
                    raising_timeout_add = sys_hi_timeout_add(NULL, raising_timeout_handle, RAISING_TIMEOUT_TIME);
                } else {
                    sys_hi_timer_modify(raising_timeout_add, RAISING_TIMEOUT_TIME);
                }
            } else if (ch == 0) {
                key_type = SLIDE_UP;            //case 8~9.
            } else {
                key_type = SLIDE_DOWN;          //case 8~9.
            }
        } else {
            if (falling_time_interval == SHORT_TIME) {
                if (raising_timeout_add) {
                    sys_hi_timeout_del(raising_timeout_add);
                    raising_timeout_add = 0;
                    key_type = SHORT_CLICK;     //case 6~7
                    if (ch == 0) {
                        fast_slide_type = SLIDE_UP;
                    } else {
                        fast_slide_type = SLIDE_DOWN;
                    }
                } else if (ch == 0) {
                    key_type = SLIDE_UP;        //case 10.
                } else {
                    key_type = SLIDE_DOWN;      //case 10.
                }
            } else if (ch == 0) {
                key_type = SLIDE_UP;            //case 8~9.
            } else {
                key_type = SLIDE_DOWN;          //case 8~9.
            }
        }
    }
    return key_type;
}

static u8 get_slide_event_ch(void)
{
    u8 event_ch = 1;
    for (u8 ch = 0; ch < LP_CTMU_CHANNEL_SIZE; ch ++) {
        if (__this->config->ch[ch].enable) {
            event_ch = ch;
            break;
        }
    }
    return event_ch;
}

static u8 check_slide_key_type(u8 event, u8 ch)
{
    u8 key_type = 0;
    u8 event_ch = get_slide_event_ch();
    if (event == (CTMU_P2M_CH0_FALLING_EVENT + ch * 8)) {
        if (ch == event_ch) {
            check_channel_falling_info(0);
        } else {
            check_channel_falling_info(1);
        }
    } else if (event == (CTMU_P2M_CH0_RAISING_EVENT + ch * 8)) {
        if (ch == event_ch) {
            if ((last_key_type == LONG_CLICK) || (last_key_type == LONG_HOLD_CLICK)) {
                key_type = LONG_UP_CLICK;       //一定是长按抬起
            } else {
                key_type = check_channel_raising_info_and_key_type(0);
            }
        } else {
            key_type = check_channel_raising_info_and_key_type(1);
        }
    } else if (event == (CTMU_P2M_CH0_LONG_KEY_EVENT + event_ch * 8)) {//长按只判断通道号小的那个按键
        key_type = LONG_CLICK;
    } else if (event == (CTMU_P2M_CH0_HOLD_KEY_EVENT + event_ch * 8)) {//长按只判断通道号小的那个按键
        key_type = LONG_HOLD_CLICK;
    }

    if (key_type) {
        falling_order[0] = FALLING_NULL;
        falling_order[1] = FALLING_NULL;
        raising_order[0] = RAISING_NULL;
        raising_order[1] = RAISING_NULL;
        falling_time_interval = 0;
        last_key_type = key_type;
        if (send_keytone_timeout_add) {
            sys_hi_timeout_del(send_keytone_timeout_add);
            send_keytone_timeout_add = 0;
        }
        if (key_type == SHORT_CLICK) {                  //case 6~7 的进一步处理
            if (first_short_click_timeout_add) {
                sys_hi_timeout_del(first_short_click_timeout_add);
                first_short_click_timeout_add = 0;

                if (slide_click_timeout_add) {
                    sys_hi_timeout_del(slide_click_timeout_add);
                    slide_click_timeout_add = 0;
                    if (fast_slide_type) {
                        key_type = fast_slide_type;
                        last_key_type = key_type;
                    } else {
                        key_type = 0xff;
                        last_key_type = key_type;
                    }
                    fast_slide_cnt = 0;
                } else {
                    if (fast_slide_type) {
                        fast_slide_cnt ++;
                        if (fast_slide_cnt >= FAST_SLIDE_CNT_MAX) {
                            fast_slide_cnt = 0;
                            key_type = fast_slide_type;
                            last_key_type = key_type;
                        } else {
                            key_type = 0xff;
                            last_key_type = key_type;
                        }
                    } else {
                        fast_slide_cnt = 0;
                        key_type = 0xff;
                        last_key_type = key_type;
                    }
                }
            } else {
                if (send_keytone_flag == 0) {
                    send_keytone_event();
                }
                if (slide_click_timeout_add) {
                    sys_hi_timeout_del(slide_click_timeout_add);
                    slide_click_timeout_add = 0;
                }
                fast_slide_cnt = 0;
            }
            if (click_idle_timeout_add == 0) {
                click_idle_timeout_add = sys_hi_timeout_add(NULL, click_idle_timeout_handle, CLICK_IDLE_TIMEOUT_TIME);
            } else {
                sys_hi_timer_modify(click_idle_timeout_add, CLICK_IDLE_TIMEOUT_TIME);
            }
        } else {
            fast_slide_cnt = 0;
        }

        if ((key_type == SLIDE_UP) || (key_type == SLIDE_DOWN)) {
            if (slide_click_timeout_add == 0) {
                slide_click_timeout_add = sys_hi_timeout_add(NULL, slide_click_timeout_handle, SLIDE_CLICK_TIMEOUT_TIME);
            } else {
                sys_hi_timer_modify(slide_click_timeout_add, SLIDE_CLICK_TIMEOUT_TIME);
            }
        }
    }

    return key_type;
}

static void ctmu_send_slide_key_type_event(u8 key_type)
{
    struct sys_event e;
    u8 event_ch = get_slide_event_ch();
    switch (key_type) {
    case SHORT_CLICK:           //单击
        ctmu_short_click_handle(event_ch);
        break;
    case LONG_CLICK:            //长按
#if CTMU_CHECK_LONG_CLICK_BY_RES
        if (lp_touch_key_check_long_click_by_ctmu_res(event_ch)) {
            last_key_type = 0;
            return;
        }
#endif
        ctmu_long_click_handle(event_ch);
        break;
    case LONG_HOLD_CLICK:       //长按保持
#if CTMU_CHECK_LONG_CLICK_BY_RES
        if (lp_touch_key_check_long_click_by_ctmu_res(event_ch)) {
            last_key_type = 0;
            return;
        }
#endif
        ctmu_hold_click_handle(event_ch);
        break;
    case LONG_UP_CLICK:         //长按抬起
        ctmu_raise_click_handle(event_ch);
        break;
    case SLIDE_UP:              //向上滑动
        e.u.key.event = TOUCH_KEY_EVENT_SLIDE_UP;
        e.u.key.value = __this->config->slide_mode_key_value;
        printf("key.event = TOUCH_KEY_EVENT_SLIDE_UP\n");
        printf("key.value = %d\n", e.u.key.value);
        __ctmu_notify_key_event(&e, event_ch);
        break;
    case SLIDE_DOWN:            //向下滑动
        e.u.key.event = TOUCH_KEY_EVENT_SLIDE_DOWN;
        e.u.key.value = __this->config->slide_mode_key_value;
        printf("key.event = TOUCH_KEY_EVENT_SLIDE_DOWN\n");
        printf("key.value = %d\n", e.u.key.value);
        __ctmu_notify_key_event(&e, event_ch);
        break;
    default:
        break;
    }
}

#endif


u8 last_state = CTMU_P2M_EARTCH_OUT_EVENT;
void p33_ctmu_key_event_irq_handler()
{
    u8 ret = 0;
    u8 ctmu_event = P2M_CTMU_KEY_EVENT;
    u8 ch_num = P2M_CTMU_KEY_CNT;
    u16 ch_res = 0;
    u16 chx_res[LP_CTMU_CHANNEL_SIZE];

#if TCFG_LP_EARTCH_KEY_ENABLE
    if (testbox_get_key_action_test_flag(NULL)) {
        if (__this->config->eartch_en && (__this->config->eartch_ch == ch_num)) {
            ret = lp_touch_key_testbox_remote_test(ch_num, ctmu_event);
            if (ret == true) {
                return;
            }
        }
    }
#endif
    /* printf("ctmu_event = 0x%x\n", ctmu_event); */
    switch (ctmu_event) {
    case CTMU_P2M_CH0_RES_EVENT:
    case CTMU_P2M_CH1_RES_EVENT:
    case CTMU_P2M_CH2_RES_EVENT:
    case CTMU_P2M_CH3_RES_EVENT:
    case CTMU_P2M_CH4_RES_EVENT:
        chx_res[0] = (P2M_CTMU_CH0_H_RES << 8) | P2M_CTMU_CH0_L_RES;
        chx_res[1] = (P2M_CTMU_CH1_H_RES << 8) | P2M_CTMU_CH1_L_RES;
        chx_res[2] = (P2M_CTMU_CH2_H_RES << 8) | P2M_CTMU_CH2_L_RES;
        chx_res[3] = (P2M_CTMU_CH3_H_RES << 8) | P2M_CTMU_CH3_L_RES;
        chx_res[4] = (P2M_CTMU_CH4_H_RES << 8) | P2M_CTMU_CH4_L_RES;
        ch_res = chx_res[ch_num];
        /* printf("ch%d_res: %d\n", ch_num, ch_res); */

#if TWS_BT_SEND_KEY_CH_RES_DATA_ENABLE
        if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
            lpctmu_tws_send_res_data(BT_KEY_CH_RES_MSG,
                                     chx_res[0],
                                     chx_res[1],
                                     chx_res[2],
                                     chx_res[3],
                                     chx_res[4]);
        }
#endif

#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
        lp_touch_key_online_debug_send(ch_num, ch_res);
#endif

#if TCFG_LP_EARTCH_KEY_ENABLE
        if (__this->config->eartch_en && (__this->config->eartch_ch == ch_num)) {

            u16 eartch_ch_res = chx_res[ch_num];
            u16 eartch_ref_ch_res = chx_res[__this->config->eartch_ref_ch];
            u16 eartch_iir = (P2M_CTMU_EARTCH_H_IIR_VALUE << 8) | P2M_CTMU_EARTCH_L_IIR_VALUE;
            u16 eartch_trim = (P2M_CTMU_EARTCH_H_TRIM_VALUE << 8) | P2M_CTMU_EARTCH_L_TRIM_VALUE;
            u16 eartch_diff = (P2M_CTMU_EARTCH_H_DIFF_VALUE << 8) | P2M_CTMU_EARTCH_L_DIFF_VALUE;

#if TWS_BT_SEND_EARTCH_RES_DATA_ENABLE
            if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
                lpctmu_tws_send_res_data(BT_EARTCH_RES_MSG,
                                         eartch_ch_res,
                                         eartch_ref_ch_res,
                                         eartch_iir,
                                         eartch_trim,
                                         eartch_diff);
            }
#endif

#if !TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
            if (__this->eartch_trim_flag) {
                if (__this->eartch_trim_value == 0) {
                    __this->eartch_trim_value = eartch_diff;
                } else {
                    __this->eartch_trim_value = ((eartch_diff + __this->eartch_trim_value) >> 1);
                }
                if (__this->eartch_trim_flag++ > 20) {
                    __this->eartch_trim_flag = 0;
                    M2P_CTMU_CH_DEBUG &= ~BIT(ch_num);
                    int ret = syscfg_write(LP_KEY_EARTCH_TRIM_VALUE, &(__this->eartch_trim_value), sizeof(__this->eartch_trim_value));
                    log_info("write ret = %d", ret);
                    if (ret > 0) {
                        M2P_CTMU_EARTCH_TRIM_VALUE_L = (__this->eartch_trim_value & 0xFF);
                        M2P_CTMU_EARTCH_TRIM_VALUE_H = ((__this->eartch_trim_value >> 8) & 0xFF);
                        __this->eartch_inear_ok = 1;
#if TCFG_EARTCH_EVENT_HANDLE_ENABLE
                        eartch_state_update(EARTCH_STATE_TRIM_OK);
#endif
                        log_info("trim: %d\n", __this->eartch_inear_ok);
                        is_lpkey_active = 0;
                    } else {
                        __this->eartch_inear_ok = 0;
#if TCFG_EARTCH_EVENT_HANDLE_ENABLE
                        eartch_state_update(EARTCH_STATE_TRIM_ERR);
#endif
                        log_info("trim: %d\n", __this->eartch_inear_ok);
                        is_lpkey_active = 0;
                    }
                }
                log_debug("eartch ch trim value: %d, res = %d", __this->eartch_trim_value, eartch_diff);
            }
#endif

            if (P2M_CTMU_EARTCH_EVENT != last_state) {
                last_state = P2M_CTMU_EARTCH_EVENT;
                if (last_state == CTMU_P2M_EARTCH_IN_EVENT) {
                    printf("soft inear\n");
#if TWS_BT_SEND_EVENT_ENABLE
                    lpctmu_tws_send_event_data(EAR_IN, BT_EVENT_SW_MSG);
#endif
                    if (__this->eartch_inear_ok) {
                        ctmu_eartch_event_handle(EAR_IN);
                    }

                } else if (last_state == CTMU_P2M_EARTCH_OUT_EVENT) {
                    printf("soft outear");
#if TWS_BT_SEND_EVENT_ENABLE
                    lpctmu_tws_send_event_data(EAR_OUT, BT_EVENT_SW_MSG);
#endif
                    if (__this->eartch_inear_ok) {
                        ctmu_eartch_event_handle(EAR_OUT);
                    }

                }
            }

        }
#endif
        break;

    case CTMU_P2M_CH0_SHORT_KEY_EVENT:
    case CTMU_P2M_CH1_SHORT_KEY_EVENT:
    case CTMU_P2M_CH2_SHORT_KEY_EVENT:
    case CTMU_P2M_CH3_SHORT_KEY_EVENT:
    case CTMU_P2M_CH4_SHORT_KEY_EVENT:
        log_debug("CH%d: SHORT click", ch_num);
#if (TCFG_LP_SLIDE_KEY_ENABLE && TCFG_LP_SLIDE_KEY_SHORT_DISTANCE)
#else
        ctmu_short_click_handle(ch_num);
#endif
        break;
    case CTMU_P2M_CH0_LONG_KEY_EVENT:
    case CTMU_P2M_CH1_LONG_KEY_EVENT:
    case CTMU_P2M_CH2_LONG_KEY_EVENT:
    case CTMU_P2M_CH3_LONG_KEY_EVENT:
    case CTMU_P2M_CH4_LONG_KEY_EVENT:
        log_debug("CH%d: LONG click", ch_num);
        is_lpkey_active = 0;

#if (TCFG_LP_SLIDE_KEY_ENABLE && TCFG_LP_SLIDE_KEY_SHORT_DISTANCE)
#else

#if CTMU_CHECK_LONG_CLICK_BY_RES
        if (lp_touch_key_check_long_click_by_ctmu_res(ch_num)) {
            return;
        }
#endif
        ctmu_long_click_handle(ch_num);
#endif
        break;
    case CTMU_P2M_CH0_HOLD_KEY_EVENT:
    case CTMU_P2M_CH1_HOLD_KEY_EVENT:
    case CTMU_P2M_CH2_HOLD_KEY_EVENT:
    case CTMU_P2M_CH3_HOLD_KEY_EVENT:
    case CTMU_P2M_CH4_HOLD_KEY_EVENT:
        log_debug("CH%d: HOLD click", ch_num);
        is_lpkey_active = 0;

#if (TCFG_LP_SLIDE_KEY_ENABLE && TCFG_LP_SLIDE_KEY_SHORT_DISTANCE)
#else

#if CTMU_CHECK_LONG_CLICK_BY_RES
        if (lp_touch_key_check_long_click_by_ctmu_res(ch_num)) {
            return;
        }
#endif
        ctmu_hold_click_handle(ch_num);
#endif
        break;
    case CTMU_P2M_CH0_FALLING_EVENT:
    case CTMU_P2M_CH1_FALLING_EVENT:
    case CTMU_P2M_CH2_FALLING_EVENT:
    case CTMU_P2M_CH3_FALLING_EVENT:
    case CTMU_P2M_CH4_FALLING_EVENT:
        log_debug("CH%d: FALLING", ch_num);
        is_lpkey_active = 1;

#if CTMU_CHECK_LONG_CLICK_BY_RES
        falling_res_avg[ch_num] = lp_touch_key_ctmu_res_buf_avg(ch_num);
        log_debug("falling_res_avg: %d", falling_res_avg[ch_num]);
#endif
        break;
    case CTMU_P2M_CH0_RAISING_EVENT:
    case CTMU_P2M_CH1_RAISING_EVENT:
    case CTMU_P2M_CH2_RAISING_EVENT:
    case CTMU_P2M_CH3_RAISING_EVENT:
    case CTMU_P2M_CH4_RAISING_EVENT:
        log_debug("CH%d: RAISING", ch_num);
        is_lpkey_active = 0;

#if CTMU_CHECK_LONG_CLICK_BY_RES
        lp_touch_key_ctmu_res_buf_clear(ch_num);
#endif

#if (TCFG_LP_SLIDE_KEY_ENABLE && TCFG_LP_SLIDE_KEY_SHORT_DISTANCE)
#else
        ctmu_raise_click_handle(ch_num);
#endif
        break;
    case CTMU_P2M_EARTCH_IN_EVENT:
        log_debug("CH%d: IN", ch_num);
        break;
    case CTMU_P2M_EARTCH_OUT_EVENT:
        log_debug("CH%d: OUT", ch_num);
        break;
    default:
        break;
    }

#if (TCFG_LP_SLIDE_KEY_ENABLE && TCFG_LP_SLIDE_KEY_SHORT_DISTANCE)
    u8 key_type = check_slide_key_type(ctmu_event, ch_num);
    if (key_type) {
        printf("CH%d: key_type = 0x%x\n", ch_num, key_type);
        ctmu_send_slide_key_type_event(key_type);
    }
#endif
}

u8 lp_touch_key_power_on_status()
{
    extern u8 power_reset_flag;
    u8 sfr = power_reset_flag;
    u8 ret = 0;
    log_debug("P3_RST_SRC = %x, P2M_CTMU_CTMU_WKUP_MSG = 0x%x", sfr, P2M_CTMU_WKUP_MSG);
    if (P2M_CTMU_WKUP_MSG & P2M_MESSAGE_POWER_ON_FLAG) {  //长按开机查询
        if (P2M_CTMU_WKUP_MSG & (P2M_MESSAGE_KEY_ACTIVE_FLAG)) { //按键释放查询, 0:释放, 1: 触摸保持
            ret = 1; //key active
        }
    }
    return ret;
}

void lp_touch_key_disable(void)
{
    log_debug("%s", __func__);
    P2M_CTMU_WKUP_MSG &= (~(P2M_MESSAGE_SYNC_FLAG));
    lp_touch_key_send_cmd(CTMU_M2P_DISABLE);
    while (!(P2M_CTMU_WKUP_MSG & P2M_MESSAGE_SYNC_FLAG));
    __this->softoff_mode = LP_TOUCH_SOFTOFF_MODE_LEGACY;
}

void lp_touch_key_enable(void)
{
    log_debug("%s", __func__);
    lp_touch_key_send_cmd(CTMU_M2P_ENABLE);
    __this->softoff_mode = LP_TOUCH_SOFTOFF_MODE_ADVANCE;
}

void lp_touch_key_charge_mode_enter()
{
    log_debug("%s", __func__);
#if (!LP_TOUCH_KEY_CHARGE_MODE_SW_DISABLE)
    P2M_CTMU_WKUP_MSG &= (~(P2M_MESSAGE_SYNC_FLAG));
    lp_touch_key_send_cmd(CTMU_M2P_CHARGE_ENTER_MODE);
    while (!(P2M_CTMU_WKUP_MSG & P2M_MESSAGE_SYNC_FLAG));
    __this->softoff_mode = LP_TOUCH_SOFTOFF_MODE_LEGACY;
#endif
}

void lp_touch_key_charge_mode_exit()
{
    log_debug("%s", __func__);

    P2M_CTMU_WKUP_MSG &= (~(P2M_MESSAGE_RESET_ALGO_FLAG));
    lp_touch_key_send_cmd(CTMU_M2P_RESET_ALGO);
    while (!(P2M_CTMU_WKUP_MSG & P2M_MESSAGE_RESET_ALGO_FLAG));

#if (!LP_TOUCH_KEY_CHARGE_MODE_SW_DISABLE)
    lp_touch_key_send_cmd(CTMU_M2P_CHARGE_EXIT_MODE);
    __this->softoff_mode = LP_TOUCH_SOFTOFF_MODE_ADVANCE;
#endif
}

//=============================================//
//NOTE: 该函数为进关机时被库里面回调
//在板级配置struct low_power_param power_param中变量lpctmu_en配置为TCFG_LP_TOUCH_KEY_ENABLE时:
//该函数在决定softoff进触摸模式还是普通模式:
//	return 1: 进触摸模式关机(LP_TOUCH_SOFTOFF_MODE_ADVANCE);
//	return 0: 进普通模式关机(触摸关闭)(LP_TOUCH_SOFTOFF_MODE_LEGACY);
//使用场景:
// 	1)在充电舱外关机, 需要触摸开机, 进带触摸关机模式;
// 	2)在充电舱内关机，可以关闭触摸模块, 进普通关机模式, 关机功耗进一步降低.
//=============================================//
u8 lp_touch_key_softoff_mode_query(void)
{
    return __this->softoff_mode;
}

void set_lpkey_active(u8 set)
{
    is_lpkey_active = set;
}

static u8 lpkey_idle_query(void)
{
    return !is_lpkey_active;
}

REGISTER_LP_TARGET(key_lp_target) = {
    .name = "lpkey",
    .is_idle = lpkey_idle_query,
};


#endif

