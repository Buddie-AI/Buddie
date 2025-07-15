#ifndef _LP_TOUCH_KEY_HW_H_
#define _LP_TOUCH_KEY_HW_H_

/**************************************************************P11 通讯定义*****************************************************************/

enum CTMU_P2M_EVENT {
    CTMU_P2M_CH0_RES_EVENT = 0x50,
    CTMU_P2M_CH0_SHORT_KEY_EVENT,
    CTMU_P2M_CH0_LONG_KEY_EVENT,
    CTMU_P2M_CH0_HOLD_KEY_EVENT,
    CTMU_P2M_CH0_FALLING_EVENT,
    CTMU_P2M_CH0_RAISING_EVENT,

    CTMU_P2M_CH1_RES_EVENT = 0x58,
    CTMU_P2M_CH1_SHORT_KEY_EVENT,
    CTMU_P2M_CH1_LONG_KEY_EVENT,
    CTMU_P2M_CH1_HOLD_KEY_EVENT,
    CTMU_P2M_CH1_FALLING_EVENT,
    CTMU_P2M_CH1_RAISING_EVENT,

    CTMU_P2M_CH2_RES_EVENT = 0x60,
    CTMU_P2M_CH2_SHORT_KEY_EVENT,
    CTMU_P2M_CH2_LONG_KEY_EVENT,
    CTMU_P2M_CH2_HOLD_KEY_EVENT,
    CTMU_P2M_CH2_FALLING_EVENT,
    CTMU_P2M_CH2_RAISING_EVENT,

    CTMU_P2M_CH3_RES_EVENT = 0x68,
    CTMU_P2M_CH3_SHORT_KEY_EVENT,
    CTMU_P2M_CH3_LONG_KEY_EVENT,
    CTMU_P2M_CH3_HOLD_KEY_EVENT,
    CTMU_P2M_CH3_FALLING_EVENT,
    CTMU_P2M_CH3_RAISING_EVENT,

    CTMU_P2M_CH4_RES_EVENT = 0x70,
    CTMU_P2M_CH4_SHORT_KEY_EVENT,
    CTMU_P2M_CH4_LONG_KEY_EVENT,
    CTMU_P2M_CH4_HOLD_KEY_EVENT,
    CTMU_P2M_CH4_FALLING_EVENT,
    CTMU_P2M_CH4_RAISING_EVENT,

    CTMU_P2M_EARTCH_IN_EVENT = 0x78,
    CTMU_P2M_EARTCH_OUT_EVENT,
};

enum CTMU_M2P_CMD {
    CTMU_M2P_INIT = 0x50,
    CTMU_M2P_DISABLE, 		//模块关闭
    CTMU_M2P_ENABLE,  		//模块使能
    CTMU_M2P_CH0_ENABLE, 	//通道0打开
    CTMU_M2P_CH0_DISABLE, 	//通道0关闭
    CTMU_M2P_CH1_ENABLE, 	//通道1打开
    CTMU_M2P_CH1_DISABLE, 	//通道1关闭
    CTMU_M2P_CH2_ENABLE, 	//通道2打开
    CTMU_M2P_CH2_DISABLE, 	//通道2关闭
    CTMU_M2P_CH3_ENABLE, 	//通道3打开
    CTMU_M2P_CH3_DISABLE, 	//通道3关闭
    CTMU_M2P_CH4_ENABLE, 	//通道4打开
    CTMU_M2P_CH4_DISABLE, 	//通道4关闭
    CTMU_M2P_UPDATE_BASE_TIME, 	//更新时基参数
    CTMU_M2P_CHARGE_ENTER_MODE, //进仓充电模式
    CTMU_M2P_CHARGE_EXIT_MODE,  //退出充电模式
    CTMU_M2P_RESET_ALGO,        //单独复位触摸算法
};

/////////////////////////////////////////////////
//	P11:  P2M_MESSAGE_CTMU_WKUP_MSG
//	MSYS: P2M_CTMU_CTMU_WKUP_MSG
//	消息列表
//	作用: 与主系统同步消息, 缓解异步问题
/////////////////////////////////////////////////
#define P2M_MESSAGE_POWER_ON_FLAG 		BIT(0) //lpctmu长按开机标志, 0: 非长按开机, 1: 长按开机, P11/MSYS清0, P11置位, 主系统查询
#define P2M_MESSAGE_SYNC_FLAG 			BIT(1) //主系统清0, P11置位, 主系统查询置位, 解决异步问题
#define P2M_MESSAGE_KEY_ACTIVE_FLAG 	BIT(2) //lpctmu按键状态, 1: 按键按下, 0: 按键抬起, P11清0, P11置位, 主系统查询
#define P2M_MESSAGE_INIT_MODE_FLAG 		BIT(3) //lpctmu初始化模式, 0: 普通初始化(复位硬件,复位算法,初始化定时器), 1: 连续工作初始化(恢复定时器工作), 主系统清0, 主系统置位, P11查询
#define P2M_MESSAGE_RESET_ALGO_FLAG     BIT(4) //主系统清0, P11置位, 主系统查询置位，用于内置触摸算法的复位


//=======================================================================//
//                             LPCTMU ANA0                               //
//=======================================================================//
//------------------- 上限电压配置: P11_LPCTM->ANA0[7:6]
/*
上限电压表:
	0: 0.65V
	1: 0.70V
	2: 0.75V
	3: 0.80V
*/
enum LPCTM_VH_TABLE {
    LPCTMU_VH_065V = (0 << 6),
    LPCTMU_VH_070V = (1 << 6),
    LPCTMU_VH_075V = (2 << 6),
    LPCTMU_VH_080V = (3 << 6),
};

//------------------- 下限电压配置: P11_LPCTM->ANA0[5:4]
/*
下限电压表:
	0: 0.20V
	1: 0.25V
	2: 0.30V
	3: 0.35V
*/
enum LPCTM_VL_TABLE {
    LPCTMU_VL_020V = (0 << 4),
    LPCTMU_VL_025V = (1 << 4),
    LPCTMU_VL_030V = (2 << 4),
    LPCTMU_VL_035V = (3 << 4),
};

//------------------- 充放电电流配置: P11_LPCTM->ANA0[3:1]
/*
充放电电流表:
	0: 3.6  uA
	1: 7.2  uA
	2: 10.8 uA
	3: 14.4 uA
	4: 18.2 uA
	5: 21.6 uA
	6: 25.2 uA
	7: 28.8 uA
*/
enum LPCTM_ISEL_TABLE {
    LPCTMU_ISEL_036UA = (0 << 1),
    LPCTMU_ISEL_072UA = (1 << 1),
    LPCTMU_ISEL_108UA = (2 << 1),
    LPCTMU_ISEL_144UA = (3 << 1),
    LPCTMU_ISEL_180UA = (4 << 1),
    LPCTMU_ISEL_216UA = (5 << 1),
    LPCTMU_ISEL_252UA = (6 << 1),
    LPCTMU_ISEL_288UA = (7 << 1)
};

u8 get_lpctmu_ana_level(void);

#define LPCTMU_ANA0_CONFIG(x) 			(P11_LPCTM->ANA0 = x)

/**********************************************************算法流程配置**********************************************************************************/
#define CTMU_SAMPLE_RATE_PRD 			20 //kick start采样周期, 单位: ms

#define CTMU_SHORT_CLICK_DELAY_TIME 	400 	//单击事件后等待下一次单击时间(ms)
#define CTMU_HOLD_CLICK_DELAY_TIME 		200 	//long事件产生后, 发hold事件间隔(ms)
#define CTMU_LONG_KEY_DELAY_TIME 		2000 	//从按下到产生long事件的时间(ms)

#endif /* #ifndef _LP_TOUCH_KEY_HW_H_ */


