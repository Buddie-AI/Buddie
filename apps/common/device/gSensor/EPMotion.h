/*******************************************************************************/
/**
 ******************************************************************************
 * @file    EPMotion.h
 * @author  ycwang@miramems.com
 * @version V2.0
 * @date    2019-01-31
 * @brief
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, MiraMEMS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 */
/*******************************************************************************/
#ifndef __EPMOTION_h
#define __EPMOTION_h

/*******************************************************************************
Includes
********************************************************************************/
#include "mira_std.h"

/*******************************************************************************
Macro definitions - Algorithm Build Options
********************************************************************************/
#define D_TAP_ENABLE      1
#define M_TAP_ENABLE      1
#define STEP_ENABLE       1

/*******************************************************************************
Typedef definitions
********************************************************************************/
enum {
    DEBUG_ERR = 1,
    DEBUG_MSG = 1 << 1,
    DEBUG_DATA = 1 << 2,
    DEBUG_RAW = 1 << 3,
};

typedef enum {
    PIN_NONE,
    PIN_ONE,
    PIN_TWO
} tPIN_NUM;

typedef enum {
    PIN_LEVEL_NONE,
    PIN_LEVEL_LOW,
    PIN_LEVEL_HIGH
} tPIN_LEVEL;

typedef enum {
    NO_LATCHED,
    LATCHED_250MS,
    LATCHED_500MS,
    LATCHED_1S,
    LATCHED_2S,
    LATCHED_4S,
    LATCHED_8S,
    LATCHED_1MS = 10,
    LATCHED_2MS,
    LATCHED_25MS,
    LATCHED_50MS,
    LATCHED_100MS,
    LATCHED
} tLATCH_MODE;

typedef enum {
    TEVENT_NONE,
    TEVENT_TAP_NOTIFY,
} EPMotion_EVENT;

typedef enum {
    NONE_T,
    D_TAP_T,
    M_TAP_T,
    STEP_T,
} tAlgorithm;

typedef enum {
    DISABLE_T,
    ENABLE_T
} tSwitchCmd;

struct tInt_Pin_Config {
    tPIN_NUM num;
    tPIN_LEVEL level;
};

struct tREG_Func {
    s8_m(*read)(u8_m addr, u8_m *data_m, u8_m len);
    s8_m(*write)(u8_m addr, u8_m data_m);
};

struct tDEBUG_Func {
    s32_m(*myprintf)(const char *fmt, ...);
};

struct EPMotion_op_s {
    struct tInt_Pin_Config pin;
    s8_m(*mir3da_event_handle)(EPMotion_EVENT event, u8_m data_m);
    struct tREG_Func reg;
    struct tDEBUG_Func debug;
};

/*******************************************************************************
Global variables and functions
********************************************************************************/

/*******************************************************************************
* Function Name: EPMotion_Init
* Description  : This function initializes the EPMotion.
* Arguments    : EPMotion_op_s *ops
* Return Value : 0: OK; -1: Type Error; -2: OP Error; -3: Chip Error
********************************************************************************/
s8_m EPMotion_Init(struct EPMotion_op_s *ops);

/*******************************************************************************
* Function Name: EPMotion_Tap_Set_Parma
* Description  : This function sets tap parmas.
* Arguments    :
*              threshold - set interrupt threshold 0~31 ,(threshold*125)mg
*                           default is 21(2625mg)
*              latch_mode - set interrupt latched mode ,(tLATCH_MODE)
*                           default is NO_LATCHED
*              duration - set interrupt duration 0~7(50~700)ms,
*                         default is 0(50ms)
* Return Value : None
********************************************************************************/
void EPMotion_Tap_Set_Parma(u8_m threshold, tLATCH_MODE latch_mode, u8_m duration);

#if D_TAP_ENABLE
/*******************************************************************************
* Function Name: EPMotion_Tap_Set_Filter
* Description  : This function sets filter.
* Arguments    :
*              level - set level of checking data 1~3 ,default is 1
*              ths - set the value of filter ,(ths*1)mg
* Return Value : None
********************************************************************************/
void EPMotion_D_Tap_Set_Filter(u8_m level, u16_m ths);
#endif

#if M_TAP_ENABLE
/*******************************************************************************
* Function Name: EPMotion_M_Tap_Set_Dur
* Description  : This function sets M tap duration.
* Arguments    :
*              m_dur_int - set interrupt duration 20~70,(m_dur_int*10)ms,
*                         default is 50(500ms)
*              m_dur_event - set m tap event duration 20~6000,(m_dur_event*10)ms,
*                           default is 100(1000ms)
*EPMotion_M_Tap_Set_Dur第一个参数是设置两次敲击间隔的最大时间 第二参数是连续多次敲击总的有效时间
* Return Value : None
********************************************************************************/
void EPMotion_M_Tap_Set_Dur(u8_m m_dur_int, u16_m m_dur_event);
#endif

/*******************************************************************************
* Function Name: EPMotion_Reset_Tap_INT
* Description  : This function reset tap INT.
* Arguments    : None
* Return Value : None
********************************************************************************/
void EPMotion_Reset_Tap_INT(void);

/*******************************************************************************
* Function Name: EPMotion_Resume_Tap_INT
* Description  : This function resume tap INT.
* Arguments    : None
* Return Value : None
********************************************************************************/
void EPMotion_Resume_Tap_INT(void);

#if STEP_ENABLE
/*******************************************************************************
* Function Name: EPMotion_Reset_Step
* Description  : This function reset step counter.
* Arguments    : None
* Return Value : None
********************************************************************************/
void EPMotion_Reset_Step(void);

/*******************************************************************************
* Function Name: EPMotion_Get_Step
* Description  : This function gets the step.
* Arguments    : None
* Return Value : step
********************************************************************************/
u16_m EPMotion_Get_Step(void);
#endif

/*******************************************************************************
* Function Name: EPMotion_Control
* Description  : This function initializes the xMotion.
* Arguments    : name - select which algorithm to control
                 cmd - enable/disable
* Return Value : 0->Success, -1->Init Fail£¬-2£¬No Supported
********************************************************************************/
s8_m EPMotion_Control(tAlgorithm name, tSwitchCmd cmd);

/*******************************************************************************
* Function Name: EPMotion_Process_Data
* Description  : This function runs the EPMotion Algorithm.
* Arguments    : None
* Return Value : None
********************************************************************************/
void EPMotion_Process_Data(s8_m int_active);

/*******************************************************************************
* Function Name: EPMotion_Chip_Read_XYZ
* Description  : This function reads the chip xyz.
* Arguments    : x, y, z - acc data
* Return Value : 0: OK; -1: Error
********************************************************************************/
s8_m EPMotion_Chip_Read_XYZ(s16_m *x, s16_m *y, s16_m *z);

/*******************************************************************************
* Function Name: EPMotion_Get_Version
* Description  : This function gets EPMotion version.
* Arguments    : ver - EPMotion version Num
* Return Value : None
********************************************************************************/
void EPMotion_Get_Version(u8_m *ver);

/*******************************************************************************
* Function Name: EPMotion_Chip_Power_On
* Description  : This function enables the chip.
* Arguments    : None
* Return Value : 0: OK; -1: Error
********************************************************************************/
s8_m EPMotion_Chip_Power_On(void);

/*******************************************************************************
* Function Name: EPMotion_Chip_Power_Off
* Description  : This function disables on the chip.
* Arguments    : None
* Return Value : 0: OK; -1: Error
********************************************************************************/
s8_m EPMotion_Chip_Power_Off(void);

/*******************************************************************************
* Function Name: EPMotion_Set_Debug_level
* Description  : This function sets the debug log level
* Arguments    : Log level
* Return Value : None
********************************************************************************/
void EPMotion_Set_Debug_level(u8_m level);
#endif


