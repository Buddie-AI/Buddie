#ifndef __LP_IPC_H__
#define __LP_IPC_H__

//=================================消息格式========================================

//消息buf大小
#define MAX_POOL		    128

//消息类型
#define NO_MSG				0xff

//获取消息返回值
enum {
    MSG_NO_ERROR = 0,
    MSG_NO_MSG = 0,
    MSG_EVENT_EXIST = -1,
    MSG_NOT_EVENT = -2,
    MSG_EVENT_PARAM_ERROR = -3,
    MSG_BUF_NOT_ENOUGH = -4,
    MSG_CBUF_ERROR = -5,
};

//消息头格式
#define MSG_HEADER_BYTE_LEN     3
#define MSG_HEADER_BIT_LEN     (MSG_HEADER_BYTE_LEN*8)
#define MSG_HEADER_ALL_BIT     ((1L<<MSG_HEADER_BIT_LEN) - 1)

#define MSG_INDEX_BIT			7
#define MSG_ACK_BIT  			1

#define MSG_TYPE_BIT_LEN        12
#define MSG_PARAM_BIT_LEN       (MSG_HEADER_BYTE_LEN*8-MSG_TYPE_BIT_LEN-MSG_INDEX_BIT-MSG_ACK_BIT)

//=================================M2P===========================================
//M2P中断索引
enum {
    M2P_LP_INDEX    = 0,
    M2P_PF_INDEX,
    M2P_LLP_INDEX,
    M2P_P33_INDEX,
    M2P_SF_INDEX,
    M2P_CTMU_INDEX,
    M2P_CCMD_INDEX,       //common cmd
    M2P_VAD_INDEX,
    M2P_USER_INDEX,
    M2P_WDT_INDEX,
    M2P_SYNC_INDEX,
    M2P_APP_INDEX,
};

#define M2P_LRC_PRD                                     M2P_MESSAGE_ACCESS(0x000)
#define M2P_WDVDD                                		M2P_MESSAGE_ACCESS(0x001)
#define M2P_LRC_TMR_50us                                M2P_MESSAGE_ACCESS(0x002)
#define M2P_LRC_TMR_200us                               M2P_MESSAGE_ACCESS(0x003)
#define M2P_LRC_TMR_600us                               M2P_MESSAGE_ACCESS(0x004)
#define M2P_VDDIO_KEEP                                  M2P_MESSAGE_ACCESS(0x005)
#define M2P_LRC_KEEP                                    M2P_MESSAGE_ACCESS(0x006)
#define M2P_SYNC_CMD                                    M2P_MESSAGE_ACCESS(0x007)
#define M2P_MESSAGE_VAD_CMD                             M2P_MESSAGE_ACCESS(0x008)
#define M2P_MESSAGE_VAD_CBUF_RPTR                       M2P_MESSAGE_ACCESS(0x009)
#define M2P_VDDIO_KEEP_TYPE                             M2P_MESSAGE_ACCESS(0x00a)
#define M2P_RCH_FEQ_H                                   M2P_MESSAGE_ACCESS(0x00b)
#define M2P_MEM_CONTROL                                 M2P_MESSAGE_ACCESS(0x00c)
#define M2P_BTOSC_KEEP                                  M2P_MESSAGE_ACCESS(0x00d)
#define M2P_CTMU_KEEP									M2P_MESSAGE_ACCESS(0x00e)
#define M2P_RTC_KEEP                                    M2P_MESSAGE_ACCESS(0x00f)
#define M2P_WDT_SYNC                                   	M2P_MESSAGE_ACCESS(0x010)
#define M2P_LIGHT_PDOWN_DVDD_VOL                        M2P_MESSAGE_ACCESS(0x011)
#define M2P_PVDD_LEVEL_SLEEP_TRIM						M2P_MESSAGE_ACCESS(0x012)


/*触摸所有通道配置*/
#define M2P_CTMU_CMD  									M2P_MESSAGE_ACCESS(0x18)
#define M2P_CTMU_MSG                                    M2P_MESSAGE_ACCESS(0x19)
#define M2P_CTMU_PRD0               		            M2P_MESSAGE_ACCESS(0x1a)
#define M2P_CTMU_PRD1                                   M2P_MESSAGE_ACCESS(0x1b)
#define M2P_CTMU_CH_ENABLE								M2P_MESSAGE_ACCESS(0x1c)
#define M2P_CTMU_CH_DEBUG								M2P_MESSAGE_ACCESS(0x1d)
#define M2P_CTMU_CH_CFG						            M2P_MESSAGE_ACCESS(0x1e)
#define M2P_CTMU_CH_WAKEUP_EN					        M2P_MESSAGE_ACCESS(0x1f)
#define M2P_CTMU_EARTCH_CH						        M2P_MESSAGE_ACCESS(0x20)
#define M2P_CTMU_TIME_BASE          					M2P_MESSAGE_ACCESS(0x21)

#define M2P_CTMU_LONG_TIMEL                             M2P_MESSAGE_ACCESS(0x22)
#define M2P_CTMU_LONG_TIMEH                             M2P_MESSAGE_ACCESS(0x23)
#define M2P_CTMU_HOLD_TIMEL                             M2P_MESSAGE_ACCESS(0x24)
#define M2P_CTMU_HOLD_TIMEH                             M2P_MESSAGE_ACCESS(0x25)
#define M2P_CTMU_SOFTOFF_LONG_TIMEL                     M2P_MESSAGE_ACCESS(0x26)
#define M2P_CTMU_SOFTOFF_LONG_TIMEH                     M2P_MESSAGE_ACCESS(0x27)
#define M2P_CTMU_LONG_PRESS_RESET_TIME_VALUE_L  		M2P_MESSAGE_ACCESS(0x28)//长按复位
#define M2P_CTMU_LONG_PRESS_RESET_TIME_VALUE_H  		M2P_MESSAGE_ACCESS(0x29)//长按复位

#define M2P_CTMU_INEAR_VALUE_L                          M2P_MESSAGE_ACCESS(0x2a)
#define M2P_CTMU_INEAR_VALUE_H							M2P_MESSAGE_ACCESS(0x2b)
#define M2P_CTMU_OUTEAR_VALUE_L                         M2P_MESSAGE_ACCESS(0x2c)
#define M2P_CTMU_OUTEAR_VALUE_H                         M2P_MESSAGE_ACCESS(0x2d)
#define M2P_CTMU_EARTCH_TRIM_VALUE_L                    M2P_MESSAGE_ACCESS(0x2e)
#define M2P_CTMU_EARTCH_TRIM_VALUE_H                    M2P_MESSAGE_ACCESS(0x2f)

#define M2P_MASSAGE_CTMU_CH0_CFG0L                                         0x30
#define M2P_MASSAGE_CTMU_CH0_CFG0H                                         0x31
#define M2P_MASSAGE_CTMU_CH0_CFG1L                                         0x32
#define M2P_MASSAGE_CTMU_CH0_CFG1H                                         0x33
#define M2P_MASSAGE_CTMU_CH0_CFG2L                                         0x34
#define M2P_MASSAGE_CTMU_CH0_CFG2H                                         0x35

#define M2P_CTMU_CH0_CFG0L                              M2P_MESSAGE_ACCESS(0x30)
#define M2P_CTMU_CH0_CFG0H                              M2P_MESSAGE_ACCESS(0x31)
#define M2P_CTMU_CH0_CFG1L                              M2P_MESSAGE_ACCESS(0x32)
#define M2P_CTMU_CH0_CFG1H                              M2P_MESSAGE_ACCESS(0x33)
#define M2P_CTMU_CH0_CFG2L                              M2P_MESSAGE_ACCESS(0x34)
#define M2P_CTMU_CH0_CFG2H                              M2P_MESSAGE_ACCESS(0x35)

#define M2P_CTMU_CH1_CFG0L                              M2P_MESSAGE_ACCESS(0x38)
#define M2P_CTMU_CH1_CFG0H                              M2P_MESSAGE_ACCESS(0x39)
#define M2P_CTMU_CH1_CFG1L                              M2P_MESSAGE_ACCESS(0x3a)
#define M2P_CTMU_CH1_CFG1H                              M2P_MESSAGE_ACCESS(0x3b)
#define M2P_CTMU_CH1_CFG2L                              M2P_MESSAGE_ACCESS(0x3c)
#define M2P_CTMU_CH1_CFG2H                              M2P_MESSAGE_ACCESS(0x3d)

#define M2P_CTMU_CH2_CFG0L                              M2P_MESSAGE_ACCESS(0x40)
#define M2P_CTMU_CH2_CFG0H                              M2P_MESSAGE_ACCESS(0x41)
#define M2P_CTMU_CH2_CFG1L                              M2P_MESSAGE_ACCESS(0x42)
#define M2P_CTMU_CH2_CFG1H                              M2P_MESSAGE_ACCESS(0x43)
#define M2P_CTMU_CH2_CFG2L                              M2P_MESSAGE_ACCESS(0x44)
#define M2P_CTMU_CH2_CFG2H                              M2P_MESSAGE_ACCESS(0x45)

#define M2P_CTMU_CH3_CFG0L                              M2P_MESSAGE_ACCESS(0x48)
#define M2P_CTMU_CH3_CFG0H                              M2P_MESSAGE_ACCESS(0x49)
#define M2P_CTMU_CH3_CFG1L                              M2P_MESSAGE_ACCESS(0x4a)
#define M2P_CTMU_CH3_CFG1H                              M2P_MESSAGE_ACCESS(0x4b)
#define M2P_CTMU_CH3_CFG2L                              M2P_MESSAGE_ACCESS(0x4c)
#define M2P_CTMU_CH3_CFG2H                              M2P_MESSAGE_ACCESS(0x4d)

#define M2P_CTMU_CH4_CFG0L                              M2P_MESSAGE_ACCESS(0x50)
#define M2P_CTMU_CH4_CFG0H                              M2P_MESSAGE_ACCESS(0x51)
#define M2P_CTMU_CH4_CFG1L                              M2P_MESSAGE_ACCESS(0x52)
#define M2P_CTMU_CH4_CFG1H                              M2P_MESSAGE_ACCESS(0x53)
#define M2P_CTMU_CH4_CFG2L                              M2P_MESSAGE_ACCESS(0x54)
#define M2P_CTMU_CH4_CFG2H                              M2P_MESSAGE_ACCESS(0x55)
#define M2P_RVD2PVDD_EN                              	M2P_MESSAGE_ACCESS(0x56)
#define M2P_PVDD_EXTERN_DCDC                           	M2P_MESSAGE_ACCESS(0x57)

#define M2P_USER_PEND                                   (0x58)
#define M2P_USER_MSG_TYPE                               (0x59)
#define M2P_USER_MSG_LEN0                               (0x5a)
#define M2P_USER_MSG_LEN1                               (0x5b)
#define M2P_USER_ADDR0                                  (0x5c)
#define M2P_USER_ADDR1                                  (0x5d)
#define M2P_USER_ADDR2                                  (0x5e)
#define M2P_USER_ADDR3                                  (0x5f)

enum {
    CLOSE_P33_INTERRUPT = 1,
    OPEN_P33_INTERRUPT,
    LOWPOWER_PREPARE,
    M2P_SPIN_LOCK,
    M2P_SPIN_UNLOCK,
    P2M_SPIN_LOCK,
    P2M_SPIN_UNLOCK,
    M2P_WDT_CLEAR,

    P2M_RESERVED_CMD = 0xFF,
};

//=================================P2M===========================================
//P2M中断索引
enum {
    P2M_LP_INDEX    = 0,
    P2M_PF_INDEX,
    P2M_LLP_INDEX,
    P2M_WK_INDEX,
    P2M_WDT_INDEX,
    P2M_LP_INDEX2,
    P2M_CTMU_INDEX,
    P2M_CTMU_POWUP,
    P2M_REPLY_CCMD_INDEX,  //reply common cmd
    P2M_VAD_INDEX,
    P2M_USER_INDEX,
    P2M_BANK_INDEX,
    P2M_REPLY_SYNC_INDEX,
    P2M_APP_INDEX,
};

#define P2M_WKUP_SRC                                    P2M_MESSAGE_ACCESS(0x000)
#define P2M_WKUP_PND0                                   P2M_MESSAGE_ACCESS(0x001)
#define P2M_WKUP_PND1                                   P2M_MESSAGE_ACCESS(0x002)
#define P2M_REPLY_SYNC_CMD                            P2M_MESSAGE_ACCESS(0x003)
#define P2M_MESSAGE_VAD_CMD                             P2M_MESSAGE_ACCESS(0x004)
#define P2M_MESSAGE_VAD_CBUF_WPTR                       P2M_MESSAGE_ACCESS(0x005)
#define P2M_MESSAGE_BANK_ADR_L                          P2M_MESSAGE_ACCESS(0x006)
#define P2M_MESSAGE_BANK_ADR_H                          P2M_MESSAGE_ACCESS(0x007)
#define P2M_MESSAGE_BANK_INDEX                          P2M_MESSAGE_ACCESS(0x008)
#define P2M_MESSAGE_BANK_ACK                            P2M_MESSAGE_ACCESS(0x009)
#define P2M_P11_HEAP_BEGIN_ADDR_L    					P2M_MESSAGE_ACCESS(0x00A)
#define P2M_P11_HEAP_BEGIN_ADDR_H    					P2M_MESSAGE_ACCESS(0x00B)
#define P2M_P11_HEAP_SIZE_L    							P2M_MESSAGE_ACCESS(0x00C)
#define P2M_P11_HEAP_SIZE_H    							P2M_MESSAGE_ACCESS(0x00D)

#define P2M_CTMU_KEY_EVENT                              P2M_MESSAGE_ACCESS(0x010)
#define P2M_CTMU_KEY_CNT                                P2M_MESSAGE_ACCESS(0x011)
#define P2M_CTMU_WKUP_MSG                               P2M_MESSAGE_ACCESS(0x012)
#define P2M_CTMU_EARTCH_EVENT                           P2M_MESSAGE_ACCESS(0x013)

#define P2M_MASSAGE_CTMU_CH0_L_RES                                         0x014
#define P2M_MASSAGE_CTMU_CH0_H_RES                                         0x015
#define P2M_CTMU_CH0_L_RES                              P2M_MESSAGE_ACCESS(0x014)
#define P2M_CTMU_CH0_H_RES                              P2M_MESSAGE_ACCESS(0x015)
#define P2M_CTMU_CH1_L_RES                              P2M_MESSAGE_ACCESS(0x016)
#define P2M_CTMU_CH1_H_RES                              P2M_MESSAGE_ACCESS(0x017)
#define P2M_CTMU_CH2_L_RES                              P2M_MESSAGE_ACCESS(0x018)
#define P2M_CTMU_CH2_H_RES                              P2M_MESSAGE_ACCESS(0x019)
#define P2M_CTMU_CH3_L_RES                              P2M_MESSAGE_ACCESS(0x01a)
#define P2M_CTMU_CH3_H_RES                              P2M_MESSAGE_ACCESS(0x01b)
#define P2M_CTMU_CH4_L_RES                              P2M_MESSAGE_ACCESS(0x01c)
#define P2M_CTMU_CH4_H_RES                              P2M_MESSAGE_ACCESS(0x01d)

#define P2M_CTMU_EARTCH_L_IIR_VALUE                     P2M_MESSAGE_ACCESS(0x01e)
#define P2M_CTMU_EARTCH_H_IIR_VALUE                     P2M_MESSAGE_ACCESS(0x01f)
#define P2M_CTMU_EARTCH_L_TRIM_VALUE                    P2M_MESSAGE_ACCESS(0x020)
#define P2M_CTMU_EARTCH_H_TRIM_VALUE                    P2M_MESSAGE_ACCESS(0x021)
#define P2M_CTMU_EARTCH_L_DIFF_VALUE                    P2M_MESSAGE_ACCESS(0x022)
#define P2M_CTMU_EARTCH_H_DIFF_VALUE                    P2M_MESSAGE_ACCESS(0x023)

#define P2M_AWKUP_P_PND                                 P2M_MESSAGE_ACCESS(0x024)
#define P2M_AWKUP_N_PND                                 P2M_MESSAGE_ACCESS(0x025)
#define P2M_WKUP_RTC                                    P2M_MESSAGE_ACCESS(0x026)

#define P2M_CBUF_ADDR0									P2M_MESSAGE_ACCESS(0x027)
#define P2M_CBUF_ADDR1                                  P2M_MESSAGE_ACCESS(0x028)
#define P2M_CBUF_ADDR2                                  P2M_MESSAGE_ACCESS(0x029)
#define P2M_CBUF_ADDR3                                  P2M_MESSAGE_ACCESS(0x02a)

#define P2M_CBUF1_ADDR0                                 P2M_MESSAGE_ACCESS(0x02b)
#define P2M_CBUF1_ADDR1                                 P2M_MESSAGE_ACCESS(0x02c)
#define P2M_CBUF1_ADDR2                                 P2M_MESSAGE_ACCESS(0x02d)
#define P2M_CBUF1_ADDR3                                 P2M_MESSAGE_ACCESS(0x02e)

#define P2M_USER_PEND                                  (0x038)//传感器使用或者开放客户使用
#define P2M_USER_MSG_TYPE                              (0x039)
#define P2M_USER_MSG_LEN0                              (0x03a)
#define P2M_USER_MSG_LEN1                              (0x03b)
#define P2M_USER_ADDR0                                 (0x03c)
#define P2M_USER_ADDR1                                 (0x03d)
#define P2M_USER_ADDR2                                 (0x03e)
#define P2M_USER_ADDR3                                 (0x040)

#include "m2p_msg.h"
#include "p2m_msg.h"


void msys_to_p11_sync_cmd(u8 cmd);

void lp_ipc_init();

#endif
