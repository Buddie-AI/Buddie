#ifndef __ONLINE_DB_DEAL_H__
#define __ONLINE_DB_DEAL_H__
#include "typedef.h"

typedef enum {
    DB_PKT_TYPE_ACK = 0,
    DB_PKT_TYPE_PRINT,
    DB_PKT_TYPE_EQ = 5,
    DB_PKT_TYPE_ANC = 7,
    DB_PKT_TYPE_AEC = 8,
    DB_PKT_TYPE_EXPORT = 0x9,
    DB_PKT_TYPE_TOUCH = 0xA,
    DB_PKT_TYPE_DMS = 0xB,
    DB_PKT_TYPE_SPK_EQ = 0xC,
    DB_PKT_TYPE_DAT_CH0 = 0x10,
    DB_PKT_TYPE_DAT_CH1,
    DB_PKT_TYPE_DAT_CH2,
    DB_PKT_TYPE_MIC_DUT = 0x20,/*麦克风测试功能*/
    DB_PKT_TYPE_SPATIAL_EFFECT = 0x21,  /*空间音效调试功能*/
    DB_PKT_TYPE_MAX,
} db_pkt_e;

typedef enum {
    DB_COM_TYPE_NULL = 0,
    DB_COM_TYPE_SPP,
    DB_COM_TYPE_BLE,
} db_com_e;

struct db_online_api_t {
    void (*init)(db_com_e conn_type);
    void (*exit)(void);
    void (*register_send_data)(void *send_api);
    void (*send_wake_up)(void);
    int (*packet_handle)(u8 *packet, u16 size);
};

/*
   @funtion 获取可写入缓存的数据长度
   @param [in] type
   @return 可写入的长度
 */
int app_online_get_buf_remain(db_pkt_e type);

/*
   @funtion 大数据包发送,超过253,会拆包，蓝牙一次发多包
   @param [in] type
   @param [in] packet  数据地址
   @param [in] size    数据长度, range:1~512
   @return  0 sucess,others fail
 */
int app_online_db_send_more(db_pkt_e type, u8 *packet, u16 size);


/*
   @funtion 数据包发送
   @param [in] type
   @param [in] packet  数据地址
   @param [in] size    数据长度, range:1~253
   @return  0 sucess,others fail
 */
int app_online_db_send(db_pkt_e type, u8 *packet, u8 size);

/*
   @funtion 应答包发送
   @param [in] seq  应答对应接收包数据的seq值
   @param [in] packet  数据地址
   @param [in] size    数据长度, range:1~253
   @return  0 sucess,others fail
 */
int app_online_db_ack(u8 seq, u8 *packet, u8 size);

/*
   @funtion 注册接收数据包类型的处理函数
   @param [in] type
   @param [in] db_parse_datap,  funtion to callback

   int (*db_parse_data)(u8 *packet, u8 size, u8 *ext_data, u16 ext_size)
   @param [in] packet  数据地址
   @param [in] size    数据长度, range:1~253
   @param [in] ext_data ,协议层head的起始地址,db_head_t
   @param [in] ext_size ,数据 0~2
   @return  0 sucess,others fail

 */
void app_online_db_register_handle(db_pkt_e type, int (*db_parse_data)(u8 *packet, u8 size, u8 *ext_data, u16 ext_size));

/*
   @funtion get api funtion
   @return api_funtion point.
 */
struct db_online_api_t *app_online_get_api_table(void);


//debug printf,提供简单打印接口
void app_online_putchar(char a);
void app_online_puts(char *str);
void app_online_put_u8hex(u8 dat);
void app_online_put_u16hex(u16 dat);
void app_online_put_u32hex(u32 dat);
void app_online_put_buf(u8 *buf, u16 len);

#endif//
