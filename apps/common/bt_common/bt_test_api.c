
#include "typedef.h"
#include "app_config.h"
#include "task.h"
#include "btctrler_task.h"
#include "btcontroller_config.h"
#include "system/includes.h"

#define LOG_TAG             "[BT_DUT]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

//bredr test api
extern void ble_enter_dut_tx_mode(void *param);
extern void bt_ble_adv_enable(u8 enable);
extern void bredr_fcc_init(u8 mode, u8 fre);

#if 0

static void bt_dut_api(void)
{
    log_info("bt in dut \n");
#if TCFG_AUTO_SHUT_DOWN_TIME
    extern void sys_auto_shut_down_disable(void);
    sys_auto_shut_down_disable();
#endif

#if TCFG_USER_TWS_ENABLE
    extern 	void tws_cancle_all_noconn();
    tws_cancle_all_noconn() ;
#else
    //sys_timer_del(app_var.auto_stop_page_scan_timer);
    extern void bredr_close_all_scan();
    bredr_close_all_scan();
#endif

#if TCFG_USER_BLE_ENABLE
#if (CONFIG_BT_MODE == BT_NORMAL)
    bt_ble_adv_enable(0);
#endif
#endif
}

void bit_clr_ie(unsigned char index);
/* !!!Notice:when this api is called and sleep mode should be sure to exit; */
void bt_fix_fre_api(u8 fre)
{
    bt_dut_api();

    bit_clr_ie(IRQ_BREDR_IDX);
    bit_clr_ie(IRQ_BT_CLKN_IDX);

    bredr_fcc_init(BT_FRE, fre);
}
#endif

//ble test api
enum  BLE_DUT_PAYLOAD_TYPE {
    PAYLOAD_TYPE_PRBS9 = 0,
    PAYLOAD_TYPE_11110000,
    PAYLOAD_TYPE_10101010,
    PAYLOAD_TYPE_PRBS15,
    PAYLOAD_TYPE_11111111,
    PAYLOAD_TYPE_00000000,
    PAYLOAD_TYPE_00001111,
    PAYLOAD_TYPE_01010101,

    PAYLOAD_TYPE_SINGLE_CARRIER = 0xf0,
};

enum BLE_DUT_PHY_TYPE {
    BLE_1M_UNCODED_PHY = 1,
    BLE_2M_UNCODED_PHY,
    BLE_1M_CODED_PHY_S8,
    BLE_1M_CODED_PHY_S2,
};

struct ble_dut_param_set {
    u8 ch_index;		//tx ch index;(0~39 -> 2402~2480)
    u8 payload_type;	//tx payload type
    u8 payload_len;		//payload_len(0~0xff) when continuous_tx = 0;
    u8 continuous_tx;	//enable or disable continuous transmission mode(0/1)
};

/* !!!Notice:when this api is called and sleep mode should be sure to exit; */
void ble_fix_fre_api()
{
#if TCFG_USER_BLE_ENABLE
#if (CONFIG_BT_MODE == BT_NORMAL)
    bt_ble_adv_enable(0);
#endif
    os_time_dly(10);

    struct ble_dut_param_set dut_param = {
        .ch_index = 0,
        .payload_type = PAYLOAD_TYPE_10101010,
        .payload_len = 0x20,
        .continuous_tx = 1,
    };
    ble_enter_dut_tx_mode(&dut_param);
#endif
}

static void *ble_dut_hdl = NULL;
extern void ll_hci_destory(void);
void ble_dut_mode_init(void)
{
    if (!ble_dut_hdl) {
        ll_hci_destory();
        ble_dut_hdl = __ble_dut_ops->init();
    }
}

void ble_dut_mode_exit(void)
{
    if (ble_dut_hdl) {
        __ble_dut_ops->exit(ble_dut_hdl);
        ble_dut_hdl = NULL;
    }
}

void ble_dut_tx_fre_api(u8 ch)
{
    ble_dut_mode_init();
    struct ble_dut_tx_param_t tx_param = {
        .ch_index = ch,
        .payload_len = 0x20,
        .payload_type = PAYLOAD_TYPE_PRBS9,
        .phy_type = BLE_1M_UNCODED_PHY,
    };

    log_info("BLE_DUT_TX-ch:%x\n", ch);
    __ble_dut_ops->ioctrl(BLE_DUT_SET_TX_MODE, &tx_param);

}

void ble_dut_rx_fre_api(u8 ch)
{
    ble_dut_mode_init();
    struct ble_dut_rx_param_t rx_param = {
        .ch_index = ch,
        .phy_type = BLE_1M_UNCODED_PHY,
    };

    log_info("BLE_DUT_RX-ch:%x \n", ch);
    __ble_dut_ops->ioctrl(BLE_DUT_SET_RX_MODE, &rx_param);

}

int ble_dut_test_end(void)
{
    int pkt_valid_cnt = 0;
    int pkt_err_cnt = 0;
    if (ble_dut_hdl) {
        __ble_dut_ops->ioctrl(BLE_DUT_SET_TEST_END, &pkt_valid_cnt, NULL);

        log_info("pkt_rx_cnt:%d pkt_err_cnt:%d\n", pkt_valid_cnt, pkt_err_cnt);
    } else {
        log_error("ble dut hdl not inited\n");
    }

    return pkt_valid_cnt;
}

static volatile u8 bt_test_status = 0;

void ble_bqb_test_thread_init(void);
static hci_transport_config_uart_t config = {
    HCI_TRANSPORT_CONFIG_UART,
    115200,
    0,  // main baudrate
    0,  // flow control
    NULL,
};

extern void dut_hci_controller_init(const hci_transport_t *transport, const void *config);
void ble_standard_dut_test_init(void)
{
    log_info("%s\n", __FUNCTION__);
    bt_test_status = 1;
    ble_dut_mode_init();
    //ble_bqb_test_thread_init();
    dut_hci_controller_init((void *)hci_transport_uart_instance(), &config);
}

void ble_standard_dut_test_close(void)
{
    log_info("%s\n", __FUNCTION__);
    ble_dut_test_end();
    ble_dut_mode_exit();
    hci_transport_t *p_uart_trans = hci_transport_uart_instance();
    p_uart_trans->close();
    bt_test_status = 0;

}

void ble_dut_mode_key_handle(u8 type, u8 key_val)
{
    static u8 rx_ch = 0;
    static u8 tx_ch = 0;
    switch (key_val) {
    case 3:
        ble_dut_tx_fre_api(tx_ch++);
        if (tx_ch > 39) {
            tx_ch = 0;
        }
        break;

    case 1:
        ble_dut_rx_fre_api(rx_ch++);
        if (rx_ch > 39) {
            rx_ch = 0;
        }
        break;

    case 2:
        ble_dut_test_end();
        break;

    case 0:
        ble_standard_dut_test_init();
        break;

    case 4:
        ble_standard_dut_test_close();
        break;

defualt:
        break;
    }
}

static u8 bt_test_idle_query(void)
{
    return !bt_test_status;
}

REGISTER_LP_TARGET(bt_test_lp_target) = {
    .name = "bt_test",
    .is_idle = bt_test_idle_query,
};

