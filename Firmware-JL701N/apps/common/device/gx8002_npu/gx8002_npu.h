#ifndef __GX8002_NPU_H__
#define __GX8002_NPU_H__

/*
 * Message command
 */
#define MSG_TYPEBITS 8
#define MSG_NRBITS   8

#define MSG_NRMASK   ((1 << MSG_NRBITS) - 1)
#define MSG_TYPEMASK ((1 << MSG_TYPEBITS) - 1)

#define MSG_NRSHIFT  0
#define MSG_TYPESHIFT (MSG_NRSHIFT +  MSG_NRBITS)

/* Create new message */
#define NEW_MSG(type, nr) \
    (((type) << MSG_TYPESHIFT) | \
     ((nr)   << MSG_NRSHIFT))

/* Get message type */
#define MSG_TYPE(cmd) (((cmd) >> MSG_TYPESHIFT) & MSG_TYPEMASK)

/* Get message nr */
#define MSG_NR(cmd)   (((cmd) >> MSG_NRSHIFT) & MSG_NRMASK)

/*
 * Message flags
 */
#define MSG_FLAG_BCHECK (1 << 0)
#define MSG_FLAG_BCHECK_BITS 1
#define MSG_FLAG_BCHECK_MASK ((1 << MSG_FLAG_BCHECK_BITS) - 1)
#define MSG_FLAG_BCHECK_SHIFT 0

/* Check if need message body crc32 */
#define MSG_NEED_BCHECK(flags) (((flags) >> MSG_FLAG_BCHECK_SHIFT) & MSG_FLAG_BCHECK_MASK)


#define MSG_HEAD_LEN       14    /* Message header length */
#define MSG_HEAD_CHECK_LEN 4     /* Message header crc32 length */
#define MSG_MAGIC_LEN      4     /* Message magic length */
#define MSG_BODY_LEN_MAX   3072 /* Message body maximun length */
#define MSG_BODY_CHECK_LEN 4     /* Message body crc32 length */

#define MSG_SEQ_MAX 255  /* Message sequence maximum count */
#define MSG_LEN_MAX (MSG_HEAD_LEN + MSG_BODY_LEN_MAX)   /* Message maximun length */

#define MSG_MAGIC 0x58585542

#define MSG_RCV_MAGIC0 (((MSG_MAGIC) & 0x000000FF) >> 0 )
#define MSG_RCV_MAGIC1 (((MSG_MAGIC) & 0x0000FF00) >> 8 )
#define MSG_RCV_MAGIC2 (((MSG_MAGIC) & 0x00FF0000) >> 16)
#define MSG_RCV_MAGIC3 (((MSG_MAGIC) & 0xFF000000) >> 24)

typedef enum {
    MSG_TYPE_REQ = 0x1,
    MSG_TYPE_RSP = 0x2,
    MSG_TYPE_NTF = 0x3,
} MSG_TYPE;

struct message {
    unsigned short cmd;
    unsigned char seq;
    unsigned char flags;
    unsigned char *body;
    unsigned short bodylen;
} __attribute__((packed));

typedef enum {
    MSG_REQ_VOICE_EVENT       = NEW_MSG(MSG_TYPE_REQ, 0x0C),
    MSG_RSP_VOICE_EVENT       = NEW_MSG(MSG_TYPE_RSP, 0x0C),

    //查询gx固件版本
    MSG_REQ_VERSION_EVENT     = NEW_MSG(MSG_TYPE_REQ, 0x02),
    MSG_RSP_VERSION_EVENT     = NEW_MSG(MSG_TYPE_RSP, 0x02),

    //查询MIC状态
    MSG_REQ_MIC_EVENT         = NEW_MSG(MSG_TYPE_REQ, 0x70),
    MSG_RSP_MIC_EVENT         = NEW_MSG(MSG_TYPE_RSP, 0x70),

    //查询Gsensor状态
    MSG_REQ_GSENSOR_EVENT     = NEW_MSG(MSG_TYPE_REQ, 0x71),
    MSG_RSP_GSENSOR_EVENT     = NEW_MSG(MSG_TYPE_RSP, 0x71),
} UART_MSG_ID;
/* Stream read callback */
typedef int (*STREAM_READ)(unsigned char *buf, int len);

/* Stream write callback */
typedef int (*STREAM_WRITE)(const unsigned char *buf, int len);

/* Stream is empty callback */
typedef int (*STREAM_EMPTY)(void);


#endif /* #ifndef __GX8002_NPU_H__ */
