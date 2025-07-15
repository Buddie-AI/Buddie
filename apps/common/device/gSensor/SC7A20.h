/*
Copyright (c) 2019 Silan MEMS. All Rights Reserved.
*/
#ifndef SC7A20_H
#define SC7A20_H

/********添加IIC头文件******************/
//#include "i2c.h"
/***************************************/


/***使用驱动前请根据实际接线情况配置（7bit）IIC地址******/
/**SC7A20的SDO 脚接地：            0x18****************/
/**SC7A20的SDO 脚接电源：           0x19****************/
#define SC7A20_W_ADDR          (0x19U << 1 | 0x0)
#define SC7A20_R_ADDR          (0x19U << 1 | 0x1)
/*******************************************************/

#define SC7A20_CLICK_TH          0x38                //Click触发阈值,如果单次敲击无法触发中断，请将下面这个参数适当调小,如果单次敲击太过灵敏，请将下面这个参数适当调大
#define SC7A20_CLICK_WINDOWS     0x05                //Click有效窗口
#define SC7A20_CLICK_LATENCY     0x10                //Click中断持续时间,odr = 0x10时， 时间 = 16 * 2.5ms
#define SC7A20_INT_LEVEL         0x02                //0x02为中断时低电平，0x0为高电平
//如果中断能够触发，但是无法识别为单击，请调小如下函数中的参数 TH1
//如果单击能够触发，摇晃都会触发多击，请调小如下函数中的参数 TH2
//如果单击能够触发，多击功能比较难实现，请调大如下函数中的参数 TH2
#define SC7A20_TH1               40
#define SC7A20_TH2               50

#define click_pp_num    (unsigned short)10*25  //10对应0.5s(不需要管后面的*25)
#define click_max_num   (unsigned short)60*25  // 60对应3s



/*************驱动初始化函数**************/
unsigned char  SC7A20_Config(void);
/**SC7A20_Config 函数中， ODR 默认0x7f 400HZ/6f 200HZ ************/
/**SC7A20_Config 函数中， HP  默认0xOC 开启高通滤波 ************/
/**SC7A20_Config 函数中， click_int  默认0x80  将Click中断映射到INT1************/
/**SC7A20_Config 函数中， range  默认0x90  4g量程************/
/**SC7A20_Config 函数中， fifo_en  默认0x40  使能fifo************/
/**SC7A20_Config 函数中， fifo_mode  默认0x80  Stream模式************/
/**SC7A20_Config 函数中， click_mode  默认0x15  单击3轴触发************/
/**SC7A20_Config 函数中， click_th  默认0x38  Click触发阈值************/
/**SC7A20_Config 函数中， click_window  默认0x05  Click有效窗口************/
/**SC7A20_Config 函数中， click_latency  默认0x10  Click中断持续时间************/

/*************返回数据情况如下*****************/
/**return : 1       IIC通讯正常IC在线***************/
/**return : 0;      IIC通讯异常或IC不在线***************/

/*************单击状态检测**************/
char SC7A20_Click_Read(int TH1, int TH2);
/*************TH1  调节单击灵敏度，默认9000*****************/
/*************TH2  默认6000*****************/
/*************TH3  默认7000*****************/
/*************返回数据情况如下*****************/
/**return : 1       单次敲击有效***************/
/**return : 0;      单次敲击无效***************/


/*************外部中断调用Click敲击函数**************/
char SC7A20_Click_Alog(void);
/*************返回数据情况如下*****************/
/**return : 1       单次敲击有效***************/
/**return : 0;      单次敲击无效***************/


/*************定时器中断调用Click状态函数**************/
char SC7A20_click_status(void);
/*************返回数据情况如下*****************/
/**return : N       N次敲击有效***************/


/**reg map***************/
#define SC7A20_OUT_TEMP_L       0x0C
#define SC7A20_OUT_TEMP_H       0x0D
#define SC7A20_WHO_AM_I         0x0F
#define SC7A20_USER_CAL_START   0x13
#define SC7A20_USER_CAL_END     0x1A
#define SC7A20_NVM_WR           0x1E
#define SC7A20_TEMP_CFG         0x1F
#define SC7A20_CTRL_REG1        0x20
#define SC7A20_CTRL_REG2        0x21
#define SC7A20_CTRL_REG3        0x22
#define SC7A20_CTRL_REG4        0x23
#define SC7A20_CTRL_REG5        0x24
#define SC7A20_CTRL_REG6        0x25
#define SC7A20_REFERENCE        0x26
#define SC7A20_STATUS_REG       0x27
#define SC7A20_OUT_X_L          0x28
#define SC7A20_OUT_X_H          0x29
#define SC7A20_OUT_Y_L          0x2A
#define SC7A20_OUT_Y_H          0x2B
#define SC7A20_OUT_Z_L          0x2C
#define SC7A20_OUT_Z_H          0x2D
#define SC7A20_FIFO_CTRL_REG    0x2E
#define SC7A20_SRC_REG          0x2F
#define SC7A20_INT1_CFG         0x30
#define SC7A20_INT1_SOURCE      0x31
#define SC7A20_INT1_THS         0x32
#define SC7A20_INT1_DURATION    0x33
#define SC7A20_INT2_CFG         0x34
#define SC7A20_INT2_SOURCE      0x35
#define SC7A20_INT2_THS         0x36
#define SC7A20_INT2_DURATION    0x37
#define SC7A20_CLICK_CFG        0x38
#define SC7A20_CLICK_SRC        0x39
#define SC7A20_CLICK_THS        0x3A
#define SC7A20_TIME_LIMIT       0x3B
#define SC7A20_TIME_LATENCY     0x3C
#define SC7A20_TIME_WINDOW      0x3D
#define SC7A20_ACT_THS          0x3E
#define SC7A20_ACT_DURATION     0x3F

#endif // SC7A20_H

