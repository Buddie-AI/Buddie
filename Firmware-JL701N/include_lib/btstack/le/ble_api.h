/*********************************************************************************************
    *   Filename        : ble_api.h

    *   Description     :

    *   Author          : mx

    *   Email           : lmx@zh-jieli.com

    *   Last modifiled  : 2020-07-01 16:36

    *   Copyright:(c)JIELI  2011-2020  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef __BLE_API_H__
#define __BLE_API_H__


#include "typedef.h"
#include "btstack/btstack_typedef.h"


/*****注意：该文件的枚举与库编译密切相关，主要是给用户提供调用所用。用户不能自己在中间添加值.***/
/*!
*   ----user (command) codes----
*
*    @brief hci connection handle type
 */
/*************************************************************************************************/

typedef enum {
    /*
     */
    BLE_CMD_ADV_ENABLE  = 1,
    BLE_CMD_ADV_PARAM,
    BLE_CMD_ADV_DATA,
    BLE_CMD_RSP_DATA,
    BLE_CMD_DISCONNECT,
    BLE_CMD_REGIEST_THREAD,
    BLE_CMD_ATT_SEND_INIT,
    BLE_CMD_ATT_MTU_SIZE,
    BLE_CMD_ATT_VAILD_LEN,
    BLE_CMD_ATT_SEND_DATA,
    BLE_CMD_REQ_CONN_PARAM_UPDATE,

    BLE_CMD_SCAN_ENABLE,
    BLE_CMD_SCAN_PARAM,
    BLE_CMD_STACK_EXIT,
    BLE_CMD_CREATE_CONN,
    BLE_CMD_CREATE_CONN_CANCEL,

    BLE_CMD_ADV_PARAM_EXT,
    BLE_CMD_SEND_TEST_KEY_NUM,
    BLE_CMD_LATENCY_HOLD_CNT,
    BLE_CMD_SET_DATA_LENGTH,
    BLE_CMD_SET_HCI_CFG,
    BLE_CMD_SCAN_ENABLE2,
    BLE_CMD_ATT_SERVER_REQ_RESUME,

    //MULTI API,多机接口
    BLE_CMD_MULTI_ATT_SEND_INIT,
    BLE_CMD_MULTI_ATT_SET_CONN_HANDLE,
    BLE_CMD_MULTI_ATT_SEND_DATA,
    BLE_CMD_MULTI_ATT_MTU_SIZE,
    BLE_CMD_MULTI_ATT_VAILD_LEN,

    //sm
    BLE_CMD_SM_REQ_RESUME,

    //add here
    BLE_CMD_CREATE_CONN_EXT,
    BLE_CMD_DISCONNECT_EXT,
    BLE_CMD_ATT_CLEAR_SEND_DATA,

    //< ble5
    BLE_CMD_EXT_ADV_PARAM = 0x40,
    BLE_CMD_EXT_ADV_DATA,
    BLE_CMD_EXT_RSP_DATA,
    BLE_CMD_EXT_ADV_ENABLE,
    BLE_CMD_SET_PHY,
    BLE_CMD_EXT_SCAN_PARAM,
    BLE_CMD_EXT_SCAN_ENABLE,
    BLE_CMD_EXT_CREATE_CONN,
    BLE_CMD_PERIODIC_ADV_PARAM,
    BLE_CMD_PERIODIC_ADV_DATA,
    BLE_CMD_PERIODIC_ADV_ENABLE,
    BLE_CMD_PERIODIC_ADV_CREAT_SYNC,
    BLE_CMD_PERIODIC_ADV_TERMINATE_SYNC,
    BLE_CMD_PERIODIC_ADV_CREATE_SYNC_CANCEL,

    //client
    BLE_CMD_SEARCH_PROFILE = 0x80,
    BLE_CMD_WRITE_CCC,
    BLE_CMD_ONNN_PARAM_UPDATA,
} ble_cmd_type_e;

typedef enum {
    BLE_CMD_RET_SUCESS =  0, //执行成功
    BLE_CMD_RET_BUSY = -100, //命令处理忙
    BLE_CMD_PARAM_OVERFLOW,  //传数溢出
    BLE_CMD_OPT_FAIL,        //操作失败
    BLE_BUFFER_FULL,         //缓存满了
    BLE_BUFFER_ERROR,        //缓存出错
    BLE_CMD_PARAM_ERROR,     //传参出错
    BLE_CMD_STACK_NOT_RUN,   //协议栈没有运行
    BLE_CMD_CCC_FAIL,        //没有使能通知，导致NOTIFY或INDICATE发送失败，
} ble_cmd_ret_e;

//--------------------------------------------
ble_cmd_ret_e ble_user_cmd_prepare(ble_cmd_type_e cmd, int argc, ...);

struct conn_update_param_t {
    u16 interval_min;  //连接周期范围最小值(unit:1.25ms)
    u16 interval_max;  //连接周期范围最大值(unit:1.25ms)
    u16 latency;       //忽略通信次数(unit: interval)
    u16 timeout;       //(unit:10ms)
};

typedef enum {
    PFL_SERVER_UUID16 = 1, //指定16bit UUID搜索方式
    PFL_SERVER_UUID128,    //指定128bit UUID搜索方式
    PFL_SERVER_ALL,        //搜索所有的UUID
} search_profile_type_e;


//--------------------------------------------
struct create_conn_param_t {
    u16 conn_interval;        //连接周期(unit:1.25ms)
    u16 conn_latency;         //忽略通信次数(unit: interval)
    u16 supervision_timeout;  //(通信超时 unit:10ms)
    u8 peer_address_type;     //对方地址类型：0--public address,1--random address
    u8 peer_address[6];       //对方地址address
} _GNU_PACKED_;

struct create_conn_param_ext_t {
    u16 le_scan_interval; /*scan 周期(unit: 0.625ms)*/
    u16 le_scan_window;   /*scan 窗口(unit: 0.625ms)*/
    u8 initiator_filter_policy;/*过滤,set 0*/
    u8 peer_address_type; /*对方地址类型:0--public address,1--random address*/
    u8 peer_address[6];   /*对方地址*/
    u8 own_address_type;  /*本地地址类型:0--public address,1--random address*/
    u16 conn_interval_min;/*连接周期最小值(unit:1.25ms)*/
    u16 conn_interval_max;/*连接周期最大值(unit:1.25ms)*/
    u16 conn_latency;     /*忽略通信次数(unit: interval)*/
    u16 supervision_timeout;/*通信超时 unit:10ms*/
    u16 minimum_ce_length;  /*set 1*/
    u16 maximum_ce_length;  /*set 1*/
} _GNU_PACKED_;


typedef struct {
    u8   event_type;    //对方广播包类型: 0--ADV_IND,1--ADV_DIRECT_IND,2--ADV_SCAN_IND,3--ADV_NONCONN_IND,4--SCAN_RSP
    u8   address_type;  //对方地址类型：0--public address,1--random address
    u8   address[6];    //peer_address
    s8   rssi;          //range:-127 ~128 dbm
    u8   length;        //广播包长度
    u8   data[0];       //广播包内容
} adv_report_t;

typedef struct {
    //base info
    u8   type;       //< See <btstack/hci_cmds.h> SM_...
    u8   size;
    u16  con_handle; //connection 's handle, >0
    u8   addr_type;  //对方地址类型:0--public address,1--random address
    u8   address[6]; //对方地址
    //extend info
    u8   data[4];
} sm_just_event_t;

//BLE_CMD_SET_HCI_CFG
typedef enum {
    HCI_CFG_OWN_ADDRESS_TYPE =  0, //
    HCI_CFG_ADV_FILTER_POLICY, //
    HCI_CFG_SCAN_FILTER_POLICY, //
    HCI_CFG_INITIATOR_FILTER_POLICY, //
    //add here
} hci_cfg_par_e;

typedef enum {
    REMOTE_TYPE_UNKNOWN  = 0,//未查询or查询对方未响应
    REMOTE_TYPE_ANDROID,  //安卓系统
    REMOTE_TYPE_IOS,//ios系统
} remote_type_e;

/*************************************************************************************************/
/*!
 *  \brief      Gatt client角色初始化.
 */
/*************************************************************************************************/
void gatt_client_init(void);

/*************************************************************************************************/
/*!
 *  \brief      注册gatt client角色,事件回调函数.
 *
 *  \param      [in] handler     事件处理函数.
 */
/*************************************************************************************************/
void gatt_client_register_packet_handler(btstack_packet_handler_t handler);

/*************************************************************************************************/
/*!
 *  \brief      初始化配对表.
 *
 *  \note       上电初始化一次.
 */
/*************************************************************************************************/
void le_device_db_init(void);

/*************************************************************************************************/
/*!
 *  \brief      注册passkey输入回调.
 *
 *  \param      [in] reset_pk     复位按键输入.
 */
/*************************************************************************************************/
void reset_PK_cb_register(void (*reset_pk)(u32 *));

/*************************************************************************************************/
/*!
 *  \brief      设置ble蓝牙的public地址.
 *
 *  \param      [in] addr     public地址.
 *
 *  \return     0->success ,非0->fail.
 *
 *  \note       可以结合接口 ble_op_set_ownaddress_type 配置选择地址类型.
 *  \note       修改地址必须在ble非工作状态下才能生效( 没有scan,没有adv,没有connected).
 */
/*************************************************************************************************/
int le_controller_set_mac(void *addr);

/*************************************************************************************************/
/*!
 *  \brief      获取ble蓝牙的public地址.
 *
 *  \param      [out] addr       pulic地址.
 *
 *  \return     0->success ,非0->fail.
 */
/*************************************************************************************************/
int le_controller_get_mac(void *addr);

/*************************************************************************************************/
/*!
 *  \brief      初始化ble蓝牙random地址.
 *
 *  \param      [in] addr       random地址.
 *
 *  \return     0->success ,非0->fail.
 *
 *  \note       结合接口ble_op_set_own_address_type 配置选择地址类型.
 *  \note       修改地址必须在ble非工作状态下才能生效(内有scan,没有adv,没connected).
 */
/*************************************************************************************************/
int le_controller_set_random_mac(void *addr);

/*************************************************************************************************/
/*!
 *  \brief      获取ble蓝牙的random地址.
 *
 *  \param      [out] addr       random地址.
 *
 *  \return     0->success ,非0->fail.
 */
/*************************************************************************************************/
int le_controller_get_random_mac(void *addr);

/*************************************************************************************************/
/*!
 *  \brief      配置协议栈 GATT 角色处理,default server.
 *
 *  \param      [in] role       GATT role: 0--server ,1--client ,2--server + client.
 */
/*************************************************************************************************/
void ble_stack_gatt_role(u8 role);

/*************************************************************************************************/
/*!
 *  \brief      client 连接后初始化.
 *
 *  \param      [in] handle       range :>0.
 *  \param      [in] buffer       配置缓存地址.
 *  \param      [in] buffer_size  缓存大小.
 */
/*************************************************************************************************/
void user_client_init(u16 handle, u8 *buffer, u16 buffer_size);

/*************************************************************************************************/
/*!
 *  \brief      获取链路对方的信号强度.
 *
 *  \param      [in] con_handle     range :>0.
 *
 *  \return     rssi 强度 ,range :-127 ~128 dbm.
 */
/*************************************************************************************************/
s8 ble_vendor_get_peer_rssi(u16 conn_handle);

/*************************************************************************************************/
/*!
 *  \brief      使能周期interval事件上报.
 *
 *  \param      [in] con_handle     range :>0.
 *  \param      [in] enable         1 or 0.
 *
 *  \return     true or false.
 */
/*************************************************************************************************/
bool ble_vendor_interval_event_enable(u16 conn_handle, int enable);

/*************************************************************************************************/
/*!
 *  \brief      配置协议栈ATT默认的MTU大小.
 *
 *  \param      [in] mtu_size     配置MUT大小 ,range :23 ~517.
 *
 *  \return     配置后mtu_size的值.
 */
/*************************************************************************************************/
u16 ble_vendor_set_default_att_mtu(u16 mtu_size);

/*************************************************************************************************/
/*!
 *  \brief      设置 client 搜索结束.
 */
/*************************************************************************************************/
void user_client_set_search_complete(void);

/*************************************************************************************************/
/*!
 *  \brief      提供生成ble对应的类型地址.
 *
 *  \param      [out] address    类型对应地址.
 *  \param      [in] type        ble_type
 *           (eg:1.STATIC_DEVICE_ADDR.
 *               2.NON_RESOLVABLE_PRIVATE_ADDR.
 *               3.RESOLVABLE_PRIVATE_ADDR).
 *
 *  \return     true or false.
 *
 *  \note       生成RESOLVABLE_PRIVATE_ADDR,需要等协议栈初始化IRK后才能生成.
 */
/*************************************************************************************************/
bool ble_vendor_random_address_generate(u8 *address, u8 type);

/*************************************************************************************************/
/*!
 *  \brief      根据提供的edr地址生成唯一对应的ble地址.
 *
 *  \param      [out] ble_adress     BLE_address.
 *  \param      [in] edr_adress      EDR_address.
 *
 *  \note       用户可以自定义edr和ble地址关联规则,重写这个接口
 */
/*************************************************************************************************/
void lib_make_ble_address(u8 *ble_address, u8 *edr_address);

/*************************************************************************************************/
/*!
 *  \brief      配置设备的地址类型，默认为 0(public address).
 *
 *  \function   ble_cmd_ret_e ble_op_set_own_address_type(u8 address_type).
 *
 *  \param      [in] address_type   Range: 0x00 to 0x03.
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       !!!注意:设置的时候必须在、设置广播参数、或者扫描参数、或者创建连接参数前配置好.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_set_own_address_type(u8 address_type) */
#define ble_op_set_own_address_type(address_type)     \
	ble_user_cmd_prepare(BLE_CMD_SET_HCI_CFG, 2,HCI_CFG_OWN_ADDRESS_TYPE,(int)address_type)

/*************************************************************************************************/
/*!
 *  \brief      开关BLE广播.
 *
 *  \function   ble_cmd_ret_e ble_op_adv_enable(int enable).
 *
 *  \param      [in] enable     广播使能 :0(dis) or 1(en).
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       !!!注意：开广播前必现先配置好广播的参数.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_adv_enable(int enable) */
#define ble_op_adv_enable(enable)     \
	ble_user_cmd_prepare(BLE_CMD_ADV_ENABLE, 1, (int)enable)

/*************************************************************************************************/
/*!
 *  \brief      配置广播过滤策略(决定链路层如何处理扫描、连接请求);默认为 0.
 *
 *  \function   ble_cmd_ret_e ble_op_set_adv_filter_policy(u8 type).
 *
 *  \param      [in] type     类型 :Range: 0x00 to 0x03.
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       !!!注意：设置的时候必须在“设置广播参数ble_op_set_adv_param前配置好.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_set_adv_filter_policy(u8 type) */
#define ble_op_set_adv_filter_policy(type)     \
	ble_user_cmd_prepare(BLE_CMD_SET_HCI_CFG, 2,HCI_CFG_ADV_FILTER_POLICY,type)

/*************************************************************************************************/
/*!
 *  \brief      配置广播参数.
 *
 *  \function   ble_cmd_ret_e ble_op_set_adv_param(u16 adv_interval,u8 adv_type).
 *
 *  \param      [in] adv_interval     广播周期,Range: 0x0020 to 0x4000 (unit: 0.625ms).
 *  \param      [in] adv_type         广播类型,Range: 0x00 to 0x04.
 *  \param      [in] adv_channel      广播类型通道,range:Range: 0x01 to 0x07.
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       !!!注意：设置的时候必现在广播关闭的状态下.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_set_adv_param(u16 adv_interval,u8 adv_type) */
#define ble_op_set_adv_param(adv_interval,adv_type,adv_channel)     \
	ble_user_cmd_prepare(BLE_CMD_ADV_PARAM, 3, (int)adv_interval, (int)adv_type, (int)adv_channel)

/*************************************************************************************************/
/*!
 *  \brief      配置广播参数+配对信息(相比配对广播参数,这个函数主要是针对定向广播使用).
 *
 *  \function   ble_cmd_ret_e ble_op_set_adv_param_ext(u16 adv_interval,u8 adv_type,u8 adv_channel,const u8 *peer_info).
 *
 *  \param      [in] adv_interval     广播周期,Range: 0x0020 to 0x4000 (unit: 0.625ms).
 *  \param      [in] adv_type         广播类型,Range: 0x00 to 0x04.
 *  \param      [in] adv_channel      广播类型通道,range:Range: 0x01 to 0x07.
 *  \param      [in] peer_info        (全局变量地址),定向广播时，填入对方的地址信息.
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       !!!注意：设置的时候必现在广播关闭的状态下.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_set_adv_param_ext(u16 adv_interval,u8 adv_type,u8 adv_channel,const u8 *peer_info) */
#define ble_op_set_adv_param_ext(adv_interval,adv_type,adv_channel,peer_info)     \
	ble_user_cmd_prepare(BLE_CMD_ADV_PARAM_EXT, 4, (int)adv_interval, (int)adv_type, (int)adv_channel, (void*)peer_info)

/*************************************************************************************************/
/*!
 *  \brief      配置广播 Advertising Data内容.
 *
 *  \function   ble_cmd_ret_e ble_op_set_adv_data(u8 adv_len,const *u8 adv_data).
 *
 *  \param      [in] adv_len     adv 数据包长度，Range: 0x00 to 0x1f.
 *  \param      [in] adv_data    (全局变量地址)，adv数据包地址.
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       !!!注意：设置的时候必现在广播关闭的状态下.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_set_adv_data(u8 adv_len,const *u8 adv_data) */
#define ble_op_set_adv_data(adv_len,adv_data)     \
	ble_user_cmd_prepare(BLE_CMD_ADV_DATA, 2, (int)adv_len, (void*)adv_data)

/*************************************************************************************************/
/*!
 *  \brief      配置广播 Scan Response Data内容.
 *
 *  \function   ble_cmd_ret_e ble_op_set_rsp_data(u8 rsp_len,const *u8 rsp_data).
 *
 *  \param      [in] rsp_len      rsp 包长度，Range: 0x00 to 0x1f.
 *  \param      [in] adv_data     (全局变量地址)，rsp数据包地址.
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       !!!注意：设置的时候必现在广播关闭的状态下.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_set_rsp_data(u8 rsp_len,const *u8 rsp_data) */
#define ble_op_set_rsp_data(rsp_len,rsp_data)     \
	ble_user_cmd_prepare(BLE_CMD_RSP_DATA, 2, (int)rsp_len, (void*)rsp_data)

/*************************************************************************************************/
/*!
 *  \brief      请求断开 ble 连接.
 *
 *  \function   ble_cmd_ret_e ble_op_disconnect(u16 con_handle).
 *
 *  \param      [in] con_handle      range: >0.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_disconnect(u16 con_handle) */
#define ble_op_disconnect(con_handle)     \
	ble_user_cmd_prepare(BLE_CMD_DISCONNECT, 1, (int)con_handle)

/*************************************************************************************************/
/*!
 *  \brief      请求断开 ble 连接,by reason
 *
 *  \function   ble_cmd_ret_e ble_op_disconnect(u16 con_handle).
 *
 *  \param      [in] con_handle      range: >0.
 *  \param      [in] reason          range: see spec.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_disconnect_ext(u16 con_handle,reason) */
#define ble_op_disconnect_ext(con_handle,reason)     \
	ble_user_cmd_prepare(BLE_CMD_DISCONNECT_EXT, 2, (int)con_handle, reason)


/*************************************************************************************************/
/*!
 *  \brief      配置ATT发送模块RAM大小.
 *
 *  \function   ble_cmd_ret_e ble_op_att_send_init(u16 con_handle,u8 *att_ram_addr,int att_ram_size,int att_payload_size).
 *
 *  \param      [in] con_handle        range: >0.
 *  \param      [in] att_ram_addr      传入ATT发送模块ram地址，地址按4字节对齐.
 *  \param      [in] att_ram_size      传入ATT发送模块ram大小.
 *  \param      [in] att_payload_size  发送ATT包payload的最大长度 <= MTU的payload,range：20 to MTU的payload size.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_att_send_init(u16 con_handle,u8 *att_ram_addr,int att_ram_size,int att_payload_size) */
#define ble_op_att_send_init(con_handle,att_ram_addr,att_ram_size,att_payload_size)     \
	ble_user_cmd_prepare(BLE_CMD_ATT_SEND_INIT, 4, con_handle,att_ram_addr,att_ram_size,att_payload_size)

/*************************************************************************************************/
/*!
 *  \brief      MULTI API: 配置ATT发送模块初始化.
 *
 *  \function   ble_cmd_ret_e ble_op_multi_att_send_init(u8 *att_ram_addr,int att_ram_size,int att_payload_size).
 *
 *  \param      [in] att_ram_addr      传入ATT发送模块ram地址，地址按4字节对齐.
 *  \param      [in] att_ram_size      传入ATT发送模块ram大小.
 *  \param      [in] att_payload_size  发送ATT包payload的最大长度 <= MTU的payload，range：20 to MTU的payload size.
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       多机处理,只需要初始化一次就可以.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_multi_att_send_init(u8 *att_ram_addr,int att_ram_size,int att_payload_size) */
#define ble_op_multi_att_send_init(att_ram_addr,att_ram_size,att_payload_size)     \
	ble_user_cmd_prepare(BLE_CMD_MULTI_ATT_SEND_INIT, 3, att_ram_addr,att_ram_size,att_payload_size)

/*************************************************************************************************/
/*!
 *  \brief      MULTI API: 配置ATT模块 使用的连接con_handle.
 *
 *  \function   ble_cmd_ret_e ble_op_multi_att_send_conn_handle(u16 con_handle,int handle_index,int0 rolev).
 *
 *  \param      [in] con_handle         range：>0.
 *  \param      [in] handle_index       range: 0~7 //多连接的hanlde id.
 *  \param      [in] role               range: 0-gatt_sever,1-gatt_client.
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       多机处理.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_multi_att_send_conn_handle(u16 con_handle,int handle_index,int0 rolev) */
#define ble_op_multi_att_send_conn_handle(con_handle,handle_index,role)     \
	ble_user_cmd_prepare(BLE_CMD_MULTI_ATT_SET_CONN_HANDLE, 3, con_handle,handle_index,role)

/*************************************************************************************************/
/*!
 *  \brief      根据对方的接收MTU大小，配置本地可发送MTU的payload大小.
 *
 *  \function   ble_cmd_ret_e ble_op_att_set_send_mtu(u16 mtu).
 *
 *  \param      [in] mtu         对方mtu payload的大小.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_att_set_send_mtu(u16 mtu) */
#define ble_op_att_set_send_mtu(mtu)     \
	ble_user_cmd_prepare(BLE_CMD_ATT_MTU_SIZE, 1, mtu);

/*************************************************************************************************/
/*!
 *  \brief      MULTI API: 根据对方的接收MTU大小，配置本地可发送的MTU的大小.
 *
 *  \function   ble_cmd_ret_e ble_op_multi_att_set_send_mtu(u16 con_handle,u16 mtu).
 *
 *  \param      [in] con_handle     连接 con_handle,range：>0.
 *  \param      [in] mtu            对方mtu payload的大小.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_multi_att_set_send_mtu(u16 con_handle,u16 mtu) */
#define ble_op_multi_att_set_send_mtu(con_handle,mtu)     \
	ble_user_cmd_prepare(BLE_CMD_MULTI_ATT_MTU_SIZE, 2, con_handle,mtu);

/*************************************************************************************************/
/*!
 *  \brief      获取ATT发送模块，cbuffer可写入数据的长度.
 *
 *  \function   ble_cmd_ret_e ble_op_att_get_remain(int *remain_size_ptr).
 *
 *  \param      [out] remain_size_ptr       输出可写入长度值.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_att_get_remain(int *remain_size_ptr) */
#define ble_op_att_get_remain(remain_size_ptr)     \
	ble_user_cmd_prepare(BLE_CMD_ATT_VAILD_LEN, 1, remain_size_ptr)

/*************************************************************************************************/
/*!
 *  \brief      MULTI API: 获取ATT发送模块，cbuffer 可写入数据的长度.
 *
 *  \function   ble_cmd_ret_e ble_op_multi_att_get_remain(u16 con_handle,int *remain_size_ptr).
 *
 *  \param      [in] con_handle         range：>0.
 *  \param      [out] remain_size_ptr   输出可写入长度值.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_multi_att_get_remain(u16 con_handle,int *remain_size_ptr) */
#define ble_op_multi_att_get_remain(con_handle,remain_size_ptr)     \
	ble_user_cmd_prepare(BLE_CMD_MULTI_ATT_VAILD_LEN, 2,con_handle, remain_size_ptr)

/*************************************************************************************************/
/*!
 *  \brief      ATT操作handle发送数据.
 *
 *  \function   ble_cmd_ret_e ble_op_att_send_data(u16 att_handle,u8 *data,u16 len, att_op_type_e att_op_type).
 *
 *  \param      [in] att_handle      att操作的handle.
 *  \param      [in] data            数据地址.
 *  \param      [in] len             数据长度  <= cbuffer 可写入的长度.
 *  \param      [in] att_op_type     see  att_op_type_e (att.h).
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_att_send_data(u16 att_handle,u8 *data,u16 len, att_op_type_e att_op_type) */
#define ble_op_att_send_data(att_handle,data,len,att_op_type)     \
	ble_user_cmd_prepare(BLE_CMD_ATT_SEND_DATA, 4, att_handle, data, len, att_op_type)

/*************************************************************************************************/
/*!
 *  \brief      MULTI API: ATT操作handle发送数据.
 *
 *  \function   ble_cmd_ret_e ble_op_multi_att_send_data(u16 con_handle,u16 att_handle,u8 *data,u16 len, att_op_type_e att_op_type).
 *
 *  \param      [in] con_handle     连接 con_handle,range：>0.
 *  \param      [in] att_handle     att操作handle.
 *  \param      [in] data           数据地址.
 *  \param      [in] len            数据长度  <= cbuffer 可写入的长度.
 *  \param      [in] att_op_type    see  att_op_type_e (att.h).
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       多机处理.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_multi_att_send_data(u16 con_handle,u16 att_handle,u8 *data,u16 len, att_op_type_e att_op_type) */
#define ble_op_multi_att_send_data(con_handle,att_handle,data,len,att_op_type)     \
	ble_user_cmd_prepare(BLE_CMD_MULTI_ATT_SEND_DATA, 5, con_handle,att_handle, data, len, att_op_type)


/*************************************************************************************************/
/*!
 *  \brief      ATT操作,清缓存发送的数据缓存
 *
 *  \function   ble_op_att_clear_data(void).
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_att_clear_send_data(void) */
#define ble_op_att_clear_send_data(void)     \
    ble_user_cmd_prepare(BLE_CMD_ATT_CLEAR_SEND_DATA, 1, 1)

/*************************************************************************************************/
/*!
 *  \brief      从机请求更新连接参数,走l2cap命令.
 *
 *  \function   ble_cmd_ret_e ble_op_conn_param_request(u16 con_handle,const struct conn_update_param_t *con_param).
 *
 *  \param      [in] con_handle     连接 con_handle,range：>0.
 *  \param      [in] con_param      (全局变量地址),连接参数.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_conn_param_request(u16 con_handle,const struct conn_update_param_t *con_param) */
#define ble_op_conn_param_request(con_handle,con_param)     \
	ble_user_cmd_prepare(BLE_CMD_REQ_CONN_PARAM_UPDATE, 2, con_handle, (void*)con_param)

/*************************************************************************************************/
/*!
 *  \brief      发起data length 交换.(btctrl的feature打开支持DLE)
 *
 *  \function   ble_cmd_ret_e ble_op_set_data_length(u16 con_handle,u16 tx_octets,u16 tx_time).
 *
 *  \param      [in] tx_octets      DLE发送长度.
 *  \param      [in] tx_time        DLE发送事件.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_set_data_length(u16 con_handle,u16 tx_octets,u16 tx_time) */
#define ble_op_set_data_length(con_handle,tx_octets,tx_time)     \
	ble_user_cmd_prepare(BLE_CMD_SET_DATA_LENGTH, 3, con_handle, tx_octets, tx_time)

/*************************************************************************************************/
/*!
 *  \brief      ble5.0 配置广播参数.
 *
 *  \function   ble_cmd_ret_e ble_op_set_ext_adv_param(u8 *param,u16 param_len).
 *
 *  \param      [in] param      广播参数.
 *  \param      [in] tx_time    参数长度.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_set_ext_adv_param(u8 *param,u16 param_len) */
#define ble_op_set_ext_adv_param(param,param_len)     \
	ble_user_cmd_prepare(BLE_CMD_EXT_ADV_PARAM, 2, param, param_len)

/*************************************************************************************************/
/*!
 *  \brief      ble5.0 配置 adv 数据包.
 *
 *  \function   ble_cmd_ret_e ble_op_set_ext_adv_data(u8 *data,u16 data_len).
 *
 *  \param      [in] data      数据包内容.
 *  \param      [in] data_len  数据包长度.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_set_ext_adv_data(u8 *data,u16 data_len) */
#define ble_op_set_ext_adv_data(data,data_len)     \
	ble_user_cmd_prepare(BLE_CMD_EXT_ADV_DATA, 2, data, data_len)

/*************************************************************************************************/
/*!
 *  \brief      ble5.0 配置 respond 数据包.
 *
 *  \function   ble_cmd_ret_e ble_op_set_ext_rsp_data(u8 *data,u16 data_len).
 *
 *  \param      [in] data      数据包内容.
 *  \param      [in] data_len  数据包长度.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_set_ext_rsp_data(u8 *data,u16 data_len) */
#define ble_op_set_ext_rsp_data(data,data_len)     \
	ble_user_cmd_prepare(BLE_CMD_EXT_RSP_DATA, 2, data, data_len)

/*************************************************************************************************/
/*!
 *  \brief      ble5.0 开关广播.
 *
 *  \function   ble_cmd_ret_e ble_op_set_ext_adv_enable(u8 *cmd,u16 cmd_le).
 *
 *  \param      [in] cmd      命令信息.
 *  \param      [in] cmd_len  命令长度.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_set_ext_adv_enable(u8 *cmd,u16 cmd_le) */
#define ble_op_set_ext_adv_enable(cmd,cmd_len)     \
	ble_user_cmd_prepare(BLE_CMD_EXT_ADV_ENABLE, 2, cmd, cmd_len)

/*************************************************************************************************/
/*!
 *  \brief      ble5.0 配置phy.
 *
 *  \function   ble_cmd_ret_e ble_op_set_ext_phy(u16 con_handle,u16 all_phys,u16 tx_phy,u16 rx_phy,u16 phy_options).
 *
 *  \param      [in] con_handle      连接 con_handle,range：>0.
 *  \param      [in] all_phys        0.
 *  \param      [in] tx_phy          CONN_SET_CODED_PHY.
 *  \param      [in] rx_phy          CONN_SET_CODED_PHY.
 *  \param      [in] phy_options     CONN_SET_PHY_OPTIONS.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_set_ext_phy(u16 con_handle,u16 all_phys,u16 tx_phy,u16 rx_phy,u16 phy_options) */
#define ble_op_set_ext_phy(con_handle,all_phys,tx_phy,rx_phy,phy_options)     \
    ble_user_cmd_prepare(BLE_CMD_SET_PHY, 5, con_handle, all_phys, tx_phy, rx_phy, phy_options)

/*************************************************************************************************/
/*!
 *  \brief      ble5.0 配置 主机scan 参数.
 *
 *  \function   ble_cmd_ret_e ble_op_set_ext_scan_param(u8 *param,u16 param_le).
 *
 *  \param      [in] param          参数内容.
 *  \param      [in] param_len      参数长度.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_set_ext_scan_param(u8 *param,u16 param_le) */
#define ble_op_set_ext_scan_param(param,param_len)     \
        ble_user_cmd_prepare(BLE_CMD_EXT_SCAN_PARAM, 2, param, param_len)

/*************************************************************************************************/
/*!
 *  \brief      ble5.0 配置 主机scan 开关.
 *
 *  \function   ble_cmd_ret_e ble_op_ext_scan_enable(u8 *cmd,u16 cmd_le).
 *
 *  \param      [in] cmd          命令信息.
 *  \param      [in] cmd_len      命令长度.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_ext_scan_enable(u8 *cmd,u16 cmd_le) */
#define ble_op_ext_scan_enable(cmd,cmd_len)     \
        ble_user_cmd_prepare(BLE_CMD_EXT_SCAN_ENABLE, 2, cmd, cmd_len)

/*************************************************************************************************/
/*!
 *  \brief      ble5.0 配置 主机创建连接监听.
 *
 *  \function   ble_cmd_ret_e ble_op_ext_create_conn(u8 *conn_param,u16 param_len_len).
 *
 *  \param      [in] conn_param   连接参数信息.
 *  \param      [in] param_len    参数长度.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_ext_create_conn(u8 *conn_param,u16 param_len_len) */
#define ble_op_ext_create_conn(conn_param,param_len)     \
    ble_user_cmd_prepare(BLE_CMD_EXT_CREATE_CONN, 2, conn_param, param_len)

/*************************************************************************************************/
/*!
 *  \brief      忽略进入latency 作用的次数
 *
 *  \function   ble_cmd_ret_e ble_op_latency_skip(u16 con_handle,u16 skip_interval).
 *
 *  \param      [in] con_handle     range：>0.
 *  \param      [in] skip_interval  忽略的interval的次数.
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       !!!注意：让设备不进入latency模式，加快响应速度，但会耗电.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_latency_skip(u16 con_handle,u16 skip_interval) */
#define ble_op_latency_skip(con_handle,skip_interval)     \
	ble_user_cmd_prepare(BLE_CMD_LATENCY_HOLD_CNT, 2, con_handle, skip_interval)

/*************************************************************************************************/
/*!
 *  \brief      测试盒识别按键测试.
 *
 *  \function   ble_cmd_ret_e ble_op_test_key_num(u16 con_handle,u8 key_num).
 *
 *  \param      [in] con_handle     range：>0.
 *  \param      [in] key_num        按键.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_test_key_num(u16 con_handle,u8 key_num) */
#define ble_op_test_key_num(con_handle,key_num)     \
	ble_user_cmd_prepare(BLE_CMD_SEND_TEST_KEY_NUM, 2, con_handle, key_num)

/*************************************************************************************************/
/*!
 *  \brief      退出ble协议栈.
 *
 *  \function   ble_cmd_ret_e ble_op_stack_exit(u8 control).
 *
 *  \param      [in] control        : 0--退出stack(默认),1--退出stack + controller.
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       若已调用btstack_exit退出,则无需调用此接口
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_stack_exit(u8 control) */
#define ble_op_stack_exit(control)     \
	ble_user_cmd_prepare(BLE_CMD_STACK_EXIT, 1, control)

/*************************************************************************************************/
/*!
 *  \brief      挂载协议栈线程调用.
 *
 *  \function   ble_cmd_ret_e ble_op_regist_thread_call(void (*thread_callback)(void)).
 *
 *  \param      [in] thread_callback        回调函数.
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       用于同步线程处理，慎用
 *
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_regist_thread_call(void (*thread_callback)(void)) */
#define ble_op_regist_thread_call(thread_callback)     \
	ble_user_cmd_prepare(BLE_CMD_REGIEST_THREAD, 1, thread_callback)

/*************************************************************************************************/
/*!
 *  \brief      开关BLE搜索扫描.
 *
 *  \function   ble_cmd_ret_e ble_op_scan_enable(u8 enable).
 *
 *  \param      [in] enable        0 or 1.
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       filter_duplicate 默认为 1.
 *  \note       !!!注意：开搜索前必现先配置好搜索的参数ble_op_set_scan_param.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_scan_enable(u8 enable) */
#define ble_op_scan_enable(enable)     \
	ble_user_cmd_prepare(BLE_CMD_SCAN_ENABLE, 1, enable)

/*************************************************************************************************/
/*!
 *  \brief      开关BLE搜索扫描2.
 *
 *  \function   ble_cmd_ret_e ble_op_scan_enable2(u8 enable,u8 filter_duplicate).
 *
 *  \param      [in] enable             0 or 1.
 *  \param      [in] filter_duplicate   0 or 1.
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       filter_duplicate 默认为 1.
 *  \note       !!!注意：开搜索前必现先配置好搜索的参数ble_op_set_scan_param.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_scan_enable2(u8 enable,u8 filter_duplicate) */
#define ble_op_scan_enable2(enable,filter_duplicate)     \
	ble_user_cmd_prepare(BLE_CMD_SCAN_ENABLE2, 2, enable,filter_duplicate)

/*************************************************************************************************/
/*!
 *  \brief      配置scan filter policy,默认type为0.
 *
 *  \function   ble_cmd_ret_e ble_op_set_scan_filter_policy(u8 type).
 *
 *  \param      [in] type       Range: 0x00 to 0x03.
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       !!!注意：设置的时候必须在 设置扫描参数 ble_op_set_scan_param 前配置好.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_set_scan_filter_policy(u8 type) */
#define ble_op_set_scan_filter_policy(type)     \
	ble_user_cmd_prepare(BLE_CMD_SET_HCI_CFG, 2,HCI_CFG_SCAN_FILTER_POLICY,type)

/*************************************************************************************************/
/*!
 *  \brief      配置搜索参数.
 *
 *  \function   ble_cmd_ret_e ble_op_set_scan_param(u8 scan_type,u16 scan_interval,u16 scan_window).
 *
 *  \param      [in] scan_type       搜索类型 Range: 0x00 to 0x01     (unit: 0.625ms).
 *  \param      [in] scan_interval   搜索周期 Range: 0x0004 to 0x4000 (unit: 0.625ms)   >= scan_window.
 *  \param      [in] scan_window     搜索窗口 Range: 0x0004 to 0x4000 (unit: 0.625ms),  <= scan_interval.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_set_scan_param(u8 scan_type,u16 scan_interval,u16 scan_window) */
#define ble_op_set_scan_param(scan_type,scan_interval,scan_window)     \
	ble_user_cmd_prepare(BLE_CMD_SCAN_PARAM, 3, scan_type, scan_interval, scan_window)

/*************************************************************************************************/
/*!
 *  \brief      配置creat filter policy,默认type: 0.
 *
 *  \function   ble_cmd_ret_e ble_op_set_create_filter_policy(u8 type).
 *
 *  \param      [in] type   Range: 0x00 to 0x03.
 *
 *  \return     see ble_cmd_ret_e.
 *
 *  \note       !!!注意：设置的时候必须在 设置创建连接参数 ble_op_create_connection 前配置好.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_set_create_filter_policy(u8 type) */
#define ble_op_set_create_filter_policy(type)     \
	ble_user_cmd_prepare(BLE_CMD_SET_HCI_CFG, 2,HCI_CFG_INITIATOR_FILTER_POLICY,type)

/*************************************************************************************************/
/*!
 *  \brief      BLE创建连接监听.
 *
 *  \function   ble_cmd_ret_e ble_op_create_connection(struct create_conn_param_t * create_conn_param).
 *
 *  \param      [in] create_conn_param 连接参数.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_create_connection(struct create_conn_param_t * create_conn_param) */
#define ble_op_create_connection(create_conn_param)     \
	ble_user_cmd_prepare(BLE_CMD_CREATE_CONN, 1, create_conn_param)

/*************************************************************************************************/
/*!
 *  \brief      BLE创建连接监听,扩展传入参数
 *
 *  \function   ble_cmd_ret_e ble_op_create_connection_ext(cosnt struct create_conn_param_ext_t * create_conn_param_ext).
 *
 *  \param      [in] create_conn_param_ext 连接参数.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_create_connection_ext(const struct create_conn_param_ext_t * create_conn_param_ext) */
#define ble_op_create_connection_ext(create_conn_param_ext)     \
	ble_user_cmd_prepare(BLE_CMD_CREATE_CONN_EXT, 1, create_conn_param_ext)

/*************************************************************************************************/
/*!
 *  \brief      取消BLE连接监听.
 *
 *  \function   ble_cmd_ret_e ble_op_create_connection_cancel(void).
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_create_connection_cancel(void) */
#define ble_op_create_connection_cancel()     \
	ble_user_cmd_prepare(BLE_CMD_CREATE_CONN_CANCEL,0)

/*************************************************************************************************/
/*!
 *  \brief      ble 主机搜索所有服务.
 *
 *  \function   ble_cmd_ret_e ble_op_search_profile_all(void).
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_search_profile_all(void) */
#define ble_op_search_profile_all()     \
	ble_user_cmd_prepare(BLE_CMD_SEARCH_PROFILE, 2, PFL_SERVER_ALL, 0)

/*************************************************************************************************/
/*!
 *  \brief      ble 主机搜索指定UUID16服务.
 *
 *  \function   ble_cmd_ret_e ble_op_search_profile_uuid16(u16 uuid16).
 *
 *  \param      [in] uuid16      uuid.
 *
 *  \return     see ble_cmd_ret_e.
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_search_profile_uuid16(u16 uuid16) */
#define ble_op_search_profile_uuid16(uuid16)     \
	ble_user_cmd_prepare(BLE_CMD_SEARCH_PROFILE, 2, PFL_SERVER_UUID16, uuid16)

/*************************************************************************************************/
/*!
 *  \brief      ble 主机搜索指定UUID128服务.
 *
 *  \function   ble_cmd_ret_e ble_op_search_profile_uuid128(const u8 *uuid128_pt).
 *
 *  \param      [in] uuid128_pt      uuid.
 *
 *  \return     see ble_cmd_ret_e.
 *
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_search_profile_uuid128(const u8 *uuid128_pt) */
#define ble_op_search_profile_uuid128(uuid128_pt)     \
	ble_user_cmd_prepare(BLE_CMD_SEARCH_PROFILE, 2, PFL_SERVER_UUID128, uuid128_pt)

/*************************************************************************************************/
/*!
 *  \brief      ble 主机更新连接参数.
 *
 *  \function   ble_cmd_ret_e ble_op_conn_param_update(u16 con_handle,struct conn_update_param_t *con_param).
 *
 *  \param      [in] con_handle     range：>0.
 *  \param      [in] con_param      (全局变量地址)，连接参数.
 *
 *  \return     see ble_cmd_ret_e.
 *
 */
/*************************************************************************************************/
/* ble_cmd_ret_e ble_op_conn_param_update(u16 con_handle,struct conn_update_param_t *con_param) */
#define ble_op_conn_param_update(con_handle,con_param)     \
	ble_user_cmd_prepare(BLE_CMD_ONNN_PARAM_UPDATA, 2, con_handle, con_param)



/*************************************************************************************************/
/*!
 *  \brief      ble master&slave 配置配对表(可以不设置,使用sdk默认值).
 *
 *  \param      [in] pair_devices_count     记录配对设备,range: 0~10,默认10.若配置为0:则不使用配对表记录管理，不限制配对个数.
 *  \param      [in] is_allow_cover         是否允许循环覆盖记录  1 or 0,默认1.
 *
 *  \return     true or false.
 *
 *  \note       上电调用配置,若配置的个数跟之前不一样，默认清所有的配对表数据.
 *  \note       VM 掉电记录保护.
 *  \note       主从机共用一个配对表.
 */
/*************************************************************************************************/
void ble_list_config_reset(u8 pair_devices_count, u8 is_allow_cover);


/*************************************************************************************************/
/*!
 *  \brief      配置是否接受新设备请求配对,记录在VM,(可以不设置,使用sdk默认值).
 *
 *  \param      [in] enable     允许新设备配对开关,1 or 0,默认1.
 *
 *  \return     true or false.
 *
 *  \note       VM 掉电记录保护.
 *  \note       是否覆盖是由接口 ble_list_config_reset 的参数 is_allow_cover 决定.
 */
/*************************************************************************************************/
bool ble_list_pair_accept(u8 enable);


/*************************************************************************************************/
/*!
 *  \brief      获取配对表已有配对个数.
 *
 *  \return     0~10.
 */
/*************************************************************************************************/
u16 ble_list_get_count(void);


/*************************************************************************************************/
/*!
 *  \brief      绑定已配对的指定设备,清除其他配对设备.
 *
 *  \param      [in] conn_addr        对方地址6bytes.
 *  \param      [in] conn_addr_type   对方地址类型range: 0~1.
 *
 *  \return     true or false.
 *
 *  \note       VM 掉电记录保护.
 */
/*************************************************************************************************/
bool ble_list_bonding_remote(u8 *conn_addr, u8 conn_addr_type);


/*************************************************************************************************/
/*!
 *  \brief      ble 清空配对表.
 *
 *  \return     true or false.
 */
/*************************************************************************************************/
bool ble_list_clear_all(void);

/*************************************************************************************************/
/*!
 *  \brief      ble 检测连接地址是否在已配对表中.
 *
 *  \param      [in] conn_addr        对方地址6bytes.
 *  \param      [in] conn_addr_type   对方地址类型range: 0~1.
 *
 *  \return     true or false.
 */
/*************************************************************************************************/
bool ble_list_check_addr_is_exist(u8 *conn_addr, u8 conn_addr_type);


/*************************************************************************************************/
/*!
 *  \brief      ble 把设备从配对表中删除.
 *
 *  \param      [in] conn_addr        对方地址6bytes.
 *  \param      [in] conn_addr_type   对方地址类型range: 0~1.
 *
 *  \return     true or false.
 */
/*************************************************************************************************/
bool ble_list_delete_device(u8 *conn_addr, u8 conn_addr_type);


/*************************************************************************************************/
/*!
 *  \brief      ble 获取配对表中最后连接设备的 id_address (public address).
 *
 *  \param      [out] id_addr        输出最后连接设备的id地址.
 *
 *  \return     true or false.
 */
/*************************************************************************************************/
bool ble_list_get_last_id_addr(u8 *id_addr);


/*************************************************************************************************/
/*!
 *  \brief      ble 获取已配对设备连接地址对应的 id_address (public address or static address).
 *
 *  \param      [in] conn_addr        对方连接地址6bytes.
 *  \param      [in] conn_addr_type   对方连接地址类型range: 0~1.
 *  \param      [out] id_addr         输出已配对设备的id地址.
 *
 *  \return     true or false.
 */
/*************************************************************************************************/
bool ble_list_get_id_addr(u8 *conn_addr, u8 conn_addr_type, u8 *id_addr);

/*************************************************************************************************/
/*!
 *  \brief      ble 获取已配对设备的系统类型.
 *
 *  \param      [in] conn_addr        对方连接地址6bytes.
 *  \param      [in] conn_addr_type   对方连接地址类型range: 0~1.
 *  \param      [out] output_type     见 remote_type_e 定义.
 *
 *  \return     true or false.
 *
 *  \note       只支持作为从机获取手机类型.
 */
/*************************************************************************************************/
bool ble_list_get_remote_type(u8 *conn_addr, u8 conn_addr_type, u8 *output_type);

/*************************************************************************************************/
/*!
 *  \brief      ble slave: att server 连接后主动发起请求MTU交换流程.
 *
 *  \param      [in] handle     range：>0.
 */
/*************************************************************************************************/
void att_server_set_exchange_mtu(u16 con_handle);

/*************************************************************************************************/
/*!
 *  \brief      ble slave: att server 使能流控功能.
 *
 *  \param      [in] handle     range：>0.
 *  \param      [in] enable     1 or 0.
 *
 *  \note       蓝牙初始化后可调用.
 */
/*************************************************************************************************/
void att_server_flow_enable(u8 enable);

/*************************************************************************************************/
/*!
 *  \brief      ble slave: att server 控制收数流控.
 *
 *  \param      [in] handle     range：>0.
 *  \param      [in] hold_flag  1--停止收发数据，0--开始正常收发数.
 *
 *  \note       蓝牙初始化后可调用,对方client要使用write的方式发送数据才能有流控生效
 */
/*************************************************************************************************/
void att_server_flow_hold(hci_con_handle_t con_handle, u8 hold_flag);

/*************************************************************************************************/
/*!
 *  \brief      ble slave: server 配对连接时，检查对方操作系统.
 *
 *  \param      [in] handle     range：>0.
 *  \param      [in] callback   检查完回调.
 *
 *  \note       在第一次配对连接时调用，HCI_EVENT_ENCRYPTION_CHANGE 事件后.
 */
/*************************************************************************************************/
void att_server_set_check_remote(u16 con_handle, void (*callback)(u16 con_handle, remote_type_e remote_type));


/*************************************************************************************************/
/*!
 *  \brief      检测协议栈att模块是否支持多机.
 *
 *  \param      [in] server_max     传入判断的从机个数.
 *  \param      [in] client_max     传入判断的主机个数·.
 *
 *  \return     0->success ,非0->fail.
 */
/*************************************************************************************************/
int att_send_check_multi_dev(u8 server_max, u8 client_max);


/*************************************************************************************************/
/*!
 *  \brief      可修改GATT服务的profile.
 *
 *  \param      [in] profile_data       gatt profile.
 *
 *  \return     0->success ,非0->fail.
 *
 *  \note       蓝牙未连接状态下,可调用修改.
 */
/*************************************************************************************************/
int att_server_change_profile(u8 const *profile_data);





























#endif

