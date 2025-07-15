#ifndef _GX_UART_UPGRADE_UPGRADE_PORT_
#define _GX_UART_UPGRADE_UPGRADE_PORT_


int gx_upgrade_uart_porting_open(int baud, int databits, int stopbits, int parity);

int gx_upgrade_uart_porting_close(void);

int gx_upgrade_uart_porting_read(unsigned char *buf, unsigned int len, unsigned int timeout_ms);

//int gx_upgrade_uart_porting_write(const unsigned char *buf, unsigned int len, unsigned int timeout_ms);
int gx_upgrade_uart_porting_write(const unsigned char *buf, unsigned int len);

int gx_uart_upgrade_porting_wait_reply(const unsigned char *buf, unsigned int len, unsigned int timeout_ms);

#endif /* #ifndef _GX_UART_UPGRADE_UPGRADE_PORT_ */
