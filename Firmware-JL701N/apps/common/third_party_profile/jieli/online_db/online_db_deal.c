#include "app_config.h"
#include "app_action.h"
#include "system/includes.h"
#include "string.h"
#include "online_db_deal.h"
#include "circular_buf.h"
#include "bt_common.h"

#if APP_ONLINE_DEBUG

#define LOG_TAG_CONST       ONLINE_DB
#define LOG_TAG             "[ONLINE_DB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if (TCFG_AUDIO_DATA_EXPORT_ENABLE && (TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_SPP))
#define    DB_MEM_BUF_LEN      (768 * 15)
#else
#define    DB_MEM_BUF_LEN      (768)
#endif
#define    DB_ONE_PACKET_LEN   (256)


#define    DB_PRINT_DATA_EN        0//是否上报串口打印数据
#define    DB_PRINT_DELAY_TYPE     0//0-timeout check,1-time check
#define    DB_PRINT_NOT_SEND_HEAD  0//

#define    DB_TIMER_SET        (200) //unit:ms
#define    DB_PAYLOAD_MAXSIZE  (253)
#define    DB_TMP_PACKET_LEN   (660)

#if DB_PRINT_DATA_EN
#define    DB_PRINT_BUFF_LEN   (128)
#endif

#define    cbuf_get_space(a)    (a)->total_len

enum {
    DB_EVT_MSG_NULL = 0,
    DB_EVT_MSG_SEND,
};

enum {
    DB_TO_TYPE_PRINT = 0,
};

struct db_head_t {
    u8 length;
    u8 type;
    u8 seq;
    u8 data[0];
} _GNU_PACKED_;

struct db_online_info_t {
    u16 db_timer;
    u8  res_byte;
    u8  seq;
    int (*send_api)(u8 *packet, u16 size);
    cbuffer_t send_cbuf;
    u8  db_mem_buffer[DB_MEM_BUF_LEN];
    u8  tmp_send_buf[DB_TMP_PACKET_LEN];
    u16 tmp_data_len;
    u8  tmp_recieve_buf[DB_ONE_PACKET_LEN];
    u16 tmp_recieve_len;

#if DB_PRINT_DATA_EN
    u8  print_buffer[DB_PRINT_BUFF_LEN * 2];
    volatile u8 *print_buf_prt;
    volatile u16 print_count;
#endif
};

static volatile u8 db_send_busy = 0;
static volatile u8 db_timeout_type = 0;
static volatile u16 print_record_count = 0;

static struct db_online_info_t  *online_global_info;
#define __this  (online_global_info)

static int db_cb_api_table[DB_PKT_TYPE_MAX];
static volatile u8 db_active = DB_COM_TYPE_NULL;

#define JUST_THIS_IS_NULL() (__this == NULL)
//-----------------------------------------------------
extern void register_putbyte_hook(int (*hook_call)(char a));
static void db_data_try_send(void);
static void db_data_send_msg(int evt_msg);
static void db_print_test_function(void);
static void db_timeout_handle(int type);
//-----------------------------------------------------
//获取cbuff 数据
static u16 db_data_read_sub(u8 *buf, u16 buf_size)
{
    u16 ret_len;
    if (0 == cbuf_get_data_size(&__this->send_cbuf)) {
        return 0;
    }

    OS_ENTER_CRITICAL();
    if (cbuf_get_data_size(&__this->send_cbuf)) {//check again
        cbuf_read(&__this->send_cbuf, &ret_len, 2);
        if (ret_len && ret_len <= buf_size) {
            cbuf_read(&__this->send_cbuf, buf, ret_len);
        }
    }
    OS_EXIT_CRITICAL();

    return ret_len;
}

//尝试发送cbuff 数据
static void db_data_try_send(void)
{
    u16 send_len;

    if (db_send_busy) {
        return;
    }
    db_send_busy = 1;//hold

    if (__this->tmp_data_len) {
        /* log_info("send_tmp_data = %d",__this->tmp_data_len); */
        if (__this->send_api) {
            if (__this->send_api(__this->tmp_send_buf, __this->tmp_data_len)) {
                db_send_busy = 0;//fail
                return;
            }
            __this->tmp_data_len = 0;
        }
    }

    send_len = db_data_read_sub(__this->tmp_send_buf, DB_TMP_PACKET_LEN);

    if (send_len > DB_TMP_PACKET_LEN) {
        printf("err:send_len= %d\n", send_len);
        ASSERT(0);
    }

    if (send_len) {
        /* log_info("send_buf_data = %d",send_len); */
        if (__this->send_api(__this->tmp_send_buf, send_len)) {
            __this->tmp_data_len = send_len;
            db_send_busy = 0;//fail
        }
    } else {
        //not send
        db_send_busy = 0;
    }
}

//写入数据到cbuff
static u32 db_data_write_sub(u8 *head, u8 head_size, u8 *data, u16 len)
{
    u16 wlen = 0;
    u16 total_len = 0;

    if (!db_active) {
        /* log_error("not active!"); */
        return 0;
    }

    if (len > DB_ONE_PACKET_LEN) {
        ASSERT(0);
    }

    total_len = head_size + len;

    OS_ENTER_CRITICAL();
    u16 buf_space = cbuf_get_space(&__this->send_cbuf) - cbuf_get_data_size(&__this->send_cbuf);
    if (total_len + 2 > buf_space) {
        /* log_error("cbuf full!"); */
        OS_EXIT_CRITICAL();
        return 0;
    }

    wlen =  cbuf_write(&__this->send_cbuf, &total_len, 2);
    if (head_size) {
        wlen += cbuf_write(&__this->send_cbuf, head, head_size);
    }
    if (len) {
        wlen += cbuf_write(&__this->send_cbuf, data, len);
    }
    OS_EXIT_CRITICAL();

    /* db_data_try_send(); */
    if (!db_send_busy) {
        db_data_send_msg(DB_EVT_MSG_SEND);
    }
    return wlen;
}

//处理协议栈来数，分发到已注册模块解析处理
static void db_packet_handle(u8 *packet, u8 size)
{
    if (!db_active) {
        return;
    }

    struct db_head_t *db_ptr = (void *)packet;
    int ret = -1;

    if (db_ptr->length == 0) {
        log_error("len=0");
        return;
    }


    //判读是否需要缓存组包
    if (__this->tmp_recieve_len) {
        if (__this->tmp_recieve_len + size > DB_ONE_PACKET_LEN) {
            log_error("rx_len_err:%d", __this->tmp_recieve_len + size);
            __this->tmp_recieve_len = 0;
            return;
        }

        memcpy(&__this->tmp_recieve_buf[__this->tmp_recieve_len], packet, size);
        __this->tmp_recieve_len += size;
        if (__this->tmp_recieve_len < db_ptr->length) {
            /* putchar('<'); */
            return;
        }

        db_ptr = (void *)&__this->tmp_recieve_buf;
        packet = (void *)&__this->tmp_recieve_buf;
        size = __this->tmp_recieve_len;
        __this->tmp_recieve_len = 0;
        /* putchar('>'); */
    } else {
        if (size < db_ptr->length) {
            __this->tmp_recieve_len = size;
            memcpy(&__this->tmp_recieve_buf, packet, size);
            /* putchar('<'); */
            return;
        }
    }

    if (db_ptr->type == 0 || db_ptr->type >= DB_PKT_TYPE_MAX) {
        log_error("type_err");
        return;
    }


    /* int (*db_parse_data)(u8 *packet,u8 size,u8 *ext_data,ext_size); */
    int (*db_parse_data)(u8 * packet, u8 size, u8 * ext_data, u16 ext_size) = (void *)(db_cb_api_table[db_ptr->type]);

    if (db_parse_data) {
        /* log_info("db_parse_data,%08x",db_parse_data); */

        log_info("rx_data(%d):", size);
        log_info_hexdump((void *)db_ptr, size);

        u8 *tmp_buf_pt = malloc(DB_ONE_PACKET_LEN);
        if (!tmp_buf_pt) {
            log_error("malloc faill,drop data!!!");
            return;
        }

        size -= 3;
        //整理对齐
        memcpy(tmp_buf_pt, db_ptr->data, size);
        db_parse_data(tmp_buf_pt, size, &db_ptr->type, 2);
        free(tmp_buf_pt);
    }
}


//打印数据推送处理，需要临界保护
static void db_print_send_deal(void)
{
#if DB_PRINT_DATA_EN
    static volatile u8 send_flag = 0;
    u16 tmp_len = 0;
    u8 *tmp_buf;

    if (send_flag) {
        __this->print_count = 0;//drop data
        return;
    }

    //乒乓buffer
    OS_ENTER_CRITICAL();
    send_flag = 1;
    if (__this->print_count) { //check again
        tmp_len = __this->print_count;
        tmp_buf = __this->print_buf_prt;
        if (__this->print_buf_prt == __this->print_buffer) {
            __this->print_buf_prt = &__this->print_buffer[DB_PRINT_BUFF_LEN];
        } else {
            __this->print_buf_prt = __this->print_buffer;
        }
        __this->print_count = 0;
    }
    OS_EXIT_CRITICAL();
    if (tmp_len) {
        app_online_db_send(DB_PKT_TYPE_PRINT, tmp_buf, tmp_len);
    }
    send_flag = 0;
#endif
}

//截取串口打印数据处理
static int db_hook_put_byte(char a)
{
    int ret = 0; //0--keep uart,1--not uart

#if DB_PRINT_DATA_EN
    static volatile repeat_cnt = 0;

    if (!db_active) {
        return ret;
    }

    if (repeat_cnt) {
        return ret;
    }
    repeat_cnt = 1;

    if (__this->print_count > (DB_PRINT_BUFF_LEN - 4)) {
        db_print_send_deal();
    }
    __this->print_buf_prt[__this->print_count++] = (u8)a;
    repeat_cnt = 0;

#if(0 == DB_PRINT_DELAY_TYPE)
    if (0 == (db_timeout_type & BIT(DB_TO_TYPE_PRINT))) {
        db_timeout_type |= BIT(DB_TO_TYPE_PRINT);
        print_record_count = __this->print_count;
        sys_timeout_add(DB_TO_TYPE_PRINT, db_timeout_handle, DB_TIMER_SET);
    }
#endif

#endif
    return ret;
}

#if(1 == DB_PRINT_DELAY_TYPE)
//定义检测打印数据是否需要超时推送到上位机
static void db_print_timer_handle(void)
{
#if DB_PRINT_DATA_EN
    if (__this->print_count) {
        if (print_record_count == __this->print_count) { //第二次检查相同才输出打印
            db_print_send_deal();
        } else {
            print_record_count = __this->print_count;
        }
    }
#endif
}
#endif

//检测打印数据是否需要超时推送到上位机
static void db_timeout_handle(int type)
{
#if DB_PRINT_DATA_EN
#if(0 == DB_PRINT_DELAY_TYPE)
    if (!db_active) {
        return;
    }

    /* putchar('&'); */
    if (type == DB_TO_TYPE_PRINT) {
        if (__this->print_count) { //第二次检查相同才输出打印
            if (print_record_count == __this->print_count) { //第二次检查相同才输出打印
                db_timeout_type &= (~BIT(DB_TO_TYPE_PRINT));
                db_print_send_deal();
            } else {
                print_record_count = __this->print_count;
                sys_timeout_add(DB_TO_TYPE_PRINT, db_timeout_handle, DB_TIMER_SET);
            }
        } else {
            db_timeout_type &= (~BIT(DB_TO_TYPE_PRINT));
        }
    }
#endif
#endif
}

/* static u8 test_buffer_data22[512]; */
/* static void db_test_send_data(void) */
/* { */
/* app_online_db_send_more(0, test_buffer_data22, 512); */
/* } */

//上位机连上初始化
static void db_init(db_com_e conn_type)
{
    if (db_active) {
        return;
    }

    log_info("db_init");
    if (JUST_THIS_IS_NULL()) {
        __this = malloc(sizeof(struct db_online_info_t));
        if (JUST_THIS_IS_NULL()) {
            log_error("malloc faill!!!");
            return;
        }
        log_info("malloc ok");
    }

    cbuf_init(&__this->send_cbuf, __this->db_mem_buffer, DB_MEM_BUF_LEN);

#if(1 == DB_PRINT_DELAY_TYPE)
    __this->db_timer = sys_timer_add(NULL, db_print_timer_handle, DB_TIMER_SET);
#endif

    __this->tmp_data_len = 0;
    __this->tmp_recieve_len = 0;
    db_send_busy = 0;

#if DB_PRINT_DATA_EN
    __this->print_count = 0;
    __this->print_buf_prt = __this->print_buffer;
    /* register_putbyte_hook(db_hook_put_byte); */
#endif

    db_active = conn_type;
    //db_print_test_function();

    /* for (int i = 0; i < 512; i++) { */
    /* test_buffer_data22[i] = i; */
    /* } */
    /* sys_timer_add(0, db_test_send_data, 200); */
    /* printf_buf(test_buffer_data22, 512); */
}

//上位断开，退出
void db_exit(void)
{
    if (!db_active) {
        return;
    }

    log_info("db_exit");
    db_active = DB_COM_TYPE_NULL;

#if(1 == DB_PRINT_DELAY_TYPE)
    if (__this->db_timer) {
        sys_timer_del(__this->db_timer);
        __this->db_timer = 0;
    }
#endif

#if DB_PRINT_DATA_EN
    /* register_putbyte_hook(NULL); */
#endif

    OS_ENTER_CRITICAL();
    cbuf_clear(&__this->send_cbuf);
    free(__this);
    __this = NULL;
    OS_EXIT_CRITICAL();
}


//注册协议栈发送接口
static void db_register_send_data(void *send_api_call)
{
    if (!db_active) {
        return;
    }
    __this->send_api = send_api_call;
}

//协议栈发送完成，唤醒
static void db_wakeup_send_data(void *send_api_call)
{
    if (!db_active) {
        return;
    }

    db_send_busy = 0;//
    db_data_try_send();
}

static void db_data_send_msg(int evt_msg)
{
    struct sys_event e;
    e.type = SYS_DEVICE_EVENT;
    e.arg  = (void *)DEVICE_EVENT_ONLINE_DATA;
    e.u.dev.event = 0;
    e.u.dev.value = evt_msg;
    sys_event_notify(&e);
}

struct db_online_api_t de_online_api_table = {
    .init = db_init,
    .exit = db_exit,
    .packet_handle = db_packet_handle,
    .register_send_data = db_register_send_data,
    .send_wake_up = db_wakeup_send_data,
};

//获取接口列表
struct db_online_api_t *app_online_get_api_table(void)
{
    return &de_online_api_table;
}

//注册模块解析函数
void app_online_db_register_handle(db_pkt_e type, int (*db_parse_data)(u8 *packet, u8 size, u8 *ext_data, u16 ext_size))
{
    if (type == 0 || type >= DB_PKT_TYPE_MAX) {
        log_error("reg_type_err");
        return;
    }
    log_info("register_handle %d,%08x", type, db_parse_data);
    db_cb_api_table[type] = (int)db_parse_data;
}

//获取可发送数据的长度
int app_online_get_buf_remain(db_pkt_e type)
{

    if (!db_active) {
        return 0;
    }

    int w_len = 0;
    int head_size = 3;
    int buf_space = cbuf_get_space(&__this->send_cbuf) - cbuf_get_data_size(&__this->send_cbuf);
    int pack_size = head_size + DB_PAYLOAD_MAXSIZE;

    if (buf_space <= head_size) {
        w_len = 0;
    } else if (buf_space < pack_size) {
        w_len = buf_space - head_size;
    } else {
        w_len = (buf_space / pack_size) * DB_PAYLOAD_MAXSIZE;
        buf_space = buf_space % pack_size;
        if (buf_space > head_size) {
            w_len += (buf_space - head_size);
        }
    }
    return w_len;
}

//发送包，发送
int app_online_db_send(db_pkt_e type, u8 *packet, u8 size)
{
    if (!db_active) {
        return -1;
    }

    struct db_head_t db_ptr;
    db_ptr.length = size + 2;
    db_ptr.type = type;
    db_ptr.seq = __this->seq++;
    u8 head_size = 3;

    if (!size) {
        log_error("size err!!!");
        return -2;
    }

#if (DB_PRINT_DATA_EN && DB_PRINT_NOT_SEND_HEAD)
    if (DB_PKT_TYPE_PRINT == type) {
        head_size = 0;//不发送头
    }
#endif

    log_info("tx_data(%d):,type=%d", size, type);
    log_info_hexdump(packet, size);

    if (!db_data_write_sub(&db_ptr, head_size, packet, size)) {
        log_error("full fail,%d,%d", type, size);
        return -3;
    }
    return 0;
}

//发送包，发送
int app_online_db_send_more(db_pkt_e type, u8 *packet, u16 size)
{
    if (!db_active) {
        return -1;
    }

    struct db_head_t db_ptr;
    u8 head_size = 3;

    if (!size || size > 650) {
        log_error("overflow!!!%d,%d", type, size);
        return -2;
    }

    log_info("tx_more_data(%d):,type=%d", size, type);
    log_info_hexdump(packet, size);

    u16 need_buff_size = ((size / DB_PAYLOAD_MAXSIZE) * (DB_PAYLOAD_MAXSIZE + head_size));
    u16 remain_size =  size % DB_PAYLOAD_MAXSIZE;

    if (remain_size) {
        need_buff_size += (remain_size + head_size);
    }

    OS_ENTER_CRITICAL();
    if (need_buff_size + 2 > app_online_get_buf_remain(type)) {
        OS_EXIT_CRITICAL();
        log_error("more full!!!");
        return -3;
    }

    u16 payload_size;
    db_ptr.type = type;
    cbuf_write(&__this->send_cbuf, &need_buff_size, 2);

    while (size) {
        if (size > DB_PAYLOAD_MAXSIZE) {
            payload_size = DB_PAYLOAD_MAXSIZE;
        } else {
            payload_size = size;
        }
        size -= payload_size;
        db_ptr.length = payload_size + 2;
        db_ptr.seq = __this->seq++;
        cbuf_write(&__this->send_cbuf, &db_ptr, head_size);
        cbuf_write(&__this->send_cbuf, packet, payload_size);
        packet += payload_size;
    }
    OS_EXIT_CRITICAL();

    db_data_try_send();
    /* if (!db_send_busy) { */
    /* db_data_send_msg(DB_EVT_MSG_SEND); */
    /* } */
    return 0;
}


//应答包，发送
int app_online_db_ack(u8 seq, u8 *packet, u8 size)
{
    if (!db_active) {
        return -1;
    }

    struct db_head_t db_ptr;
    db_ptr.length = size + 2;
    db_ptr.type = DB_PKT_TYPE_ACK;
    db_ptr.seq = seq;

    log_info("tx_ack(%d):,seq=%d", size, seq);
    log_info_hexdump(packet, size);

    if (!db_data_write_sub(&db_ptr, 3, packet, size)) {
        log_error("send fail2,%d,%d", seq, size);
        return -1;
    }
    return 0;
}



void app_online_event_handle(int evt_value)
{
    if (!db_active) {
        return;
    }

    if (evt_value == DB_EVT_MSG_SEND) {
        /* putchar('@'); */
        db_data_try_send();
    }
}

void app_online_putchar(char a)
{
    db_hook_put_byte(a);
}

void app_online_puts(char *str)
{
    while (*str != 0) {
        db_hook_put_byte(*str++);
    }
}

static void app_online_put_u4hex(unsigned char dat)
{
    dat = 0xf & dat;
    if (dat > 9) {
        app_online_putchar(dat - 10 + 'A');
    } else {
        app_online_putchar(dat + '0');
    }
}


void app_online_put_u16hex(u16 dat)
{
    app_online_putchar('0');
    app_online_putchar('x');

    app_online_put_u4hex(dat >> 12);
    app_online_put_u4hex(dat >> 8);
    app_online_put_u4hex(dat >> 4);
    app_online_put_u4hex(dat);

    app_online_putchar(' ');
}

void app_online_put_u8hex(u8 dat)
{
    app_online_put_u4hex(dat >> 4);
    app_online_put_u4hex(dat);
    app_online_putchar(' ');
}

void app_online_put_u32hex(u32 dat)
{
    app_online_putchar('0');
    app_online_putchar('x');

    app_online_put_u4hex(dat >> 28);
    app_online_put_u4hex(dat >> 24);
    app_online_put_u4hex(dat >> 20);
    app_online_put_u4hex(dat >> 16);
    app_online_put_u4hex(dat >> 12);
    app_online_put_u4hex(dat >> 8);
    app_online_put_u4hex(dat >> 4);
    app_online_put_u4hex(dat);

    app_online_putchar(' ');
}

void app_online_put_buf(u8 *buf, u16 len)
{
    for (int i = 0; i < len; i++) {
        if ((i % 16) == 0) {
            app_online_putchar('\n') ;
        }
        app_online_put_u8hex(buf[i]) ;
    }
    app_online_putchar('\n') ;
}

static const u8 test_buffer_data[8] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
static void db_print_test_function(void)
{
    app_online_puts("print test start\n");
    app_online_putchar('@');
    app_online_put_u8hex(0x78);
    app_online_put_u16hex(0x1234);
    app_online_put_u32hex(0x33445566);
    app_online_put_buf(test_buffer_data, 32);
    app_online_puts("print test end\n");
}



#endif



