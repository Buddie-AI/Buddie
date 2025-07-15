#ifndef	_LCD_SEG3X9_DRV_H_
#define _LCD_SEG3X9_DRV_H_

typedef struct _lcd_seg3x9_var {
    u8  bCoordinateX;
    u8  bFlashChar;
    u8  bShowBuff[4]; //[0] ~ [3]显示的数字/字母显示的所有段
    u32 bShowIcon;    //每1bit对应LED4的1段, maybe icon > 7段
    u32 bFlashIcon;   //闪烁标志
} LCD_SEG3X9_VAR;

#define LCD_SEG3X9_USB   		BIT(0) 		//0: USB图标
#define LCD_SEG3X9_SD   		BIT(1) 		//1: SD图标
#define LCD_SEG3X9_VOL   		BIT(2) 		//2: SD图标
#define LCD_SEG3X9_MHZ 			BIT(3) 		//3: FM图标
#define LCD_SEG3X9_DIGIT1 		BIT(4) 		//4: 数字1
#define LCD_SEG3X9_2POINT 		BIT(5) 		//5: 冒号
#define LCD_SEG3X9_DOT 			BIT(6) 		//6: 点

typedef u32 UI_LCD_SEG3X9_ICON;

struct lcd_seg3x9_pin {
    u8 pin_com[3];
    u8 pin_seg[9];
};

enum LCD_SEG3X9_BIAS {
    LCD_SEG3X9_BIAS_OFF = 0,
    LCD_SEG3X9_BIAS_1_2, 	// 1/2 bias
    LCD_SEG3X9_BIAS_1_3, 	// 1/3 bias
    LCD_SEG3X9_BIAS_1_4,  	// 1/4 bias
};

enum LCD_SEG3X9_COM_NUMBER {
    LCD_SEG3X9_COM_NUMBER_3 = 0,
    LCD_SEG3X9_COM_NUMBER_4,
    LCD_SEG3X9_COM_NUMBER_5,
    LCD_SEG3X9_COM_NUMBER_6,
};

enum LCD_SEG3X9_VOLTAGE {
    LCD_SEG3X9_VOLTAGE_2_6V = 0, 	//000b
    LCD_SEG3X9_VOLTAGE_2_7V, 		//001b
    LCD_SEG3X9_VOLTAGE_2_8V, 		//010b
    LCD_SEG3X9_VOLTAGE_2_9V, 		//011b
    LCD_SEG3X9_VOLTAGE_3_0V, 		//100b
    LCD_SEG3X9_VOLTAGE_3_1V, 		//101b
    LCD_SEG3X9_VOLTAGE_3_2V, 		//110b
    LCD_SEG3X9_VOLTAGE_3_3V, 		//111b
};


struct lcd_seg3x9_platform_data {
    enum LCD_SEG3X9_VOLTAGE vlcd;
    enum LCD_SEG3X9_BIAS bias;
    struct lcd_seg3x9_pin pin_cfg;
};


//=================================================================================//
//                        		关于LCD段码屏接线问题 							   //
// 1.COM口需要从COM0按顺序开始连接, 如3COM, 则IO必须占用COM0/1/2，而不能COM1/2/3;
// 2.COM0/1/2与LCD屏可以不按照对应需要接线, 如:
// 		LCD_COM0 --> CHIP_COM2;
// 		LCD_COM1 --> CHIP_COM1;
// 		LCD_COM2 --> CHIP_COM0;
// 	但一般建议按顺序接线;
// 2.SEG可以根据实际情况选择SEG0~SEG21接线, 如;
// 		LCD_SEG0 --> CHIP_SEG7;
// 		LCD_SEG1 --> CHIP_SEG8;
// 		LCD_SEG2 --> CHIP_SEG9;
//=================================================================================//
#define LCD_SEG3X9_PLATFORM_DATA_BEGIN(data) \
		const struct lcd_seg3x9_platform_data data = {

#define LCD_SEG3X9_PLATFORM_DATA_END() \
};

//UI LED7 API:
//=================================================================================//
//                        		模块初始化显示接口                    			   //
//=================================================================================//
void lcd_seg3x9_init(const struct lcd_seg3x9_platform_data *_data);
const struct ui_display_api *ui_lcd_seg3x9_init(void *para);
//=================================================================================//
//                        		设置显示坐标接口                    			   //
/*
  	   ___    ___    ___
	  |___|  |___|  |___|
	  |___|  |___|  |___|
	 ---0------1------2----> X
*/
//=================================================================================//
void lcd_seg3x9_setX(u8 X);

//=================================================================================//
//                        		字符类显示接口                    				   //
//=================================================================================//
void lcd_seg3x9_show_char(u8 chardata); 			//显示字符(追加方式)
void lcd_seg3x9_flash_char_start(u8 index);       	//闪烁单个字符
void lcd_seg3x9_flash_char_stop(u8 index);        	//取消闪烁单个字符
void lcd_seg3x9_show_string(u8 *str);   			//显示字符串(追加方式)
void lcd_seg3x9_show_string_reset_x(u8 *str);  		//显示字符串, x从0开始
void lcd_seg3x9_show_string_align_right(u8 *str); //led7显示字符串(清屏&右方式)
void lcd_seg3x9_show_string_align_left(u8 *str);  //led7显示字符串(清屏&左方式)

//=================================================================================//
//                        		数字类显示接口                    				   //
//=================================================================================//
void lcd_seg3x9_show_number(u16 val); //显示数字(清屏&高位显示0)
void lcd_seg3x9_show_number2(u16 val); //显示数字(清屏&高位不显示)
void lcd_seg3x9_show_number_add(u16 val); //显示数字(追加方式)

//=================================================================================//
//                        		图标类显示接口                    				   //
//=================================================================================//
void lcd_seg3x9_show_icon(UI_LCD_SEG3X9_ICON icon); //显示单个图标(追加)
void lcd_seg3x9_flash_icon(UI_LCD_SEG3X9_ICON icon); //闪烁单个图标(追加)

//=================================================================================//
//                        		模式类类显示接口                    			   //
//=================================================================================//

//=================================================================================//
//                        		清屏类显示接口                    				   //
//=================================================================================//
void lcd_seg3x9_show_null(void); //清除所有显示(数字,字符串和图标)
void lcd_seg3x9_clear_string(void); //清除显示数字和字母
void lcd_seg3x9_clear_icon(UI_LCD_SEG3X9_ICON icon); //清除显示单个图标
void lcd_seg3x9_clear_all_icon(void); //清除显示所有图标


#endif /* #ifndef _LCD_SEG3X9_DRV_H_ */

