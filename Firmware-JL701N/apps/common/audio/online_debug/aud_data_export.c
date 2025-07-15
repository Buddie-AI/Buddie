#include "aud_data_export.h"
#include "audio_online_debug.h"
#include "circular_buf.h"
#include "timer.h"

#define AUD_DE_LOG	y_printf

/*支持的数据通道数：1~3*/
#define AUDIO_DATA_EXPORT_CH    1

enum {
    AEC_ST_STOP = 0,
    AEC_ST_INIT,
    AEC_ST_START,
};

typedef struct {
    u8 state;
    u8 ch;	/*export data ch num*/
    u16 send_timer;
    u8 packet[256];
#if (AUDIO_DATA_EXPORT_CH > 0)
    u8 buf0[1024];
    cbuffer_t data_cbuf0;
    u32 seqn0;
#endif /*AUDIO_DATA_EXPORT_CH*/
#if (AUDIO_DATA_EXPORT_CH > 1)
    u8 buf1[1024];
    cbuffer_t data_cbuf1;
    u32 seqn1;
#endif /*AUDIO_DATA_EXPORT_CH*/
#if (AUDIO_DATA_EXPORT_CH > 2)
    u8 buf2[1024];
    cbuffer_t data_cbuf2;
    u32 seqn2;
#endif /*AUDIO_DATA_EXPORT_CH*/
} aud_data_export_t;
aud_data_export_t *aud_de = NULL;

typedef struct {
    int cmd;
    int data;
} rec_cmd_t;

int audio_data_export_start(u8 ch)
{
    if (aud_de) {
        AUD_DE_LOG("aud_de re-malloc !!!");
        return 0;
    }
    aud_de = zalloc(sizeof(aud_data_export_t));
    if (aud_de == NULL) {
        AUD_DE_LOG("aud_de malloc  failed!!!");
        return -1;
    }
    if (ch <= AUDIO_DATA_EXPORT_CH) {
        aud_de->ch = ch;
    } else {
        aud_de->ch = AUDIO_DATA_EXPORT_CH;
    }
#if (AUDIO_DATA_EXPORT_CH > 0)
    cbuf_init(&aud_de->data_cbuf0, aud_de->buf0, sizeof(aud_de->buf0));
    aud_de->seqn0 = 0;
#endif /*AUDIO_DATA_EXPORT_CH*/
#if (AUDIO_DATA_EXPORT_CH > 1)
    cbuf_init(&aud_de->data_cbuf1, aud_de->buf1, sizeof(aud_de->buf1));
    aud_de->seqn1 = 0;
#endif /*AUDIO_DATA_EXPORT_CH*/
#if (AUDIO_DATA_EXPORT_CH > 2)
    cbuf_init(&aud_de->data_cbuf2, aud_de->buf2, sizeof(aud_de->buf2));
    aud_de->seqn2 = 0;
#endif /*AUDIO_DATA_EXPORT_CH*/
    aud_de->state = AEC_ST_START;
    return 0;
}

void audio_data_export_stop(void)
{
    if (aud_de) {
        if (aud_de->send_timer) {
            sys_timer_del(aud_de->send_timer);
            aud_de->send_timer = 0;
        }
        aud_de->state = AEC_ST_STOP;
        free(aud_de);
        aud_de = NULL;
    }
}
static int spp_data_export(u8 ch, u8 *buf, u16 len)
{
    u8 data_ch;
    putchar('.');
    if (ch == 0) {
        data_ch = DB_PKT_TYPE_DAT_CH0;
    } else if (ch == 1) {
        data_ch = DB_PKT_TYPE_DAT_CH1;
    } else {
        data_ch = DB_PKT_TYPE_DAT_CH2;
    }
    int err = app_online_db_send_more(data_ch, buf, len);
    if (err) {
        printf("tx_err:%d", err);
    }
    return len;
}
int audio_data_export_run(u8 ch, u8 *data, int len)
{
    int ret = 0;
    if (aud_de && (aud_de->state == AEC_ST_START)) {
#if (AUDIO_DATA_EXPORT_CH > 0)
        if (ch == 0) {
            int wlen = cbuf_write(&aud_de->data_cbuf0, data, len);
            if (cbuf_get_data_size(&aud_de->data_cbuf0) >= 640) {
                u8 tmp_buf[RECORD_CH0_LENGTH];
                memcpy(tmp_buf, &aud_de->seqn0, 4);
                s16 *data_buf = &tmp_buf[4];
                cbuf_read(&aud_de->data_cbuf0, data_buf, 640);
                spp_data_export(ch, tmp_buf, 644);
                aud_de->seqn0 ++;
                putchar('s');

            }
        }
#endif /*AUDIO_DATA_EXPORT_CH*/
#if (AUDIO_DATA_EXPORT_CH > 1)
        else if (ch == 1) {
            int wlen = cbuf_write(&aud_de->data_cbuf1, data, len);
            if (cbuf_get_data_size(&aud_de->data_cbuf1) >= 640) {
                u8 tmp_buf[RECORD_CH1_LENGTH];
                memcpy(tmp_buf, &aud_de->seqn1, 4);
                s16 *data_buf = &tmp_buf[4];
                cbuf_read(&aud_de->data_cbuf1, data_buf, 640);
                spp_data_export(ch, tmp_buf, 644);
                aud_de->seqn1 ++;
                putchar('s');
            }

        }
#endif /*AUDIO_DATA_EXPORT_CH*/
#if (AUDIO_DATA_EXPORT_CH > 2)
        else if (ch == 2) {
            int wlen = cbuf_write(&aud_de->data_cbuf2, data, len);
            if (cbuf_get_data_size(&aud_de->data_cbuf2) >= 640) {
                u8 tmp_buf[RECORD_CH2_LENGTH];
                memcpy(tmp_buf, &aud_de->seqn2, 4);
                s16 *data_buf = &tmp_buf[4];
                cbuf_read(&aud_de->data_cbuf2, data_buf, 640);
                spp_data_export(ch, tmp_buf, 644);
                aud_de->seqn2 ++;
                putchar('s');
            }
        }
#endif /*AUDIO_DATA_EXPORT_CH*/
    }
    return ret;
}
static int aud_online_data_export_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size)
{
    int res_data = 0;
    rec_cmd_t rec_cmd;
    int err = 0;
    u8 parse_seq = ext_data[1];
    //AUD_DE_LOG("aec_spp_rx,seq:%d,size:%d\n", parse_seq, size);
    //put_buf(packet, size);
    memcpy(&rec_cmd, packet, sizeof(rec_cmd_t));
    switch (rec_cmd.cmd) {
    case AUD_RECORD_COUNT:
        if (aud_de) {
            /* res_data = aud_de->ch; */
            res_data = AUDIO_DATA_EXPORT_CH;
        } else {
            res_data = AUDIO_DATA_EXPORT_CH;
        }
        err = app_online_db_ack(parse_seq, (u8 *)&res_data, 4);
        AUD_DE_LOG("query record_ch num:%d\n", res_data);
        break;
    case AUD_RECORD_START:
        audio_data_export_start(AUDIO_DATA_EXPORT_CH);
        err = app_online_db_ack(parse_seq, (u8 *)&res_data, 1); //该命令随便ack一个byte即可
        AUD_DE_LOG("record_start\n");
        break;
    case AUD_RECORD_STOP:
        AUD_DE_LOG("record_stop\n");
        audio_data_export_stop();
        app_online_db_ack(parse_seq, (u8 *)&res_data, 1); //该命令随便ack一个byte即可
        break;
    case ONLINE_OP_QUERY_RECORD_PACKAGE_LENGTH:
        if (rec_cmd.data == 0) {
            res_data = RECORD_CH0_LENGTH;
        } else if (rec_cmd.data == 1) {
            res_data = RECORD_CH1_LENGTH;
        } else {
            res_data = RECORD_CH2_LENGTH;
        }
        AUD_DE_LOG("query record ch%d packet length:%d\n", rec_cmd.data, res_data);
        err = app_online_db_ack(parse_seq, (u8 *)&res_data, 4); //回复对应的通道数据长度
        break;
    }
    return 0;
}

int aud_data_export_open(void)
{
    app_online_db_register_handle(DB_PKT_TYPE_EXPORT, aud_online_data_export_parse);
    return 0;
}

int aud_data_export_close(void)
{
    return 0;
}
