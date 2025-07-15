#include "board_config.h"


#define _CAT2(a,b) a ## _ ## b
#define CAT2(a,b) _CAT2(a,b)

#define _CAT3(a,b,c) a ## _ ## b ##  _  ## c
#define CAT3(a,b,c) _CAT3(a,b,c)

#define _CAT4(a,b,c,d) a ## _ ## b ##  _  ## c ## _ ## d
#define CAT4(a,b,c,d) _CAT4(a,b,c,d)

#ifndef CONFIG_DOWNLOAD_MODEL
#define CONFIG_DOWNLOAD_MODEL                   USB                   //下载模式选择，可选配置USB\SERIAL
#endif

#ifndef CONFIG_DEVICE_NAME
#define CONFIG_DEVICE_NAME                      JlVirtualJtagSerial   //串口通讯的设备名(配置串口通讯时使用)
#endif

#ifndef CONFIG_SERIAL_BAUD_RATE
#define CONFIG_SERIAL_BAUD_RATE                 1000000               //串口通讯的波特率(配置串口通讯时使用)
#endif

#ifndef CONFIG_SERIAL_CMD_OPT
#define CONFIG_SERIAL_CMD_OPT                   2                    //串口通讯公共配置参数(配置串口通讯时使用)
#endif

#ifndef CONFIG_SERIAL_CMD_RATE
#define CONFIG_SERIAL_CMD_RATE                  100                   //串口通讯时公共配置参数(配置串口通讯时使用)[n*10000]
#endif

#ifndef CONFIG_SERIAL_CMD_RES
#define CONFIG_SERIAL_CMD_RES                   0                     //串口通讯时公共配置参数(配置串口通讯时使用)
#endif

#ifndef CONFIG_SERIAL_INIT_BAUD_RATE
#define CONFIG_SERIAL_INIT_BAUD_RATE            9600                  //串口通信初始化时通讯的波特率(配置串口通讯时使用)
#endif

#ifndef CONFIG_LOADER_BAUD_RATE
#define CONFIG_LOADER_BAUD_RATE                 1000000               //写入loader文件时通讯的波特率(配置串口通讯时使用)
#endif

#ifndef CONFIG_LOADER_ASK_BAUD_RATE
#define CONFIG_LOADER_ASK_BAUD_RATE             1000000
#endif
#ifndef CONFIG_SERIAL_SEND_KEY
#define CONFIG_SERIAL_SEND_KEY                  YES                   //SERIAL_SEND_KEY：串口交互时数据是否需要进行加密(配置串口通讯时使用，有效值：YES)
#endif

#ifndef CONFIG_BREFORE_LOADER_WAIT_TIME
#define CONFIG_BREFORE_LOADER_WAIT_TIME         150                   //写入loader前延时时间(配置串口通讯时使用)
#endif

#ifndef CONFIG_ENTRY_ADDRESS
#define CONFIG_ENTRY_ADDRESS                    0x6000120             //程序入口地址，一般不需要修改(跟张恺讨论过把RESERVED_OPT=0合并到一个配置项)
#endif


/* #ifndef CONFIG_SDK_TYPE */
/* #define CONFIG_SDK_TYPE                         SOUNDBOX              // SOUNDBOX:音箱方案 OTHER:其他方案（比如:耳机,BLE）  SDK类型：当前仅支持音箱和非音箱两种类型 */
/* #endif */

#ifndef CONFIG_SPI_DATA_WIDTH
#define CONFIG_SPI_DATA_WIDTH                   2                     //data_width[0 1 2 3 4] 3的时候uboot自动识别2或者4线
#endif

#ifndef CONFIG_SPI_CLK_DIV
#define CONFIG_SPI_CLK_DIV                      3                     //clk [0-255]
#endif

//mode:
//	  0 RD_OUTPUT,		 1 cmd 		 1 addr
//    1 RD_I/O,   		 1 cmd 		 x addr
//	  2 RD_I/O_CONTINUE] no_send_cmd x add
#ifndef CONFIG_SPI_MODE
#define CONFIG_SPI_MODE                         0
#endif
//port:
//	  0  优先选A端口  CS:PD3  CLK:PD0  D0:PD1  D1:PD2  D2:PB7  D3:PD5
//	  1  优先选B端口  CS:PA13 CLK:PD0  D0:PD1  D1:PA14 D2:PA15 D3:PD5
#ifndef CONFIG_SPI_PORT
#define CONFIG_SPI_PORT                         0
#endif

//uboot and ota.bin串口tx
#ifndef CONFIG_UBOOT_DEBUG_PIN
#define CONFIG_UBOOT_DEBUG_PIN                  PB02
#endif

//uboot and ota.bin串口波特率[EXTRA_CFG_PARAM]
#ifndef CONFIG_UBOOT_DEBUG_BAUD_RATE
#define CONFIG_UBOOT_DEBUG_BAUD_RATE            1000000
#endif

[EXTRA_CFG_PARAM]
#if CONFIG_DOUBLE_BANK_ENABLE
BR22_TWS_DB = YES;	//dual bank flash framework enable
FLASH_SIZE = CONFIG_FLASH_SIZE;		//flash_size cfg
BR22_TWS_VERSION = 0; //default fw version
FORCE_4K_ALIGN = YES; // force aligin with 4k bytes
SPECIAL_OPT = 0;		// only generate one flash.bin
#if CONFIG_DB_UPDATE_DATA_GENERATE_EN
DB_UPDATE_DATA = YES; //generate db_update_data.bin
#endif
#else
NEW_FLASH_FS = YES;	//enable single bank flash framework
#endif 				//CONFIG_DOUBLE_BANK_ENABLE

#if ALIGN_UNIT_256B
AREA_ALIGN = ALIGN_UNIT_256B; //using n*256B unit for boundary alignment
#endif

CHIP_NAME = CONFIG_CHIP_NAME;
ENTRY = CONFIG_ENTRY_ADDRESS;
PID = CONFIG_PID;
VID = CONFIG_VID;

SDK_VERSION = JL701N_V1.6.1; //jenkins脚本生成

RESERVED_OPT = 0;

UFW_ELEMENT = 0x10 - 0x0, 0x1 - 0x0;

DOWNLOAD_MODEL = CONFIG_DOWNLOAD_MODEL; //
SERIAL_DEVICE_NAME = CONFIG_DEVICE_NAME;
SERIAL_BARD_RATE = CONFIG_SERIAL_BAUD_RATE;
SERIAL_CMD_OPT = CONFIG_SERIAL_CMD_OPT;
SERIAL_CMD_RATE = CONFIG_SERIAL_CMD_RATE;
SERIAL_CMD_RES = CONFIG_SERIAL_CMD_RES;
SERIAL_INIT_BAUD_RATE = CONFIG_SERIAL_INIT_BAUD_RATE;
LOADER_BAUD_RATE = CONFIG_LOADER_BAUD_RATE;
LOADER_ASK_BAUD_RATE = CONFIG_LOADER_ASK_BAUD_RATE;
BEFORE_LOADER_WAIT_TIME = CONFIG_BREFORE_LOADER_WAIT_TIME;
SERIAL_SEND_KEY = CONFIG_SERIAL_SEND_KEY;

#if CONFIG_ANC_ENABLE
COMPACT_SETTING = YES;
#endif

// #####匹配的芯片版本,请勿随意改动######
[CHIP_VERSION]
SUPPORTED_LIST = P, B;

[SYS_CFG_PARAM]
SPI = CAT4(CONFIG_SPI_DATA_WIDTH, CONFIG_SPI_CLK_DIV, CONFIG_SPI_MODE, CONFIG_SPI_PORT);	//width_clk_mode_port;

#if (CONFIG_PLL_SOURCE_USING_LRC == 1)
PLL_SRC = LRC; //PLL时钟源：屏蔽或！=LRC; 默认选择晶振。 值=LRC,且用no_ota_uboot,则时钟源选LRC
#endif

#ifdef CONFIG_UBOOT_DEBUG_PIN
UTTX = CONFIG_UBOOT_DEBUG_PIN; //uboot串口tx
UTBD = CONFIG_UBOOT_DEBUG_BAUD_RATE; //uboot串口波特率
#endif

//外部FLASH 硬件连接配置
#ifdef CONFIG_EXTERN_FLASH_SIZE
EX_FLASH = PC03_1A_PC08;	//CS_pin / spi (0/1/2) /port(A/B) / power_io
EX_FLASH_IO = 2_PC01_PC02_PC04_PC05_PC00;	//data_width / CLK_pin / DO_pin / DI_pin / D2_pin / D3_pin   当data_width为4的时候，D2_pin和D3_pin才有效
#endif
/* #0:disable */
/* #1:PA9 PA10  */
/* #2:USB */
/* #3:PB1 PB2 */
/* #4:PB6 PB7 */

/* #sdtap=2; */

/* #enable: */
/* #	  0: uboot 不初始化psram; */
/* #	  1: uboot 初始化psram */
/* #mode: */
/* #	  0: 1线模式; */
/* #	  1: 4线模式0, 命令1线, 地址4线, 数据4线,  */
/* #	  2: 4线模式1, 命令4线, 地址4线, 数据4线, */
/* #div: */
/* #	  1分频,  192M,  div = 0; */
/* #	  2分频,  96M,   div = 4; */
/* #	  3分频,  64M,   div = 1; */
/* #	  4分频,  48M,   div = 8; */
/* #	  5分频,  38.4M, div = 2; */
/* #	  6分频,  32M,   div = 5; */
/* #	  7分频,  27.4M, div = 3; */
/* #	  8分频,  24M,   div = C; */
/* #	  10分频, 19.2M, div = 6; */
/* #	  12分频, 16M,   div = 9; */
/* #	  14分频, 13.7M, div = 7; */
/* #	  20分频, 9.6M,  div = A; */
/* #	  24分频, 8M,    div = D; */
/* #	  28分频, 6.8M,  div = B; */
/* #	  40分频, 4.8M,  div = E; */
/* #	  56分频, 3.4M,  div = F; */
//#enable_mode_div
#ifdef CONFIG_PSRAM_ENABLE
PSRAM = 1_2_A;
#endif


#ifdef CONFIG_UART_UPDATE_PIN
UTRX = CONFIG_UART_UPDATE_PIN; //串口升级[PB00 PB05 PA05]
#endif

/* RESET = CAT3(CONFIG_RESET_PIN, CONFIG_RESET_TIME, CONFIG_RESET_LEVEL);	//port口_长按时间_有效电平（长按时间有00、01、02、04、08三个值可选，单位为秒，当长按时间为00时，则关闭长按复位功能。） */

#ifdef CONFIG_SUPPORT_RESET1
RESET1 = CAT3(CONFIG_RESET1_PIN, CONFIG_RESET1_TIME, CONFIG_RESET1_LEVEL);	//port口_长按时间_有效电平（长按时间有00、01、02、04、08三个值可选，单位为秒，当长按时间为00时，则关闭长按复位功能。）
#endif

#ifdef CONFIG_VDDIO_LVD_LEVEL
VLVD = CONFIG_VDDIO_LVD_LEVEL; //VDDIO_LVD挡位，0: 1.8V   1: 1.9V   2: 2.0V   3: 2.1V   4: 2.2V   5: 2.3V   6: 2.4V   7: 2.5V
#endif

/* #ifdef CONFIG_UPDATE_JUMP_TO_MASK */
/* UPDATE_JUMP = CONFIG_UPDATE_JUMP_TO_MASK; */
/* #endif */

#ifdef CONFIG_CUSTOM_CFG1_TYPE
CONFIG_CUSTOM_CFG1_TYPE = CONFIG_CUSTOM_CFG1_VALUE;
#endif
#ifdef CONFIG_CUSTOM_CFG2_TYPE
CONFIG_CUSTOM_CFG2_TYPE = CONFIG_CUSTOM_CFG2_VALUE;
#endif
#ifdef CONFIG_CUSTOM_CFG3_TYPE
CONFIG_CUSTOM_CFG3_TYPE = CONFIG_CUSTOM_CFG3_VALUE;
#endif

//#############################################################################################################################################

#ifndef CONFIG_VM_ADDR
#define CONFIG_VM_ADDR		0
#endif

#ifndef CONFIG_VM_LEAST_SIZE
#if ALIGN_UNIT_256B
#define CONFIG_VM_LEAST_SIZE	0x200
#else
#define CONFIG_VM_LEAST_SIZE	32K
#endif
#endif

#ifndef CONFIG_VM_OPT
#define CONFIG_VM_OPT			1
#endif

#ifndef CONFIG_BTIF_ADDR
#define CONFIG_BTIF_ADDR	AUTO
#endif

#ifndef CONFIG_BTIF_LEN
#if ALIGN_UNIT_256B
#define CONFIG_BTIF_LEN		0x200
#else
#define CONFIG_BTIF_LEN		0x1000
#endif
#endif

#ifndef CONFIG_BTIF_OPT
#define CONFIG_BTIF_OPT		1
#endif

#ifndef CONFIG_PRCT_ADDR
#define CONFIG_PRCT_ADDR	0
#endif

#ifndef CONFIG_PRCT_LEN
#define CONFIG_PRCT_LEN		CODE_LEN
#endif

#ifndef CONFIG_PRCT_OPT
#define CONFIG_PRCT_OPT		2
#endif

#ifndef CONFIG_EXIF_ADDR
#define CONFIG_EXIF_ADDR	AUTO
#endif

#ifndef CONFIG_EXIF_LEN
#define CONFIG_EXIF_LEN		0x1000
#endif

#ifndef CONFIG_EXIF_OPT
#define CONFIG_EXIF_OPT		1
#endif

#ifndef CONIFG_BURNER_INFO_SIZE
#define CONFIG_BURNER_INFO_SIZE		32
#endif

[FW_ADDITIONAL]
FILE_LIST = (file = ota.bin: type = 100);

// ########flash空间使用配置区域###############################################
// #PDCTNAME:    产品名，对应此代码，用于标识产品，升级时可以选择匹配产品名
// #BOOT_FIRST:  1=代码更新后，提示APP是第一次启动；0=代码更新后，不提示
// #UPVR_CTL：   0：不允许高版本升级低版本   1：允许高版本升级低版本
// #XXXX_ADR:    区域起始地址	AUTO：由工具自动分配起始地址
// #XXXX_LEN:    区域长度		CODE_LEN：代码长度
// #XXXX_OPT:    区域操作属性
// #
// #
// #
// #操作符说明  OPT:
// #	0:  下载代码时擦除指定区域
// #	1:  下载代码时不操作指定区域
// #	2:  下载代码时给指定区域加上保护
// ############################################################################
[RESERVED_CONFIG]
BTIF_ADR = CONFIG_BTIF_ADDR;
BTIF_LEN = CONFIG_BTIF_LEN;
BTIF_OPT = CONFIG_BTIF_OPT;

#if (CONFIG_APP_OTA_ENABLE && !CONFIG_DOUBLE_BANK_ENABLE)
EXIF_ADR = CONFIG_EXIF_ADDR;
EXIF_LEN = CONFIG_EXIF_LEN;
EXIF_OPT = CONFIG_EXIF_OPT;
#endif

// #WTIF_ADR=BEGIN_END;
// #WTIF_LEN=0x1000;
// #WTIF_OPT=1;

//forprotect area cfg
PRCT_ADR = CONFIG_PRCT_ADDR;
PRCT_LEN = CONFIG_PRCT_LEN;
PRCT_OPT = CONFIG_PRCT_OPT;

//for volatile memory area cfg
//VM大小默认为CONFIG_VM_LEAST_SIZE，如果代码空间不够可以适当改小，需要满足4*2*n; 改小可能会导致不支持测试盒蓝牙升级（不影响串口升级）
VM_ADR = CONFIG_VM_ADDR;
VM_LEN = CONFIG_VM_LEAST_SIZE;
VM_OPT = CONFIG_VM_OPT;

#ifdef CONFIG_RESERVED_AREA1
CAT2(CONFIG_RESERVED_AREA1, ADR) = CONFIG_RESERVED_AREA1_ADDR;
CAT2(CONFIG_RESERVED_AREA1, LEN) = CONFIG_RESERVED_AREA1_LEN;
CAT2(CONFIG_RESERVED_AREA1, OPT) = CONFIG_RESERVED_AREA1_OPT;

#ifdef CONFIG_RESERVED_AREA1_FILE
CAT2(CONFIG_RESERVED_AREA1, FILE) = CONFIG_RESERVED_AREA1_FILE;
#endif

#endif

#ifdef CONFIG_RESERVED_AREA2
CAT2(CONFIG_RESERVED_AREA2, ADR) = CONFIG_RESERVED_AREA2_ADDR;
CAT2(CONFIG_RESERVED_AREA2, LEN) = CONFIG_RESERVED_AREA2_LEN;
CAT2(CONFIG_RESERVED_AREA2, OPT) = CONFIG_RESERVED_AREA2_OPT;

#ifdef CONFIG_RESERVED_AREA2_FILE
CAT2(CONFIG_RESERVED_AREA2, FILE) = CONFIG_RESERVED_AREA2_FILE;
#endif

#endif
#if TCFG_EQ_ONLINE_ENABLE || TCFG_MIC_EFFECT_ONLINE_ENABLE
EFFECT_ADR = AUTO;
EFFECT_LEN = 4K;
EFFECT_OPT = 1;
#endif

/*
 ****************************************************************************
 *								ANC配置区
 *Notes:
 *(1)需要根据flash容量手动配置ANC配置区起始地址:
 *	 4Mbit:0x7E000 	8Mbit:0xFE000 	16Mbit:0x1FE000
 *(2)加载ANC增益/系数配置文件，则表示使用配置文件的增益/系数配置
 *(3)ANC两个配置区是连续的，即第一个配置区起始地址加上自身长度，即为第二个配置
 *	 区起始地址：CONFIG_ANCIF1_ADDR = CONFIG_ANCIF_ADDR + CONFIG_ANCIF_LEN
 *****************************************************************************
 */
#if CONFIG_ANC_ENABLE

/*******************用户配置区************************/
//加载ANC增益配置文件使能
#define CONFIG_ANCIF_GAINS_FILE_ENABLE		1
//加载ANC系数配置文件使能
#define CONFIG_ANCIF1_COEFF_FILE_ENABLE		1

//ANC配置区起始地址配置(跟进芯片flash容量进行配置)
//4Mbit:0x7E000 8Mbit:0xFE000 16Mbit:0x1FE000
#ifndef CONFIG_ANCIF_ADDR
#define CONFIG_ANCIF_ADDR	0xFD000
#endif/*CONFIG_ANCIF_ADDR*/
#ifndef CONFIG_ANCIF1_ADDR
#define CONFIG_ANCIF1_ADDR	0xFD100
#endif/*CONFIG_ANCIF1_ADDR*/

//ANC配置区下载操作配置
#ifndef CONFIG_ANCIF_OPT
#define CONFIG_ANCIF_OPT	1
#endif
#ifndef CONFIG_ANCIF1_OPT
#define CONFIG_ANCIF1_OPT	1
#endif
/*******************用户配置区************************/


/*******************非用户配置区**********************/
//ANC配置区长度定义
#define CONFIG_ANCIF_LEN	0x100
/* #if ANC_MAX_ORDER > 10 */
#define CONFIG_ANCIF1_LEN	0x1F00
/* #else */
/* #define CONFIG_ANCIF1_LEN	0xF00 */
/* #endif#<{(|ANC_MAX_ORDER > 10 |)}># */

//ANC系数配置保留区0
#if CONFIG_ANCIF_GAINS_FILE_ENABLE
ANCIF_FILE = anc_gains.bin;
#endif
ANCIF_ADR = CONFIG_ANCIF_ADDR;
ANCIF_LEN = CONFIG_ANCIF_LEN;
ANCIF_OPT = CONFIG_ANCIF_OPT;

//ANC系数配置保留区1
#if CONFIG_ANCIF1_COEFF_FILE_ENABLE
ANCIF1_FILE = anc_coeff.bin;
#endif
ANCIF1_ADR = CONFIG_ANCIF1_ADDR;
ANCIF1_LEN = CONFIG_ANCIF1_LEN;
ANCIF1_OPT = CONFIG_ANCIF1_OPT;
/*******************非用户配置区**********************/
#endif/*CONFIG_ANC_ENABLE*/

[BURNER_CONFIG]
SIZE = CONFIG_BURNER_INFO_SIZE;



