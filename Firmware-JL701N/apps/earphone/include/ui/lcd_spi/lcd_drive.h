#ifndef __SPI_LCD_DRIVER_H
#define __SPI_LCD_DRIVER_H


#include "asm/spi.h"


#if (SPI_LCD_DEBUG_ENABLE == 0)
#define lcd_d(...)
#define lcd_w(...)
#define lcd_e(fmt, ...)	printf("[LCD ERROR]: "fmt, ##__VA_ARGS__)
#elif (SPI_LCD_DEBUG_ENABLE == 1)
#define lcd_d(...)
#define lcd_w(fmt, ...)	printf("[LCD WARNING]: "fmt, ##__VA_ARGS__)
#define lcd_e(fmt, ...)	printf("[LCD ERROR]: "fmt, ##__VA_ARGS__)
#else
#define lcd_d(fmt, ...)	printf("[LCD DEBUG]: "fmt, ##__VA_ARGS__)
#define lcd_w(fmt, ...)	printf("[LCD WARNING]: "fmt, ##__VA_ARGS__)
#define lcd_e(fmt, ...)	printf("[LCD ERROR]: "fmt, ##__VA_ARGS__)
#endif

// 两毫秒延时
extern void delay_2ms(int cnt);
#define delay2ms(t) delay_2ms(t)


/* 定义初始化数据结构体 */
typedef struct {
    u8 cmd;		// 地址
    u8 cnt;		// 数据个数
    u8 dat[64];	// 数据
} InitCode;

#define REGFLAG_DELAY		0xFF

struct spi_lcd_init {
    char *name;			// 名称
    u8 spi_pending;
    u8 soft_spi;
    u16 lcd_width;
    u16 lcd_height;
    u8  color_format;
    u8  interface;
    u8  column_addr_align;
    u8  row_addr_align;
    u8  backlight_status;
    u8  *dispbuf;
    u32 bufsize;
    InitCode *initcode;	// 初始化代码
    u16 initcode_cnt;	// 初始化代码条数
    void (*Init)(void);
    void (*WriteComm)(u16 cmd);	// 写命令
    void (*WriteData)(u16 dat);	// 写数据
    void (*WriteMap)(char *map, int size);	// 写整个buf
    void (*WritePAGE)(char *map, u8 page_star, u8 page_len);	// 写page
    void (*SetDrawArea)(int, int, int, int);
    void (*Reset)(void);
    void (*BackLightCtrl)(u8);
    void (*EnterSleep)();
    void (*ExitSleep)();
};


void spi_dma_send_byte(u8 dat);
void spi_dma_send_map(u8 *map, u32 size);
void SPI_LcdTest();




struct lcd_spi_platform_data {
    u32  pin_reset;
    u32  pin_cs;
    u32  pin_rs;
    u32  pin_bl;
    spi_dev spi_cfg;
    const struct spi_platform_data *spi_pdata;
};



// LCD 初始化接口
#define REGISTER_LCD_DRIVE() \
	const struct spi_lcd_init dev_drive

enum LCD_COLOR {
    LCD_COLOR_RGB565,
    LCD_COLOR_MONO,
};

enum LCD_IF {
    LCD_SPI,
    LCD_EMI,
};

struct lcd_info {
    u16 width;
    u16 height;
    u8 color_format;
    u8 interface;
    u8 col_align;
    u8 row_align;
    u8 bl_status;
};


struct lcd_interface {
    void (*init)(void *);
    void (*get_screen_info)(struct lcd_info *info);
    void (*buffer_malloc)(u8 **buf, u32 *size);
    void (*buffer_free)(u8 *buf);
    void (*draw)(u8 *buf, u32 len, u8 wait);
    void (*set_draw_area)(u16 xs, u16 xe, u16 ys, u16 ye);
    void (*clear_screen)(u16 color);
    int (*backlight_ctrl)(u8 on);
    void (*draw_page)(u8 *buf, u8 page_star, u8 page_len);
};

extern struct lcd_interface lcd_interface_begin[];
extern struct lcd_interface lcd_interface_end[];

#define REGISTER_LCD_INTERFACE(lcd) \
    static const struct lcd_interface lcd sec(.lcd_if_info) __attribute__((used))

struct lcd_interface *lcd_get_hdl();

#define LCD_SPI_PLATFORM_DATA_BEGIN(data) \
		const struct lcd_spi_platform_data data = {

#define LCD_SPI__PLATFORM_DATA_END() \
};


void lcd_reset_l();
void lcd_reset_h();
void lcd_cs_l();
void lcd_cs_h();
void lcd_rs_l();
void lcd_rs_h();
void lcd_bl_l();
void lcd_bl_h();
u8   lcd_bl_io();

extern int lcd_backlight_status();
u8 lcd_spi_recv_byte();
int lcd_spi_send_byte(u8 byte);

#endif


