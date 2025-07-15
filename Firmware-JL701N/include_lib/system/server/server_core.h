#ifndef SERVER_H
#define SERVER_H

#include "generic/typedef.h"
#include "system/task.h"
#include "spinlock.h"
#include "list.h"


#define REQ_COMPLETE_CALLBACK 			0x01000000
#define REQ_WAIT_COMPLETE 				0x02000000
#define REQ_HI_PRIORITY 				0x04000000


#define REQ_TYPE_MASK 					0x00ffffff


struct server_req {
    int type;
    int err;
    void *server_priv;
    struct list_head entry;
    struct server *server;
    void *user;
    const char *owner;
    OS_SEM sem;
    union {
        int state;
        void (*func)(void *, void *, int);
    } complete;
    u32 arg[0];
};


struct server_info {
    const char *name;
    u16 reqlen;
    u8  reqnum;
    void *(*open)(void *, void *);
    void (*close)(void *);
};

#define REQ_BUF_LEN  	512

struct server {
    bool avaliable;
    void *server;
    OS_SEM sem;
    OS_MUTEX mutex;
    spinlock_t lock;
    struct list_head *req_buf;
    struct list_head free;
    struct list_head pending;
    const struct server_info *info;
    const char *owner;
    void *handler_priv;
    void (*event_handler)(void *,  int argc, int *argv);
};



#define SERVER_REGISTER(info) \
	const struct server_info info sec(.server_info)


#define server_load(server) \
	load_module(server)

struct server *server_open(const char *name, void *arg);

void server_register_event_handler(struct server *server, void *priv,
                                   void (*handler)(void *, int argc, int *argv));

void server_close(struct server *server);

int server_request(struct server *server, int req_type, void *arg);

int server_request_async(struct server *server, int req_type, void *arg, ...);

int server_req_complete(struct server_req *req);

int server_event_handler(void *_server, int argc, int *argv);

#endif

