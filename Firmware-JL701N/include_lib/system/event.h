#ifndef SYS_EVENT_H
#define SYS_EVENT_H

#include "generic/typedef.h"
#include "generic/list.h"
#include "generic/rect.h"

#define     KEY_POWER_START     0
#define     KEY_POWER           1
#define     KEY_PREV            2
#define     KEY_NEXT            3
#define 	KEY_OK 			    4
#define 	KEY_CANCLE 			5
#define 	KEY_MENU 			6
#define 	KEY_MODE 			7
#define     KEY_PHOTO           8
#define     KEY_ENC             9
#define     KEY_VOLUME_DEC      10
#define     KEY_VOLUME_INC      11
#define     KEY_PHONE           12


#define  	KEY_LEFT 			37
#define  	KEY_UP 				38
#define  	KEY_RIGHT 			39
#define  	KEY_DOWN 			40

#define 	KEY_0 				48
#define 	KEY_1 				49
#define 	KEY_2 				50
#define 	KEY_3 				51
#define 	KEY_4 				52
#define 	KEY_5 				53
#define 	KEY_6 				54
#define 	KEY_7 				55
#define 	KEY_8 				56
#define 	KEY_9 				57


#define 	KEY_F1 				60


#define SYS_ALL_EVENT           0xffff
#define SYS_KEY_EVENT 			0x0001
#define SYS_TOUCH_EVENT 		0x0002
#define SYS_DEVICE_EVENT 		0x0004
#define SYS_NET_EVENT 		    0x0008
#define SYS_BT_EVENT 		    0x0010
#define SYS_IR_EVENT 		    0x0020
#define SYS_PBG_EVENT           0x0040
#define SYS_BT_AI_EVENT 		0x0080
#define SYS_AI_EVENT 		    0x0100
#define SYS_MATRIX_KEY_EVENT    0x0200
#define SYS_TOUCHPAD_EVENT      0x0400
#define SYS_ADT_EVENT           0x0800
#define SYS_AUD_EVENT           0x1000




#define DEVICE_EVENT_FROM_AT_UART      (('A' << 24) | ('T' << 16) | ('U' << 8) | '\0')
#define DEVICE_EVENT_FROM_CHARGE	   (('C' << 24) | ('H' << 16) | ('G' << 8) | '\0')
#define DEVICE_EVENT_FROM_CFG_TOOL	   (('C' << 24) | ('F' << 16) | ('G' << 8) | '\0')
#define DEVICE_EVENT_FROM_POWER		   (('P' << 24) | ('O' << 16) | ('W' << 8) | '\0')
#define DEVICE_EVENT_FROM_CI_UART	   (('C' << 24) | ('I' << 16) | ('U' << 8) | '\0')
#define DEVICE_EVENT_FROM_CI_TWS 	   (('C' << 24) | ('I' << 16) | ('T' << 8) | '\0')
#define DEVICE_EVENT_CHARGE_STORE	   (('S' << 24) | ('T' << 16) | ('O' << 8) | '\0')
#define DEVICE_EVENT_UMIDIGI_CHARGE_STORE	   (('Q' << 24) | ('C' << 16) | ('Y' << 8) | '\0')
#define DEVICE_EVENT_TEST_BOX    	   (('T' << 24) | ('S' << 16) | ('B' << 8) | '\0')
#define DEVICE_EVENT_FROM_TONE		   (('T' << 24) | ('N' << 16) | ('E' << 8) | '\0')
#define DEVICE_EVENT_FROM_FM		   (('F' << 24) | ('M' << 16) | ('\0'<< 8) | '\0')
#define KEY_EVENT_FROM_TWS             (('T' << 24) | ('W' << 16) | ('S' << 8) | '\0')
#define SYS_BT_EVENT_FROM_TWS          (('T' << 24) | ('W' << 16) | ('S' << 8) | '\0')
#define DEVICE_EVENT_FROM_LINEIN	   (('A' << 24) | ('U' << 16) | ('X' << 8) | '\0')
#define DRIVER_EVENT_FROM_SD0          (('S' << 24) | ('D' << 16) | ('0' << 8) | '\0')
#define DRIVER_EVENT_FROM_SD1          (('S' << 24) | ('D' << 16) | ('1' << 8) | '\0')
#define DRIVER_EVENT_FROM_SD2          (('S' << 24) | ('D' << 16) | ('2' << 8) | '\0')
#define DEVICE_EVENT_FROM_MUSIC		   (('M' << 24) | ('S' << 16) | ('C' << 8) | '\0')
#define DEVICE_EVENT_FROM_USB_HOST     (('U' << 24) | ('H' << 16) | '\0' | '\0')
#define DEVICE_EVENT_FROM_OTG          (('O' << 24) | ('T' << 16) | ('G' << 8) | '\0')
#define DEVICE_EVENT_FROM_PC		   (('P' << 24) | ('C' << 16) | '\0' | '\0')
#define DEVICE_EVENT_FROM_UAC          (('U' << 24) | ('A' << 16) | ('C' << 8) | '\0')
#define DEVICE_EVENT_FROM_ALM		   (('A' << 24) | ('L' << 16) | ('M' << 8) | '\0')
#define SYS_BT_EVENT_TYPE_CON_STATUS   (('C' << 24) | ('O' << 16) | ('N' << 8) | '\0')
#define SYS_BT_EVENT_TYPE_HCI_STATUS   (('H' << 24) | ('C' << 16) | ('I' << 8) | '\0')
#define SYS_BT_EVENT_BLE_STATUS        (('B' << 24) | ('L' << 16) | ('E' << 8) | '\0')
#define SYS_BT_EVENT_FORM_COMMON       (('C' << 24) | ('M' << 16) | ('M' << 8) | '\0')
#define DEVICE_EVENT_FROM_KEY		   (('K' << 24) | ('E' << 16) | ('Y' << 8) | '\0')
#define DEVICE_EVENT_FROM_CUSTOM	   (('C' << 24) | ('S' << 16) | ('T' << 8) | '\0')
#define SYS_BT_AI_EVENT_TYPE_STATUS    (('B' << 24) | ('A' << 16) | ('I' << 8) | '\0')
#define DEVICE_EVENT_FROM_UART_RX_OVERFLOW		(('U' << 24) | ('R' << 16) | ('F' << 8) | '\0')
#define DEVICE_EVENT_FROM_UART_RX_OUTTIME		(('U' << 24) | ('R' << 16) | ('T' << 8) | '\0')
#define DEVICE_EVENT_FROM_DAC		   (('D' << 24) | ('A' << 16) | ('C' << 8) | '\0')
#define SYS_EVENT_FROM_CTRLER          (('C' << 24) | ('T' << 16) | ('R' << 8) | '\0')
#define SYS_EVENT_FROM_RECORD          (('R' << 24) | ('E' << 16) | ('C' << 8) | '\0')
#define DEVICE_EVENT_FROM_ENDLESS_LOOP_DEBUG          (('E' << 24) | ('L' << 16) | ('D' << 8) | '\0')
#define DEVICE_EVENT_FROM_EARTCH	   (('E' << 24) | ('T' << 16) | ('H' << 8) | '\0')
#define DEVICE_EVENT_ONLINE_DATA	   (('O' << 24) | ('L' << 16) | ('D' << 8) | '\0')
#define SYS_BT_EVENT_FROM_KEY       (('K' << 24) | ('E' << 16) | ('Y' << 8) | '\0')
#define SYS_BT_EVENT_FORM_SELF  (('S' << 24) | ('E' << 16) | ('F' << 8) | '\0')
#define DEVICE_EVENT_FROM_ANC   	   (('A' << 24) | ('N' << 16) | ('C' << 8) | '\0')
#define SYS_BT_EVENT_FORM_AT          (('I' << 24) | ('A' << 16) | ('T' << 8) | '\0')
#define DEVICE_EVENT_FROM_ADAPTER      (('A' << 24) | ('D' << 16) | ('A' << 8) | '\0')
#define DEVICE_EVENT_FROM_BOARD_UART   (('B' << 24) | ('D' << 16) | ('U' << 8) | '\0')
#define AUDIO_EVENT_TRIM_IMU_START       (('A' << 24) | ('T' << 16) | ('S' << 8) | '\0')
#define AUDIO_EVENT_TRIM_IMU_STOP       (('A' << 24) | ('T' << 16) | ('E' << 8) | '\0')

enum {
    KEY_EVENT_CLICK,
    KEY_EVENT_LONG,
    KEY_EVENT_HOLD,
    KEY_EVENT_UP,
    KEY_EVENT_DOUBLE_CLICK,
    KEY_EVENT_TRIPLE_CLICK,
    KEY_EVENT_FOURTH_CLICK,
    KEY_EVENT_FIRTH_CLICK,
    KEY_EVENT_USER,
    KEY_EVENT_MAX,
};


enum {
    DEVICE_EVENT_IN,
    DEVICE_EVENT_OUT,
    DEVICE_EVENT_ONLINE,
    DEVICE_EVENT_OFFLINE,
    DEVICE_EVENT_CHANGE,
};

enum {
    TOUCH_EVENT_DOWN,
    TOUCH_EVENT_MOVE,
    TOUCH_EVENT_HOLD,
    TOUCH_EVENT_UP,
    TOUCH_EVENT_CLICK,
    TOUCH_EVENT_DOUBLE_CLICK,
};

enum {
    NET_EVENT_CMD,
    NET_EVENT_DATA,
    NET_EVENT_CONNECTED,
    NET_EVENT_DISCONNECTED,
    NET_EVENT_SMP_CFG_TIMEOUT,
};


struct key_event {
    u8 init;
    u8 type;
    u16 event;
    u32 value;
    u32 tmr;
};

struct ir_event {
    u8 event;
};

struct msg_event {
    u8 event;
    u8 value;
};

#if EVENT_TOUCH_ENABLE_CONFIG
struct touch_event {
    u8 event;
    struct position pos;
};
#endif

struct device_event {
    u8 event;
    int value;
};

struct chargestore_event {
    u8 event;
    u8 *packet;
    u8 size;
};

struct testbox_event {
    u8 event;
    u8 *packet;
    u8 size;
};

struct cfg_tool_event {
    u8 event;
    u8 *packet ;
    u8 size;
};

struct umidigi_chargestore_event {
    u8 event;
    u8 value;
};

struct ancbox_event {
    u8 event;
    u32 value;
};

struct net_event {
    u8 event;
    u8 value;
};
struct bt_event {
    u8 event;
    u8 args[7];
    u32 value;
};

struct axis_event {
    u8 event;
    s16 x;
    s16 y;
};

struct codesw_event {
    u8 event;
    s8 value;
};

struct pbg_event {
    u8 event;
    u8 args[3];
};

struct adt_event {
    u8 event;
    u8 args[3];
};

struct uart_event {
    void *ut_bus;
};

struct uart_cmd_event {
    u8 type;
    u8 cmd;
};

struct ai_event {
    u32 value;
};

struct ear_event {
    u8 value;
};

struct rcsp_event {
    u8 event;
    u8 args[6];
    u8 size;
};

struct chargebox_event {
    u8 event;
};

struct matrix_key_event {
    u16 args[6];            //最多推6个按键出来，如果需要推多个按键需要自行修改，每个u16 低八位标识row 高八位标识col
    u8 *map;
};

struct touchpad_event {
    u8 gesture_event;       //手势事件
    s8 x;
    s8 y;
};

struct sys_event {
    u16 type;
    u8 consumed;
    void *arg;
    union {
        struct key_event key;
        struct axis_event axis;
        struct codesw_event codesw;
#if EVENT_TOUCH_ENABLE_CONFIG
        struct touch_event 	touch;
#endif
        struct device_event dev;
        struct net_event 	net;
        struct bt_event 	bt;
        struct msg_event 	msg;
        struct chargestore_event chargestore;
        struct cfg_tool_event cfg_tool;
        struct ir_event     ir;
        struct pbg_event    pbg;
        struct uart_event	uart;
        struct uart_cmd_event	uart_cmd;
        struct ai_event     ai;
        struct ear_event    ear;
        struct rcsp_event	rcsp;
        struct chargebox_event chargebox;
        struct testbox_event testbox;
        struct umidigi_chargestore_event umidigi_chargestore;
        struct ancbox_event ancbox;
        struct matrix_key_event  matrix_key;
        struct touchpad_event touchpad;
        struct adt_event    adt;
    } u;
};




struct static_event_handler {
    int event_type;
    void (*handler)(struct sys_event *);
};


#define SYS_EVENT_HANDLER(type, fn, pri) \
	const struct static_event_handler __event_handler_##fn sec(.sys_event.pri.handler) = { \
		.event_type = type, \
		.handler = fn, \
	}

extern struct static_event_handler sys_event_handler_begin[];
extern struct static_event_handler sys_event_handler_end[];

#define list_for_each_static_event_handler(p) \
	for (p = sys_event_handler_begin; p < sys_event_handler_end; p++)



int register_sys_event_handler(int event_type, int from, u8 priority,
                               void (*handler)(struct sys_event *));


void unregister_sys_event_handler(void (*handler)(struct sys_event *));


/*
 * 事件通知函数,系统有事件发生时调用此函数
 */
void sys_event_notify(struct sys_event *e);

void sys_event_clear(struct sys_event *e);

void sys_key_event_disable();


void sys_key_event_enable();

void sys_key_event_filter_disable();

void sys_key_event_filter_enable();

void sys_touch_event_disable();


void sys_touch_event_enable();

/*
 *下面四个为系统事件消耗函数，调用此函数后则对应的事件不在分发给其它任务
 *
 */
void sys_event_consume(struct sys_event *e);

void sys_key_event_consume(struct key_event *e);

#if EVENT_TOUCH_ENABLE_CONFIG
void sys_touch_event_consume(struct touch_event *e);
#endif

void sys_device_event_consume(struct device_event *e);


/*
 * 下面两个函数为按键和触摸事件接管函数，调用此函数后则对应的事件只发到当前任务
 *
 * on=true: 开始接管， on=false: 取消接管
 *
 * once:  on = false 时有效，当前这次不接管, 事件可以继续发送到其它任务
 *
 */
void sys_key_event_takeover(bool on, bool once);

void sys_touch_event_takeover(bool on, bool once);


int sys_key_event_map(struct key_event *org, struct key_event *new);
int sys_key_event_unmap(struct key_event *org, struct key_event *new);


#endif
