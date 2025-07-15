#ifndef BANK_SWITCH_H
#define BANK_SWITCH_H

#ifdef CONFIG_CODE_BANK_ENABLE
#define _BANK_ENTRY(num)   __attribute__((section(".bank.code."#num))) __attribute__((banknum(num)))
#define __BANK_ENTRY(num)  _BANK_ENTRY(num)
#define _BANK_NUM(num)     __attribute__((section(".bank.code."#num))) __attribute__((banknum(num)))
#define __BANK_NUM(num)   _BANK_NUM(num)
#define __BANK_COMMON()   __attribute__((section(".common")))
#else
#define __BANK_ENTRY(num)
#define __BANK_NUM(num)
#endif


#ifdef CONFIG_BANK_COMM
#define __BANK_EDR_RX         __BANK_COMMON()
#define __BANK_EDR_TX         __BANK_COMMON()
#define __BANK_TWS_LINK       __BANK_COMMON()
#define __BANK_EDR_FRAME      __BANK_COMMON()
#else
#define __BANK_EDR_RX
#define __BANK_EDR_TX
#define __BANK_TWS_LINK
#define __BANK_EDR_FRAME
#endif


#ifdef CONFIG_BANK_NUM_INIT
#define __BANK_INIT_ENTRY   __BANK_ENTRY(CONFIG_BANK_NUM_INIT)
#define __BANK_INIT         __BANK_NUM(CONFIG_BANK_NUM_INIT)
#else
#define __BANK_INIT_ENTRY
#define __BANK_INIT
#endif

#ifdef CONFIG_BANK_NUM_RF
#define __BANK_RF_ENTRY   __BANK_ENTRY(CONFIG_BANK_NUM_RF)
#define __BANK_RF         __BANK_NUM(CONFIG_BANK_NUM_RF)
#else
#define __BANK_RF_ENTRY
#define __BANK_RF
#endif


#ifdef CONFIG_BANK_NUM_RF_TRIM
#define __BANK_RF_TRIM         __BANK_NUM(CONFIG_BANK_NUM_RF_TRIM)
#else
#define __BANK_RF_TRIM
#endif

#ifdef CONFIG_BANK_NUM_DUT
#define __BANK_DUT_ENTRY   __BANK_ENTRY(CONFIG_BANK_NUM_DUT)
#define __BANK_DUT         __BANK_NUM(CONFIG_BANK_NUM_DUT)
#else
#define __BANK_DUT_ENTRY
#define __BANK_DUT
#endif

#ifdef CONFIG_BANK_NUM_ECDH
#define __BANK_ECDH_ENTRY   __BANK_ENTRY(CONFIG_BANK_NUM_ECDH)
#define __BANK_ECDH         __BANK_NUM(CONFIG_BANK_NUM_ECDH)
#else
#define __BANK_ECDH_ENTRY
#define __BANK_ECDH
#endif

#ifdef CONFIG_BANK_NUM_ENC
#define __BANK_ENC_ENTRY   __BANK_ENTRY(CONFIG_BANK_NUM_ENC)
#define __BANK_ENC         __BANK_NUM(CONFIG_BANK_NUM_ENC)
#else
#define __BANK_ENC_ENTRY
#define __BANK_ENC
#endif

#ifdef CONFIG_BANK_NUM_A2DP
#define __BANK_A2DP_ENTRY   __BANK_ENTRY(CONFIG_BANK_NUM_A2DP)
#define __BANK_A2DP         __BANK_NUM(CONFIG_BANK_NUM_A2DP)
#else
#define __BANK_A2DP_ENTRY
#define __BANK_A2DP
#endif

#ifdef CONFIG_BANK_NUM_AVCTP
#define __BANK_AVCTP_ENTRY   __BANK_ENTRY(CONFIG_BANK_NUM_AVCTP)
#define __BANK_AVCTP         __BANK_NUM(CONFIG_BANK_NUM_AVCTP)
#else
#define __BANK_AVCTP_ENTRY
#define __BANK_AVCTP
#endif


#ifdef CONFIG_BANK_NUM_RFCOMM
#define __BANK_RFCOMM_ENTRY   __BANK_ENTRY(CONFIG_BANK_NUM_RFCOMM)
#define __BANK_RFCOMM         __BANK_NUM(CONFIG_BANK_NUM_RFCOMM)
#else
#define __BANK_RFCOMM_ENTRY
#define __BANK_RFCOMM
#endif


#ifdef CONFIG_BANK_NUM_SDP
#define __BANK_SDP_ENTRY   __BANK_ENTRY(CONFIG_BANK_NUM_SDP)
#define __BANK_SDP         __BANK_NUM(CONFIG_BANK_NUM_SDP)
#else
#define __BANK_SDP_ENTRY
#define __BANK_SDP
#endif

#ifdef CONFIG_BANK_NUM_BT_HID
#define __BANK_BT_HID_ENTRY   __BANK_ENTRY(CONFIG_BANK_NUM_BT_HID)
#define __BANK_BT_HID         __BANK_NUM(CONFIG_BANK_NUM_BT_HID)
#else
#define __BANK_BT_HID_ENTRY
#define __BANK_BT_HID
#endif

#ifdef CONFIG_BANK_NUM_BLE
#define __BANK_BLE_ENTRY   __BANK_ENTRY(CONFIG_BANK_NUM_BLE)
#define __BANK_BLE         __BANK_NUM(CONFIG_BANK_NUM_BLE)
#else
#define __BANK_BLE_ENTRY
#define __BANK_BLE
#endif

#ifdef CONFIG_BANK_NUM_TWS_BLE
#define __BANK_TWS_BLE_ENTRY   __BANK_ENTRY(CONFIG_BANK_NUM_TWS_BLE)
#define __BANK_TWS_BLE         __BANK_NUM(CONFIG_BANK_NUM_TWS_BLE)
#else
#define __BANK_TWS_BLE_ENTRY
#define __BANK_TWS_BLE
#endif

#ifdef CONFIG_BANK_NUM_TONE
#define __BANK_TONE_ENTRY   __BANK_ENTRY(CONFIG_BANK_NUM_TONE)
#define __BANK_TONE         __BANK_NUM(CONFIG_BANK_NUM_TONE)
#else
#define __BANK_TONE_ENTRY
#define __BANK_TONE
#endif

#ifdef CONFIG_BANK_NUM_LMP_SLAVE
#define __BANK_LMP_SLAVE_ENTRY   __BANK_ENTRY(CONFIG_BANK_NUM_LMP_SLAVE)
#define __BANK_LMP_SLAVE         __BANK_NUM(CONFIG_BANK_NUM_LMP_SLAVE)
#else
#define __BANK_LMP_SLAVE_ENTRY
#define __BANK_LMP_SLAVE
#endif

#ifdef CONFIG_BANK_NUM_LMP_MASTER
#define __BANK_LMP_MASTER_ENTRY   __BANK_ENTRY(CONFIG_BANK_NUM_LMP_MASTER)
#define __BANK_LMP_MASTER         __BANK_NUM(CONFIG_BANK_NUM_LMP_MASTER)
#else
#define __BANK_LMP_MASTER_ENTRY
#define __BANK_LMP_MASTER
#endif




#ifdef CONFIG_BANK_NUM_CLOCK
#define __BANK_CLOCK_ENTRY   __BANK_ENTRY(CONFIG_BANK_NUM_CLOCK)
#define __BANK_CLOCK         __BANK_NUM(CONFIG_BANK_NUM_CLOCK)
#else
#define __BANK_CLOCK_ENTRY
#define __BANK_CLOCK
#endif



void load_overlay_code(int num);
void bank_syscall_entry(void);
void load_common_code(void);



#endif

