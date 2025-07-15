#include "app_config.h"

#ifdef CONFIG_BOARD_JL701N_DEMO

#include "system/includes.h"
#include "media/includes.h"
#include "asm/sdmmc.h"
#include "asm/chargestore.h"
#include "asm/umidigi_chargestore.h"
#include "asm/charge.h"
#include "asm/pwm_led.h"
#include "tone_player.h"
#include "audio_config.h"
#include "gSensor/gSensor_manage.h"
#include "imuSensor_manage.h"
#include "key_event_deal.h"
#include "asm/lp_touch_key_api.h"
#include "user_cfg.h"
#include "audio_link.h"
#if TCFG_DEV_MANAGER_ENABLE
#include "dev_manager.h"
#endif
#if TCFG_PC_ENABLE
#include "usb/otg.h"
#endif
#include "norflash_sfc.h"
#include "asm/power/power_port.h"
#include "app_umidigi_chargestore.h"
#if TCFG_AUDIO_ANC_ENABLE
#include "audio_anc.h"
#endif/*TCFG_AUDIO_ANC_ENABLE*/
#if TCFG_MPU6887P_ENABLE
#include "imu_sensor/mpu6887/mpu6887p.h"
#endif /*TCFG_MPU6887P_ENABLE*/
#if TCFG_QMI8658_ENABLE
#include "imu_sensor/qmi8658/qmi8658c.h"
#endif /*TCFG_QMI8658_ENABLE*/
#if TCFG_LSM6DSL_ENABLE
#include "imu_sensor/lsm6dsl/lsm6dsl.h"
#endif /*TCFG_LSM6DSL_ENABLE*/
#if TCFG_ICM42670P_ENABLE
#include "imu_sensor/icm_42670p/icm_42670p.h"
#endif /*TCFG_ICM42670P_ENABLE*/

#define LOG_TAG_CONST       BOARD
#define LOG_TAG             "[BOARD]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

void board_power_init(void);

/*各个状态下默认的闪灯方式和提示音设置，如果USER_CFG中设置了USE_CONFIG_STATUS_SETTING为1，则会从配置文件读取对应的配置来填充改结构体*/
STATUS_CONFIG status_config = {
    //灯状态设置
    .led = {
        .charge_start  = PWM_LED1_ON,
        .charge_full   = PWM_LED0_ON,
        .power_on      = PWM_LED0_ON,
        .power_off     = PWM_LED1_FLASH_THREE,
        .lowpower      = PWM_LED1_SLOW_FLASH,
        .max_vol       = PWM_LED_NULL,
        .phone_in      = PWM_LED_NULL,
        .phone_out     = PWM_LED_NULL,
        .phone_activ   = PWM_LED_NULL,
        .bt_init_ok    = PWM_LED0_LED1_SLOW_FLASH,
        .bt_connect_ok = PWM_LED0_ONE_FLASH_5S,
        .bt_disconnect = PWM_LED0_LED1_FAST_FLASH,
        .tws_connect_ok = PWM_LED0_LED1_FAST_FLASH,
        .tws_disconnect = PWM_LED0_LED1_SLOW_FLASH,
    },
    //提示音设置
    .tone = {
        .charge_start  = IDEX_TONE_NONE,
        .charge_full   = IDEX_TONE_NONE,
        .power_on      = IDEX_TONE_POWER_ON,
        .power_off     = IDEX_TONE_POWER_OFF,
        .lowpower      = IDEX_TONE_LOW_POWER,
        .max_vol       = IDEX_TONE_MAX_VOL,
        .phone_in      = IDEX_TONE_NONE,
        .phone_out     = IDEX_TONE_NONE,
        .phone_activ   = IDEX_TONE_NONE,
        .bt_init_ok    = IDEX_TONE_BT_MODE,
        .bt_connect_ok = IDEX_TONE_BT_CONN,
        .bt_disconnect = IDEX_TONE_BT_DISCONN,
        .tws_connect_ok   = IDEX_TONE_TWS_CONN,
        .tws_disconnect   = IDEX_TONE_TWS_DISCONN,
    }
};

#define __this (&status_config)

/************************** KEY MSG****************************/
/*各个按键的消息设置，如果USER_CFG中设置了USE_CONFIG_KEY_SETTING为1，则会从配置文件读取对应的配置来填充改结构体*/
#if (TCFG_APP_AUX_EN || TCFG_APP_MUSIC_EN || TCFG_APP_PC_EN)
u8 key_table[KEY_NUM_MAX][KEY_EVENT_MAX] = {
    // SHORT           LONG              HOLD              UP              DOUBLE           TRIPLE
    {KEY_MUSIC_PP,   KEY_POWEROFF,  KEY_POWEROFF_HOLD,  KEY_NULL,     KEY_CALL_LAST_NO,     KEY_MODE_SWITCH},  //KEY_0
    {KEY_MUSIC_NEXT, KEY_VOL_UP,    KEY_VOL_UP,         KEY_NULL,     KEY_OPEN_SIRI,        KEY_NULL},   //KEY_1
    {KEY_MUSIC_PREV, KEY_VOL_DOWN,  KEY_VOL_DOWN,       KEY_NULL,     KEY_HID_CONTROL,      KEY_NULL},   //KEY_2
};
#else
u8 key_table[KEY_NUM_MAX][KEY_EVENT_MAX] = {
    // SHORT           LONG              HOLD              UP              DOUBLE           TRIPLE
    {KEY_MUSIC_PP,   KEY_POWEROFF,  KEY_POWEROFF_HOLD,  KEY_NULL,     KEY_CALL_LAST_NO,     KEY_NULL},   //KEY_0
    {KEY_MUSIC_NEXT, KEY_VOL_UP,    KEY_VOL_UP,         KEY_NULL,     KEY_OPEN_SIRI,        KEY_NULL},   //KEY_1
    {KEY_MUSIC_PREV, KEY_VOL_DOWN,  KEY_VOL_DOWN,       KEY_NULL,     KEY_HID_CONTROL,      KEY_NULL},   //KEY_2
};
#endif


// *INDENT-OFF*
/************************** UART config****************************/
#if TCFG_UART0_ENABLE
UART0_PLATFORM_DATA_BEGIN(uart0_data)
    .tx_pin = TCFG_UART0_TX_PORT,                             //串口打印TX引脚选择
    .rx_pin = TCFG_UART0_RX_PORT,                             //串口打印RX引脚选择
    .baudrate = TCFG_UART0_BAUDRATE,                          //串口波特率

    .flags = UART_DEBUG,                                      //串口用来打印需要把改参数设置为UART_DEBUG
UART0_PLATFORM_DATA_END()
#endif //TCFG_UART0_ENABLE

/************************** CHARGE config****************************/
#if TCFG_CHARGE_ENABLE
CHARGE_PLATFORM_DATA_BEGIN(charge_data)
    .charge_en              = TCFG_CHARGE_ENABLE,              //内置充电使能
    .charge_poweron_en      = TCFG_CHARGE_POWERON_ENABLE,      //是否支持充电开机
    .charge_full_V          = TCFG_CHARGE_FULL_V,              //充电截止电压
    .charge_full_mA			= TCFG_CHARGE_FULL_MA,             //充电截止电流
    .charge_mA				= TCFG_CHARGE_MA,                  //恒流充电电流
    .charge_trickle_mA		= TCFG_CHARGE_TRICKLE_MA,          //涓流充电电流
/*ldo5v拔出过滤值，过滤时间 = (filter*2 + 20)ms,ldoin<0.6V且时间大于过滤时间才认为拔出
 对于充满直接从5V掉到0V的充电仓，该值必须设置成0，对于充满由5V先掉到0V之后再升压到xV的
 充电仓，需要根据实际情况设置该值大小*/
	.ldo5v_off_filter		= 100,
    .ldo5v_on_filter        = 50,
    .ldo5v_keep_filter      = 220,
    .ldo5v_pulldown_lvl     = CHARGE_PULLDOWN_200K,            //下拉电阻档位选择
    .ldo5v_pulldown_keep    = 0,
//1、对于自动升压充电舱,若充电舱需要更大的负载才能检测到插入时，请将该变量置1,并且根据需求配置下拉电阻档位
//2、对于按键升压,并且是通过上拉电阻去提供维持电压的舱,请将该变量设置1,并且根据舱的上拉配置下拉需要的电阻挡位
//3、对于常5V的舱,可将改变量设为0,省功耗
//4、为LDOIN防止被误触发唤醒,可设置为200k下拉
	.ldo5v_pulldown_en		= 1,
CHARGE_PLATFORM_DATA_END()
#endif//TCFG_CHARGE_ENABLE

/************************** chargestore config****************************/
#if TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE || TCFG_ANC_BOX_ENABLE
CHARGESTORE_PLATFORM_DATA_BEGIN(chargestore_data)
    .io_port                = TCFG_CHARGESTORE_PORT,
CHARGESTORE_PLATFORM_DATA_END()
#endif

/************************** DAC ****************************/
#if TCFG_AUDIO_DAC_ENABLE
struct dac_platform_data dac_data = {
	.mode          = TCFG_AUDIO_DAC_MODE,                       //dac输出模式
    .ldo_id        = TCFG_AUDIO_DAC_LDO_SEL,                    //保留位
    .pa_mute_port  = TCFG_AUDIO_DAC_PA_PORT,                    //暂时无作用
    .vcmo_en       = 0,                                         //是否打开VCOMO
    .pa_mute_value = 1,                                         //暂时无作用
    .output        = TCFG_AUDIO_DAC_CONNECT_MODE,               //DAC输出配置，和具体硬件连接有关，需根据硬件来设置
    .lpf_isel = 0xf,
    .sys_vol_type  = SYS_VOL_TYPE,                              //系统音量选择：模拟音量/数字音量，调节时调节对应的音量
    .max_ana_vol   = MAX_ANA_VOL,                               //模拟音量最大等级
    .max_dig_vol   = MAX_DIG_VOL,                               //数字音量最大等级
    /* .dig_vol_tab   = (s16 *)dig_vol_table,                      //数字音量表 */
	.vcm_cap_en = 1,                                         //配1代表走外部通路,vcm上有电容时,可以提升电路抑制电源噪声能力，提高ADC的性能，配0相当于vcm上无电容，抑制电源噪声能力下降,ADC性能下降
#if (SYS_VOL_TYPE == VOL_TYPE_AD)
    .digital_gain_limit = 16384,
#endif // #if (SYS_VOL_TYPE == VOL_TYPE_AD)

    .power_on_mode = 0,
};
#endif

/************************** ADC ****************************/
#if TCFG_AUDIO_ADC_ENABLE
#ifndef TCFG_AUDIO_MIC0_BIAS_EN
#define TCFG_AUDIO_MIC0_BIAS_EN				0
#endif/*TCFG_AUDIO_MIC0_BIAS_EN*/
#ifndef TCFG_AUDIO_MIC1_BIAS_EN
#define TCFG_AUDIO_MIC1_BIAS_EN				0
#endif/*TCFG_AUDIO_MIC1_BIAS_EN*/
#ifndef TCFG_AUDIO_MIC2_BIAS_EN
#define TCFG_AUDIO_MIC2_BIAS_EN				0
#endif/*TCFG_AUDIO_MIC2_BIAS_EN*/
#ifndef TCFG_AUDIO_MIC3_BIAS_EN
#define TCFG_AUDIO_MIC3_BIAS_EN				0
#endif/*TCFG_AUDIO_MIC3_BIAS_EN*/
#ifndef TCFG_AUDIO_MIC_LDO_EN
#define TCFG_AUDIO_MIC_LDO_EN				0
#endif/*TCFG_AUDIO_MIC_LDO_EN*/

struct adc_platform_data adc_data = {
/*MIC LDO电流档位设置：
    0:0.625ua    1:1.25ua    2:1.875ua    3:2.5ua*/
	.mic_ldo_isel   = TCFG_AUDIO_ADC_LD0_SEL,

/*mic_mode 工作模式定义
	#define AUDIO_MIC_CAP_MODE          0   //单端隔直电容模式
	#define AUDIO_MIC_CAP_DIFF_MODE     1   //差分隔直电容模式
	#define AUDIO_MIC_CAPLESS_MODE      2   //单端省电容模式
*/
	.mic_mode = TCFG_AUDIO_MIC_MODE,
	.mic1_mode = TCFG_AUDIO_MIC1_MODE,
	.mic2_mode = TCFG_AUDIO_MIC2_MODE,
	.mic3_mode = TCFG_AUDIO_MIC3_MODE,

	.mic_bias_inside = TCFG_AUDIO_MIC0_BIAS_EN,
	.mic1_bias_inside = TCFG_AUDIO_MIC1_BIAS_EN,
	.mic2_bias_inside = TCFG_AUDIO_MIC2_BIAS_EN,
	.mic3_bias_inside = TCFG_AUDIO_MIC3_BIAS_EN,

/*MICLDO供电输出到PAD(PA0)控制使能*/
	.mic_ldo_pwr = TCFG_AUDIO_MIC_LDO_EN,	// MIC LDO 输出到 PA0

/*MIC免电容方案需要设置，影响MIC的偏置电压
	0b0001~0b1001 : 0.5k ~ 4.5k step = 0.5k
	0b1010~0b1111 : 5k ~ 10k step = 1k */
    .mic_bias_res    = 4,
    .mic1_bias_res   = 4,
    .mic2_bias_res   = 4,
    .mic3_bias_res   = 4,
/*MIC LDO电压档位设置,也会影响MIC的偏置电压
    3:2.0v  4:2.2v  5:2.4v  6:2.6v  7:2.8v */
	.mic_ldo_vsel  = 5,
 //mic的去直流dcc寄存器配置值,可配0到15,数值越大,其高通转折点越低
    .mic_dcc       = 8,
	/*ADC低功耗等级,越大功耗越低，对应影响THD和底噪, 范围 (0 - 2)*/
	.lowpower_lvl = 0,
    .adc_max_ch_en = 0,
    .max_channel_num = 3,
};
#endif

const struct vad_mic_platform_data vad_mic_data = {
    .mic_data = { //
        .mic_mode = TCFG_AUDIO_MIC_MODE,
        .mic_ldo_isel = 2,
        .mic_ldo_vsel = 5,
        .mic_ldo2PAD_en = 1,
        .mic_bias_en = 0,
        .mic_bias_res = 0,
        .mic_bias_inside = TCFG_AUDIO_MIC0_BIAS_EN,
        /* ADC偏置电阻配置*/
        .adc_rbs = 3,
        /* ADC输入电阻配置*/
        .adc_rin = 3,
        /*.adc_test = 1,*/
    },
    .power_data = {
        /*VADLDO电压档位：0~7*/
        .ldo_vs = 3,
        /*VADLDO误差运放电流档位：0~3*/
#if TCFG_VAD_LOWPOWER_CLOCK == VAD_CLOCK_USE_PMU_STD12M
        .ldo_is = 1,
#else
        .ldo_is = 2,
#endif
        .clock = TCFG_VAD_LOWPOWER_CLOCK, /*VAD时钟选项*/
        .acm_select = 8,
    },
};
/* struct audio_pf_data audio_pf_d = { */
/* #if TCFG_AUDIO_DAC_ENABLE */
    /* .adc_pf_data = &adc_data, */
/* #endif */

/* #if TCFG_AUDIO_ADC_ENABLE */
    /* .dac_pf_data = &dac_data, */
/* #endif */
/* }; */

/* struct audio_platform_data audio_data = { */
    /* .private_data = (void *) &audio_pf_d, */
/* }; */


/************************** IO KEY ****************************/
#if TCFG_IOKEY_ENABLE
const struct iokey_port iokey_list[] = {
    {
        .connect_way = TCFG_IOKEY_POWER_CONNECT_WAY,          //IO按键的连接方式
        .key_type.one_io.port = TCFG_IOKEY_POWER_ONE_PORT,    //IO按键对应的引脚
        .key_value = 0,                                       //按键值
    },
};
const struct iokey_platform_data iokey_data = {
    .enable = TCFG_IOKEY_ENABLE,                              //是否使能IO按键
    .num = ARRAY_SIZE(iokey_list),                            //IO按键的个数
    .port = iokey_list,                                       //IO按键参数表
};

#if MULT_KEY_ENABLE
//组合按键消息映射表
//配置注意事项:单个按键按键值需要按照顺序编号,如power:0, prev:1, next:2
//bit_value = BIT(0) | BIT(1) 指按键值为0和按键值为1的两个按键被同时按下,
//remap_value = 3指当这两个按键被同时按下后重新映射的按键值;
const struct key_remap iokey_remap_table[] = {
	{.bit_value = BIT(0) | BIT(1), .remap_value = 3},
	{.bit_value = BIT(0) | BIT(2), .remap_value = 4},
	{.bit_value = BIT(1) | BIT(2), .remap_value = 5},
};

const struct key_remap_data iokey_remap_data = {
	.remap_num = ARRAY_SIZE(iokey_remap_table),
	.table = iokey_remap_table,
};
#endif

#endif
/*********************** LP TOUCH KEY ****************************/
#if TCFG_LP_TOUCH_KEY_ENABLE
const struct lp_touch_key_platform_data lp_touch_key_config = {
	/*触摸按键*/
	.ch[0].enable = TCFG_LP_TOUCH_KEY0_EN,
	.ch[0].wakeup_enable = TCFG_LP_TOUCH_KEY0_WAKEUP_EN,
	.ch[0].port = IO_PORTB_00,
	.ch[0].sensitivity = TCFG_LP_TOUCH_KEY0_SENSITIVITY,
	.ch[0].key_value = 0,

	.ch[1].enable = TCFG_LP_TOUCH_KEY1_EN,
	.ch[1].wakeup_enable = TCFG_LP_TOUCH_KEY1_WAKEUP_EN,
	.ch[1].port = IO_PORTB_01,
	.ch[1].sensitivity = TCFG_LP_TOUCH_KEY1_SENSITIVITY,
	.ch[1].key_value = 0,

	.ch[2].enable = TCFG_LP_TOUCH_KEY2_EN,
	.ch[2].wakeup_enable = TCFG_LP_TOUCH_KEY2_WAKEUP_EN,
	.ch[2].port = IO_PORTB_02,
	.ch[2].sensitivity = TCFG_LP_TOUCH_KEY2_SENSITIVITY,
	.ch[2].key_value = 1,

	.ch[3].enable = TCFG_LP_TOUCH_KEY3_EN,
	.ch[3].wakeup_enable = TCFG_LP_TOUCH_KEY3_WAKEUP_EN,
	.ch[3].port = IO_PORTB_04,
	.ch[3].sensitivity = TCFG_LP_TOUCH_KEY3_SENSITIVITY,
	.ch[3].key_value = 2,

	.ch[4].enable = TCFG_LP_TOUCH_KEY4_EN,
	.ch[4].wakeup_enable = TCFG_LP_TOUCH_KEY4_WAKEUP_EN,
	.ch[4].port = IO_PORTB_05,
	.ch[4].sensitivity = TCFG_LP_TOUCH_KEY4_SENSITIVITY,
	.ch[4].key_value = 3,

    //把触摸按键之间的滑动也当做按键处理，有上滑，下滑，左滑，右滑
    .slide_mode_en = TCFG_LP_SLIDE_KEY_ENABLE,
    .slide_mode_key_value = 3,

    //入耳检测相关的配置
    .eartch_en = TCFG_LP_EARTCH_KEY_ENABLE,
    .eartch_ch = TCFG_LP_EARTCH_KEY_CH,
    .eartch_ref_ch = TCFG_LP_EARTCH_KEY_REF_CH,
    .eartch_soft_inear_val = TCFG_LP_EARTCH_SOFT_INEAR_VAL,
    .eartch_soft_outear_val = TCFG_LP_EARTCH_SOFT_OUTEAR_VAL,
};
#endif /* #if TCFG_LP_TOUCH_KEY_ENABLE */

/************************** PLCNT TOUCH_KEY ****************************/
#if TCFG_TOUCH_KEY_ENABLE
const const struct touch_key_port touch_key_list[] = {
    {
	    .press_delta    = TCFG_TOUCH_KEY0_PRESS_DELTA,
        .port           = TCFG_TOUCH_KEY0_PORT,
        .key_value      = TCFG_TOUCH_KEY0_VALUE,
    },
    {
	    .press_delta    = TCFG_TOUCH_KEY1_PRESS_DELTA,
	    .port           = TCFG_TOUCH_KEY1_PORT,
        .key_value      = TCFG_TOUCH_KEY1_VALUE,
    },
};

const struct touch_key_platform_data touch_key_data = {
    .num = ARRAY_SIZE(touch_key_list),
    .port_list = touch_key_list,
};
#endif  /* #if TCFG_TOUCH_KEY_ENABLE */

/************************** AD KEY ****************************/
#if TCFG_ADKEY_ENABLE
const struct adkey_platform_data adkey_data = {
    .enable = TCFG_ADKEY_ENABLE,                              //AD按键使能
    .adkey_pin = TCFG_ADKEY_PORT,                             //AD按键对应引脚
    .ad_channel = TCFG_ADKEY_AD_CHANNEL,                      //AD通道值
    .extern_up_en = TCFG_ADKEY_EXTERN_UP_ENABLE,              //是否使用外接上拉电阻
    .ad_value = {                                             //根据电阻算出来的电压值
        TCFG_ADKEY_VOLTAGE0,
        TCFG_ADKEY_VOLTAGE1,
        TCFG_ADKEY_VOLTAGE2,
        TCFG_ADKEY_VOLTAGE3,
        TCFG_ADKEY_VOLTAGE4,
        TCFG_ADKEY_VOLTAGE5,
        TCFG_ADKEY_VOLTAGE6,
        TCFG_ADKEY_VOLTAGE7,
        TCFG_ADKEY_VOLTAGE8,
        TCFG_ADKEY_VOLTAGE9,
    },
    .key_value = {                                             //AD按键各个按键的键值
        TCFG_ADKEY_VALUE0,
        TCFG_ADKEY_VALUE1,
        TCFG_ADKEY_VALUE2,
        TCFG_ADKEY_VALUE3,
        TCFG_ADKEY_VALUE4,
        TCFG_ADKEY_VALUE5,
        TCFG_ADKEY_VALUE6,
        TCFG_ADKEY_VALUE7,
        TCFG_ADKEY_VALUE8,
        TCFG_ADKEY_VALUE9,
    },
};
#endif

/************************** RDEC_KEY ****************************/
#if TCFG_RDEC_KEY_ENABLE
const struct rdec_device rdeckey_list[] = {
    {
        .index = RDEC0 ,
        .sin_port0 = TCFG_RDEC0_ECODE1_PORT,
        .sin_port1 = TCFG_RDEC0_ECODE2_PORT,
        .key_value0 = TCFG_RDEC0_KEY0_VALUE | BIT(7),
        .key_value1 = TCFG_RDEC0_KEY1_VALUE | BIT(7),
    },

    {
        .index = RDEC1 ,
        .sin_port0 = TCFG_RDEC1_ECODE1_PORT,
        .sin_port1 = TCFG_RDEC1_ECODE2_PORT,
        .key_value0 = TCFG_RDEC1_KEY0_VALUE | BIT(7),
        .key_value1 = TCFG_RDEC1_KEY1_VALUE | BIT(7),
    },

    {
        .index = RDEC2 ,
        .sin_port0 = TCFG_RDEC2_ECODE1_PORT,
        .sin_port1 = TCFG_RDEC2_ECODE2_PORT,
        .key_value0 = TCFG_RDEC2_KEY0_VALUE | BIT(7),
        .key_value1 = TCFG_RDEC2_KEY1_VALUE | BIT(7),
    },


};
const struct rdec_platform_data  rdec_key_data = {
    .enable = 1, //TCFG_RDEC_KEY_ENABLE,                              //是否使能RDEC按键
    .num = ARRAY_SIZE(rdeckey_list),                            //RDEC按键的个数
    .rdec = rdeckey_list,                                       //RDEC按键参数表
};
#endif

/************************** IIS config ****************************/
#if (TCFG_AUDIO_INPUT_IIS || TCFG_AUDIO_OUTPUT_IIS)
ALINK_PARM alink0_platform_data = {
    .module = ALINK0,
    .mclk_io = TCFG_IIS_MCLK_IO,
    .sclk_io = TCFG_SCLK_IO,
    .lrclk_io = TCFG_LRCLK_IO,
    .ch_cfg[0].data_io = TCFG_DATA0_IO,
    .ch_cfg[1].data_io = TCFG_DATA1_IO,
    .ch_cfg[2].data_io = TCFG_DATA2_IO,
    .ch_cfg[3].data_io = TCFG_DATA3_IO,
    .mode = ALINK_MD_IIS,
#if TCFG_IIS_MODE
    .role = ALINK_ROLE_SLAVE,
#else
    .role = ALINK_ROLE_MASTER,
#endif /*TCFG_IIS_MODE*/
    .clk_mode = ALINK_CLK_FALL_UPDATE_RAISE_SAMPLE,
    .bitwide = ALINK_LEN_16BIT,
    .sclk_per_frame = ALINK_FRAME_32SCLK,
	.dma_len = 4 * 1024,
    .sample_rate = TCFG_IIS_SR,
    .buf_mode = ALINK_BUF_CIRCLE,
    /*.iperiod = 64, //配置该项可以控制输入的延时*/
};
#endif

/************************** PWM_LED ****************************/
#if TCFG_PWMLED_ENABLE
LED_PLATFORM_DATA_BEGIN(pwm_led_data)
	.io_mode = TCFG_PWMLED_IOMODE,              //推灯模式设置:支持单个IO推两个灯和两个IO推两个灯
	.io_cfg.one_io.pin = TCFG_PWMLED_PIN,       //单个IO推两个灯的IO口配置
LED_PLATFORM_DATA_END()
#endif

const struct soft_iic_config soft_iic_cfg[] = {
    //iic0 data
    {
        .scl = TCFG_SW_I2C0_CLK_PORT,                   //IIC CLK脚
        .sda = TCFG_SW_I2C0_DAT_PORT,                   //IIC DAT脚
        .delay = TCFG_SW_I2C0_DELAY_CNT,                //软件IIC延时参数，影响通讯时钟频率
        .io_pu = 1,                                     //是否打开上拉电阻，如果外部电路没有焊接上拉电阻需要置1
    },
#if 0
    //iic1 data
    {
        .scl = IO_PORTA_05,
        .sda = IO_PORTA_06,
        .delay = 50,
        .io_pu = 1,
    },
#endif
};


const struct hw_iic_config hw_iic_cfg[] = {
    //iic0 data
    {
        /*硬件IIC端口下选择
 			    SCL         SDA
		  	{IO_PORT_DP,  IO_PORT_DM},    //group a
        	{IO_PORTC_04, IO_PORTC_05},  //group b
        	{IO_PORTC_02, IO_PORTC_03},  //group c
        	{IO_PORTA_05, IO_PORTA_06},  //group d
         */
        .port = TCFG_HW_I2C0_PORTS,
        .baudrate = TCFG_HW_I2C0_CLK,      //IIC通讯波特率
        .hdrive = 0,                       //是否打开IO口强驱
        .io_filter = 1,                    //是否打开滤波器（去纹波）
        .io_pu = 1,                        //是否打开上拉电阻，如果外部电路没有焊接上拉电阻需要置1
    },
};

#if TCFG_SD0_ENABLE
SD0_PLATFORM_DATA_BEGIN(sd0_data)
	.port = {
        TCFG_SD0_PORT_CMD,
        TCFG_SD0_PORT_CLK,
        TCFG_SD0_PORT_DA0,
        TCFG_SD0_PORT_DA1,
        TCFG_SD0_PORT_DA2,
        TCFG_SD0_PORT_DA3,
    },
	.data_width             = TCFG_SD0_DAT_MODE,
	.speed                  = TCFG_SD0_CLK,
	.detect_mode            = TCFG_SD0_DET_MODE,
	.priority				= 3,

#if (TCFG_SD0_DET_MODE == SD_IO_DECT)
    .detect_io              = TCFG_SD0_DET_IO,
    .detect_io_level        = TCFG_SD0_DET_IO_LEVEL,
    .detect_func            = sdmmc_0_io_detect,
    .power                  = sd_set_power,
    /* .power                  = NULL, */
#elif (TCFG_SD0_DET_MODE == SD_CLK_DECT)
    .detect_io_level        = TCFG_SD0_DET_IO_LEVEL,
    .detect_func            = sdmmc_0_clk_detect,
    .power                  = sd_set_power,
    /* .power                  = NULL, */
#else
    .detect_func            = sdmmc_cmd_detect,
    .power                  = NULL,
#endif

SD0_PLATFORM_DATA_END()
#endif /* #if TCFG_SD0_ENABLE */

/************************** otg data****************************/
#if TCFG_OTG_MODE
struct otg_dev_data otg_data = {
    .usb_dev_en = TCFG_OTG_USB_DEV_EN,
	.slave_online_cnt = TCFG_OTG_SLAVE_ONLINE_CNT,
	.slave_offline_cnt = TCFG_OTG_SLAVE_OFFLINE_CNT,
	.host_online_cnt = TCFG_OTG_HOST_ONLINE_CNT,
	.host_offline_cnt = TCFG_OTG_HOST_OFFLINE_CNT,
	.detect_mode = TCFG_OTG_MODE,
	.detect_time_interval = TCFG_OTG_DET_INTERVAL,
};
#endif

#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
GSENSOR_PLATFORM_DATA_BEGIN(motion_sensor_data)
    .iic = 0,
    .gSensor_name = "mpu6050",
    .gSensor_int_io = NO_CONFIG_PORT,
GSENSOR_PLATFORM_DATA_END();
#endif/*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/

REGISTER_DEVICES(device_table) = {
    /* { "audio", &audio_dev_ops, (void *) &audio_data }, */

#if TCFG_CHARGE_ENABLE
    { "charge", &charge_dev_ops, (void *)&charge_data },
#endif
#if TCFG_SD0_ENABLE
	{ "sd0", 	&sd_dev_ops,     (void *)&sd0_data},
#endif
#if TCFG_OTG_MODE
    { "otg",    &usb_dev_ops,    (void *)&otg_data},
#endif
};

/************************** imu sensor ****************************/
#if TCFG_IMUSENSOR_ENABLE
IMU_SENSOR_PLATFORM_DATA_BEGIN(imu_sensor_data)
#if TCFG_SH3001_ENABLE
    {
    .peripheral_hdl = TCFG_SH3001_USER_IIC_INDEX,     //iic_hdl     or  spi_hdl
    .peripheral_param0= 0,   //iic_delay(iic byte间间隔)   or spi_cs_pin
    .imu_sensor_name = "sh3001",
    .imu_sensor_int_io = TCFG_SH3001_DETECT_IO,
    },
#endif
#if TCFG_TP_MPU9250_ENABLE
    {
    .peripheral_hdl = TCFG_MPU9250_USER_IIC_INDEX,     //iic_hdl     or  spi_hdl
    .peripheral_param0= 1,  //iic_delay(iic byte间间隔)   or spi_cs_pin//iic byte间间隔
    .imu_sensor_name = "mpu9250",
    .imu_sensor_int_io = TCFG_MPU9250_DETECT_IO,
    },
#endif
#if TCFG_MPU6887P_ENABLE
    {
    .peripheral_hdl = TCFG_MPU6887P_USER_IIC_INDEX,     //iic_hdl     or  spi_hdl
    .peripheral_param0= 0,  //iic_delay(iic byte间间隔)   or spi_cs_pin//iic byte间间隔
    .imu_sensor_name = "mpu6887p",
    .imu_sensor_int_io = TCFG_MPU6887P_DETECT_IO,
    },
#endif
#if TCFG_ICM42670P_ENABLE
    {
    .peripheral_hdl = TCFG_ICM42670P_USER_IIC_INDEX,     //iic_hdl     or  spi_hdl
    .peripheral_param0= 0,  //iic_delay(iic byte间间隔)   or spi_cs_pin//iic byte间间隔
    .imu_sensor_name = "icm42670p",
    .imu_sensor_int_io = TCFG_ICM42670P_DETECT_IO,
    },
#endif
#if TCFG_MPU6050_EN //未整合参数如GSENSOR_PLATFORM_DATA_BEGIN配置
    {
    .peripheral_hdl = 0,     //iic_hdl     or  spi_hdl
    .peripheral_param0= 0,  //iic_delay(iic byte间间隔)   or spi_cs_pin//iic byte间间隔
    .imu_sensor_name = "mpu6050",
    .imu_sensor_int_io = -1,
    },
#endif
#if TCFG_QMI8658_ENABLE
    {
    .peripheral_hdl = TCFG_QMI8658_USER_IIC_INDEX,     //iic_hdl     or  spi_hdl
    .peripheral_param0= 0,  //iic_delay(iic byte间间隔)   or spi_cs_pin//iic byte间间隔
    .imu_sensor_name = "qmi8658",
    .imu_sensor_int_io = TCFG_QMI8658_DETECT_IO,
    },
#endif
#if TCFG_LSM6DSL_ENABLE
    {
    .peripheral_hdl = TCFG_LSM6DSL_USER_IIC_INDEX,     //iic_hdl     or  spi_hdl
    .peripheral_param0= 0,  //iic_delay(iic byte间间隔)   or spi_cs_pin//iic byte间间隔
    .imu_sensor_name = "lsm6dsl",
    .imu_sensor_int_io = TCFG_LSM6DSL_DETECT_IO,
    },
#endif
IMU_SENSOR_PLATFORM_DATA_END();
#endif

/************************** LOW POWER config ****************************/
const struct low_power_param power_param = {
    .config         = TCFG_LOWPOWER_LOWPOWER_SEL,         //低功耗使能，蓝牙&&系统空闲可进入低功耗
    .btosc_hz       = TCFG_CLOCK_OSC_HZ,                  //蓝牙晶振频率
    .delay_us       = TCFG_CLOCK_SYS_HZ / 1000000L,        //提供给低功耗模块的延时(不需要需修改)
    .vddiom_lev     = TCFG_LOWPOWER_VDDIOM_LEVEL,         //vddiom等级
    .osc_type       = TCFG_LOWPOWER_OSC_TYPE,			  //低功耗晶振类型，btosc/lrc
#if (TCFG_LOWPOWER_RAM_SIZE)
    .mem_init_con   = MEM_PWR_RAM_SET(TCFG_LOWPOWER_RAM_SIZE),
#else
    .mem_init_con   = 0,
#endif
#if (TCFG_LP_TOUCH_KEY_ENABLE       && \
    (TCFG_LP_TOUCH_KEY0_WAKEUP_EN   || \
     TCFG_LP_TOUCH_KEY1_WAKEUP_EN   || \
     TCFG_LP_TOUCH_KEY2_WAKEUP_EN   || \
     TCFG_LP_TOUCH_KEY3_WAKEUP_EN   || \
     TCFG_LP_TOUCH_KEY4_WAKEUP_EN   ))
	.lpctmu_en 		= 1,
#else
	.lpctmu_en 		= 0,
#endif
};

/************************** wk_param ****************************/
struct port_wakeup port0 = {
    .pullup_down_enable = ENABLE,                            //配置I/O 内部上下拉是否使能
    .edge               = FALLING_EDGE,                      //唤醒方式选择,可选：上升沿\下降沿
    .filter             = PORT_FLT_8ms,
    .iomap              = IO_PORTB_01,                       //唤醒口选择
};

#if (TCFG_TEST_BOX_ENABLE || TCFG_CHARGESTORE_ENABLE || TCFG_ANC_BOX_ENABLE || TCFG_UMIDIGI_BOX_ENABLE)
struct port_wakeup port1 = {
    .pullup_down_enable = DISABLE,                            //配置I/O 内部上下拉是否使能
#if (TCFG_TEST_BOX_ANY_IO_UPDATE)
    .edge               = RISING_EDGE,                      //唤醒方式选择,可选：上升沿\下降沿
#else
    .edge               = FALLING_EDGE,                      //唤醒方式选择,可选：上升沿\下降沿
#endif
    .filter             = PORT_FLT_1ms,
    .iomap              = TCFG_CHARGESTORE_PORT,             //唤醒口选择
};
#endif

#if TCFG_CHARGE_ENABLE
struct port_wakeup charge_port = {
    .edge               = RISING_EDGE,                       //唤醒方式选择,可选：上升沿\下降沿\双边沿
    .filter             = PORT_FLT_16ms,
    .iomap              = IO_CHGFL_DET,                      //唤醒口选择
};

struct port_wakeup vbat_port = {
    .edge               = BOTH_EDGE,                      //唤醒方式选择,可选：上升沿\下降沿\双边沿
    .filter             = PORT_FLT_4ms,
    .iomap              = IO_VBTCH_DET,                      //唤醒口选择
};

struct port_wakeup ldoin_port = {
    .edge               = BOTH_EDGE,                      //唤醒方式选择,可选：上升沿\下降沿\双边沿
    .filter             = PORT_FLT_4ms,
    .iomap              = IO_LDOIN_DET,                      //唤醒口选择
};
#endif

const struct wakeup_param wk_param = {
#if (!(TCFG_LP_TOUCH_KEY_ENABLE && TCFG_LP_TOUCH_KEY1_EN))
	.port[1] = &port0,
#endif
#if (TCFG_TEST_BOX_ENABLE || TCFG_CHARGESTORE_ENABLE || TCFG_ANC_BOX_ENABLE || TCFG_UMIDIGI_BOX_ENABLE)
	.port[2] = &port1,
#endif
#if TCFG_CHARGE_ENABLE
    .aport[0] = &charge_port,
    .aport[1] = &vbat_port,
    .aport[2] = &ldoin_port,
#endif
};

void gSensor_wkupup_disable(void)
{
    log_info("gSensor wkup disable\n");
    power_wakeup_index_enable(1, 0);
}

void gSensor_wkupup_enable(void)
{
    log_info("gSensor wkup enable\n");
    power_wakeup_index_enable(1, 1);
}

void debug_uart_init(const struct uart_platform_data *data)
{
#if TCFG_UART0_ENABLE
    if (data) {
        uart_init(data);
    } else {
        uart_init(&uart0_data);
    }
#endif
}


STATUS *get_led_config(void)
{
    return &(__this->led);
}

STATUS *get_tone_config(void)
{
    return &(__this->tone);
}

u8 get_sys_default_vol(void)
{
    return 21;
}

u8 get_power_on_status(void)
{
#if TCFG_IOKEY_ENABLE
    struct iokey_port *power_io_list = NULL;
    power_io_list = iokey_data.port;

    if (iokey_data.enable) {
        if (gpio_read(power_io_list->key_type.one_io.port) == power_io_list->connect_way){
            return 1;
        }
    }
#endif

#if TCFG_ADKEY_ENABLE
    if (adkey_data.enable) {
    	return 1;
    }
#endif

#if TCFG_LP_TOUCH_KEY_ENABLE
        return lp_touch_key_power_on_status();
#endif

    return 0;
}


static void board_devices_init(void)
{
#if TCFG_PWMLED_ENABLE
    pwm_led_init(&pwm_led_data);
#endif

#if (TCFG_IOKEY_ENABLE || TCFG_ADKEY_ENABLE || TCFG_RDEC_KEY_ENABLE || TCFG_TOUCH_KEY_ENABLE)
	key_driver_init();
#endif

#if TCFG_UART_KEY_ENABLE
	extern int uart_key_init(void);
	uart_key_init();
#endif /* #if TCFG_UART_KEY_ENABLE */

#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
    extern void imu_sensor_power_ctl(u32 gpio,u8 value);
    extern void imu_sensor_ad0_selete(u32 gpio, u8 value);
    imu_sensor_power_ctl(TCFG_IMU_SENSOR_PWR_PORT,1);
    os_time_dly(1);
#if TCFG_MPU6050_EN
    gravity_sensor_init(&motion_sensor_data);
#endif /*TCFG_MPU6050_EN*/
#if TCFG_MPU6887P_ENABLE
    imu_sensor_ad0_selete(TCFG_MPU6887P_AD0_SELETE_IO, MPU6887P_SA0_IIC_ADDR);
#endif /*TCFG_MPU6887P_ENABLE*/
#if TCFG_QMI8658_ENABLE
    imu_sensor_ad0_selete(TCFG_QMI8658_AD0_SELETE_IO, QMI8658_SD0_IIC_ADDR);
#endif /*TCFG_QMI8658_ENABLE*/
#if TCFG_LSM6DSL_ENABLE
    imu_sensor_ad0_selete(TCFG_LSM6DSL_AD0_SELETE_IO, LSM6DSL_SD0_IIC_ADDR);
#endif /*TCFG_LSM6DSL_ENABLE*/
#if TCFG_ICM42670P_ENABLE
    imu_sensor_ad0_selete(TCFG_ICM42670P_AD0_SELETE_IO, ICM42670P_SA0_IIC_ADDR);
#endif /*TCFG_ICM42670P_ENABLE*/

#if TCFG_IMUSENSOR_ENABLE
    imu_sensor_init(imu_sensor_data,sizeof(imu_sensor_data));
#endif /*TCFG_IMUSENSOR_ENABLE*/

    /* //MPU进入睡眠 */
#if TCFG_MPU6887P_ENABLE
    imu_sensor_io_ctl("mpu6887p", IMU_SENSOR_SLEEP, NULL);
#endif /*TCFG_MPU6887P_ENABLE*/
#if TCFG_QMI8658_ENABLE
    imu_sensor_io_ctl("qmi8658", IMU_SENSOR_SLEEP, NULL);
#endif /*TCFG_QMI8658_ENABLE*/
#if TCFG_LSM6DSL_ENABLE
    imu_sensor_io_ctl("lsm6dsl", IMU_SENSOR_SLEEP, NULL);
#endif /*TCFG_LSM6DSL_ENABLE*/
#if TCFG_ICM42670P_ENABLE
    imu_sensor_io_ctl("icm42670p", IMU_SENSOR_SLEEP, NULL);
#endif /*TCFG_ICM42670P_ENABLE*/
#endif/*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/

#if TCFG_LP_TOUCH_KEY_ENABLE
        lp_touch_key_init(&lp_touch_key_config);
#endif /* #if TCFG_LP_TOUCH_KEY_ENABLE */

#if (!TCFG_CHARGE_ENABLE)
    CHARGE_EN(0);
    CHGGO_EN(0);
#endif

#if TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE || TCFG_ANC_BOX_ENABLE
    chargestore_api_init(&chargestore_data);
#endif
}

extern void cfg_file_parse(u8 idx);
void board_init()
{
    board_power_init();
    //adc_vbg_init();
    adc_init();
    cfg_file_parse(0);

#if TCFG_SD0_ENABLE
	dev_manager_init();
#else
    devices_init();
#endif

#if TCFG_AUDIO_ANC_ENABLE
	anc_init();
#endif/*TCFG_AUDIO_ANC_ENABLE*/

	board_devices_init();

#if TCFG_CHARGE_ENABLE
    if(get_charge_online_flag())
#else
    if (0)
#endif
    {
        power_set_mode(PWR_LDO15);
    }else{
        power_set_mode(TCFG_LOWPOWER_POWER_SEL);
    }

    //针对硅mic要输出1给mic供电
    /* gpio_set_pull_up(IO_PORTA_04, 0); */
    /* gpio_set_pull_down(IO_PORTA_04, 0); */
    /* gpio_set_direction(IO_PORTA_04, 0); */
    /* gpio_set_output_value(IO_PORTA_04,1); */

#if TCFG_UART0_ENABLE
    if (uart0_data.rx_pin < IO_MAX_NUM) {
        gpio_set_die(uart0_data.rx_pin, 1);
    }
#endif

#if TCFG_SMART_VOICE_ENABLE
    int audio_smart_voice_detect_init(struct vad_mic_platform_data *mic_data);
    audio_smart_voice_detect_init((struct vad_mic_platform_data *)&vad_mic_data);
#endif /* #if TCFG_SMART_VOICE_ENABLE */

#ifdef CONFIG_BOARD_AISPEECH_VAD_ASR
    extern int audio_ais_platform_asr_init(struct vad_mic_platform_data *mic_data);
    audio_ais_platform_asr_init((struct vad_mic_platform_data *)&vad_mic_data);
#endif /*CONFIG_BOARD_AISPEECH_VAD_ASR*/

#ifdef AUDIO_PCM_DEBUG
	extern void uartSendInit();
	uartSendInit();
#endif/*AUDIO_PCM_DEBUG*/

    extern void bredr_set_dut_enble(u8 en, u8 phone);
    bredr_set_dut_enble(1, 1);
}

/*进软关机之前默认将IO口都设置成高阻状态，需要保留原来状态的请修改该函数*/
extern void dac_power_off(void);
void board_set_soft_poweroff(void)
{
	//power按键
#if TCFG_IOKEY_ENABLE
    soff_gpio_protect(TCFG_IOKEY_POWER_ONE_PORT);
#endif

#if (!(TCFG_LP_TOUCH_KEY_ENABLE && TCFG_LP_TOUCH_KEY1_EN))
    //默认唤醒io
    soff_gpio_protect(IO_PORTB_01);
#endif

	board_set_soft_poweroff_common(NULL);

#if (TCFG_TEST_BOX_ENABLE || TCFG_CHARGESTORE_ENABLE || TCFG_ANC_BOX_ENABLE || TCFG_UMIDIGI_BOX_ENABLE)
#if (TCFG_CHARGESTORE_PORT != IO_PORTP_00)
    gpio_set_pull_up(TCFG_CHARGESTORE_PORT, 0);
    gpio_set_pull_down(TCFG_CHARGESTORE_PORT, 1);
    gpio_set_die(TCFG_CHARGESTORE_PORT, 1);
    gpio_set_dieh(TCFG_CHARGESTORE_PORT, 1);
    gpio_set_direction(TCFG_CHARGESTORE_PORT, 1);
#else
	power_wakeup_index_enable(2, 0);
#endif
#endif

    dac_power_off();
}

#define     APP_IO_DEBUG_0(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}
#define     APP_IO_DEBUG_1(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}

void sleep_exit_callback(u32 usec)
{
    sleep_exit_callback_common(NULL);

	putchar('>');
}

void sleep_enter_callback(u8  step)
{
    /* 此函数禁止添加打印 */
    if (step == 1) {
        putchar('<');
    } else {
        gpio_set_pull_up(TCFG_CHARGESTORE_PORT, 0);
        gpio_set_pull_down(TCFG_CHARGESTORE_PORT, 0);
        gpio_set_die(TCFG_CHARGESTORE_PORT, 1);
        gpio_set_dieh(TCFG_CHARGESTORE_PORT, 1);
        gpio_set_direction(TCFG_CHARGESTORE_PORT, 1);

		sleep_enter_callback_common(NULL);
	}
}

static void port_wakeup_callback(u8 index, u8 gpio)
{
	/* log_info("%s:%d,%d",__FUNCTION__,index,gpio); */
#if TCFG_UMIDIGI_BOX_ENABLE
	if (index == 2) {
		ldo_port_wakeup_to_cmessage();
	}
#endif
    switch (index) {
#if (TCFG_TEST_BOX_ENABLE || TCFG_CHARGESTORE_ENABLE || TCFG_ANC_BOX_ENABLE)
        case 2:
            extern void chargestore_ldo5v_fall_deal(void);
            chargestore_ldo5v_fall_deal();
            break;
#endif
    }
}

static void aport_wakeup_callback(u8 index, u8 gpio, u8 edge)
{
    /* log_info("%s:%d,%d",__FUNCTION__,index,gpio); */
#if TCFG_CHARGE_ENABLE
    switch (gpio) {
        case IO_CHGFL_DET://charge port
            charge_wakeup_isr();
            break;
        case IO_VBTCH_DET://vbat port
        case IO_LDOIN_DET://ldoin port
            ldoin_wakeup_isr();
            break;
    }
#endif
}

void board_power_init(void)
{
    log_info("Power init : %s", __FILE__);

    power_init(&power_param);

    power_set_callback(TCFG_LOWPOWER_LOWPOWER_SEL, sleep_enter_callback, sleep_exit_callback, board_set_soft_poweroff);

#if TCFG_UMIDIGI_BOX_ENABLE
    gpio_set_die(TCFG_CHARGESTORE_PORT, 1);
	umidigi_chargestore_message_callback(app_umidigi_chargetore_message_deal);
#endif

    power_keep_dacvdd_en(0);

	power_wakeup_init(&wk_param);

    power_awakeup_set_callback(aport_wakeup_callback);
    power_wakeup_set_callback(port_wakeup_callback);
}
#endif /* #ifdef CONFIG_BOARD_JL701N_DEMO */
