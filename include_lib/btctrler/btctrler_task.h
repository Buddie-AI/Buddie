#ifndef BTCTRLER_TASK_H
#define BTCTRLER_TASK_H

#include "typedef.h"
#include "system/task.h"

enum {
    LMP_EVENT = Q_USER +  1,
    LMP_HCI_CMD,
    LMP_HCI_CMD_TO_CONN,
    HCI_COMMON_CMD,
    LL_EVENT,
    HCI_CMD_TO_LL,
    TWS_LMP_EVENT,
    MSG_BT_UPDATA_START,
    MSG_BT_UPDATE_LOADER_DOWNLOAD_START,
    MSG_BLE_TEST_UPDATA_START,
    MSG_BLE_TEST_OTA_LOADER_DOWNLOAD_START,
    MSG_TASK_READY,
    MSG_TASK_DEL,
};

enum {
    BTCTRLER_EVENT_RESUME_REQ = 1,
};

#define SYS_EVENT_FROM_CTRLER   (('C' << 24) | ('T' << 16) | ('R' << 8) | '\0')




int bredr_link_event(void *link, int argc, ...);

int bredr_tws_link_event(void *link, int argc, ...);

int btctrler_hci_cmd_to_task(int cmd, int argc, ...);

int lmp_hci_cmd_to_conn_for_handle(u16 handle, int argc, ...);

int lmp_hci_cmd_to_conn_for_addr(u8 *addr, int argc, ...);

int lmp_hci_cmd_to_conn(void *conn, int argc, ...);


#define lmp_hci_cmd_to_task(argc, ...)      btctrler_hci_cmd_to_task(LMP_HCI_CMD, argc, ## __VA_ARGS__)

#define ll_hci_cmd_to_task(argc, ...)       btctrler_hci_cmd_to_task(HCI_CMD_TO_LL, argc, ## __VA_ARGS__)


int btctrler_task_init(const void *transport, const void *config);

void btctrler_resume_req();

void btctrler_resume();

int btctrler_suspend(u8 suepend_rx_bulk);

int btctrler_task_ready();

int btctrler_task_exit();

int btctrler_task_close_bredr();
void  btctrler_task_init_bredr();

void set_idle_period_slot(u16 slot);
enum {
    TESTBOX_INFO_VBAT_VALUE = 0,	//(u16 (*handle)(void))
    TESTBOX_INFO_VBAT_PERCENT,		//(u8 (*handle)(void))
    TESTBOX_INFO_BURN_CODE,			//(u8 *(*handle)(u8 *len))
    TESTBOX_INFO_SDK_VERSION,		//(u8 *(*handle)(u8 *len))
};

void bt_testbox_ex_info_get_handle_register(u8 info_type, void *handle);
u8  bredr_bulk_change_rx_bulk(u8 mode);
void lmp_set_features_req_step(u8 *addr);

struct ble_dut_tx_param_t {
    u8 ch_index;	//data[0]
    u8 payload_len;	//data[1]
    u8 payload_type;//data[2]
    u8 phy_type; 	//data[3]
};

struct ble_dut_rx_param_t {
    u8 ch_index;	//data[0]
    u8 phy_type;	//data[1]
};

enum BLE_DUT_CTRL_TYPE {
    BLE_DUT_SET_RX_MODE = 0,		//param1:struct ble_dut_rx_param_t *param;
    BLE_DUT_SET_TX_MODE,			//param1:struct ble_dut_tx_param_t *param;
    BLE_DUT_SET_TEST_END,			//param1:u16 *pkt_valid_cnt,param2:u16 *pkt_err_cnt;
};

struct ble_dut_ops_t {

    /* *****************************************************************************/
    /**
     * @brief :      initialize the ble dut test module
     *
     * @param :       void
     *
     * @return :      pointer to instance of test module
     */
    /* *****************************************************************************/
    void *(*init)(void);

    /* *****************************************************************************/
    /**
     * @brief :        ble dut test control api,such as setting rx/tx mode,stop testing
     *
     * @param :       control type,using enum BLE_DUT_CTRL_TYPE value;
     * @param :       vary from different control type;
     */
    /* *****************************************************************************/
    int (*ioctrl)(int ctrl, ...);

    /* *****************************************************************************/
    /**
     * @brief :       exit the ble dut test module
     *
     * @param :       poniter to instance of test module
     */
    /* *****************************************************************************/
    void (*exit)(void *priv);
};

extern const struct ble_dut_ops_t *__ble_dut_ops;

#endif
