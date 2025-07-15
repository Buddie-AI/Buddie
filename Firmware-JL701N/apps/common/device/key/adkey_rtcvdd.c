#include "includes.h"
#include "key_driver.h"
#include "adkey.h"
#include "gpio.h"
#include "system/event.h"
#include "app_config.h"
#include "asm/power/p33.h"

#if TCFG_ADKEY_RTCVDD_ENABLE

#define ADKEY_RTCVDD_DEBUG 			1
#if ADKEY_RTCVDD_DEBUG
#define adkey_rtcvdd_debug(fmt, ...) printf("[ADKEY_RTCVDD] "fmt, ##__VA_ARGS__)
#else
#define adkey_rtcvdd_debug(fmt, ...)
#endif

#define ADC_KEY_NUMBER 		10

#define FULL_ADC   			0x3ffL

#define ADC_FULL(x)          (x)
#define ADC_VOLTAGE(x,y,z)   ((x*y) / (y + z))              //x当前满幅电压，y分压电阻，z上拉电阻
#define ADC_ZERRO(x)         (0)

u16 ad_rtcvdd_key_table[ADC_KEY_NUMBER + 1] = {0};

#define FULL_AD_VOLTAGE 	0x3FFF

volatile u8 adkey_lock_cnt = 0;
static u8 rtcvdd_cnt = 10;
static u8 rtcvdd_full_cnt = 0xff;
u16 rtcvdd_full_value = FULL_AD_VOLTAGE;
u16 max_value = 0;
u16 min_value = 0xffff;
u32 total_value = 0;
static u8 check_rtcvdd_cnt = 0;

static const struct adkey_rtcvdd_platform_data *__this = NULL;


u8 adkey_rtcvdd_get_key_value(void);
//按键驱动扫描参数列表
struct key_driver_para adkey_rtcvdd_scan_para = {
    .scan_time 	  	  = 10,				//按键扫描频率, 单位: ms
    .last_key 		  = NO_KEY,  		//上一次get_value按键值, 初始化为NO_KEY;
    .filter_time  	  = 2,				//按键消抖延时;
    .long_time 		  = 75,  			//按键判定长按数量
    .hold_time 		  = (75 + 15),  	//按键判定HOLD数量
    .click_delay_time = 20,				//按键被抬起后等待连击延时数量
    .key_type		  = KEY_DRIVER_TYPE_RTCVDD_AD,
    .get_value 		  = adkey_rtcvdd_get_key_value,
};

static void set_rtcvdd_table(u16 adc_rtcvdd)
{
    u8 i;
    u32 extern_up_res_value = __this->extern_up_res_value;

    if (extern_up_res_value == 0) {  //使用内部上拉
        extern_up_res_value = 100;
    }

    for (i = 0; i < __this->adkey_num; i++) {
        if (i == (__this->adkey_num - 1)) {
            ad_rtcvdd_key_table[i]  = (ADC_VOLTAGE(adc_rtcvdd, __this->res_value[i], extern_up_res_value) + ADC_FULL(adc_rtcvdd)) / 2;
            //adkey_rtcvdd_debug("recvdd = %d, res_value[%d] = %d", adc_rtcvdd, i, __this->res_value[i]);
        } else {
            ad_rtcvdd_key_table[i]  = (ADC_VOLTAGE(adc_rtcvdd, __this->res_value[i], extern_up_res_value) + ADC_VOLTAGE(adc_rtcvdd, __this->res_value[i + 1], extern_up_res_value)) / 2;
            //adkey_rtcvdd_debug("res_value[%d] = %d, res_value[%d] = %d", i, __this->res_value[i], i + 1, __this->res_value[i+1]);
        }
    }
}

static void SET_ADKEY_LOCK_CNT(u8 cnt)
{
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();

    adkey_lock_cnt = cnt;

    OS_EXIT_CRITICAL();
}

static u8 GET_ADKEY_LOCK_CNT(void)
{
    u8 val;
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();

    val = adkey_lock_cnt;

    OS_EXIT_CRITICAL();
    return val;
}

static void POST_ADKEY_LOCK_CNT(void)
{
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();

    adkey_lock_cnt --;

    OS_EXIT_CRITICAL();
}

/*----------------------------------------------------------------------------*/
/**@brief   ad按键初始化
   @param   void
   @param   void
   @return  void
   @note    void ad_key0_init(void)
*/
/*----------------------------------------------------------------------------*/
int adkey_rtcvdd_init(const struct adkey_rtcvdd_platform_data *adkey_data)
{
    adkey_rtcvdd_debug("ad key init\n");

    __this = adkey_data;
    if (!__this) {
        return -EINVAL;
    }

    if (__this->extern_up_res_value == 0) {    //使用内部上拉
        gpio_set_pull_up(__this->adkey_pin, 1);
    } else {
        gpio_set_pull_up(__this->adkey_pin, 0);
    }
    gpio_set_direction(__this->adkey_pin, 1);
    gpio_set_pull_down(__this->adkey_pin, 0);
    gpio_set_die(__this->adkey_pin, 0);

    adc_add_sample_ch(__this->ad_channel);          //注意：初始化AD_KEY之前，先初始化ADC
    adc_add_sample_ch(AD_CH_RTCVDD);

    set_rtcvdd_table(FULL_ADC);

    return 0;
}


/*把cnt个值里的最大值和最小值去掉，求剩余cnt-2个数的平均值*/
static u16 rtcvdd_full_vaule_update(u16 value)
{
    u16 full_value = FULL_ADC;
    if (rtcvdd_full_cnt == 0xff) {
        rtcvdd_full_cnt = 0;
        SET_ADKEY_LOCK_CNT(50);
        return value;   //first time
    } else {
        rtcvdd_full_cnt ++;
        if (value > max_value) {
            max_value = value;
        }
        if (value < min_value) {
            min_value = value;
        }
        total_value += value;

        if (rtcvdd_full_cnt > 10 - 1) { //算10个数
            full_value = (total_value - max_value - min_value) / (rtcvdd_full_cnt - 2);
            rtcvdd_full_cnt = 0;
            max_value = 0;
            min_value = 0xffff;
            total_value = 0;
        } else {
            return rtcvdd_full_value;
        }
    }
    return full_value;
}

u8 get_rtcvdd_level(void)
{
    u8 level = GET_RTCVDD_VOL();
    return level;
}

void set_rtcvdd_level(u8 level)
{
    if (level > 7 || level < 0) {
        return;
    }
    RTCVDD_VOL_SEL(level);
}
/*检测到RTCVDD 比 VDDIO 高的时候自动把RTCVDD降一档*/
static u8 rtcvdd_auto_match_vddio_lev(u32 rtcvdd_value)
{
    u8 rtcvdd_lev = 0;
    if (rtcvdd_value >= FULL_ADC) { //trim rtcvdd < vddio
        if (rtcvdd_cnt > 10) {
            rtcvdd_cnt = 0;
            rtcvdd_lev = get_rtcvdd_level();
            if (rtcvdd_lev < 8) {
                rtcvdd_lev++;  //降一档
                /* rtcvdd_lev--;  //降一档 */
                set_rtcvdd_level(rtcvdd_lev);
                SET_ADKEY_LOCK_CNT(50);
                return 1;
            }
        } else {
            rtcvdd_cnt ++;
        }
    } else {
        rtcvdd_cnt = 0;
        rtcvdd_full_value = rtcvdd_full_vaule_update(rtcvdd_value);
    }
    return 0;
}



/*----------------------------------------------------------------------------*/
/**@brief   获取ad按键值
   @param   void
   @param   void
   @return  key_number
   @note    tu8 adkey_rtcvdd_get_key_value(void)
*/
/*----------------------------------------------------------------------------*/
u8 adkey_rtcvdd_get_key_value(void)
{
    u8 key_number, i;
    u32 ad_value;
    u16 rtcvdd_value = 0;

    rtcvdd_value = 2 * adc_get_value(AD_CH_RTCVDD);
    ad_value = adc_get_value(__this->ad_channel);

    /* printf("rtcvdd_value = %d, ad_value = %d", rtcvdd_value, ad_value); */
    if (rtcvdd_auto_match_vddio_lev(rtcvdd_value)) {
        return NO_KEY;
    }

    if (GET_ADKEY_LOCK_CNT()) {
        POST_ADKEY_LOCK_CNT();
        return NO_KEY;
    }

    set_rtcvdd_table(rtcvdd_full_value);

    for (i = 0; i < __this->adkey_num; i++) {
        if (ad_value <= ad_rtcvdd_key_table[i] && (ad_rtcvdd_key_table[i] < 0x3FFL)) {
            return __this->key_value[i];
        }
    }

    return NO_KEY;
}


#endif /* #if TCFG_ADKEY_RTCVDD_ENABLE */

