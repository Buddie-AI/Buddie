#ifndef __ADAPTER_ODEV_EDR_H__
#define __ADAPTER_ODEV_EDR_H__

#include "generic/typedef.h"
#include "user_cfg.h"
#include "event.h"
#include "sbc_enc.h"


#define  SEARCH_BD_ADDR_LIMITED     0
#define  SEARCH_BD_NAME_LIMITED     1
#define  SEARCH_CUSTOM_LIMITED      2
#define  SEARCH_NULL_LIMITED        3

#define SEARCH_LIMITED_MODE         SEARCH_BD_NAME_LIMITED

enum {
    ODEV_EDR_CLOSE,
    ODEV_EDR_OPEN,
    ODEV_EDR_START,
    ODEV_EDR_STOP,
};

struct _odev_edr {
    struct list_head inquiry_noname_head;
    u8 edr_search_busy;
    u8 edr_read_name_start;
    u8 edr_search_spp;
    u8 status;

    int (*fun)(void *, u8, u8, void *);
    void *priv;
    u8 mode;

    u16 call_timeout;
    volatile u16 start_a2dp_flag;
    struct _odev_edr_parm *parm;
};

struct _odev_edr_parm {
    u8 poweron_start_search : 1;
    u8 disable_filt : 1;
    u8 discon_always_search : 1;
    u8 inquiry_len;
    u8 search_type;
    u8 bt_name_filt_num;
    u8(*bt_name_filt)[LOCAL_NAME_LEN];
    u8 spp_name_filt_num;
    u8(*spp_name_filt)[LOCAL_NAME_LEN];
    u8 bd_addr_filt_num;
    u8(*bd_addr_filt)[6];
};

typedef enum {
    AVCTP_OPID_VOLUME_UP   = 0x41,
    AVCTP_OPID_VOLUME_DOWN = 0x42,
    AVCTP_OPID_MUTE        = 0x43,
    AVCTP_OPID_PLAY        = 0x44,
    AVCTP_OPID_STOP        = 0x45,
    AVCTP_OPID_PAUSE       = 0x46,
    AVCTP_OPID_NEXT        = 0x4B,
    AVCTP_OPID_PREV        = 0x4C,
} AVCTP_CMD_TYPE;


int adapter_odev_edr_open(void *priv);
int adapter_odev_edr_start(void *priv);
int adapter_odev_edr_stop(void *priv);
int adapter_odev_edr_close(void *priv);
int adapter_odev_edr_get_status(void *priv);
int adapter_odev_edr_config(int cmd, void *parm);
int adapter_odev_edr_pp(u8 pp);
int adapter_odev_edr_media_prepare(u8 mode, int (*fun)(void *, u8, u8, void *), void *priv);

void adapter_odev_edr_search_device(void);
void adapter_odev_edr_search_stop();

extern struct _odev_edr odev_edr;
#endif//__ADAPTER_ODEV_BT_H__

