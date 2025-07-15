#ifndef __JL_PACKET_H__
#define __JL_PACKET_H__

#include "typedef.h"


#define READ_BIG_U16(a)   ((*((u8*)(a)) <<8)  + *((u8*)(a)+1))
#define READ_BIG_U32(a)   ((*((u8*)(a)) <<24) + (*((u8*)(a)+1)<<16) + (*((u8*)(a)+2)<<8) + *((u8*)(a)+3))

#define READ_LIT_U16(a)   (*((u8*)(a))  + (*((u8*)(a)+1)<<8))
#define READ_LIT_U32(a)   (*((u8*)(a))  + (*((u8*)(a)+1)<<8) + (*((u8*)(a)+2)<<16) + (*((u8*)(a)+3)<<24))


#define WRITE_BIG_U16(a,src)   {*((u8*)(a)+0) = (u8)(src>>8); *((u8*)(a)+1) = (u8)(src&0xff); }
#define WRITE_BIG_U32(a,src)   {*((u8*)(a)+0) = (u8)((src)>>24);  *((u8*)(a)+1) = (u8)(((src)>>16)&0xff);*((u8*)(a)+2) = (u8)(((src)>>8)&0xff);*((u8*)(a)+3) = (u8)((src)&0xff);}

#define WRITE_LIT_U16(a,src)   {*((u8*)(a)+1) = (u8)(src>>8); *((u8*)(a)+0) = (u8)(src&0xff); }
#define WRITE_LIT_U32(a,src)   {*((u8*)(a)+3) = (u8)((src)>>24);  *((u8*)(a)+2) = (u8)(((src)>>16)&0xff);*((u8*)(a)+1) = (u8)(((src)>>8)&0xff);*((u8*)(a)+0) = (u8)((src)&0xff);}


#pragma pack(1)
typedef union __HEAD_BIT {
    struct {
        u16 _OpCode: 8; //OpCode val
        u16 _unsed : 6; //unsed
        u16 _resp : 1; //request for response
        u16 _type : 1; //command or response
    } _i;
    u16         _t;
} HEAD_BIT;

struct __JL_PACKET {
    u8          tag[3];
    HEAD_BIT    head;
    u16         length;
    u8          data[0];
};
#pragma pack()
typedef struct __JL_PACKET JL_PACKET;


#define JL_PACK_START_TAG0          (0xfe)
#define JL_PACK_START_TAG1          (0xdc)
#define JL_PACK_START_TAG2          (0xba)
#define JL_PACK_END_TAG             (0xef)
#define JL_ONE_PACKET_LEN(n)        (sizeof(JL_PACKET) + n + 1)

#ifdef JL_RCSP_UBOOT_LIB
#define JL_MTU_RESV                 (540L)
#define JL_MTU_SEND                 (128L)
#define JL_RECIEVE_BUF_SIZE         ((JL_MTU_RESV + sizeof(JL_PACKET))*2)
#define JL_CMD_POOL_SIZE            (JL_MTU_SEND)
#define JL_RESP_POOL_SIZE           (JL_MTU_SEND*2)
#define JL_WAIT_RESP_POOL_SIZE      (JL_MTU_SEND)
#else
#define JL_MTU_RESV                 (540L)
#define JL_MTU_SEND                 (128L)
#define JL_RECIEVE_BUF_SIZE         (JL_MTU_RESV + sizeof(JL_PACKET) + 128)
#define JL_CMD_POOL_SIZE            (JL_MTU_SEND*4)
#define JL_RESP_POOL_SIZE           (JL_MTU_SEND*2)
#define JL_WAIT_RESP_POOL_SIZE      (JL_MTU_SEND*2)
#endif

void set_jl_rcsp_recieve_buf_size(u16 size);
u16 rcsp_packet_write_alloc_len(void);

u32 rcsp_packet_need_buf_size(void);
u32 rcsp_packet_buf_init(u8 *buf, u32 len);

u16 JL_packet_get_rx_max_mtu(void);
u16 JL_packet_get_tx_max_mtu(void);
u16 JL_packet_set_mtu(u16 mtu);
void JL_packet_recieve(void *buf, u16 len);
u32 JL_pack_data_read_all(void *buf, u16 len);
void JL_packet_clear_all_data(void);
bool JL_packet_find(u8 *r_buf, JL_PACKET **packet);
void JL_packet_init(void);
void JL_packet_clear(void);

void JL_packet_packing(
    JL_PACKET *packet,
    u8 OpCode,
    u8 type,
    u8 request_rsp,
    u8 *extra_param,
    u16 extra_len,
    u8 *data,
    u16 len);

void set_jl_mtu_resv(u16 jl_mtu_resv_var);
void set_jl_mtu_send(u16 jl_mtu_send_var);

extern u16 jl_mtu_resv;
extern u16 jl_mtu_send;
extern u16 jl_recieve_buf_size;
extern u16 jl_cmd_pool_size;
extern u16 jl_rcsp_pool_size;
extern u16 jl_wait_rcsp_pool_size;

#endif//__JL_PACKET_H__


