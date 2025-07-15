#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/*
 * 系统打印总开关
 */

#define LIB_DEBUG    1
#define CONFIG_DEBUG_LIB(x)         (x & LIB_DEBUG)

// #define CONFIG_DEBUG_ENABLE

#ifndef CONFIG_DEBUG_ENABLE
//#define CONFIG_DEBUG_LITE_ENABLE  //轻量级打印开关, 默认关闭
#endif


//*********************************************************************************//
//                                  AI配置                                       //
//*********************************************************************************//
#define CONFIG_APP_BT_ENABLE

#ifdef CONFIG_APP_BT_ENABLE
#define    TRANS_DATA_EN             0
#define    RCSP_BTMATE_EN            0
#define    RCSP_ADV_EN               1
#define    AI_APP_PROTOCOL           0
#define    LL_SYNC_EN                0
#define    TUYA_DEMO_EN              0
#else
#define    TRANS_DATA_EN             0
#define    RCSP_BTMATE_EN            0
#define    RCSP_ADV_EN               0
#define    AI_APP_PROTOCOL           0
#define    LL_SYNC_EN                0
#define    TUYA_DEMO_EN              0
#endif


#include "board_config.h"
#ifdef  TCFG_AUDIO_CVP_NS_MODE
#if (TCFG_AUDIO_CVP_NS_MODE==CVP_DNS_MODE) || (TCFG_KWS_VOICE_RECOGNITION_ENABLE == 1)||(TCFG_AUDIO_ANC_ENABLE==1&&TCFG_AUDIO_DUAL_MIC_ENABLE==1)
#undef CONFIG_MOVABLE_ENABLE
#define CONFIG_MOVABLE_ENABLE //省ram空间，将部分ram空间的代码挪到flash
#endif
#endif

#if defined(TCFG_CVP_DEVELOP_ENABLE) && (TCFG_CVP_DEVELOP_ENABLE == 1)
#undef CONFIG_MOVABLE_ENABLE
#define CONFIG_MOVABLE_ENABLE //省ram空间，将部分ram空间的代码挪到flash
#endif

#if CONFIG_UPDATE_WITH_MD5_CHECK_EN
#define UPDATE_MD5_ENABLE			1
#else
#define UPDATE_MD5_ENABLE			0
#endif


//升级需要打开CONFIG_APP_OTA_ENABLE,同时单双备份的配置也在board_xxx_global_cfg里配置，需要注意只有JL_RCSP才支持单备份，其余升级都是只支持双备份升级
//支持TWS同步升级，OTA_TWS_SAME_TIME_NEW宏需要配置为1，旧的流程已不再支持
#if (RCSP_ADV_EN)                       //rcsp需要打开ble
#define    JL_EARPHONE_APP_EN  1
// #define CONFIG_CHARGESTORE_REMAP_ENABLE //充电仓重映射接收函数使能
#if CONFIG_APP_OTA_ENABLE
#define RCSP_UPDATE_EN		         1	   //是否支持rcsp升级
#if CONFIG_DOUBLE_BANK_ENABLE              //双备份才能打开同步升级流程
#define OTA_TWS_SAME_TIME_ENABLE     1     //是否支持TWS同步升级
#define OTA_TWS_SAME_TIME_NEW        1     //使用新的tws ota流程
#else
#define OTA_TWS_SAME_TIME_ENABLE     0     //是否支持TWS同步升级
#define OTA_TWS_SAME_TIME_NEW        0     //使用新的tws ota流程
#endif      //CONFIG_DOUBLE_BANK_ENABLE
#else
#define RCSP_UPDATE_EN 		         0     //是否支持rcsp升级
#define OTA_TWS_SAME_TIME_ENABLE     0     //是否支持TWS同步升级
#define OTA_TWS_SAME_TIME_NEW        0     //使用新的tws ota流程
#endif
#undef     TCFG_USER_BLE_ENABLE
#define    TCFG_USER_BLE_ENABLE      1     //BLE功能使能
#elif (AI_APP_PROTOCOL)
#define    BT_MIC_EN                 0
#define    OTA_TWS_SAME_TIME_ENABLE  0     //是否支持TWS同步升级
#elif (LL_SYNC_EN)
#define    JL_EARPHONE_APP_EN        0
#define    OTA_TWS_SAME_TIME_ENABLE  1
#define    OTA_TWS_SAME_TIME_NEW     1     //使用新的tws ota流程
#elif (TUYA_DEMO_EN)
#define    JL_EARPHONE_APP_EN        0
#define    OTA_TWS_SAME_TIME_ENABLE  0
#define    OTA_TWS_SAME_TIME_NEW     0     //使用新的tws ota流程
#else
#define    JL_EARPHONE_APP_EN        0
#define    OTA_TWS_SAME_TIME_ENABLE  0
#define    OTA_TWS_SAME_TIME_NEW     0     //使用新的tws ota流程
#endif

#define CONFIG_MEDIA_LIB_USE_MALLOC    1



#include "usb_std_class_def.h"

#undef USB_MALLOC_ENABLE
#define     USB_MALLOC_ENABLE           1
///USB 配置重定义
// #undef USB_DEVICE_CLASS_CONFIG
// #define USB_DEVICE_CLASS_CONFIG 									(AUDIO_CLASS)
/////要确保 上面 undef 后在include usb
#include "usb_common_def.h"

#define USB_PC_NO_APP_MODE                        0

#if ((TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE || TCFG_ANC_BOX_ENABLE) \
     && TCFG_CHARGESTORE_PORT == IO_PORT_DP)
#undef  TCFG_OTG_MODE
#define TCFG_OTG_MODE (TCFG_OTG_MODE_HOST|TCFG_OTG_MODE_SLAVE|TCFG_OTG_MODE_CHARGE|OTG_DET_DM_ONLY)
#endif


#include "btcontroller_mode.h"

#include "user_cfg_id.h"

#ifndef __LD__
#include "bt_profile_cfg.h"
#endif


// #ifdef CONFIG_APP_BT_ENABLE
// #if(APP_ONLINE_DEBUG)
// #error "they can not enable at the same time,just select one!!!"
// #endif
// #endif


#ifdef CONFIG_SDFILE_ENABLE

#define SDFILE_DEV				"sdfile"
#define SDFILE_MOUNT_PATH     	"mnt/sdfile"

#if (USE_SDFILE_NEW)
#define SDFILE_APP_ROOT_PATH       	SDFILE_MOUNT_PATH"/app/"  //app分区
#define SDFILE_RES_ROOT_PATH       	SDFILE_MOUNT_PATH"/res/"  //资源文件分区
#else
#define SDFILE_RES_ROOT_PATH       	SDFILE_MOUNT_PATH"/C/"
#endif

#endif

#define STYLE_JL_WTACH              (1)//彩屏demo
#define STYLE_JL_SOUNDBOX           (2)//点阵屏demo
#define STYLE_JL_CHARGE             (3)//点阵屏充电仓
#define STYLE_JL_LED7               (4)//led7
#define STYLE_UI_SIMPLE             (5)//没有ui框架


//*********************************************************************************//
//                                 测试模式配置                                    //
//*********************************************************************************//
#if (CONFIG_BT_MODE != BT_NORMAL)
#undef  TCFG_BD_NUM
#define TCFG_BD_NUM						          1

#undef  TCFG_USER_TWS_ENABLE
#define TCFG_USER_TWS_ENABLE                      0     //tws功能使能

#undef  TCFG_USER_BLE_ENABLE
#define TCFG_USER_BLE_ENABLE                      1     //BLE功能使能

#undef  TCFG_AUTO_SHUT_DOWN_TIME
#define TCFG_AUTO_SHUT_DOWN_TIME		          0

#undef  TCFG_SYS_LVD_EN
#define TCFG_SYS_LVD_EN						      0

#undef  TCFG_LOWPOWER_LOWPOWER_SEL
#define TCFG_LOWPOWER_LOWPOWER_SEL                0

#undef TCFG_AUDIO_DAC_LDO_VOLT
#define TCFG_AUDIO_DAC_LDO_VOLT			   DUT_AUDIO_DAC_LDO_VOLT

#undef TCFG_LOWPOWER_POWER_SEL
#define TCFG_LOWPOWER_POWER_SEL				PWR_LDO15

#undef  TCFG_PWMLED_ENABLE
#define TCFG_PWMLED_ENABLE					DISABLE_THIS_MOUDLE

#undef  TCFG_ADKEY_ENABLE
#define TCFG_ADKEY_ENABLE                   DISABLE_THIS_MOUDLE

#undef  TCFG_IOKEY_ENABLE
#define TCFG_IOKEY_ENABLE					DISABLE_THIS_MOUDLE

#undef TCFG_TEST_BOX_ENABLE
#define TCFG_TEST_BOX_ENABLE			    0

#undef TCFG_AUTO_SHUT_DOWN_TIME
#define TCFG_AUTO_SHUT_DOWN_TIME		          0

#undef TCFG_POWER_ON_NEED_KEY
#define TCFG_POWER_ON_NEED_KEY				      0

/* #undef TCFG_UART0_ENABLE
#define TCFG_UART0_ENABLE					DISABLE_THIS_MOUDLE */

#endif //(CONFIG_BT_MODE != BT_NORMAL)


#if TCFG_USER_TWS_ENABLE

#define CONFIG_TWS_COMMON_ADDR_AUTO             0       /* 自动生成TWS配对后的MAC地址 */
#define CONFIG_TWS_COMMON_ADDR_USED_LEFT        1       /* 使用左耳的MAC地址作为TWS配对后的地址
                                                           可配合烧写器MAC地址自增功能一起使用
                                                           多台交叉配对会出现MAC地址相同情况 */
#define CONFIG_TWS_COMMON_ADDR_SELECT           CONFIG_TWS_COMMON_ADDR_USED_LEFT

//*********************************************************************************//
//                                 对耳配置方式配置                                    //
//*********************************************************************************//
#define CONFIG_TWS_CONNECT_SIBLING_TIMEOUT    4    /* 开机或超时断开后对耳互连超时时间，单位s */
// #define CONFIG_TWS_REMOVE_PAIR_ENABLE              [> 不连手机的情况下双击按键删除配对信息 <]
#define CONFIG_TWS_POWEROFF_SAME_TIME         1    /*按键关机时两个耳机同时关机*/

#define ONE_KEY_CTL_DIFF_FUNC                 1    /*通过左右耳实现一个按键控制两个功能*/
#define CONFIG_TWS_SCO_ONLY_MASTER			  0	   /*通话的时候只有主机出声音*/

/* 配对方式选择 */
#define CONFIG_TWS_PAIR_BY_CLICK            0      /* 按键发起配对 */
#define CONFIG_TWS_PAIR_BY_AUTO             1      /* 开机自动配对 */
#define CONFIG_TWS_PAIR_BY_FAST_CONN        2      /* 开机快速连接,连接速度比自动配对快,不支持取消配对操作 */
#define CONFIG_TWS_PAIR_MODE                CONFIG_TWS_PAIR_BY_AUTO


/* 声道确定方式选择 */
#define CONFIG_TWS_MASTER_AS_LEFT             0 //主机作为左耳
#define CONFIG_TWS_AS_LEFT_CHANNEL            1 //固定左耳
#define CONFIG_TWS_AS_RIGHT_CHANNEL           2 //固定右耳
#define CONFIG_TWS_LEFT_START_PAIR            3 //双击发起配对的耳机做左耳
#define CONFIG_TWS_RIGHT_START_PAIR           4 //双击发起配对的耳机做右耳
#define CONFIG_TWS_EXTERN_UP_AS_LEFT          5 //外部有上拉电阻作为左耳
#define CONFIG_TWS_EXTERN_DOWN_AS_LEFT        6 //外部有下拉电阻作为左耳
#define CONFIG_TWS_SECECT_BY_CHARGESTORE      7 //充电仓决定左右耳
#define CONFIG_TWS_CHANNEL_SELECT             CONFIG_TWS_EXTERN_DOWN_AS_LEFT //配对方式选择
// #define CONFIG_TWS_CHANNEL_SELECT             CONFIG_TWS_AS_LEFT_CHANNEL
// #define CONFIG_TWS_CHANNEL_SELECT             CONFIG_TWS_AS_RIGHT_CHANNEL

#define CONFIG_TWS_CHANNEL_CHECK_IO           IO_PORTB_05					//上下拉电阻检测引脚


#if CONFIG_TWS_PAIR_MODE != CONFIG_TWS_PAIR_BY_CLICK
#if (CONFIG_TWS_CHANNEL_SELECT == CONFIG_TWS_LEFT_START_PAIR) ||\
    (CONFIG_TWS_CHANNEL_SELECT == CONFIG_TWS_RIGHT_START_PAIR)
#undef CONFIG_TWS_CHANNEL_SELECT
#define CONFIG_TWS_CHANNEL_SELECT             CONFIG_TWS_MASTER_AS_LEFT
#endif

#if CONFIG_TWS_PAIR_MODE == CONFIG_TWS_PAIR_BY_AUTO
#define CONFIG_TWS_AUTO_PAIR_WITHOUT_UNPAIR     /* 不取消配对也可以配对新的耳机 */
#endif

#if CONFIG_TWS_PAIR_MODE == CONFIG_TWS_PAIR_BY_FAST_CONN
#undef CONFIG_TWS_REMOVE_PAIR_ENABLE
#endif

#endif

#define CONFIG_A2DP_GAME_MODE_ENABLE            0  //游戏模式
#define CONFIG_A2DP_GAME_MODE_DELAY_TIME        35  //游戏模式延时ms

//*********************************************************************************//
//      低延时游戏模式脚步声、枪声增强,需使能蓝牙音乐10段eq以及蓝牙音乐drc
//      用户开关宏AUDIO_GAME_EFFECT_CONFIG(开关蓝牙低延时模式的游戏音效)
//      低延时eq效果文件使用eq_game_eff.bin,调试时需保存成该文件,并在批处理-res后添加
//      非低延时eq效果文件使用eq_cfg_hw.bin,也需在批处理-res后添加
//*********************************************************************************//
#if CONFIG_A2DP_GAME_MODE_ENABLE
#define AUDIO_GAME_EFFECT_CONFIG  1  //低延时游戏模式脚步声、枪声增强 1:使能、0：关闭
#else
#define AUDIO_GAME_EFFECT_CONFIG  0  //低延时游戏模式脚步声、枪声增强 1:使能、0：关闭
#endif
/***********************************非用户配置区***********************************/
//以下宏定义，是游戏音效使能后，需要开启的宏定义,已配好，用户不用修改
#if AUDIO_GAME_EFFECT_CONFIG
#if (TCFG_EQ_ENABLE == 0)
#undef TCFG_EQ_ENABLE
#define TCFG_EQ_ENABLE  1
#endif/* (TCFG_EQ_ENABLE == 0) */
#if (TCFG_BT_MUSIC_EQ_ENABLE == 0)
#undef TCFG_BT_MUSIC_EQ_ENABLE
#define TCFG_BT_MUSIC_EQ_ENABLE 1
#endif/*(TCFG_BT_MUSIC_EQ_ENABLE == 0)*/
#if (TCFG_DRC_ENABLE == 0)
#undef TCFG_DRC_ENABLE
#define TCFG_DRC_ENABLE 1
#endif/* (TCFG_DRC_ENABLE == 0) */
#if (TCFG_BT_MUSIC_DRC_ENABLE == 0)
#undef TCFG_BT_MUSIC_DRC_ENABLE
#define TCFG_BT_MUSIC_DRC_ENABLE 1
#endif/* (TCFG_BT_MUSIC_DRC_ENABLE == 0) */
#if (EQ_SECTION_MAX < 10)
#undef EQ_SECTION_MAX
#define EQ_SECTION_MAX 10
#endif/* (EQ_SECTION_MAX < 10) */
#if (TCFG_USE_EQ_FILE == 0)
#undef TCFG_USE_EQ_FILE
#define TCFG_USE_EQ_FILE 1
#endif/* TCFG_USE_EQ_FILE */
#endif/* AUDIO_GAME_EFFECT_CONFIG */
/**********************************************************************************/



#if CONFIG_TWS_PAIR_MODE == CONFIG_TWS_PAIR_BY_CLICK
#define CONFIG_TWS_BY_CLICK_PAIR_WITHOUT_PAIR     /*双击按键可以配对已配对过的样机，即交叉配对 */
#ifdef CONFIG_TWS_BY_CLICK_PAIR_WITHOUT_PAIR
#define CONFIG_TWS_AUTO_PAIR_WITHOUT_UNPAIR     /* 不取消配对也可以配对新的耳机 */
#endif
#endif


#if TCFG_CHARGESTORE_ENABLE
#undef CONFIG_TWS_CHANNEL_SELECT
#define CONFIG_TWS_CHANNEL_SELECT             CONFIG_TWS_SECECT_BY_CHARGESTORE	//充电仓区分左右
#endif //TCFG_CHARGESTORE_ENABLE

#if TCFG_TEST_BOX_ENABLE && (!TCFG_CHARGESTORE_ENABLE)
#define CONFIG_TWS_SECECT_CHARGESTORE_PRIO    1 //测试盒配置左右耳优先
#else
#define CONFIG_TWS_SECECT_CHARGESTORE_PRIO    0
#endif //TCFG_TEST_BOX_ENABLE

//*********************************************************************************//
//                                 对耳电量显示方式                                //
//*********************************************************************************//

#if BT_SUPPORT_DISPLAY_BAT
#define CONFIG_DISPLAY_TWS_BAT_LOWER          1 //对耳手机端电量显示，显示低电量耳机的电量
#define CONFIG_DISPLAY_TWS_BAT_HIGHER         2 //对耳手机端电量显示，显示高电量耳机的电量
#define CONFIG_DISPLAY_TWS_BAT_LEFT           3 //对耳手机端电量显示，显示左耳的电量
#define CONFIG_DISPLAY_TWS_BAT_RIGHT          4 //对耳手机端电量显示，显示右耳的电量

#define CONFIG_DISPLAY_TWS_BAT_TYPE           CONFIG_DISPLAY_TWS_BAT_LOWER
#endif //BT_SUPPORT_DISPLAY_BAT

#define CONFIG_DISPLAY_DETAIL_BAT             0 //BLE广播显示具体的电量
#define CONFIG_NO_DISPLAY_BUTTON_ICON         1 //BLE广播不显示按键界面,智能充电仓置1

#endif //TCFG_USER_TWS_ENABLE

#ifndef CONFIG_BT_RX_BUFF_SIZE
#define CONFIG_BT_RX_BUFF_SIZE  (14 * 1024)
#endif

#ifdef CONFIG_APP_BT_ENABLE
#if TCFG_BT_SUPPORT_AAC || TCFG_BT_SUPPORT_LDAC
#define CONFIG_BT_TX_BUFF_SIZE  (5 * 1024)
#else
#define CONFIG_BT_TX_BUFF_SIZE  (4 * 1024)
#endif
#else
#if TCFG_BT_SUPPORT_AAC || TCFG_BT_SUPPORT_LDAC
#define CONFIG_BT_TX_BUFF_SIZE  (4 * 1024)
#else
#define CONFIG_BT_TX_BUFF_SIZE  (3 * 1024)
#endif
#endif


#ifndef CONFIG_NEW_BREDR_ENABLE

#if TCFG_USER_TWS_ENABLE

#ifdef CONFIG_LOCAL_TWS_ENABLE
#define CONFIG_TWS_BULK_POOL_SIZE  (4 * 1024)
#else
#define CONFIG_TWS_BULK_POOL_SIZE  (2 * 1024)
#endif
#endif
#endif


#if (CONFIG_BT_MODE != BT_NORMAL)
////bqb 如果测试3M tx buf 最好加大一点
#undef  CONFIG_BT_TX_BUFF_SIZE
#define CONFIG_BT_TX_BUFF_SIZE  (6 * 1024)

#endif

//*********************************************************************************//
//                                 电源切换配置                                    //
//*********************************************************************************//

#define PHONE_CALL_USE_LDO15	CONFIG_PHONE_CALL_USE_LDO15

//*********************************************************************************//
//                                 时钟切换配置                                    //
//*********************************************************************************//
#define BT_NORMAL_HZ	            CONFIG_BT_NORMAL_HZ
#define BT_CONNECT_HZ               CONFIG_BT_CONNECT_HZ

#define BT_A2DP_HZ	        	    CONFIG_BT_A2DP_HZ

#define BT_CALL_HZ		            CONFIG_BT_CALL_HZ
#define BT_CALL_ADVANCE_HZ          CONFIG_BT_CALL_ADVANCE_HZ
#define BT_CALL_16k_HZ	            CONFIG_BT_CALL_16k_HZ
#define BT_CALL_16k_ADVANCE_HZ      CONFIG_BT_CALL_16k_ADVANCE_HZ


#define MUSIC_DEC_FASTEST_CLOCK		CONFIG_MUSIC_DEC_FASTEST_CLOCK
#define MUSIC_DEC_FAST_CLOCK		CONFIG_MUSIC_DEC_FAST_CLOCK
#define MUSIC_DEC_CLOCK			    CONFIG_MUSIC_DEC_CLOCK
#define MUSIC_DEC_IDLE_CLOCK		CONFIG_MUSIC_IDLE_CLOCK
#define MUSIC_FSCAN_CLOCK		    CONFIG_MUSIC_FSCAN_CLOCK
#define LINEIN_CLOCK				CONFIG_LINEIN_CLOCK
#define FM_CLOCK			    	CONFIG_FM_CLOCK
#define FM_EMITTER_CLOCK	    	CONFIG_FM_EMITTER_CLOCK
#define PC_CLOCK					CONFIG_PC_CLOCK
#define RECODRD_CLOCK				CONFIG_RECORD_CLOCK
#define SPDIF_CLOCK			    	CONFIG_SPDIF_CLOCK

////////////////////////
#if TCFG_BT_SUPPORT_AAC || TCFG_BT_SUPPORT_LDAC
#define BT_A2DP_STEREO_EQ_HZ    48 * 1000000L
#else
#define BT_A2DP_STEREO_EQ_HZ    32 * 1000000L
#endif
#define BT_A2DP_AAC_HZ          48 * 1000000L
#define BT_A2DP_TWS_AAC_HZ      64 * 1000000L
#define BT_A2DP_MONO_EQ_HZ    	32 * 1000000L
#define BT_A2DP_ONLINE_EQ_HZ    48 * 1000000L

#define BT_CALL_SIMPLEX_HZ      96 * 1000000L
#ifdef CONFIG_ANS_V2
//#define BT_CALL_16k_HZ	        96 * 1000000L
//#define BT_CALL_16k_ADVANCE_HZ  120 * 1000000L
#else
//#define BT_CALL_16k_HZ	        80 * 1000000L
//#define BT_CALL_16k_ADVANCE_HZ  96 * 1000000L
#endif
#define BT_CALL_16k_SIMPLEX_HZ  120 * 1000000L
////////////////////////

#ifdef CONFIG_FPGA_ENABLE

// #undef TCFG_CLOCK_OSC_HZ
// #define TCFG_CLOCK_OSC_HZ		12000000

#endif


#ifdef CONFIG_CPU_BR26
#undef BT_CALL_16k_HZ
#undef BT_CALL_16k_ADVANCE_HZ
#define BT_CALL_16k_HZ	        96 * 1000000L
#define BT_CALL_16k_ADVANCE_HZ  96 * 1000000L
#endif

#ifdef CONFIG_CPU_BR23
#undef BT_A2DP_STEREO_EQ_HZ
#define BT_A2DP_STEREO_EQ_HZ	48 * 1000000L
#undef BT_A2DP_MONO_EQ_HZ
#define BT_A2DP_MONO_EQ_HZ    	48 * 1000000L
#endif

#ifdef CONFIG_CPU_BR25
#undef BT_A2DP_STEREO_EQ_HZ
#define BT_A2DP_STEREO_EQ_HZ	48 * 1000000L
#undef BT_A2DP_MONO_EQ_HZ
#define BT_A2DP_MONO_EQ_HZ    	48 * 1000000L
#endif

#ifdef CONFIG_FPGA_ENABLE

// #undef TCFG_CLOCK_OSC_HZ
// #define TCFG_CLOCK_OSC_HZ		12000000

#undef  TCFG_MC_BIAS_AUTO_ADJUST
#define TCFG_MC_BIAS_AUTO_ADJUST	 MC_BIAS_ADJUST_DISABLE

#endif
//*********************************************************************************//
//                                  低功耗配置                                     //
//*********************************************************************************//
#if TCFG_IRKEY_ENABLE
#undef  TCFG_LOWPOWER_LOWPOWER_SEL
#define TCFG_LOWPOWER_LOWPOWER_SEL			0                     //开红外不进入低功耗
#endif  /* #if TCFG_IRKEY_ENABLE */

//*********************************************************************************//
//                            LED使用 16SLOT TIMER 同步                            //
//*********************************************************************************//
//LED模块使用slot timer同步使用注意点:
//	1.soundbox不开该功能, 原因: 默认打开osc时钟, 使用原来的osc流程同步即可
//	2.带sd卡earphone不开该功能, 一般为单耳, 不需要同步, 使用原来的流程(lrc)
//	3.一般用在tws应用中, 而且默认关闭osc;
#if TCFG_USER_TWS_ENABLE
#define TCFG_PWMLED_USE_SLOT_TIME			ENABLE_THIS_MOUDLE
#endif

//*********************************************************************************//
//                                 升级配置                                        //
//*********************************************************************************//
//升级LED显示使能
#define UPDATE_LED_REMIND
//升级提示音使能
#define UPDATE_VOICE_REMIND

#ifndef CONFIG_UPDATE_JUMP_TO_MASK
#define CONFIG_UPDATE_JUMP_TO_MASK 	0
#endif

#if CONFIG_UPDATE_JUMP_TO_MASK
//升级IO保持使能
#define DEV_UPDATE_SUPPORT_JUMP           //目前只有br23\br25支持
#endif


#if TCFG_APP_MUSIC_EN
#define CONFIG_SD_UPDATE_ENABLE
#define CONFIG_USB_UPDATE_ENABLE
#endif
//*********************************************************************************//
//                                 Audio配置                                        //
//*********************************************************************************//

#if TCFG_AUDIO_ANC_ENABLE
#if ((defined VOL_TYPE_ANALOG) && (SYS_VOL_TYPE == VOL_TYPE_ANALOG)) || \
	((defined VOL_TYPE_AD) && (SYS_VOL_TYPE == VOL_TYPE_AD))
#error	"ANC can not use VOL_TYPE_ANALOG and VOL_TYPE_AD!!!"
#endif/*(SYS_VOL_TYPE = VOL_TYPE_ANALOG) || (SYS_VOL_TYPE = VOL_TYPE_AD)*/
#endif/*TCFG_AUDIO_ANC_ENABLE*/

/*通话语音处理算法放在.data段*/
#ifndef TCFG_AUDIO_CVP_CODE_AT_RAM
#define TCFG_AUDIO_CVP_CODE_AT_RAM		1
#endif/*TCFG_AUDIO_CVP_CODE_AT_RAM*/

/*AAC解码算法放在.data段*/
#ifndef TCFG_AUDIO_AAC_CODE_AT_RAM
#define TCFG_AUDIO_AAC_CODE_AT_RAM		1
#endif/*TCFG_AUDIO_AAC_CODE_AT_RAM*/

#if (TCFG_AUDIO_DUAL_MIC_ENABLE && TCFG_AUDIO_TRIPLE_MIC_ENABLE)
#error "TCFG_AUDIO_DUAL_MIC_ENABLE and TCFG_AUDIO_TRIPLE_MIC_ENABLE can not enable together !!!"
#endif

//*********************************************************************************//
//                    充电中按键清除手机配对信息配置                               //
//*********************************************************************************//

#define CHARGING_CLEAN_PHONE_INFO       0



#endif
