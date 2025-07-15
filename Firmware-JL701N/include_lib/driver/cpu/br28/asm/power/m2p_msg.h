#ifndef __M2P_MSG_H__
#define __M2P_MSG_H__

//m2p用户消息类型
enum {
    M2P_MSG_ACK = BIT(0),
    M2P_MSG_TEST = BIT(1),
    M2P_MSG_COMMOM = BIT(2),
    M2P_MSG_CTMU = BIT(3),
    M2P_MSG_SENSOR = BIT(4),
    M2P_MSG_VAD = BIT(5),
};

//测试
struct m2p_msg_test {
    u8 dat;
};

//测试
struct m2p_msg_ack {
    u8 dat;
};

//公共消息
struct m2p_msg_common {
    u8 dat;
};

//触摸消息
struct m2p_msg_ctmu {
    u8 dat;
};

//vad
struct m2p_msg_vad {
    u8 dat;
};

//m2p用户消息格式
struct m2p_msg_head {
u16 type :
    MSG_TYPE_BIT_LEN;
u16 len  :
    MSG_PARAM_BIT_LEN;
u8 index :
    MSG_INDEX_BIT;
u8 ack   :
    MSG_ACK_BIT;
} __attribute__((packed));

struct m2p_msg {
    struct m2p_msg_head head;
    union {
        struct m2p_msg_ack ack;
        struct m2p_msg_test test;
        struct m2p_msg_common com;
        struct m2p_msg_ctmu ctmu;
        struct m2p_msg_vad vad;
    } u;
} __attribute__((packed));

//m2p用户消息对应处理
struct m2p_msg_handler {
    u8 type;
    void (*handler)(struct m2p_msg *);
} __attribute__((packed));

#define REGISTER_M2P_MSG_HANDLER(_type, fn, pri) \
	const struct m2p_msg_handler _##fn SEC_USED(.m2p_msg_handler)= { \
		.type = _type, \
		.handler = fn, \
	}

extern struct m2p_msg_handler m2p_msg_handler_begin[];
extern struct m2p_msg_handler m2p_msg_handler_end[];

#define list_for_each_m2p_msg_handler(p) \
	for (p = m2p_msg_handler_begin; p < m2p_msg_handler_end; p++)

int m2p_get_msg(int len, struct m2p_msg *msg);

int m2p_post_msg(int len, struct m2p_msg *msg);

int m2p_post_sync_msg(int len, struct m2p_msg *msg, u8 abandon, int timeout);

void msys_to_p11_sys_cmd(u8 cmd);


#endif
