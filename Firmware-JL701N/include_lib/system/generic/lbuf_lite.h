#ifndef LBUF_LITE_H
#define LBUF_LITE_H


#include "typedef.h"
#include "list.h"
#include "system/spinlock.h"


struct lbuff_lite_head {
    int magic_a;
    struct list_head head;
    struct list_head free;
    spinlock_t lock;
    u8 align;
    u16 priv_len;
    u32 total_size;
    u32 last_addr;
    void *priv;
    int magic_b;
};

struct lbuff_lite_state {
    u32 avaliable;
    u32 fragment;
    u32 max_continue_len;
    int num;
};



struct lbuff_lite_head *lbuf_lite_init(void *buf, u32 len, int align, int priv_head_len);

void *lbuf_lite_alloc(struct lbuff_lite_head *head, u32 len);

void *lbuf_lite_realloc(void *lbuf, int size);

void lbuf_lite_free(void *lbuf);

u32 lbuf_lite_free_space(struct lbuff_lite_head *head);

void lbuf_lite_state(struct lbuff_lite_head *head, struct lbuff_lite_state *state);

void lbuf_lite_dump(struct lbuff_lite_head *head);

int lbuf_lite_avaliable(struct lbuff_lite_head *head, int size);

#endif


