/*********************************************************************************************
    *   Filename        : ll_config.h

    *   Description     : Lto 优化Macro 定义

    *   Author          : Bingquan

    *   Email           : bingquan_cai@zh-jieli.com

    *   Last modifiled  : 2018-12-19 16:12

    *   Copyright:(c)JIELI  2011-2017  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef _LL_CONFIG_H_
#define _LL_CONFIG_H_
#include <stdint.h> //for UINT64_C

/*
 *-------------------LE FEATURE SUPPORT
 *  brief : 运行时优化（LTO）下，代码空间优化；
 */

/* --- Core Spec 4.0 --- */
#define LL_FEAT_ENCRYPTION                          (UINT64_C(1) << ( 0))   /*!< Encryption supported. */
/* --- Core Spec 4.2 --- */
#define LL_FEAT_CONN_PARAM_REQ_PROC                 (UINT64_C(1) << ( 1))   /*!< Connection Parameters Request Procedure supported. */
#define LL_FEAT_EXT_REJECT_IND                      (UINT64_C(1) << ( 2))   /*!< Extended Reject Indication supported. */
#define LL_FEAT_SLV_INIT_FEAT_EXCH                  (UINT64_C(1) << ( 3))   /*!< Slave-Initiated Features Exchange supported. */
#define LL_FEAT_LE_PING                             (UINT64_C(1) << ( 4))   /*!< LE Ping supported. */
#define LL_FEAT_DATA_LEN_EXT                        (UINT64_C(1) << ( 5))   /*!< Data Length Extension supported. */
#define LL_FEAT_PRIVACY                             (UINT64_C(1) << ( 6))   /*!< LL Privacy supported. */
#define LL_FEAT_EXT_SCAN_FILT_POLICY                (UINT64_C(1) << ( 7))   /*!< Extended Scan Filter Policy supported. */
/* --- Core Spec 5.0 --- */
#define LL_FEAT_LE_2M_PHY                           (UINT64_C(1) << ( 8))   /*!< LE 2M PHY supported. */
#define LL_FEAT_STABLE_MOD_IDX_TRANSMITTER          (UINT64_C(1) << ( 9))   /*!< Stable Modulation Index - Transmitter supported. */
#define LL_FEAT_STABLE_MOD_IDX_RECEIVER             (UINT64_C(1) << (10))   /*!< Stable Modulation Index - Receiver supported. */
#define LL_FEAT_LE_CODED_PHY                        (UINT64_C(1) << (11))   /*!< LE Coded PHY supported. */
#define LL_FEAT_LE_EXT_ADV                          (UINT64_C(1) << (12))   /*!< LE Extended Advertising supported. */
#define LL_FEAT_LE_PER_ADV                          (UINT64_C(1) << (13))   /*!< LE Periodic Advertising supported. */
#define LL_FEAT_CH_SEL_2                            (UINT64_C(1) << (14))   /*!< Channel Selection Algorithm #2 supported. */
#define LL_FEAT_LE_POWER_CLASS_1                    (UINT64_C(1) << (15))   /*!< LE Power Class 1 supported. */
#define LL_FEAT_MIN_NUM_USED_CHAN                   (UINT64_C(1) << (16))   /*!< Minimum Number of Used Channels supported. */
/* --- Core Spec 5.1 --- */
#define LL_FEAT_CONN_CTE_REQ                        (UINT64_C(1) << (17))   /*!< Connection CTE Request supported */
#define LL_FEAT_CONN_CTE_RSP                        (UINT64_C(1) << (18))   /*!< Connection CTE Response supported */
#define LL_FEAT_CONNLESS_CTE_TRANS                  (UINT64_C(1) << (19))   /*!< Connectionless CTE Transmitter supported */
#define LL_FEAT_CONNLESS_CTE_RECV                   (UINT64_C(1) << (20))   /*!< Connectionless CTE Receiver supported */
#define LL_FEAT_ANTENNA_SWITCH_AOD                  (UINT64_C(1) << (21))   /*!< Anetenna Switching during CTE Transmission (AoD) supported */
#define LL_FEAT_ANTENNA_SWITCH_AOA                  (UINT64_C(1) << (22))   /*!< Anetenna Switching during CTE Reception (AoA) supported */
#define LL_FEAT_RECV_CTE                            (UINT64_C(1) << (23))   /*!< Receive Constant Tone Extension supported */
#define LL_FEAT_PAST_SENDER                         (UINT64_C(1) << (24))   /*!< Periodic Advertising Sync Transfer – Sender supported. */
#define LL_FEAT_PAST_RECIPIENT                      (UINT64_C(1) << (25))   /*!< Periodic Advertising Sync Transfer – Recipient supported. */
#define LL_FEAT_SCA_UPDATE                          (UINT64_C(1) << (26))   /*!< Sleep Clock Accuracy Updates supported. */
#define LL_FEAT_REMOTE_PUB_KEY_VALIDATION           (UINT64_C(1) << (27))   /*!< Remote Public Key Validation supported. */
/* --- Core Spec 5.2 --- */
#define LL_FEAT_CIS_MASTER_ROLE                     (UINT64_C(1) << (28))   /*!< Connected Isochronous Stream Master Role supported. */
#define LL_FEAT_CIS_SLAVE_ROLE                      (UINT64_C(1) << (29))   /*!< Connected Isochronous Stream Slave Role supported. */
#define LL_FEAT_ISO_BROADCASTER                     (UINT64_C(1) << (30))   /*!< Isochronous Broadcaster Role supported. */
#define LL_FEAT_ISO_SYNC                            (UINT64_C(1) << (31))   /*!< Isochronous Synchronizer Role supported. */
#define LL_FEAT_ISO_HOST_SUPPORT                    (UINT64_C(1) << (32))   /*!< Host support for ISO Channels. */
#define LL_FEAT_POWER_CONTROL_REQUEST               (UINT64_C(1) << (33))   /*!< Power control requests supported. */
#define LL_FEAT_POWER_CHANGE_IND                    (UINT64_C(1) << (34))   /*!< Power control power change indication supported. */
#define LL_FEAT_PATH_LOSS_MONITOR                   (UINT64_C(1) << (35))   /*!< Path loss monitoring supported. */

#define LL_FEAT_PATH_LOSS_MONITOR                   (UINT64_C(1) << (35))   /*!< Path loss monitoring supported. */
#define LL_FEAT_PERIODIC_ADV_ADI_SUPPORT            (UINT64_C(1) << (36))   /*!< Periodic Advertising ADI supported. */
#define LL_FEAT_CONN_SUBRATE                        (UINT64_C(1) << (37))   /*!< Connection subrating supported. */
#define LL_FEAT_CONN_SUBRATE_HOST_SUPPORT           (UINT64_C(1) << (38))   /*!< Connection subratingHost supported. */
#define LL_FEAT_CHANNEL_CLASSIFICATION              (UINT64_C(1) << (39))   /*!< Channel classification supported. */

#define LE_ENCRYPTION                       LL_FEAT_ENCRYPTION
#define CONNECTION_PARAMETER_REQUEST        LL_FEAT_CONN_PARAM_REQ_PROC
#define EXTENDED_REJECT_INDICATION          LL_FEAT_EXT_REJECT_IND
#define LE_SLAVE_INIT_FEATURES_EXCHANGE     LL_FEAT_SLV_INIT_FEAT_EXCH
#define LE_PING                             LL_FEAT_LE_PING
#define LE_DATA_PACKET_LENGTH_EXTENSION     LL_FEAT_DATA_LEN_EXT
#define LL_PRIVACY                          LL_FEAT_PRIVACY
#define EXTENDED_SCANNER_FILTER_POLICIES    LL_FEAT_EXT_SCAN_FILT_POLICY
#define LE_2M_PHY                           LL_FEAT_LE_2M_PHY
#define LE_CODED_PHY                        LL_FEAT_LE_CODED_PHY
#define LE_EXTENDED_ADVERTISING             LL_FEAT_LE_EXT_ADV
#define LE_PERIODIC_ADVERTISING             LL_FEAT_LE_PER_ADV
#define CHANNEL_SELECTION_ALGORITHM_2       LL_FEAT_CH_SEL_2


#define LE_CORE_V50_FEATURES \
( \
    LE_2M_PHY | \
    LE_CODED_PHY | \
    LE_EXTENDED_ADVERTISING | \
    LE_PERIODIC_ADVERTISING | \
    CHANNEL_SELECTION_ALGORITHM_2 | \
    0 \
)

#if (LE_CORE_V50_FEATURES & LE_PERIODIC_ADVERTISING)
#if ((LE_CORE_V50_FEATURES & LE_EXTENDED_ADVERTISING) == 0)
#error "enable <LE_PERIODIC_ADVERTISING> must enable <LE_EXTENDED_ADVERTISING> at the same time"
#endif
#endif

#define LE_FEATURES_CONST_TONE          (LL_FEAT_CONN_CTE_REQ | \
                                            LL_FEAT_CONN_CTE_RSP | \
                                            LL_FEAT_CONNLESS_CTE_TRANS | \
                                            LL_FEAT_CONNLESS_CTE_RECV | \
                                            LL_FEAT_ANTENNA_SWITCH_AOD | \
                                            LL_FEAT_ANTENNA_SWITCH_AOA | \
                                            LL_FEAT_RECV_CTE)

#define LE_FEATURES_PAST                (LL_FEAT_PAST_SENDER | \
                                            LL_FEAT_PAST_RECIPIENT)

#define LE_FEATURES_CIS                 (LL_FEAT_CIS_MASTER_ROLE | \
                                            LL_FEAT_CIS_SLAVE_ROLE | \
                                            LL_FEAT_ISO_HOST_SUPPORT)

#define LE_FEATURES_BIS                 (LL_FEAT_ISO_BROADCASTER | \
                                            LL_FEAT_ISO_SYNC | \
                                            LL_FEAT_ISO_HOST_SUPPORT)

#define LE_FEATURES_ISO                 (LE_FEATURES_BIS|LE_FEATURES_CIS)

#define LE_FEATURES_POWER_CONTROL       (LL_FEAT_POWER_CONTROL_REQUEST | \
                                            LL_FEAT_POWER_CHANGE_IND | \
                                            LL_FEAT_PATH_LOSS_MONITOR)

extern const uint64_t config_btctler_le_features;
#define LE_FEATURES_IS_SUPPORT(x)           (config_btctler_le_features & (x))

#define LE_FEATURES_IS_SUPPORT_OPTIMIZE(x)  if (LE_FEATURES_IS_SUPPORT(x) == 0x0)   return
/*-----------------------------------------------------------*/

/*
 *-------------------LE ROLES SUPPORT
 *  brief : 运行时优化（LTO）下，代码空间优化；
 */
#define LE_MASTER                           BIT(0)
#define LE_SLAVE                            BIT(1)
#define LE_ADV                              BIT(2)
#define LE_SCAN                             BIT(3)
#define LE_INIT                             BIT(4)

/*! \brief      Combination */
#define LE_CONN                             (LE_MASTER|LE_SLAVE)

extern const int config_btctler_le_roles;
#define LE_ROLES_IS_SUPPORT(x)              (config_btctler_le_roles & x)

#define LE_ROLES_IS_SUPPORT_OPTIMIZE(x)     if (LE_ROLES_IS_SUPPORT(x) == 0x0)   return
/*-----------------------------------------------------------*/

extern const int config_btctler_le_tws;
#define LE_TWS_IS_SUPPORT()                (config_btctler_le_tws)


/*-----------------------------------------------------------*/

extern const int config_btctler_le_afh_en;
#define LE_AFH_IS_SUPPORT()                (config_btctler_le_afh_en)

#define LE_AFH_IS_SUPPORT_OPTIMIZE(x)     if (LE_AFH_IS_SUPPORT() == 0x0)   return

/*
 *-------------------LE PARAM CHECK
 *  brief : 运行时优化（LTO）下，代码空间优化；
 */
// extern const int config_btctler_le_param_check;
#define LE_PARAM_IS_CHECK()                TRUE//(config_btctler_le_param_check)
/*
 *-------------------LE RAM CONTROL
 *
 */
extern const int config_btctler_le_hw_nums;
extern const int config_btctler_le_rx_nums;
extern const int config_btctler_le_acl_packet_length;
extern const int config_btctler_le_acl_total_nums;
extern const int config_btctler_le_slave_conn_update_winden;
/*-----------------------------------------------------------*/

/*
 *-------------------LE Multi-link CONTROL
 */
extern const int config_btctler_le_master_multilink;
/*-----------------------------------------------------------*/

/*
 *-------------------LE Vendor baseband CONTROL
 */
#define VENDOR_BB_PIS_EN                BIT(0)
#define VENDOR_BB_PIS_HB                BIT(1)
#define VENDOR_BB_PIS_HB_M              BIT(2)
#define VENDOR_BB_MD_CLOSE              BIT(3)
#define VENDOR_BB_CONNECT_SLOT          BIT(4)
#define VENDOR_BB_NEW_SCAN_STRATEGY		BIT(5)
#define VENDOR_BB_PIS_HB_R              BIT(6)
#define VENDOR_BB_WL_COEX_ROLE_EN       BIT(6)
#define VENDOR_BB_WL_COEX_ROLE          BIT(7)
#define VENDOR_BB_ADV_PDU_INT(x)        ((x) << 8)  /* 4bit */
#define VENDOR_BB_EVT_HOLD_TRIGG(x)     ((x) << 12) /* 6bit */
#define VENDOR_BB_RX_PAYLOAD_LEN(x)     ((x) << 18) /* 7bit */
#define VENDOR_BB_PIS_TX_PAYLOAD_LEN(x) ((x) << 25) /* 7bit*/

extern const u32 config_vendor_le_bb;
extern const int config_rf_oob;
/*-----------------------------------------------------------*/

/*
 *-------------------LE close wait
 */
extern const int ble_disable_wait_enable;
/*-----------------------------------------------------------*/

#endif //_LL_CONFIG_H_
