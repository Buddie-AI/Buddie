#ifndef TWS_API_H
#define TWS_API_H

#include "typedef.h"

#include "classic/tws_event.h"
#include "classic/tws_local_media_sync.h"
#include "classic/tws_data_trans.h"

#define TWS_ROLE_MASTER   			0
#define TWS_ROLE_SLAVE    			1

/*
 * tws 状态
 */
#define TWS_STA_SIBLING_DISCONNECTED        0x00000001      //tws未连接
#define TWS_STA_SIBLING_CONNECTED           0x00000002      //tws已连接
#define TWS_STA_PHONE_DISCONNECTED          0x00000004      //手机未连接
#define TWS_STA_PHONE_CONNECTED             0x00000008      //手机已连接
#define TWS_STA_ESCO_OPEN                   0x00000010      //正在打电话
#define TWS_STA_SBC_OPEN                    0x00000020      //正在播歌
#define TWS_STA_MONITOR_START               0x00000040      //tws从机开始监听手机链路
#define TWS_STA_LOCAL_TWS_OPEN              0x00000080      //开启local_tws
#define TWS_STA_ESCO_OPEN_LINK              0x00000100      //正在打电话，create link
#define TWS_STA_MONITOR_ING                 0x00000200      //tws主从收到监听信息

#define TWS_SYNC_CALL_TX                1
#define TWS_SYNC_CALL_RX                2

struct tws_sync_call {
    int uuid;
    void (*func)(int priv, int err);
    const char *task_name;
};

extern const struct tws_sync_call  tws_sync_call_begin[];
extern const struct tws_sync_call  tws_sync_call_end[];

#define list_for_each_tws_sync_call(p) \
    for (p = tws_sync_call_begin; p < tws_sync_call_end; p++)


#define TWS_SYNC_CALL_REGISTER(sync_call) \
    static const struct tws_sync_call __tws_##sync_call sec(.tws_sync_call)


#define TWS_FUNC_ID(a, b, c, d)     (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

typedef void (*tws_func_t)(void *data, u16 len, bool rx);

struct tws_func_stub {
    u32 func_id;
    tws_func_t func;        //call from irq
};

#define REGISTER_TWS_FUNC_STUB(stub) \
    static const struct tws_func_stub stub sec(.tws_func_stub)

extern const struct tws_func_stub tws_func_stub_begin[];
extern const struct tws_func_stub tws_func_stub_end[];

static inline tws_func_t tws_function_get_by_id(u32 id)
{
    const struct tws_func_stub *p;

    for (p = tws_func_stub_begin; p < tws_func_stub_end; p++) {
        if (p->func_id == id) {
            return p->func;
        }
    }
    return NULL;
}

static inline void tws_function_call_by_id(u32 id, void *data, u16 len, bool rx)
{
    const struct tws_func_stub *p;

    for (p = tws_func_stub_begin; p < tws_func_stub_end; p++) {
        if (p->func_id == id) {
            p->func(data, len, rx);
            break;
        }
    }
}


/*
 * 通过搜索码搜索tws设备
 */
int tws_api_search_sibling_by_code(u16 code, int timeout_ms);

/*
 *打开可发现, 可连接，可被手机和tws搜索到
 */
int tws_api_wait_pair_by_code(u16 code, const char *name, int timeout_ms);
int tws_api_wait_pair_by_ble(u16 code, const char *name, int timeout_ms);

int tws_api_wait_tws_pair(int code, const char *name);

int tws_api_wait_phone_pair(int code, const char *name);


int tws_wait_tws_pair(u16 code, const char *name);

int tws_wait_phone_pair(u16 code, const char *name);


/*
 *取消可发现, 可连接，可被tws搜索到
 */
int tws_api_cancle_wait_pair();



/*
 * 搜索并连接已经配对过的tws
 * timeout: 单位ms， 0 表示不超时
 * 返回值:  0: 函数调用成功
 */
int tws_api_create_connection(int timeout);


/*
 * 取消搜索已配对的tws
 */
int tws_api_cancle_create_connection();


/*
 * 打开可发现，可连接, 可以被手机和已配对过的tws连接
 */
int tws_api_wait_connection();


/*
 * 断开tws直接的连接
 * reason: 断开原因
 */
int tws_api_detach(enum tws_detach_reason reason);

/*
 * 获取主从, 播歌和打电话状态下结果不可靠,请勿调用
 */
int tws_api_get_role();

/*
 * 获取tws 连接的状态
 * 返回值： 详见顶部TWS_STA_**
 */
int tws_api_get_tws_state();

/*
 * 设置tws对方地址
 */
int tws_api_set_sibling_addr(u8 *addr);

/*
 * 获取tws对方地址
 */
int tws_api_get_sibling_addr(u8 *addr);


/*
 * 获取tws本地地址
 */
int tws_api_get_local_addr(u8 *addr);

/*
 *发送解除配对命令给对方, 成功后会收到TWS_EVENT_REMOVE_PAIRS事件
 */
int tws_api_remove_pairs();


/*
 * 设置本地声道
 * 'L': 左声道
 * 'R': 右声道
 * 'U': 双声道合并
 */
void tws_api_set_local_channel(char channel);

/*
 * 获取本地声道
 */
char tws_api_get_local_channel();

/*
 * 通过uuid，主从同步调用相同函数
 */
int tws_api_sync_call_by_uuid(int uuid, int priv, int delay_ms);

/*
 * tws 数据发送函数, 要求 len <= 512
 */
int tws_api_send_data_to_sibling(void *data, u16 len, u32 func_id);

int tws_api_send_data_to_slave(void *data, int len, u32 func_id);

int tws_profile_init();
int tws_profile_exit();



int tws_api_connect_in_esco();
int tws_api_cancle_connect_in_esco();
int tws_disconnect();

/*
 * 使能对耳自动主从切换
 */
void tws_api_auto_role_switch_enable();

/*
 * 关闭对耳自动主从切换
 */
void tws_api_auto_role_switch_disable();

int tws_api_get_low_latency_state();

int tws_api_low_latency_enable(bool enable);


void tws_api_set_quick_connect_addr(u8 *addr);

u8 *tws_api_get_quick_connect_addr();

void tws_api_common_addr_en(u8 en);

void tws_api_pair_all_way(u8 en);

int tws_api_power_saving_mode_enable();

int tws_api_power_saving_mode_disable();

int tws_api_enter_pure_monitor_mode();

void tws_try_connect_disable(void);

void tws_conn_switch_role();

void tws_api_role_switch();

#endif
