#ifndef __BR28_CHARGESTORE_H__
#define __BR28_CHARGESTORE_H__

enum {
    CMD_COMPLETE,
    CMD_RECVDATA,
};

enum {
    MODE_RECVDATA,
    MODE_SENDDATA,
};

enum {
    TYPE_NORMAL,
    TYPE_F95,
};

#define LDOIN_BIND_IO   IO_PORTP_00

struct chargestore_platform_data {
    u32 baudrate;
    u32 io_port;
    u8 uart_irq;
    void (*init)(const struct chargestore_platform_data *);
    void (*open)(u8 mode);
    void (*close)(void);
    void (*write)(u8 *, u8);
};

struct chargestore_data_handler {
    int (*data_cb)(u8 *buf, u8 len);
};

#define CHARGESTORE_HANDLE_REG(name, data_callback) \
	const struct chargestore_data_handler chargestore_##name \
		 SEC_USED(.chargestore_callback_txt) = {data_callback};

extern struct chargestore_data_handler chargestore_handler_begin[];
extern struct chargestore_data_handler chargestore_handler_end[];

#define list_for_each_loop_chargestore(h) \
	for (h=chargestore_handler_begin; h<chargestore_handler_end; h++)


#define CHARGESTORE_PLATFORM_DATA_BEGIN(data) \
    static const struct chargestore_platform_data data = {

#define CHARGESTORE_PLATFORM_DATA_END() \
    .baudrate               = 9600, \
    .init                   = chargestore_init, \
    .open                   = chargestore_open, \
    .close                  = chargestore_close, \
    .write                  = chargestore_write, \
};

extern void chargestore_open(u8 mode);
extern void chargestore_close(void);
extern void chargestore_write(u8 *data, u8 len);
extern void chargestore_init(const struct chargestore_platform_data *);
extern void chargestore_set_update_ram(void);
extern u8 chargestore_get_det_level(u8 chip_type);

//app层使用的接口
extern void chargestore_api_close(void);
extern int chargestore_api_write(u8 *buf, u8 len);
extern void chargestore_api_init(const struct chargestore_platform_data *arg);
extern void chargestore_api_wait_complete(void);
extern void chargestore_api_set_timeout(u16 timeout);
extern void chargestore_api_stop(void);
extern void chargestore_api_restart(void);
extern u8 chargestore_api_crc8(u8 *ptr, u8 len);

#endif
