#ifndef __BR22_WDT_H__
#define __BR22_WDT_H__

#define WDT_1MS 		0x00
#define WDT_2MS			0x01
#define WDT_4MS			0x02
#define WDT_8MS 		0x03
#define WDT_16MS 		0x04
#define WDT_32MS 		0x05
#define WDT_64MS 		0x06
#define WDT_128MS 		0x07
#define WDT_256MS 		0x08
#define WDT_512MS 		0x09
#define WDT_1S 			0x0A
#define WDT_2S 			0x0B
#define WDT_4S 			0x0C
#define WDT_8S 			0x0D
#define WDT_16S 		0x0E
#define WDT_32S 		0x0F
#define WDT_64S 		0x10
#define WDT_128S 		0x11

void wdt_init(u8 time);
void wdt_close(void);
void wdt_clear(void);

void wdt_enable(void);
void wdt_disable(void);

u32 wdt_get_time(void);//ms

#endif
