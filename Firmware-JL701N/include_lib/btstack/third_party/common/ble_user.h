#ifndef __BLE_USER_H__
#define __BLE_USER_H__

#include "typedef.h"

typedef enum {
    BLE_ST_NULL = 0,
    BLE_ST_INIT_OK,             //协议栈初始化ok
    BLE_ST_IDLE,                //关闭广播或扫描状态
    BLE_ST_CONNECT,             //链路刚连上
    BLE_ST_SEND_DISCONN,        //发送断开命令，等待链路断开
    BLE_ST_DISCONN,             //链路断开状态
    BLE_ST_CONNECT_FAIL,        //连接失败
    BLE_ST_CONNECTION_UPDATE_OK,//更新连接参数完成

    BLE_ST_ADV = 0x20,          //设备处于广播状态
    BLE_ST_NOTIFY_IDICATE,      //设备已连上,允许发数(已被主机使能通知)

    BLE_ST_SCAN = 0x40,             //设备处于搜索状态
    BLE_ST_CREATE_CONN,             //发起设备连接
    BLE_ST_SEND_CREATE_CONN_CANNEL, //取消发起设备连接
    BLE_ST_SEARCH_COMPLETE,         //链路连上，已搜索完profile，可以发送数据操作

    BLE_ST_SEND_STACK_EXIT = 0x60,  //发送退出协议栈命令，等待完成
    BLE_ST_STACK_EXIT_COMPLETE,     //协议栈退出成功

} ble_state_e;

enum {
    APP_BLE_NO_ERROR = 0,
    APP_BLE_BUFF_FULL,       //buffer 满，会丢弃当前发送的数据包
    APP_BLE_BUFF_ERROR,      //
    APP_BLE_OPERATION_ERROR, //操作错误
    APP_BLE_IS_DISCONN,      //链路已断开
    APP_BLE_NO_WRITE_CCC,    //主机没有 write Client Characteristic Configuration
};

struct BLE_CONFIG_VAR {
    ble_state_e  JL_ble_status;
    struct ble_server_operation_t *rcsp_ble;
};

#endif//__BLE_USER_H__
