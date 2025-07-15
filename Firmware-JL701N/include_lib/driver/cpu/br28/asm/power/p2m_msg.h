#ifndef __P2M_MSG_H__
#define __P2M_MSG_H__

//p2m用户消息实例
enum {
    P2M_MSG_ACK = BIT(0),
    P2M_MSG_TEST = BIT(1),
    P2M_MSG_COMMOM = BIT(2),
    P2M_MSG_CTMU = BIT(3),
    P2M_MSG_SENSOR = BIT(4),
    P2M_MSG_VAD = BIT(5),

};

//测试
struct p2m_msg_test {
    u8 dat;
};

//测试
struct p2m_msg_ack {
    u8 dat;
};

//公共消息
struct p2m_msg_common {
    u8 dat;
};

//触摸消息
struct p2m_msg_ctmu {
    u8 dat;
};

//vad
struct p2m_msg_vad {
    u8 dat;
};

//p2m用户消息格式
struct p2m_msg_head {
u16 type :
    MSG_TYPE_BIT_LEN;
u16 len  :
    MSG_PARAM_BIT_LEN;
u8 index :
    MSG_INDEX_BIT;
u8 ack   :
    MSG_ACK_BIT;
} __attribute__((packed));

struct p2m_msg {
    struct p2m_msg_head head;
    union {
        struct p2m_msg_ack ack;
        struct p2m_msg_test test;
        struct p2m_msg_common com;
        struct p2m_msg_ctmu ctmu;
        struct p2m_msg_vad vad;
    } u;
} __attribute__((packed));

//p2m用户消息对应处理
struct p2m_msg_handler {
    u8 type;
    void (*handler)(struct p2m_msg *);
};

#define REGISTER_P2M_MSG_HANDLER(_type, fn, pri) \
	const struct p2m_msg_handler _##fn sec(.p2m_msg_handler)= { \
		.type = _type, \
		.handler = fn, \
	}

extern struct p2m_msg_handler p2m_msg_handler_begin[];
extern struct p2m_msg_handler p2m_msg_handler_end[];

#define list_for_each_p2m_msg_handler(p) \
	for (p = p2m_msg_handler_begin; p < p2m_msg_handler_end; p++)



int p2m_get_msg(int len, struct p2m_msg *msg);

int p2m_post_msg(int len, struct p2m_msg *msg);

int p2m_post_sync_msg(int len, struct p2m_msg *msg, u8 abandon, int timeout);

#endif
