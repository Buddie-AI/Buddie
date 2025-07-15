#ifndef __POWER_API_H__
#define __POWER_API_H__

#define PMU_NEW_FLOW 0
#define TRIM_WVDD  		 BIT(0)
#define TRIM_PVDD  		 BIT(1)
#define TRIM_IOVDD       BIT(2)

//=========================电源参数配置==================================
struct low_power_param {
    u8 config;				//低功耗使能，蓝牙&&系统空闲可进入低功耗
    u8 osc_type;			//低功耗晶振类型，btosc/lrc
    u32 btosc_hz;			//蓝牙晶振频率

    //vddiow_lev不需要配置，sleep、softoff模式会保持电压，除非配置使用nkeep_vddio(功耗相差不大)
    u8 vddiom_lev;			//vddiom
    u8 vddiow_lev;			//vddiow
    u8 nkeep_vddio;			//softoff模式下不保持vddio

    u32 osc_delay_us;		//低功耗晶振起振延时，为预留配置。
    u8 rtc_clk;				//rtc时钟源，softoff模式根据此配置是否保持住相应时钟
    u8 lpctmu_en;			//低功耗触摸，softoff模式根据此配置是否保持住该模块

    u8 mem_init_con;		//初始化ram电源
    u8 mem_lowpower_con;	//低功耗ram电源
    u8 rvdd2pvdd;			//低功耗外接dcdc
    u8 pvdd_dcdc_port;

    u8 lptmr_flow;			//低功耗参数由用户配置
    u32 t1;
    u32 t2;
    u32 t3_lrc;
    u32 t4_lrc;
    u32 t3_btosc;
    u32 t4_btosc;

    u8 flash_pg;			//iomc: 外置flash power gate

    u8 btosc_disable;
    u8 delay_us;

    u8 vddio_keep_type_pd;	//sleep keep vddio使用类型
    u8 vddio_keep_type_sf;  //soff keep vddio使用类型
};

//config
#define SLEEP_EN                            BIT(0)
#define DEEP_SLEEP_EN                       BIT(1)

enum {
    VDDIO_KEEP_TYPE_TRIM = 0,//vddio keep使用trim值：默认
    VDDIO_KEEP_TYPE_PG,		 //vddio keep使用 mvvdio：功耗代价100ua
};

//osc_type
enum {
    OSC_TYPE_LRC = 0,
    OSC_TYPE_BT_OSC,
};

//DCVDD电源模式
enum {
    PWR_LDO15,
    PWR_DCDC15,
};

//PVDD电源模式
enum PVDD_MODE {
    PWR_PVDD_LDO,
    PWR_PVDD_DCDC,
};

//==============================软关机参数配置============================
//软关机会复位寄存器，该参数为传给rom配置的参数。
struct soft_flag0_t {
    u8 wdt_dis: 1;
    u8 poweroff: 1;
    u8 is_port_b: 1;
    u8 lvd_en: 1;
    u8 pmu_en: 1;
    u8 iov2_ldomode: 1;
    u8 res: 2;
};

struct soft_flag1_t {
    u8 usbdp: 2;
    u8 usbdm: 2;
    u8 uart_key_port: 1;
    u8 ldoin: 3;
};

struct soft_flag2_t {
    u8 pg2: 4;
    u8 pg3: 4;
};

struct soft_flag3_t {
    u8 pg4: 4;
    u8 res: 4;
};

struct soft_flag4_t {
    u8 fast_boot: 1;
    u8 flash_stable_delay_sel: 2;
    u8 res: 5;
};

struct soft_flag5_t {
    u8 mvddio: 3;
    u8 wvbg: 4;
    u8 res: 1;
};

struct boot_soft_flag_t {
    union {
        struct soft_flag0_t boot_ctrl;
        u8 value;
    } flag0;
    union {
        struct soft_flag1_t misc;
        u8 value;
    } flag1;
    union {
        struct soft_flag2_t pg2_pg3;
        u8 value;
    } flag2;
    union {
        struct soft_flag3_t pg4_res;
        u8 value;
    } flag3;
    union {
        struct soft_flag4_t fast_boot_ctrl;
        u8 value;
    } flag4;
    union {
        struct soft_flag5_t level;
        u8 value;
    } flag5;
};

enum soft_flag_io_stage {
    SOFTFLAG_HIGH_RESISTANCE,
    SOFTFLAG_PU,
    SOFTFLAG_PD,

    SOFTFLAG_OUT0,
    SOFTFLAG_OUT0_HD0,
    SOFTFLAG_OUT0_HD,
    SOFTFLAG_OUT0_HD0_HD,

    SOFTFLAG_OUT1,
    SOFTFLAG_OUT1_HD0,
    SOFTFLAG_OUT1_HD,
    SOFTFLAG_OUT1_HD0_HD,
};

//==============================电源接口============================

#define AT_VOLATILE_RAM_CODE_POWER        AT(.power_driver.text.cache.L1)
#define AT_VOLATILE_CACHE_CODE_POWER      AT(.power_driver.text.cache.L2)

void power_set_mode(u8 mode);

void power_set_pvdd_mode(enum PVDD_MODE mode);

u8 get_pvdd_dcdc_cfg();

void power_init(const struct low_power_param *param);

void power_keep_dacvdd_en(u8 en);

void sdpg_config(int enable);

void p11_init(void);

u8 get_wvdd_trim_level();

u8 get_pvdd_level();

u8 get_pvdd_trim_level();

u8 get_miovdd_trim_level();

u8 get_wiovdd_trim_level();

void store_pmu_trim_value_to_vm(u8 wvdd_level, u8 pvdd_level, u8 miovdd_lev, u8 wiovdd_lev);

u8 load_pmu_trim_value_from_vm();

void volatage_trim_init();

//==============================sleep接口============================
//注意：所有接口在临界区被调用，请勿使用阻塞操作
//sleep模式介绍
//1.所有数字模块停止，包括cpu、periph、audio、rf等
//2.所有模拟模块停止，包括pll、btosc、rc等
//3.只保留pmu模块

//light_sleep: 不切电源域
//normal_sleep: dvdd低电
//deepsleep：dvdd掉电

//----------------低功耗线程查询是否满足低功耗状态, 被动等待------------
struct low_power_operation {

    const char *name;

    u32(*get_timeout)(void *priv);

    void (*suspend_probe)(void *priv);

    void (*suspend_post)(void *priv, u32 usec);

    void (*resume)(void *priv, u32 usec);

    void (*resume_post)(void *priv, u32 usec);
};

enum LOW_POWER_LEVEL {
    LOW_POWER_MODE_LIGHT_SLEEP = 1,
    LOW_POWER_MODE_SLEEP,
    LOW_POWER_MODE_DEEP_SLEEP,
};

typedef u8(*idle_handler_t)(void);
typedef enum LOW_POWER_LEVEL(*level_handler_t)(void);

typedef u8(*idle_handler_t)(void);

struct lp_target {
    char *name;
    level_handler_t level;
    idle_handler_t is_idle;
};

#define REGISTER_LP_TARGET(target) \
        const struct lp_target target sec(.lp_target)


extern const struct lp_target lp_target_begin[];
extern const struct lp_target lp_target_end[];

#define list_for_each_lp_target(p) \
    for (p = lp_target_begin; p < lp_target_end; p++)


//--------------低功耗线程请求进入低功耗, 主动发出------------
struct lp_request {
    char *name;
    u8(*request_enter)(u32 timeout);
    u8(*request_exit)(u32 timeout);
};

#define REGISTER_LP_REQUEST(target) \
        const struct lp_request target sec(.lp_request)

extern const struct lp_request lp_request_begin[];
extern const struct lp_request lp_request_end[];

#define list_for_each_lp_request(p) \
    for (p = lp_request_begin; p < lp_request_end; p++)

//-----------------------深度睡眠处理--------------------------
struct deepsleep_target {
    char *name;
    u8(*enter)(void);
    u8(*exit)(void);
};

#define DEEPSLEEP_TARGET_REGISTER(target) \
        const struct deepsleep_target target sec(.deepsleep_target)


extern const struct deepsleep_target deepsleep_target_begin[];
extern const struct deepsleep_target deepsleep_target_end[];

#define list_for_each_deepsleep_target(p) \
    for (p = deepsleep_target_begin; p < deepsleep_target_end; p++)


#define NEW_BASEBAND_COMPENSATION       0

u32 __tus_carry(u32 x);

#define  power_is_poweroff_post()   0

void *low_power_get(void *priv, const struct low_power_operation *ops);

void low_power_put(void *priv);

void low_power_sys_request(void *priv);

void *low_power_sys_get(void *priv, const struct low_power_operation *ops);

void low_power_sys_put(void *priv);

u8 is_low_power_mode(enum LOW_POWER_LEVEL level);

u8 low_power_sys_is_idle(void);

s32 low_power_trace_drift(u32 usec);

void low_power_reset_osc_type(u8 type);

u8 low_power_get_default_osc_type(void);

u8 low_power_get_osc_type(void);

void low_power_on(void);

void low_power_request(void);

u8 low_power_sys_request_enter(u32 timeout);

u8 low_power_sys_request_exit(u32 timeout);

u32 get_rch_hz();

void cap_rch_enable();

void cap_rch_disable();

//==============================soft接口============================


void power_set_soft_poweroff();

void power_set_soft_poweroff_advance();

void mask_softflag_config(const struct boot_soft_flag_t *softflag);

void power_set_callback(u8 mode, void (*powerdown_enter)(u8 step), void (*powerdown_exit)(u32), void (*soft_poweroff_enter)(void));



#endif
