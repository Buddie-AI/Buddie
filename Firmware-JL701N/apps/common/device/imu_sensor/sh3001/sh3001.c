/******************************************************************
* Senodia 6D IMU(Gyroscope+Accelerometer) SH3001 driver, By Senodia AE;
*
*
* 1. unsigned char SH3001_init(void) : SH3001 initialize function
* 2. void SH3001_GetImuData( ... ): SH3001 read gyro and accel data function
* 3. float SH3001_GetTempData(void)£ºSH3001 read termperature function
* 4. void SH3001_pre_INT_config(void): Before using other INT config functions, need to call the function
*
*   For example: set accelerometer data ready INT
*   SH3001_pre_INT_config();
*
*	SH3001_INT_Config(	SH3001_INT0_LEVEL_HIGH,
*						SH3001_INT_NO_LATCH,
*						SH3001_INT1_LEVEL_HIGH,
*						SH3001_INT_CLEAR_STATUS,
*						SH3001_INT1_NORMAL,
*						SH3001_INT0_NORMAL,
*						3);
*
*   SH3001_INT_Enable(  SH3001_INT_ACC_READY,
*                       SH3001_INT_ENABLE,
*                       SH3001_INT_MAP_INT);
*
* 5. void SH3001_INT_Flat_Config( unsigned char flatTimeTH, unsigned char flatTanHeta2)£º
*    If using the function, need to config SH3001_INT_Orient_Config( ... ) function.
*
* 6. delay_ms(x): delay x millisecond.
*
* 7. New version SH3001: SH3001_CHIP_ID1(0xDF) = 0x61;
*    SH3001_CompInit(... ): initialize compensation coefficients;
*    SH3001_GetImuCompData( ... ): SH3001 read gyro and accel compensation data function
*
*
******************************************************************/
#include "app_config.h"
#include "asm/clock.h"
#include "asm/cpu.h"
#include "generic/typedef.h"
#include "generic/gpio.h"
#include "typedef.h"
#include "system/includes.h"
#include "media/includes.h"
#include "asm/iic_hw.h"
#include "asm/iic_soft.h"
#include "asm/timer.h"
#include "sh3001.h"
#include "imuSensor_manage.h"


#if TCFG_SH3001_ENABLE


/*************Betterlife ic debug***********/
#undef LOG_TAG_CONST
#define LOG_TAG     "[sh3001]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

void delay(volatile u32 t);
void udelay(u32 us);
#define   MDELAY(n)    mdelay(n)


static sh3001_param *sh3001_info;

// static SH3001_data SH3001_raw_data={0};

/******************************************************************
* Description:	I2C or SPI bus interface functions	and delay time function
*
* Parameters:
*   devAddr: I2C device address, modify the macro(SH3001_ADDRESS @SH3001.h) based on your I2C function.
*            If SPI interface, please ingnore the parameter.
*   regAddr: register address
*   readLen: data length to read
*   *readBuf: data buffer to read
*   writeLen: data length to write
*   *writeBuf: data buffer to write
*
******************************************************************/


#if (SH3001_USER_INTERFACE==0)

#if TCFG_SH3001_USER_IIC_TYPE
#define iic_init(iic)                       hw_iic_init(iic)
#define iic_uninit(iic)                     hw_iic_uninit(iic)
#define iic_start(iic)                      hw_iic_start(iic)
#define iic_stop(iic)                       hw_iic_stop(iic)
#define iic_tx_byte(iic, byte)              hw_iic_tx_byte(iic, byte)
#define iic_rx_byte(iic, ack)               hw_iic_rx_byte(iic, ack)
#define iic_read_buf(iic, buf, len)         hw_iic_read_buf(iic, buf, len)
#define iic_write_buf(iic, buf, len)        hw_iic_write_buf(iic, buf, len)
#define iic_suspend(iic)                    hw_iic_suspend(iic)
#define iic_resume(iic)                     hw_iic_resume(iic)
#else
#define iic_init(iic)                       soft_iic_init(iic)
#define iic_uninit(iic)                     soft_iic_uninit(iic)
#define iic_start(iic)                      soft_iic_start(iic)
#define iic_stop(iic)                       soft_iic_stop(iic)
#define iic_tx_byte(iic, byte)              soft_iic_tx_byte(iic, byte)
#define iic_rx_byte(iic, ack)               soft_iic_rx_byte(iic, ack)
#define iic_read_buf(iic, buf, len)         soft_iic_read_buf(iic, buf, len)
#define iic_write_buf(iic, buf, len)        soft_iic_write_buf(iic, buf, len)
#define iic_suspend(iic)                    soft_iic_suspend(iic)
#define iic_resume(iic)                     soft_iic_resume(iic)
#endif

//return: readLen:ok, other:fail
unsigned char I2C_Read_NBytes(unsigned char devAddr,
                              unsigned char regAddr,
                              unsigned char readLen,
                              unsigned char *readBuf)
{
    u8 i = 0;
    iic_start(sh3001_info->iic_hdl);
    if (0 == iic_tx_byte(sh3001_info->iic_hdl, devAddr)) {
        log_error("sh3001 iic read err1");
        iic_stop(sh3001_info->iic_hdl);
        return 0;
    }
    delay(sh3001_info->iic_delay);
    if (0 == iic_tx_byte(sh3001_info->iic_hdl, regAddr)) {
        log_error("sh3001 iic read err2");
        iic_stop(sh3001_info->iic_hdl);
        return 0;
    }
    delay(sh3001_info->iic_delay);

    iic_start(sh3001_info->iic_hdl);
    if (0 == iic_tx_byte(sh3001_info->iic_hdl, devAddr + 1)) {
        log_error("sh3001 iic read err3");
        iic_stop(sh3001_info->iic_hdl);
        return 0;
    }
    for (i = 0; i < readLen; i++) {
        delay(sh3001_info->iic_delay);
        if (i == (readLen - 1)) {
            *readBuf++ = iic_rx_byte(sh3001_info->iic_hdl, 0);
        } else {
            *readBuf++ = iic_rx_byte(sh3001_info->iic_hdl, 1);
        }
    }
    iic_stop(sh3001_info->iic_hdl);

    return i;
}

unsigned char I2C_Write_NBytes(unsigned char devAddr,
                               unsigned char regAddr,
                               unsigned char writeLen,
                               unsigned char *writeBuf)
{
    u8 i;
    iic_start(sh3001_info->iic_hdl);
    if (0 == iic_tx_byte(sh3001_info->iic_hdl, devAddr)) {
        log_error("sh3001 iic write err1");
        iic_stop(sh3001_info->iic_hdl);
        return 0;
    }
    delay(sh3001_info->iic_delay);
    if (0 == iic_tx_byte(sh3001_info->iic_hdl, regAddr)) {
        log_error("sh3001 iic write err2");
        iic_stop(sh3001_info->iic_hdl);
        return 0;
    }

    for (i = 0; i < writeLen; i++) {
        delay(sh3001_info->iic_delay);
        if (0 == iic_tx_byte(sh3001_info->iic_hdl, writeBuf[i])) {
            log_error("sh3001 iic write err3:%d", i);
            iic_stop(sh3001_info->iic_hdl);
            return 0;
        }
    }
    iic_stop(sh3001_info->iic_hdl);
    return i;
}

IMU_read    SH3001_read     = I2C_Read_NBytes;
IMU_write   SH3001_write    = I2C_Write_NBytes;

#else

#define spi_cs_init() \
    do { \
        gpio_write(sh3001_info->spi_cs_pin, 1); \
        gpio_set_direction(sh3001_info->spi_cs_pin, 0); \
        gpio_set_die(sh3001_info->spi_cs_pin, 1); \
    } while (0)
#define spi_cs_uninit() \
    do { \
        gpio_set_die(sh3001_info->spi_cs_pin, 0); \
        gpio_set_direction(sh3001_info->spi_cs_pin, 1); \
        gpio_set_pull_up(sh3001_info->spi_cs_pin, 0); \
        gpio_set_pull_down(sh3001_info->spi_cs_pin, 0); \
    } while (0)
#define spi_cs_h()                  gpio_write(sh3001_info->spi_cs_pin, 1)
#define spi_cs_l()                  gpio_write(sh3001_info->spi_cs_pin, 0)

#define spi_read_byte()             spi_recv_byte(sh3001_info->spi_hdl, NULL)
#define spi_write_byte(x)           spi_send_byte(sh3001_info->spi_hdl, x)
#define spi_dma_read(x, y)          spi_dma_recv(sh3001_info->spi_hdl, x, y)
#define spi_dma_write(x, y)         spi_dma_send(sh3001_info->spi_hdl, x, y)
#define spi_set_width(x)            spi_set_bit_mode(sh3001_info->spi_hdl, x)
#define spi_init()              spi_open(sh3001_info->spi_hdl)
#define spi_closed()            spi_close(sh3001_info->spi_hdl)
#define spi_suspend()           hw_spi_suspend(sh3001_info->spi_hdl)
#define spi_resume()            hw_spi_resume(sh3001_info->spi_hdl)

unsigned char SPI_readNBytes(unsigned char devAddr,
                             unsigned char regAddr,
                             unsigned char readLen,
                             unsigned char *readBuf)
{
    unsigned char data;
    data = (regAddr > 0x7F) ? 0x01 : 0x00;
    spi_cs_l();
    spi_write_byte(SH3001_SPI_REG_ACCESS);
    spi_write_byte(data);
    spi_cs_h();
    delay(60);

    spi_cs_l();
    spi_write_byte(regAddr | 0x80);
    spi_dma_read(readBuf, readLen);
    spi_cs_h();
    //SPIWrite(SH3001_SPI_REG_ACCESS, &u8Data, 1);
    //SPIRead((regAddr | 0x80), readBuf, readLen);
    return (SH3001_TRUE);
}

unsigned char SPI_writeNBytes(unsigned char devAddr,
                              unsigned char regAddr,
                              unsigned char writeLen,
                              unsigned char *writeBuf)
{
    unsigned char data;
    data = (regAddr > 0x7F) ? 0x01 : 0x00;
    spi_cs_l();
    spi_write_byte(SH3001_SPI_REG_ACCESS);
    spi_write_byte(data);
    spi_cs_h();
    delay(60);

    spi_cs_l();
    spi_write_byte(regAddr & 0x7F);
    spi_dma_write(writeBuf, writeLen);
    spi_cs_h();
    //SPIWrite(SH3001_SPI_REG_ACCESS, &u8Data, 1);
    //SPIWrite((regAddr & 0x7F), writeBuf, writeLen);
    return (SH3001_TRUE);
}

IMU_read 	SH3001_read		= SPI_readNBytes;
IMU_write	SH3001_write	= SPI_writeNBytes;

#endif


void delay_ms(unsigned char mSecond)
{
    //your delay code(mSecond: millisecond):
    MDELAY(mSecond);
}






/******************************************************************
* Description:	Local Variable
******************************************************************/
static compCoefType compCoef;
static unsigned char storeAccODR;


/******************************************************************
* Description:	Local Function Prototypes
******************************************************************/
static void SH3001_SoftReset(void);
static void SH3001_ADCReset(void);
static void SH3001_CVAReset(void);
static void SH3001_DriveStart(void);
static void SH3001_ModuleReset(void);

static void SH3001_Acc_Config(unsigned char accODR,
                              unsigned char accRange,
                              unsigned char accCutOffRreq,
                              unsigned char accFilterEnble);

static void SH3001_Gyro_Config(unsigned char gyroODR,
                               unsigned char gyroRangeX,
                               unsigned char gyroRangeY,
                               unsigned char gyroRangeZ,
                               unsigned char gyroCutOffRreq,
                               unsigned char gyroFilterEnble);

static void SH3001_Temp_Config(unsigned char tempODR,
                               unsigned char tempEnable);

static void SH3001_CompInit(compCoefType *compCoef);




/******************************************************************
* Description: reset Register;
*
* Parameters: void
*
* return:	void
*
******************************************************************/
static void SH3001_SoftReset(void)
{
    unsigned char regData = 0;

    regData = 0x84;
    SH3001_write(SH3001_ADDRESS, 0xD4, 1, &regData);
    delay_ms(1);
    regData = 0x04;
    SH3001_write(SH3001_ADDRESS, 0xD4, 1, &regData);
    delay_ms(1);
    regData = 0x08;
    SH3001_write(SH3001_ADDRESS, 0x2F, 1, &regData);
    delay_ms(1);
    regData = 0x73;
    SH3001_write(SH3001_ADDRESS, 0x00, 1, &regData);
    delay_ms(50);
}

/******************************************************************
* Description: SH3001_DriveStart function;
*
* Parameters: void
*
* return:	void
*
******************************************************************/
static void SH3001_DriveStart(void)
{
    unsigned char regAddr[3] = {0x2E, 0xC0, 0xC1};
    unsigned char regData[3] = {0};
    unsigned char regDataBack[3] = {0};
    unsigned char i = 0;

    for (i = 0; i < 3; i++) {
        SH3001_read(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
    }

    regDataBack[0] = 0xBF;
    regDataBack[1] = 0x38;
    regDataBack[2] = 0xFF;

    for (i = 0; i < 3; i++) {
        SH3001_write(SH3001_ADDRESS, regAddr[i], 1, &regDataBack[i]);
    }

    delay_ms(100);

    for (i = 0; i < 3; i++) {
        SH3001_write(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);;
    }

    delay_ms(50);
}

/******************************************************************
* Description: reset ADC;
*
* Parameters: void
*
* return:	void
*
******************************************************************/
static void SH3001_ADCReset(void)
{
    unsigned char regAddr[2] = {0xD3, 0xD5};
    unsigned char regData[2] = {0};
    unsigned char i = 0;

    for (i = 0; i < 2; i++) {
        SH3001_read(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
    }

    regData[0] = (regData[0] & 0xFC) | 0x01;
    regData[1] = (regData[1] & 0xF9) | 0x02;
    SH3001_write(SH3001_ADDRESS, 0xD5, 1, &regData[1]);
    SH3001_write(SH3001_ADDRESS, 0xD3, 1, &regData[0]);
    delay_ms(1);
    regData[0] = (regData[0] & 0xFC) | 0x02;
    SH3001_write(SH3001_ADDRESS, 0xD3, 1, &regData[0]);
    delay_ms(1);
    regData[1] = (regData[1] & 0xF9);
    SH3001_write(SH3001_ADDRESS, 0xD5, 1, &regData[1]);
    delay_ms(50);
}

/******************************************************************
* Description: reset CVA;
*
* Parameters: void
*
* return:	void
*
******************************************************************/
static void SH3001_CVAReset(void)
{
    unsigned char regData = 0;

    SH3001_read(SH3001_ADDRESS, 0xD4, 1, &regData);
    regData |= 0x08;
    SH3001_write(SH3001_ADDRESS, 0xD4, 1, &regData);
    delay_ms(10);
    regData &= 0xF7;
    SH3001_write(SH3001_ADDRESS, 0xD4, 1, &regData);
}

/******************************************************************
* Description: reset some internal modules;
*
* Parameters: void
*
* return:	void
*
******************************************************************/
static void SH3001_ModuleReset(void)
{
    //unsigned char regData = 0;

    SH3001_SoftReset();

#if (SH3001_USER_INTERFACE)
    if (sh3001_info->spi_work_mode) {
        SH3001_SPI_Config(SH3001_SPI_3_WIRE);
    } else {
        SH3001_SPI_Config(SH3001_SPI_4_WIRE);
    }
#endif

    SH3001_DriveStart();

    SH3001_ADCReset();

    SH3001_CVAReset();

    //set INT and INT1 Pin to Open-drain, in order to measure chip current
    //regData = 0x00;
    //SH3001_write(SH3001_ADDRESS, 0x44, 1, &regData);
}


/******************************************************************
* Description:	1.Switch SH3001 power mode;
*               2.Normal mode: 1.65mA; Sleep mode: 162uA; Acc normal mode:393uA;
*
* Parameters:   powerMode
*               SH3001_NORMAL_MODE
*               SH3001_SLEEP_MODE
*               SH3001_POWERDOWN_MODE
*				SH3001_ACC_NORMAL_MODE
*
* return:	SH3001_FALSE or SH3001_TRUE
*
******************************************************************/
unsigned char SH3001_SwitchPowerMode(unsigned char powerMode)
{
    unsigned char regAddr[11] = {0xCF, 0x22, 0x2F, 0xCB, 0xCE, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8};
    unsigned char regData[11] = {0};
    unsigned char i = 0;
    unsigned char accODR = SH3001_ODR_1000HZ;

    if ((powerMode != SH3001_NORMAL_MODE)
        && (powerMode != SH3001_SLEEP_MODE)
        && (powerMode != SH3001_POWERDOWN_MODE)
        && (powerMode != SH3001_ACC_NORMAL_MODE)) {
        return (SH3001_FALSE);
    }


    for (i = 0; i < 11; i++) {
        SH3001_read(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
    }

    switch (powerMode) {
    case SH3001_NORMAL_MODE:
        // restore accODR
        SH3001_write(SH3001_ADDRESS, SH3001_ACC_CONF1, 1, &storeAccODR);

        regData[0] = (regData[0] & 0xF8);
        regData[1] = (regData[1] & 0x7F);
        regData[2] = (regData[2] & 0xF7);
        regData[3] = (regData[3] & 0xF7);
        regData[4] = (regData[4] & 0xFE);
        regData[5] = (regData[5] & 0xFC) | 0x02;
        regData[6] = (regData[6] & 0x9F);
        regData[7] = (regData[7] & 0xF9);
        regData[10] = (regData[10] & 0xF7);
        for (i = 0; i < 8; i++) {
            SH3001_write(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
        }
        SH3001_write(SH3001_ADDRESS, regAddr[10], 1, &regData[10]);

        regData[7] = (regData[7] & 0x87);
        regData[8] = (regData[8] & 0x1F);
        regData[9] = (regData[9] & 0x03);
        regData[10] = (regData[10] & 0x1F);
        for (i = 7; i < 11; i++) {
            SH3001_write(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
        }
        SH3001_DriveStart();
        SH3001_ADCReset();
        SH3001_CVAReset();
        return (SH3001_TRUE);

    case SH3001_SLEEP_MODE:
        // store current acc ODR
        SH3001_read(SH3001_ADDRESS, SH3001_ACC_CONF1, 1, &storeAccODR);
        // set acc ODR=1000Hz
        SH3001_write(SH3001_ADDRESS, SH3001_ACC_CONF1, 1, &accODR);

        regData[0] = (regData[0] & 0xF8) | 0x07;
        regData[1] = (regData[1] & 0x7F) | 0x80;
        regData[2] = (regData[2] & 0xF7) | 0x08;
        regData[3] = (regData[3] & 0xF7) | 0x08;
        regData[4] = (regData[4] & 0xFE);
        regData[5] = (regData[5] & 0xFC) | 0x01;
        regData[6] = (regData[6] & 0x9F);
        regData[7] = (regData[7] & 0xF9) | 0x06;
        regData[10] = (regData[10] & 0xF7);
        for (i = 0; i < 8; i++) {
            SH3001_write(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
        }
        SH3001_write(SH3001_ADDRESS, regAddr[10], 1, &regData[10]);

        regData[7] = (regData[7] & 0x87);
        regData[8] = (regData[8] & 0x1F);
        regData[9] = (regData[9] & 0x03);
        regData[10] = (regData[10] & 0x1F);
        for (i = 7; i < 11; i++) {
            SH3001_write(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
        }
        return (SH3001_TRUE);

    case SH3001_POWERDOWN_MODE:
        regData[0] = (regData[0] & 0xF8);
        regData[1] = (regData[1] & 0x7F) | 0x80;
        regData[2] = (regData[2] & 0xF7) | 0x08;
        regData[3] = (regData[3] & 0xF7) | 0x08;
        regData[4] = (regData[4] & 0xFE);
        regData[5] = (regData[5] & 0xFC) | 0x01;
        regData[6] = (regData[6] & 0x9F) | 0x60;
        regData[7] = (regData[7] & 0xF9) | 0x06;
        regData[10] = (regData[10] & 0xF7);
        for (i = 0; i < 8; i++) {
            SH3001_write(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
        }
        SH3001_write(SH3001_ADDRESS, regAddr[10], 1, &regData[10]);

        regData[7] = (regData[7] & 0x87);
        regData[8] = (regData[8] & 0x1F);
        regData[9] = (regData[9] & 0x03);
        regData[10] = (regData[10] & 0x1F) | 0xE0;
        for (i = 7; i < 11; i++) {
            SH3001_write(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
        }
        return (SH3001_TRUE);

    case SH3001_ACC_NORMAL_MODE:
        regData[0] = (regData[0] & 0xF8);
        regData[1] = (regData[1] & 0x7F);
        regData[2] = (regData[2] & 0xF7);
        regData[3] = (regData[3] & 0xF7);
        regData[4] = (regData[4] | 0x01);
        regData[5] = (regData[5] & 0xFC) | 0x01;
        regData[6] = (regData[6] & 0x9F);
        regData[7] = (regData[7] & 0xF9) | 0x06;
        regData[10] = (regData[10] & 0xF7);
        for (i = 0; i < 8; i++) {
            SH3001_write(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
        }
        SH3001_write(SH3001_ADDRESS, regAddr[10], 1, &regData[10]);

        regData[7] = (regData[7] & 0x87) | 0x78;
        regData[8] = (regData[8] & 0x1F) | 0xE0;
        regData[9] = (regData[9] & 0x03) | 0xFC;
        regData[10] = (regData[10] & 0x1F);
        for (i = 7; i < 11; i++) {
            SH3001_write(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
        }
        return (SH3001_TRUE);

    default:
        return (SH3001_FALSE);
    }
}


/******************************************************************
* Description:	1.set accelerometer parameters;
*               2.accCutOffRreq = accODR * 0.40 or accODR * 0.25 or accODR * 0.11 or accODR * 0.04;
*
* Parameters: 	accODR              accRange                accCutOffFreq       accFilterEnble
*               SH3001_ODR_1000HZ	SH3001_ACC_RANGE_16G	SH3001_ACC_ODRX040	SH3001_ACC_FILTER_EN
*               SH3001_ODR_500HZ	SH3001_ACC_RANGE_8G		SH3001_ACC_ODRX025	SH3001_ACC_FILTER_DIS
*               SH3001_ODR_250HZ	SH3001_ACC_RANGE_4G		SH3001_ACC_ODRX011
*               SH3001_ODR_125HZ	SH3001_ACC_RANGE_2G		SH3001_ACC_ODRX004
*               SH3001_ODR_63HZ
*               SH3001_ODR_31HZ
*               SH3001_ODR_16HZ
*               SH3001_ODR_2000HZ
*               SH3001_ODR_4000HZ
*               SH3001_ODR_8000HZ
*
* return:	void
*
******************************************************************/
static void SH3001_Acc_Config(unsigned char accODR,
                              unsigned char accRange,
                              unsigned char accCutOffFreq,
                              unsigned char accFilterEnble)
{
    unsigned char regData = 0;

    // enable acc digital filter
    SH3001_read(SH3001_ADDRESS, SH3001_ACC_CONF0, 1, &regData);
    regData |= 0x01;
    SH3001_write(SH3001_ADDRESS, SH3001_ACC_CONF0, 1, &regData);

    // set acc ODR
    storeAccODR = accODR;
    SH3001_write(SH3001_ADDRESS, SH3001_ACC_CONF1, 1, &accODR);

    // set acc Range
    SH3001_write(SH3001_ADDRESS, SH3001_ACC_CONF2, 1, &accRange);

    // bypass acc low pass filter or not
    SH3001_read(SH3001_ADDRESS, SH3001_ACC_CONF3, 1, &regData);
    regData &= 0x17;
    regData |= (accCutOffFreq | accFilterEnble);
    SH3001_write(SH3001_ADDRESS, SH3001_ACC_CONF3, 1, &regData);
}


/******************************************************************
* Description:	1.set gyroscope parameters;
*               2.gyroCutOffRreq is at Page 32 of SH3001 datasheet.
*
* Parameters:   gyroODR             gyroRangeX,Y,Z          gyroCutOffFreq      gyroFilterEnble
*               SH3001_ODR_1000HZ	SH3001_GYRO_RANGE_125	SH3001_GYRO_ODRX00	SH3001_GYRO_FILTER_EN
*               SH3001_ODR_500HZ	SH3001_GYRO_RANGE_250	SH3001_GYRO_ODRX01	SH3001_GYRO_FILTER_DIS
*               SH3001_ODR_250HZ	SH3001_GYRO_RANGE_500	SH3001_GYRO_ODRX02
*               SH3001_ODR_125HZ	SH3001_GYRO_RANGE_1000  SH3001_GYRO_ODRX03
*               SH3001_ODR_63HZ		SH3001_GYRO_RANGE_2000
*               SH3001_ODR_31HZ
*               SH3001_ODR_16HZ
*               SH3001_ODR_2000HZ
*               SH3001_ODR_4000HZ
*               SH3001_ODR_8000HZ
*               SH3001_ODR_16000HZ
*               SH3001_ODR_32000HZ
*
* return:	void
*
******************************************************************/
static void SH3001_Gyro_Config(unsigned char gyroODR,
                               unsigned char gyroRangeX,
                               unsigned char gyroRangeY,
                               unsigned char gyroRangeZ,
                               unsigned char gyroCutOffFreq,
                               unsigned char gyroFilterEnble)
{
    unsigned char regData = 0;

    // enable gyro digital filter
    SH3001_read(SH3001_ADDRESS, SH3001_GYRO_CONF0, 1, &regData);
    regData |= 0x01;
    SH3001_write(SH3001_ADDRESS, SH3001_GYRO_CONF0, 1, &regData);

    // set gyro ODR
    SH3001_write(SH3001_ADDRESS, SH3001_GYRO_CONF1, 1, &gyroODR);

    // set gyro X\Y\Z range
    SH3001_write(SH3001_ADDRESS, SH3001_GYRO_CONF3, 1, &gyroRangeX);
    SH3001_write(SH3001_ADDRESS, SH3001_GYRO_CONF4, 1, &gyroRangeY);
    SH3001_write(SH3001_ADDRESS, SH3001_GYRO_CONF5, 1, &gyroRangeZ);

    // bypass gyro low pass filter or not
    SH3001_read(SH3001_ADDRESS, SH3001_GYRO_CONF2, 1, &regData);
    regData &= 0xE3;
    regData |= (gyroCutOffFreq | gyroFilterEnble);
    SH3001_write(SH3001_ADDRESS, SH3001_GYRO_CONF2, 1, &regData);
}


/******************************************************************
* Description:	set temperature parameters;
*
* Parameters: 	tempODR                 tempEnable
*               SH3001_TEMP_ODR_500		SH3001_TEMP_EN
*               SH3001_TEMP_ODR_250		SH3001_TEMP_DIS
*               SH3001_TEMP_ODR_125
*               SH3001_TEMP_ODR_63
*
* return:	void
*
******************************************************************/
static void SH3001_Temp_Config(unsigned char tempODR,
                               unsigned char tempEnable)
{
    unsigned char regData = 0;

    // enable temperature, set ODR
    SH3001_read(SH3001_ADDRESS, SH3001_TEMP_CONF0, 1, &regData);
    regData &= 0x4F;
    regData |= (tempODR | tempEnable);
    SH3001_write(SH3001_ADDRESS, SH3001_TEMP_CONF0, 1, &regData);
}


/******************************************************************
* Description:	read temperature parameters;
*
* Parameters: 	void
*
* return:	temperature data(deg);
*
******************************************************************/
float SH3001_GetTempData(void)
{
    unsigned char regData[2] = {0};
    unsigned short int tempref[2] = {0};

    // read temperature data, unsigned 12bits;   SH3001_TEMP_CONF0..SH3001_TEMP_CONF1
    SH3001_read(SH3001_ADDRESS, SH3001_TEMP_CONF0, 2, &regData[0]);
    tempref[0] = ((unsigned short int)(regData[0] & 0x0F) << 8) | regData[1];

    SH3001_read(SH3001_ADDRESS, SH3001_TEMP_ZL, 2, &regData[0]);
    tempref[1] = ((unsigned short int)(regData[1] & 0x0F) << 8) | regData[0];

    return ((((float)(tempref[1] - tempref[0])) / 16.0f) + 25.0f);
}


/******************************************************************
* Description:	enable or disable INT, mapping interrupt to INT pin or INT1 pin
*
* Parameters:   intType                 intEnable               intPinSel
*               SH3001_INT_LOWG         SH3001_INT_ENABLE       SH3001_INT_MAP_INT
*               SH3001_INT_HIGHG        SH3001_INT_DISABLE      SH3001_INT_MAP_INT1
*               SH3001_INT_INACT
*               SH3001_INT_ACT
*               SH3001_INT_DOUBLE_TAP
*               SH3001_INT_SINGLE_TAP
*               SH3001_INT_FLAT
*               SH3001_INT_ORIENTATION
*               SH3001_INT_FIFO_GYRO
*               SH3001_INT_GYRO_READY
*               SH3001_INT_ACC_FIFO
*               SH3001_INT_ACC_READY
*               SH3001_INT_FREE_FALL
*               SH3001_INT_UP_DOWN_Z
* return:	void
*
******************************************************************/
void SH3001_INT_Enable(unsigned short int intType,
                       unsigned char intEnable,
                       unsigned char intPinSel)
{
    unsigned char regData[2] = {0};
    unsigned short int u16IntVal = 0, u16IntTemp = 0;

    // Z axis change between UP to DOWN
    if ((intType & 0x0040) == SH3001_INT_UP_DOWN_Z) {
        SH3001_read(SH3001_ADDRESS, SH3001_ORIEN_INTCONF0, 1, &regData[0]);
        regData[0] = (intEnable == SH3001_INT_ENABLE) \
                     ? (regData[0] | 0x40) : (regData[0] & 0xBF);
        SH3001_write(SH3001_ADDRESS, SH3001_ORIEN_INTCONF0, 1, &regData[0]);
    }

    if ((intType & 0xFF1F)) {
        // enable or disable INT
        SH3001_read(SH3001_ADDRESS, SH3001_INT_ENABLE0, 2, &regData[0]);
        u16IntVal = ((unsigned short int)regData[0] << 8) | regData[1];

        u16IntVal = (intEnable == SH3001_INT_ENABLE) \
                    ? (u16IntVal | intType) : (u16IntVal & ~intType);

        regData[0] = (unsigned char)(u16IntVal >> 8);
        regData[1] = (unsigned char)(u16IntVal);
        SH3001_write(SH3001_ADDRESS, SH3001_INT_ENABLE0, 1, &regData[0]);
        SH3001_write(SH3001_ADDRESS, SH3001_INT_ENABLE1, 1, &regData[1]);


        // mapping interrupt to INT pin or INT1 pin
        SH3001_read(SH3001_ADDRESS, SH3001_INT_PIN_MAP0, 2, &regData[0]);
        u16IntVal = ((unsigned short int)regData[0] << 8) | regData[1];

        u16IntTemp = (intType << 1) & 0xE03E;
        u16IntTemp = u16IntTemp | (intType & 0x0F00);
        if (intType & SH3001_INT_LOWG) {
            u16IntTemp |= 0x0001;
        }

        u16IntVal = (intPinSel == SH3001_INT_MAP_INT1) \
                    ? (u16IntVal | u16IntTemp) : (u16IntVal & ~u16IntTemp);

        regData[0] = (unsigned char)(u16IntVal >> 8);
        regData[1] = (unsigned char)(u16IntVal);
        SH3001_write(SH3001_ADDRESS, SH3001_INT_PIN_MAP0, 1, &regData[0]);
        SH3001_write(SH3001_ADDRESS, SH3001_INT_PIN_MAP1, 1, &regData[1]);
    }
}


/******************************************************************
* Description:	1.config INT function
*               2.intCount is valid only when intLatch is equal to SH3001_INT_NO_LATCH;
*
* Parameters:   int0Level					intLatch             	int1Level                 intClear
*               SH3001_INT0_LEVEL_HIGH		SH3001_INT_NO_LATCH		SH3001_INT1_LEVEL_HIGH    SH3001_INT_CLEAR_ANY
*               SH3001_INT0_LEVEL_LOW	  	SH3001_INT_LATCH		SH3001_INT1_LEVEL_LOW     SH3001_INT_CLEAR_STATUS
*
*
*               int1Mode					int0Mode				intTime
*               SH3001_INT1_NORMAL			SH3001_INT0_NORMAL		Unit: 2mS
*               SH3001_INT1_OD		  		SH3001_INT0_OD
*
* return:	void
*
******************************************************************/
void SH3001_INT_Config(unsigned char int0Level,
                       unsigned char intLatch,
                       unsigned char int1Level,
                       unsigned char intClear,
                       unsigned char int1Mode,
                       unsigned char int0Mode,
                       unsigned char intTime)
{
    unsigned char regData = 0;

    SH3001_read(SH3001_ADDRESS, SH3001_INT_CONF, 1, &regData);

    regData = (int0Level == SH3001_INT0_LEVEL_LOW) \
              ? (regData | SH3001_INT0_LEVEL_LOW) : (regData & SH3001_INT0_LEVEL_HIGH);

    regData = (intLatch == SH3001_INT_NO_LATCH) \
              ? (regData | SH3001_INT_NO_LATCH) : (regData & SH3001_INT_LATCH);

    regData = (int1Level == SH3001_INT1_LEVEL_LOW) \
              ? (regData | SH3001_INT1_LEVEL_LOW) : (regData & SH3001_INT1_LEVEL_HIGH);

    regData = (intClear == SH3001_INT_CLEAR_ANY) \
              ? (regData | SH3001_INT_CLEAR_ANY) : (regData & SH3001_INT_CLEAR_STATUS);

    regData = (int1Mode == SH3001_INT1_NORMAL) \
              ? (regData | SH3001_INT1_NORMAL) : (regData & SH3001_INT1_OD);

    regData = (int0Mode == SH3001_INT0_NORMAL) \
              ? (regData | SH3001_INT0_NORMAL) : (regData & SH3001_INT0_OD);

    SH3001_write(SH3001_ADDRESS, SH3001_INT_CONF, 1, &regData);

    if (intLatch == SH3001_INT_NO_LATCH) {
        if (intTime != 0) {
            regData = intTime;
            SH3001_write(SH3001_ADDRESS, SH3001_INT_LIMIT, 1, &regData);
        }
    }
}

/******************************************************************
* Description:	1.lowG INT config;
*               2.lowGThres: x=0.5mG@2G or x=1mG@4G or x=2mG@8G or x=4mG@16G
*
* Parameters: 	lowGEnDisIntAll	            lowGThres		lowGTimsMs
*               SH3001_LOWG_ALL_INT_EN		Unit: x mG		Unit: 2mS
*               SH3001_LOWG_ALL_INT_DIS
*
* return:	void
*
******************************************************************/
void SH3001_INT_LowG_Config(unsigned char lowGEnDisIntAll,
                            unsigned char lowGThres,
                            unsigned char lowGTimsMs)
{
    unsigned char regData = 0;

    SH3001_read(SH3001_ADDRESS, SH3001_HIGHLOW_G_INT_CONF, 1, &regData);
    regData &= 0xFE;
    regData |= lowGEnDisIntAll;
    SH3001_write(SH3001_ADDRESS, SH3001_HIGHLOW_G_INT_CONF, 1, &regData);

    regData = lowGThres;
    SH3001_write(SH3001_ADDRESS, SH3001_LOWG_INT_THRESHOLD, 1, &regData);

    regData = lowGTimsMs;
    SH3001_write(SH3001_ADDRESS, SH3001_LOWG_INT_TIME, 1, &regData);
}


/******************************************************************
* Description:	1.highG INT config;
*               2.highGThres: x=0.5mG@2G, x=1mG@4G, x=2mG@8G, x=4mG@16G
*
* Parameters: 	highGEnDisIntX              highGEnDisIntY              highGEnDisIntZ
*               SH3001_HIGHG_X_INT_EN		SH3001_HIGHG_Y_INT_EN		SH3001_HIGHG_Z_INT_EN
*               SH3001_HIGHG_X_INT_DIS		SH3001_HIGHG_Y_INT_DIS		SH3001_HIGHG_Z_INT_DIS
*
*
*               highGEnDisIntAll			highGThres					highGTimsMs
*               SH3001_HIGHG_ALL_INT_EN		Unit: x mG					Unit: 2mS
*               SH3001_HIGHG_ALL_INT_DIS
*
* return:	void
*
******************************************************************/
void SH3001_INT_HighG_Config(unsigned char highGEnDisIntX,
                             unsigned char highGEnDisIntY,
                             unsigned char highGEnDisIntZ,
                             unsigned char highGEnDisIntAll,
                             unsigned char highGThres,
                             unsigned char highGTimsMs)
{
    unsigned char regData = 0;

    SH3001_read(SH3001_ADDRESS, SH3001_HIGHLOW_G_INT_CONF, 1, &regData);
    regData &= 0x0F;
    regData |= highGEnDisIntX | highGEnDisIntY | highGEnDisIntZ | highGEnDisIntAll;
    SH3001_write(SH3001_ADDRESS, SH3001_HIGHLOW_G_INT_CONF, 1, &regData);

    regData = highGThres;
    SH3001_write(SH3001_ADDRESS, SH3001_HIGHG_INT_THRESHOLD, 1, &regData);

    regData = highGTimsMs;
    SH3001_write(SH3001_ADDRESS, SH3001_HIGHG_INT_TIME, 1, &regData);
}


/******************************************************************
* Description:	inactivity INT config;
*
* Parameters: 	inactEnDisIntX              inactEnDisIntY				inactEnDisIntZ
*               SH3001_INACT_X_INT_EN		SH3001_INACT_Y_INT_EN		SH3001_INACT_Z_INT_EN
*               SH3001_INACT_X_INT_DIS	    SH3001_INACT_Y_INT_DIS	    SH3001_INACT_Z_INT_DIS
*
*
*               inactIntMode				inactLinkStatus				inactTimeS
*               SH3001_INACT_AC_MODE		SH3001_LINK_PRE_STA			Unit: S
*               SH3001_INACT_DC_MODE		SH3001_LINK_PRE_STA_NO
*
*
*               inactIntThres				inactG1
*               (3 Bytes)
*
* return:	void
*
******************************************************************/
void SH3001_INT_Inact_Config(unsigned char inactEnDisIntX,
                             unsigned char inactEnDisIntY,
                             unsigned char inactEnDisIntZ,
                             unsigned char inactIntMode,
                             unsigned char inactLinkStatus,
                             unsigned char inactTimeS,
                             unsigned int inactIntThres,
                             unsigned short int inactG1)
{
    unsigned char regData = 0;

    SH3001_read(SH3001_ADDRESS, SH3001_ACT_INACT_INT_CONF, 1, &regData);
    regData &= 0xF0;
    regData |= inactEnDisIntX | inactEnDisIntY | inactEnDisIntZ | inactIntMode;
    SH3001_write(SH3001_ADDRESS, SH3001_ACT_INACT_INT_CONF, 1, &regData);

    regData = (unsigned char)inactIntThres;
    SH3001_write(SH3001_ADDRESS, SH3001_INACT_INT_THRESHOLDL, 1, &regData);
    regData = (unsigned char)(inactIntThres >> 8);
    SH3001_write(SH3001_ADDRESS, SH3001_INACT_INT_THRESHOLDM, 1, &regData);
    regData = (unsigned char)(inactIntThres >> 16);
    SH3001_write(SH3001_ADDRESS, SH3001_INACT_INT_THRESHOLDH, 1, &regData);

    regData = inactTimeS;
    SH3001_write(SH3001_ADDRESS, SH3001_INACT_INT_TIME, 1, &regData);

    regData = (unsigned char)inactG1;
    SH3001_write(SH3001_ADDRESS, SH3001_INACT_INT_1G_REFL, 1, &regData);
    regData = (unsigned char)(inactG1 >> 8);
    SH3001_write(SH3001_ADDRESS, SH3001_INACT_INT_1G_REFH, 1, &regData);


    SH3001_read(SH3001_ADDRESS, SH3001_ACT_INACT_INT_LINK, 1, &regData);
    regData &= 0xFE;
    regData |= inactLinkStatus;
    SH3001_write(SH3001_ADDRESS, SH3001_ACT_INACT_INT_LINK, 1, &regData);
}



/******************************************************************
* Description:	activity INT config;
*				actIntThres: 1 LSB is 0.24mg@+/-2G, 0.48mg@+/-4G, 0.97mg@+/-8G, 1.95mg@+/-16G
*
* Parameters: 	actEnDisIntX			actEnDisIntY			actEnDisIntZ
*               SH3001_ACT_X_INT_EN		SH3001_ACT_Y_INT_EN		SH3001_ACT_Z_INT_EN
*               SH3001_ACT_X_INT_DIS	SH3001_ACT_Y_INT_DIS	SH3001_ACT_Z_INT_DIS
*
*
*               actIntMode				actTimeNum		actIntThres		actLinkStatus
*               SH3001_ACT_AC_MODE		(1 Byte)		(1 Byte)		SH3001_LINK_PRE_STA
*               SH3001_ACT_DC_MODE										SH3001_LINK_PRE_STA_NO
*
* return:	void
*
******************************************************************/
void SH3001_INT_Act_Config(unsigned char actEnDisIntX,
                           unsigned char actEnDisIntY,
                           unsigned char actEnDisIntZ,
                           unsigned char actIntMode,
                           unsigned char actTimeNum,
                           unsigned char actIntThres,
                           unsigned char actLinkStatus)
{
    unsigned char regData = 0;

    SH3001_read(SH3001_ADDRESS, SH3001_ACT_INACT_INT_CONF, 1, &regData);
    regData &= 0x0F;
    regData |= actEnDisIntX | actEnDisIntY | actEnDisIntZ | actIntMode;
    SH3001_write(SH3001_ADDRESS, SH3001_ACT_INACT_INT_CONF, 1, &regData);

    regData = actIntThres;
    SH3001_write(SH3001_ADDRESS, SH3001_ACT_INT_THRESHOLD, 1, &regData);

    regData = actTimeNum;
    SH3001_write(SH3001_ADDRESS, SH3001_ACT_INT_TIME, 1, &regData);

    SH3001_read(SH3001_ADDRESS, SH3001_ACT_INACT_INT_LINK, 1, &regData);
    regData &= 0xFE;
    regData |= actLinkStatus;
    SH3001_write(SH3001_ADDRESS, SH3001_ACT_INACT_INT_LINK, 1, &regData);
}



/******************************************************************
* Description:	1.tap INT config;
*               2.tapWaitTimeWindowMs is more than tapWaitTimeMs;
*
* Parameters: 	tapEnDisIntX				tapEnDisIntY				tapEnDisIntZ
*               SH3001_TAP_X_INT_EN			SH3001_TAP_Y_INT_EN			SH3001_TAP_Z_INT_EN
*               SH3001_TAP_X_INT_DIS		SH3001_TAP_Y_INT_DIS		SH3001_TAP_Z_INT_DIS
*
*
*               tapIntThres		tapTimeMs	    tapWaitTimeMs	    tapWaitTimeWindowMs
*               (1 Byte)		Unit: mS		Unit: mS			Unit: mS
*
* return:	void
*
******************************************************************/
void SH3001_INT_Tap_Config(unsigned char tapEnDisIntX,
                           unsigned char tapEnDisIntY,
                           unsigned char tapEnDisIntZ,
                           unsigned char tapIntThres,
                           unsigned char tapTimeMs,
                           unsigned char tapWaitTimeMs,
                           unsigned char tapWaitTimeWindowMs)
{
    unsigned char regData = 0;

    SH3001_read(SH3001_ADDRESS, SH3001_ACT_INACT_INT_LINK, 1, &regData);
    regData &= 0xF1;
    regData |= tapEnDisIntX | tapEnDisIntY | tapEnDisIntZ;
    SH3001_write(SH3001_ADDRESS, SH3001_ACT_INACT_INT_LINK, 1, &regData);

    regData = tapIntThres;
    SH3001_write(SH3001_ADDRESS, SH3001_TAP_INT_THRESHOLD, 1, &regData);

    regData = tapTimeMs;
    SH3001_write(SH3001_ADDRESS, SH3001_TAP_INT_DURATION, 1, &regData);

    regData = tapWaitTimeMs;
    SH3001_write(SH3001_ADDRESS, SH3001_TAP_INT_LATENCY, 1, &regData);

    regData = tapWaitTimeWindowMs;
    SH3001_write(SH3001_ADDRESS, SH3001_DTAP_INT_WINDOW, 1, &regData);
}


/******************************************************************
* Description:	flat INT time threshold, unit: mS;
*
* Parameters: 	flatTimeTH					flatTanHeta2
*               SH3001_FLAT_TIME_500MS		0 ~ 63
*               SH3001_FLAT_TIME_1000MS
*               SH3001_FLAT_TIME_2000MS
*
* return:	void
*
******************************************************************/
void SH3001_INT_Flat_Config(unsigned char flatTimeTH, unsigned char flatTanHeta2)
{
    unsigned char regData = 0;

    //SH3001_read(SH3001_ADDRESS, SH3001_FLAT_INT_CONF, 1, &regData);
    regData = (flatTimeTH & 0xC0) | (flatTanHeta2 & 0x3F);
    SH3001_write(SH3001_ADDRESS, SH3001_FLAT_INT_CONF, 1, &regData);
}


/******************************************************************
* Description:	orientation config;
*
* Parameters: 	orientBlockMode				orientMode					orientTheta
*               SH3001_ORIENT_BLOCK_MODE0	SH3001_ORIENT_SYMM			(1 Byte)
*               SH3001_ORIENT_BLOCK_MODE1	SH3001_ORIENT_HIGH_ASYMM
*               SH3001_ORIENT_BLOCK_MODE2	SH3001_ORIENT_LOW_ASYMM
*               SH3001_ORIENT_BLOCK_MODE3
*
*               orientG1point5				orientSlope					orientHyst
*               (2 Bytes)					(2 Bytes)					(2 Bytes)
*
* return:	void
*
******************************************************************/
void SH3001_INT_Orient_Config(unsigned char 	orientBlockMode,
                              unsigned char 	orientMode,
                              unsigned char	orientTheta,
                              unsigned short	orientG1point5,
                              unsigned short 	orientSlope,
                              unsigned short 	orientHyst)
{
    unsigned char regData[2] = {0};

    SH3001_read(SH3001_ADDRESS, SH3001_ORIEN_INTCONF0, 1, &regData[0]);
    regData[0] |= (regData[0] & 0xC0) | (orientTheta & 0x3F);
    SH3001_write(SH3001_ADDRESS, SH3001_ORIEN_INTCONF0, 1, &regData[0]);

    SH3001_read(SH3001_ADDRESS, SH3001_ORIEN_INTCONF1, 1, &regData[0]);
    regData[0] &= 0xF0;
    regData[0] |= (orientBlockMode & 0x0C) | (orientMode & 0x03);
    SH3001_write(SH3001_ADDRESS, SH3001_ORIEN_INTCONF1, 1, &regData[0]);

    regData[0] = (unsigned char)orientG1point5;
    regData[1] = (unsigned char)(orientG1point5 >> 8);
    SH3001_write(SH3001_ADDRESS, SH3001_ORIEN_INT_LOW, 1, &regData[0]);
    SH3001_write(SH3001_ADDRESS, SH3001_ORIEN_INT_HIGH, 1, &regData[1]);

    regData[0] = (unsigned char)orientSlope;
    regData[1] = (unsigned char)(orientSlope >> 8);
    SH3001_write(SH3001_ADDRESS, SH3001_ORIEN_INT_SLOPE_LOW, 1, &regData[0]);
    SH3001_write(SH3001_ADDRESS, SH3001_ORIEN_INT_SLOPE_HIGH, 1, &regData[1]);

    regData[0] = (unsigned char)orientHyst;
    regData[1] = (unsigned char)(orientHyst >> 8);
    SH3001_write(SH3001_ADDRESS, SH3001_ORIEN_INT_HYST_LOW, 1, &regData[0]);
    SH3001_write(SH3001_ADDRESS, SH3001_ORIEN_INT_HYST_HIGH, 1, &regData[1]);
}


/******************************************************************
* Description:	1.freeFall INT config;
*               2.freeFallThres: x=0.5mG@2G or x=1mG@4G or x=2mG@8G or x=4mG@16G
*
* Parameters: 	freeFallThres	freeFallTimsMs
*               Unit: x mG		Unit: 2mS
*
* return:	void
*
******************************************************************/
void SH3001_INT_FreeFall_Config(unsigned char freeFallThres,
                                unsigned char freeFallTimsMs)
{
    unsigned char regData = 0;

    regData = freeFallThres;
    SH3001_write(SH3001_ADDRESS, SH3001_FREEFALL_INT_THRES, 1, &regData);

    regData = freeFallTimsMs;
    SH3001_write(SH3001_ADDRESS, SH3001_FREEFALL_INT_TIME, 1, &regData);
}



/******************************************************************
* Description:	1.read INT status0
*
* Parameters: 	void
*
* return:	unsigned short
*           bit 11: Acc Data Ready Interrupt status
*           bit 10: Acc FIFO Watermark Interrupt status
*           bit 9: Gyro Ready Interrupt status
*           bit 8: Gyro FIFO Watermark Interrupt status
*           bit 7: Free-fall Interrupt status
*           bit 6: Orientation Interrupt status
*           bit 5: Flat Interrupt status
*           bit 4: Tap Interrupt status
*           bit 3: Single Tap Interrupt status
*           bit 2: Double Tap Interrupt status
*           bit 1: Activity Interrupt status
*           bit 0: Inactivity Interrupt status
*               0: Not Active
*               1: Active
*
******************************************************************/
unsigned short SH3001_INT_Read_Status0(void)
{
    unsigned char regData[2] = {0};

    SH3001_read(SH3001_ADDRESS, SH3001_INT_STA0, 2, &regData[0]);

    return (((unsigned short)(regData[1] & 0x0F) << 8) | regData[0]);
}


/******************************************************************
* Description:	1.read INT status2
*
* Parameters: 	void
*
* return:	unsigned char
*           bit 7: Sign of acceleration that trigger High-G Interrupt
*               0: Positive
*               1: Negative
*           bit 6: Whether High-G Interrupt is triggered by X axis
*           bit 5: Whether High-G Interrupt is triggered by Y axis
*           bit 4: Whether High-G Interrupt is triggered by Z axis
*               0: No
*               1: Yes
*           bit 3: Reserved
*           bit 2: Reserved
*           bit 1: Reserved
*           bit 0: Low-G Interrupt status
*               0: Not Active
*               1: Active
*
******************************************************************/
unsigned char SH3001_INT_Read_Status2(void)
{
    unsigned char regData = 0;

    SH3001_read(SH3001_ADDRESS, SH3001_INT_STA2, 1, &regData);

    return ((regData & 0xF1));
}


/******************************************************************
* Description:	1.read INT status3
*
* Parameters: 	void
*
* return:	unsigned char
*           bit 7: Sign of acceleration that trigger Activity or Inactivity Interrupt
*               0: Positive
*               1: Negative
*           bit 6: Whether Activity or Inactivity Interrupt is triggered by X axis
*           bit 5: Whether Activity or Inactivity Interrupt is triggered by Y axis
*           bit 4: Whether Activity or Inactivity Interrupt is triggered by Z axis
*               0: No
*               1: Yes
*			bit 3: Sign of acceleration that trigger Single or Double Tap Interrup
*               0: Positive
*               1: Negative
*			bit 2: Whether Single or Double Tap Interrupt is triggered by X axis
*			bit 1: Whether Single or Double Tap Interrupt is triggered by Y axis
*			bit 0: Whether Single or Double Tap Interrupt is triggered by Z axis
*               0: No
*               1: Yes
******************************************************************/
unsigned char SH3001_INT_Read_Status3(void)
{
    unsigned char regData = 0;

    SH3001_read(SH3001_ADDRESS, SH3001_INT_STA3, 1, &regData);

    return (regData);
}



/******************************************************************
* Description:	1.read INT status4
*
* Parameters: 	void
*
* return:	unsigned char
*           bit 7: Reserved
*           bit 6: Reserved
*           bit 5: Reserved
*           bit 4: Reserved
*           bit 3: Reserved
*           bit 2: Orientation Interrupt Value of Z-axis
*               0: Upward
*               1: Downward
*           bit 1..0: Orientation Interrupt Value of X and Y-axis
*               00: Landscape left
*               01: Landscape right
*               10: Portrait upside down
*               11: Portrait upright
*
******************************************************************/
unsigned char SH3001_INT_Read_Status4(void)
{
    unsigned char regData = 0;

    SH3001_read(SH3001_ADDRESS, SH3001_INT_STA4, 1, &regData);

    return (regData & 0x07);
}


/******************************************************************
* Description:	Before running other INT config functions, need to call SH3001_pre_INT_config();
*
* Parameters: 	void
*
* return:	void
*
*
******************************************************************/
void SH3001_pre_INT_config(void)
{
    unsigned char regData = 0;

    regData = 0x20;
    SH3001_write(SH3001_ADDRESS, SH3001_ORIEN_INTCONF0, 1, &regData);

    regData = 0x0A;
    SH3001_write(SH3001_ADDRESS, SH3001_ORIEN_INTCONF1, 1, &regData);

    regData = 0x01;
    SH3001_write(SH3001_ADDRESS, SH3001_ORIEN_INT_HYST_HIGH, 1, &regData);
}





/******************************************************************
* Description:	reset FIFO controller;
*
* Parameters: 	fifoMode
*               SH3001_FIFO_MODE_DIS
*               SH3001_FIFO_MODE_FIFO
*               SH3001_FIFO_MODE_STREAM
*               SH3001_FIFO_MODE_TRIGGER
*
* return:	void
*
******************************************************************/
void SH3001_FIFO_Reset(unsigned char fifoMode)
{
    unsigned char regData = 0;

    SH3001_read(SH3001_ADDRESS, SH3001_FIFO_CONF0, 1, &regData);
    regData |= 0x80;
    SH3001_write(SH3001_ADDRESS, SH3001_FIFO_CONF0, 1, &regData);
    regData &= 0x7F;
    SH3001_write(SH3001_ADDRESS, SH3001_FIFO_CONF0, 1, &regData);

    regData = fifoMode & 0x03;
    SH3001_write(SH3001_ADDRESS, SH3001_FIFO_CONF0, 1, &regData);
}


/******************************************************************
* Description:	1.FIFO down sample frequency config;
*               2.fifoAccFreq:	accODR*1/2, *1/4, *1/8, *1/16, *1/32, *1/64, *1/128, *1/256
*               3.fifoGyroFreq:	gyroODR*1/2, *1/4, *1/8, *1/16, *1/32, *1/64, *1/128, *1/256
*
* Parameters: 	fifoAccDownSampleEnDis		fifoAccFreq/fifoGyroFreq	fifoGyroDownSampleEnDis
*               SH3001_FIFO_ACC_DOWNS_EN	SH3001_FIFO_FREQ_X1_2		SH3001_FIFO_GYRO_DOWNS_EN
*               SH3001_FIFO_ACC_DOWNS_DIS	SH3001_FIFO_FREQ_X1_4		SH3001_FIFO_GYRO_DOWNS_DIS
*											SH3001_FIFO_FREQ_X1_8
*											SH3001_FIFO_FREQ_X1_16
*											SH3001_FIFO_FREQ_X1_32
*											SH3001_FIFO_FREQ_X1_64
*											SH3001_FIFO_FREQ_X1_128
*											SH3001_FIFO_FREQ_X1_256
* return:	void
*
******************************************************************/
void SH3001_FIFO_Freq_Config(unsigned char fifoAccDownSampleEnDis,
                             unsigned char fifoAccFreq,
                             unsigned char fifoGyroDownSampleEnDis,
                             unsigned char fifoGyroFreq)
{
    unsigned char regData = 0;

    regData |= fifoAccDownSampleEnDis | fifoGyroDownSampleEnDis;
    regData |= (fifoAccFreq << 4) | fifoGyroFreq;
    SH3001_write(SH3001_ADDRESS, SH3001_FIFO_CONF4, 1, &regData);
}


/******************************************************************
* Description:	1.data type config in FIFO;
*               2.fifoWaterMarkLevel is less than or equal to 1024;
*
* Parameters: 	fifoMode				    fifoWaterMarkLevel
*               SH3001_FIFO_EXT_Z_EN	    <=1024
*               SH3001_FIFO_EXT_Y_EN
*               SH3001_FIFO_EXT_X_EN
*               SH3001_FIFO_TEMPERATURE_EN
*               SH3001_FIFO_GYRO_Z_EN
*               SH3001_FIFO_GYRO_Y_EN
*               SH3001_FIFO_GYRO_X_EN
*               SH3001_FIFO_ACC_Z_EN
*               SH3001_FIFO_ACC_Y_EN
*               SH3001_FIFO_ACC_X_EN
*               SH3001_FIFO_ALL_DIS
*
* return:	void
*
******************************************************************/
void SH3001_FIFO_Data_Config(unsigned short fifoMode,
                             unsigned short fifoWaterMarkLevel)
{
    unsigned char regData = 0;

    if (fifoWaterMarkLevel > 1024) {
        fifoWaterMarkLevel = 1024;
    }

    SH3001_read(SH3001_ADDRESS, SH3001_FIFO_CONF2, 1, &regData);
    regData = (regData & 0xC8) \
              | ((unsigned char)(fifoMode >> 8) & 0x30) \
              | (((unsigned char)(fifoWaterMarkLevel >> 8)) & 0x07);
    SH3001_write(SH3001_ADDRESS, SH3001_FIFO_CONF2, 1, &regData);

    regData = (unsigned char)fifoMode;
    SH3001_write(SH3001_ADDRESS, SH3001_FIFO_CONF3, 1, &regData);

    regData = (unsigned char)fifoWaterMarkLevel;
    SH3001_write(SH3001_ADDRESS, SH3001_FIFO_CONF1, 1, &regData);
}



/******************************************************************
* Description:	1. read Fifo status and fifo entries count
*
* Parameters: 	*fifoEntriesCount, store fifo entries count, less than or equal to 1024;
*
*
* return:	unsigned char
*           bit 7: 0
*           bit 6: 0
*           bit 5: Whether FIFO Watermark has been reached
*           bit 4: Whether FIFO is full
*           bit 3: Whether FIFO is empty
*               0: No
*               1: Yes
*           bit 2: 0
*           bit 1: 0
*           bit 0: 0
*
*
******************************************************************/
unsigned char SH3001_FIFO_Read_Status(unsigned short int *fifoEntriesCount)
{
    unsigned char regData[2] = {0};

    SH3001_read(SH3001_ADDRESS, SH3001_FIFO_STA0, 2, &regData[0]);
    *fifoEntriesCount = ((unsigned short int)(regData[1] & 0x07) << 8) | regData[0];

    return (regData[1] & 0x38);
}


/******************************************************************
* Description:	1. read Fifo data
*
* Parameters: 	*fifoReadData		fifoDataLength
*               data				data length
*
* return:	void
*
*
******************************************************************/
void SH3001_FIFO_Read_Data(unsigned char *fifoReadData, unsigned short int fifoDataLength)
{
    if (fifoDataLength > 1024) {
        fifoDataLength = 1024;
    }

    while (fifoDataLength--) {
        SH3001_read(SH3001_ADDRESS, SH3001_FIFO_DATA, 1, fifoReadData);
        fifoReadData++;
    }
}






/******************************************************************
* Description:	reset Mater I2C;
*
* Parameters: 	void
*
* return:	void
*
******************************************************************/
void SH3001_MI2C_Reset(void)
{
    unsigned char regData = 0;

    SH3001_read(SH3001_ADDRESS, SH3001_MI2C_CONF0, 1, &regData);
    regData |= 0x80;
    SH3001_write(SH3001_ADDRESS, SH3001_MI2C_CONF0, 1, &regData);
    regData &= 0x7F;
    SH3001_write(SH3001_ADDRESS, SH3001_MI2C_CONF0, 1, &regData);

}

/******************************************************************
* Description:	1.master I2C config;
*               2.master I2C clock frequency is (1MHz/(6+3*mi2cFreq));19.6k ~ 166.666k
*               3.mi2cSlaveAddr: slave device address, 7 bits;
*
*
* Parameters: 	mi2cReadMode					mi2cODR							mi2cFreq
*               SH3001_MI2C_READ_MODE_AUTO		SH3001_MI2C_READ_ODR_200HZ		<=15
*               SH3001_MI2C_READ_MODE_MANUAL	SH3001_MI2C_READ_ODR_100HZ
*												SH3001_MI2C_READ_ODR_50HZ
*											    SH3001_MI2C_READ_ODR_25HZ
* return:	void
*
******************************************************************/
void SH3001_MI2C_Bus_Config(unsigned char mi2cReadMode,
                            unsigned char mi2cODR,
                            unsigned char mi2cFreq)
{
    unsigned char regData = 0;

    if (mi2cFreq > 15) {
        mi2cFreq = 15;
    }

    SH3001_read(SH3001_ADDRESS, SH3001_MI2C_CONF1, 1, &regData);
    regData = (regData & 0xC0) | (mi2cODR & 0x30) | mi2cFreq;
    SH3001_write(SH3001_ADDRESS, SH3001_MI2C_CONF1, 1, &regData);

    SH3001_read(SH3001_ADDRESS, SH3001_MI2C_CONF0, 1, &regData);
    regData = (regData & 0xBF) | mi2cReadMode;
    SH3001_write(SH3001_ADDRESS, SH3001_MI2C_CONF0, 1, &regData);
}



/******************************************************************
* Description:	1.master I2C address and command config;
*               2.mi2cSlaveAddr: slave device address, 7 bits;
*
* Parameters: 	mi2cSlaveAddr		mi2cSlaveCmd		mi2cReadMode
*               (1 Byte)			(1 Byte)			SH3001_MI2C_READ_MODE_AUTO
*                                                       SH3001_MI2C_READ_MODE_MANUAL
*
* return:	void
*
******************************************************************/
void SH3001_MI2C_Cmd_Config(unsigned char mi2cSlaveAddr,
                            unsigned char mi2cSlaveCmd,
                            unsigned char mi2cReadMode)
{
    unsigned char regData = 0;

    SH3001_read(SH3001_ADDRESS, SH3001_MI2C_CONF0, 1, &regData);
    regData = (regData & 0xBF) | mi2cReadMode;
    SH3001_write(SH3001_ADDRESS, SH3001_MI2C_CONF0, 1, &regData);

    regData = mi2cSlaveAddr << 1;
    SH3001_write(SH3001_ADDRESS, SH3001_MI2C_CMD0, 1, &regData);
    SH3001_write(SH3001_ADDRESS, SH3001_MI2C_CMD1, 1, &mi2cSlaveCmd);
}


/******************************************************************
* Description:	1.master I2C write data fucntion;
*               2.mi2cWriteData: write data;
*
* Parameters: 	mi2cWriteData
*
* return:	SH3001_TRUE or SH3001_FALSE
*
******************************************************************/
unsigned char SH3001_MI2C_Write(unsigned char mi2cWriteData)
{
    unsigned char regData = 0;
    unsigned char i = 0;

    SH3001_write(SH3001_ADDRESS, SH3001_MI2C_WR, 1, &mi2cWriteData);

    //Master I2C enable, write-operation
    SH3001_read(SH3001_ADDRESS, SH3001_MI2C_CONF0, 1, &regData);
    regData = (regData & 0xFC) | 0x02;
    SH3001_write(SH3001_ADDRESS, SH3001_MI2C_CONF0, 1, &regData);

    // wait write-operation to end
    while (i++ < 20) {
        SH3001_read(SH3001_ADDRESS, SH3001_MI2C_CONF0, 1, &regData);
        if (regData & 0x30) {
            break;
        }
    }

    if ((regData & 0x30) == SH3001_MI2C_SUCCESS) {
        return (SH3001_TRUE);
    } else {
        return (SH3001_FALSE);
    }
}

/******************************************************************
* Description:	1.master I2C read data fucntion;
*               2.*mi2cReadData: read data;
*
* Parameters: 	*mi2cReadData
*
* return:	SH3001_TRUE or SH3001_FALSE
*
******************************************************************/
unsigned char SH3001_MI2C_Read(unsigned char *mi2cReadData)
{
    unsigned char regData = 0;
    unsigned char i = 0;


    SH3001_read(SH3001_ADDRESS, SH3001_MI2C_CONF0, 1, &regData);
    if ((regData & 0x40) == 0) {
        //Master I2C enable, read-operation
        regData |= 0x03;
        SH3001_write(SH3001_ADDRESS, SH3001_MI2C_CONF0, 1, &regData);
    }

    // wait read-operation to end
    while (i++ < 20) {
        SH3001_read(SH3001_ADDRESS, SH3001_MI2C_CONF0, 1, &regData);
        if (regData & 0x30) {
            break;
        }
    }

    SH3001_read(SH3001_ADDRESS, SH3001_MI2C_CONF0, 1, &regData);
    if ((regData & 0x30) == SH3001_MI2C_SUCCESS) {
        SH3001_read(SH3001_ADDRESS, SH3001_MI2C_RD, 1, &regData);
        *mi2cReadData = regData;

        return (SH3001_TRUE);
    } else {
        return (SH3001_FALSE);
    }
}





/******************************************************************
* Description:	1.SPI interface config;  Default: SH3001_SPI_4_WIRE
*
* Parameters: 	spiInterfaceMode
*               SH3001_SPI_3_WIRE
*               SH3001_SPI_4_WIRE
*
* return:	void
*
******************************************************************/
void SH3001_SPI_Config(unsigned char spiInterfaceMode)
{
    unsigned char regData = spiInterfaceMode;

    SH3001_write(SH3001_ADDRESS, SH3001_SPI_CONF, 1, &regData);
}






/******************************************************************
* Description:	read some compensation coefficients
*
* Parameters: 	compCoefType
*
* return:	void
*
******************************************************************/
static void SH3001_CompInit(compCoefType *compCoef)
{
    unsigned char coefData[2] = {0};

    // Acc Cross
    SH3001_read(SH3001_ADDRESS, 0x81, 2, coefData);
    compCoef->cYX = (signed char)coefData[0];
    compCoef->cZX = (signed char)coefData[1];
    SH3001_read(SH3001_ADDRESS, 0x91, 2, coefData);
    compCoef->cXY = (signed char)coefData[0];
    compCoef->cZY = (signed char)coefData[1];
    SH3001_read(SH3001_ADDRESS, 0xA1, 2, coefData);
    compCoef->cXZ = (signed char)coefData[0];
    compCoef->cYZ = (signed char)coefData[1];

    // Gyro Zero
    SH3001_read(SH3001_ADDRESS, 0x60, 1, coefData);
    compCoef->jX = (signed char)coefData[0];
    SH3001_read(SH3001_ADDRESS, 0x68, 1, coefData);
    compCoef->jY = (signed char)coefData[0];
    SH3001_read(SH3001_ADDRESS, 0x70, 1, coefData);
    compCoef->jZ = (signed char)coefData[0];

    SH3001_read(SH3001_ADDRESS, SH3001_GYRO_CONF3, 1, coefData);
    coefData[0] = coefData[0] & 0x07;
    compCoef->xMulti = ((coefData[0] < 2) || (coefData[0] >= 7)) ? 1 : (1 << (6 - coefData[0]));
    SH3001_read(SH3001_ADDRESS, SH3001_GYRO_CONF4, 1, coefData);
    coefData[0] = coefData[0] & 0x07;
    compCoef->yMulti = ((coefData[0] < 2) || (coefData[0] >= 7)) ? 1 : (1 << (6 - coefData[0]));
    SH3001_read(SH3001_ADDRESS, SH3001_GYRO_CONF5, 1, coefData);
    coefData[0] = coefData[0] & 0x07;
    compCoef->zMulti = ((coefData[0] < 2) || (coefData[0] >= 7)) ? 1 : (1 << (6 - coefData[0]));

    SH3001_read(SH3001_ADDRESS, 0x2E, 1, coefData);
    compCoef->paramP0 = coefData[0] & 0x1F;
}










/******************************************************************
* Description: 	SH3001 initialization function
*
* Parameters:	void
*
* return:	SH3001_TRUE or SH3001_FALSE
*
******************************************************************/
unsigned char SH3001_init(void) //no fifo no int
{
    unsigned char regData = 0;
    unsigned char i = 0;

#if (SH3001_USER_INTERFACE)
    if (sh3001_info->spi_work_mode) {
        SH3001_SPI_Config(SH3001_SPI_3_WIRE);
    } else {
        SH3001_SPI_Config(SH3001_SPI_4_WIRE);
    }
#endif

    // SH3001 chipID = 0x61;
    while ((regData != 0x61) && (i++ < 3)) {
        SH3001_read(SH3001_ADDRESS, SH3001_CHIP_ID, 1, &regData);
        if ((i == 3) && (regData != 0x61)) {
            log_error("SH3001 init fail 0x%x\n", regData);
            return SH3001_FALSE;
        }
    }

    // reset internal module
    SH3001_ModuleReset();

    // 500Hz, 16G, cut off Freq(BW)=500*0.25Hz=125Hz, enable filter;
    SH3001_Acc_Config(SH3001_ODR_500HZ,
                      SH3001_ACC_RANGE_16G,
                      SH3001_ACC_ODRX025,
                      SH3001_ACC_FILTER_EN);

    // 500Hz, X\Y\Z 2000deg/s, cut off Freq(BW)=181Hz, enable filter;
    SH3001_Gyro_Config(SH3001_ODR_500HZ,
                       SH3001_GYRO_RANGE_2000,
                       SH3001_GYRO_RANGE_2000,
                       SH3001_GYRO_RANGE_2000,
                       SH3001_GYRO_ODRX00,
                       SH3001_GYRO_FILTER_EN);

    // temperature ODR is 63Hz, enable temperature measurement
    SH3001_Temp_Config(SH3001_TEMP_ODR_63, SH3001_TEMP_EN);

    SH3001_read(SH3001_ADDRESS, SH3001_CHIP_ID1, 1, &regData);
    if (regData == 0x61) {
        // read compensation coefficient
        SH3001_CompInit(&compCoef);
    }

    return SH3001_TRUE;
}
static u8 sh3001_int_pin = -1;
//return:0:fail, 1:ok
u8 SH3001_sensor_init(void *priv)
{
    if (priv == NULL) {
        log_error("sh3001 init fail(no param)\n");
        return SH3001_FALSE;
    }
    sh3001_info = (sh3001_param *)priv;

#if (SH3001_USER_INTERFACE==0) //iic interface
    iic_init(sh3001_info->iic_hdl);
#else//spi interface
    spi_cs_init();
    spi_init();
#endif
//int module io init
    gpio_set_die(sh3001_int_pin, 1);
    gpio_set_direction(sh3001_int_pin, 1);
    /* gpio_set_pull_up(sh3001_int_pin, 1); */
    /* gpio_set_pull_down(sh3001_int_pin, 0); */
    gpio_set_die(SH3001_INT_IO2, 1);
    gpio_set_direction(SH3001_INT_IO2, 1);
    /* gpio_set_pull_up(SH3001_INT_IO2, 1); */
    /* gpio_set_pull_down(SH3001_INT_IO2, 0); */
    return SH3001_init();
}



/******************************************************************
* Description: 	Read SH3001 gyroscope and accelerometer data
*
* Parameters:	accData[3]: acc X,Y,Z;  gyroData[3]: gyro X,Y,Z;
*
* return:	void
*
******************************************************************/
void SH3001_GetImuData(short accData[3], short gyroData[3])
{
    unsigned char regData[12] = {0};

    SH3001_read(SH3001_ADDRESS, SH3001_ACC_XL, 12, regData);
    accData[0] = ((short)regData[1] << 8) | regData[0];
    accData[1] = ((short)regData[3] << 8) | regData[2];
    accData[2] = ((short)regData[5] << 8) | regData[4];

    gyroData[0] = ((short)regData[7] << 8) | regData[6];
    gyroData[1] = ((short)regData[9] << 8) | regData[8];
    gyroData[2] = ((short)regData[11] << 8) | regData[10];

    log_info("sh3001:acc_x:%d acc_y:%d acc_z:%d gyro_x:%d gyro_y:%d gyro_z:%d", accData[0], accData[1], accData[2], gyroData[0], gyroData[1], gyroData[2]);
}


/******************************************************************
* Description: 	1.Read SH3001 gyroscope and accelerometer data after compensation
*				2.Only When the chipID is 0x61 at register 0xDF, can use SH3001_GetImuCompData() to get sensor data;
*
* Parameters:	accData[3]: acc X,Y,Z;  gyroData[3]: gyro X,Y,Z;
*
*
* return:	void
*
******************************************************************/
void SH3001_GetImuCompData(short accData[3], short gyroData[3])
{
    unsigned char regData[15] = {0};
    unsigned char paramP;
    int accTemp[3], gyroTemp[3];

    SH3001_read(SH3001_ADDRESS, SH3001_ACC_XL, 15, regData);
    accData[0] = ((short)regData[1] << 8) | regData[0];
    accData[1] = ((short)regData[3] << 8) | regData[2];
    accData[2] = ((short)regData[5] << 8) | regData[4];
    gyroData[0] = ((short)regData[7] << 8) | regData[6];
    gyroData[1] = ((short)regData[9] << 8) | regData[8];
    gyroData[2] = ((short)regData[11] << 8) | regData[10];
    paramP = regData[14] & 0x1F;

    // Acc Cross
    accTemp[0] = (int)(accData[0] + \
                       accData[1] * ((float)compCoef.cXY / 1024.0f) + \
                       accData[2] * ((float)compCoef.cXZ / 1024.0f));

    accTemp[1] = (int)(accData[0] * ((float)compCoef.cYX / 1024.0f) + \
                       accData[1] + \
                       accData[2] * ((float)compCoef.cYZ / 1024.0f));

    accTemp[2] = (int)(accData[0] * ((float)compCoef.cZX / 1024.0f) + \
                       accData[1] * ((float)compCoef.cZY / 1024.0f) + \
                       accData[2]);

    if (accTemp[0] > 32767) {
        accTemp[0] = 32767;
    } else if (accTemp[0] < -32768) {
        accTemp[0] = -32768;
    }
    if (accTemp[1] > 32767) {
        accTemp[1] = 32767;
    } else if (accTemp[1] < -32768) {
        accTemp[1] = -32768;
    }
    if (accTemp[2] > 32767) {
        accTemp[2] = 32767;
    } else if (accTemp[2] < -32768) {
        accTemp[2] = -32768;
    }
    accData[0] = (short)accTemp[0];
    accData[1] = (short)accTemp[1];
    accData[2] = (short)accTemp[2];

    gyroTemp[0] = gyroData[0] - (paramP - compCoef.paramP0) * compCoef.jX * compCoef.xMulti;
    gyroTemp[1] = gyroData[1] - (paramP - compCoef.paramP0) * compCoef.jY * compCoef.yMulti;
    gyroTemp[2] = gyroData[2] - (paramP - compCoef.paramP0) * compCoef.jZ * compCoef.zMulti;
    if (gyroTemp[0] > 32767) {
        gyroTemp[0] = 32767;
    } else if (gyroTemp[0] < -32768) {
        gyroTemp[0] = -32768;
    }
    if (gyroTemp[1] > 32767) {
        gyroTemp[1] = 32767;
    } else if (gyroTemp[1] < -32768) {
        gyroTemp[1] = -32768;
    }
    if (gyroTemp[2] > 32767) {
        gyroTemp[2] = 32767;
    } else if (gyroTemp[2] < -32768) {
        gyroTemp[2] = -32768;
    }

    // Gyro Zero
    gyroData[0] = (short)gyroTemp[0];
    gyroData[1] = (short)gyroTemp[1];
    gyroData[2] = (short)gyroTemp[2];

    /* log_info("sh3001:acc_x:%d acc_y:%d acc_z:%d gyro_x:%d gyro_y:%d gyro_z:%d", accData[0], accData[1], accData[2], gyroData[0], gyroData[1], gyroData[2]); */
}

static u8 init_flag = 0;
static u8 imu_busy = 0;
static sh3001_param sh3001_info_data;
/********************int test*******************/
//  For example: set accelerometer data ready INT
void sh3001_int_mode()
{
    SH3001_pre_INT_config();
    SH3001_INT_Config(SH3001_INT0_LEVEL_HIGH,  //低有效
                      SH3001_INT_NO_LATCH,    //锁住中断,只有清中断才恢复
                      SH3001_INT1_LEVEL_HIGH, //高有效
                      SH3001_INT_CLEAR_STATUS,   //读任意寄存器清中断
                      SH3001_INT1_NORMAL,     //int pin 普通输出
                      SH3001_INT0_NORMAL,
                      3);
    SH3001_INT_Enable(SH3001_INT_ACC_READY,     //
                      SH3001_INT_ENABLE,
                      SH3001_INT_MAP_INT);    //该中断映射到INT0 PIN
}
void sh3001_int_callback()
{
    if (init_flag == 0) {
        log_error("sh3001 init fail!");
        return ;
    }
    if (imu_busy) {
        log_error("sh3001 busy!");
        return ;
    }
    imu_busy = 1;

    short accData[3], gyroData[3];
    float TempData;
    SH3001_GetImuCompData(accData, gyroData);
    TempData = SH3001_GetTempData();
    /* SH3001_INT_Read_Status0(); */
    imu_busy = 0;
    log_info("sh3001:acc_x:%d acc_y:%d acc_z:%d gyro_x:%d gyro_y:%d gyro_z:%d", accData[0], accData[1], accData[2], gyroData[0], gyroData[1], gyroData[2]);
    log_info("temp:%d.%d\n", (u16)TempData, (u16)(((u16)(TempData * 100)) % 100));
}

s8 sh3001_dev_init(void *arg)
{
    if (arg == NULL) {
        log_error("sh3001 init fail(no arg)\n");
        return -1;
    }
#if (SH3001_USER_INTERFACE==0)
    sh3001_info_data.iic_hdl = ((struct imusensor_platform_data *)arg)->peripheral_hdl;
    sh3001_info_data.iic_delay = ((struct imusensor_platform_data *)arg)->peripheral_param0;   //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
    // u8 iic_clk;      //iic_clk: <=400kHz
#else
    sh3001_info_data.spi_hdl = ((struct imusensor_platform_data *)arg)->peripheral_hdl,     //SPIx (role:master)
                     sh3001_info_data.spi_cs_pin = ((struct imusensor_platform_data *)arg)->peripheral_param0; //IO_PORTA_05
    sh3001_info_data.spi_work_mode = ((struct imusensor_platform_data *)arg)->peripheral_param1; //1:3wire(SPI_MODE_UNIDIR_1BIT) or 0:4wire(SPI_MODE_BIDIR_1BIT) (与spi结构体一样)
    // u8 port;         //SPIx group:A,B,C,D (spi结构体)
    // U8 spi_clk;      //spi_clk: <=1MHz (spi结构体)
#endif

    sh3001_int_pin = ((struct imusensor_platform_data *)arg)->imu_sensor_int_io;
    if (imu_busy) {
        log_error("sh3001 busy!");
        return -1;
    }
    imu_busy = 1;

    if (SH3001_sensor_init(&sh3001_info_data)) { //no fifo
        log_info("sh3001 init success!\n");

#if (TCFG_SH3001_USER_INT_EN==0) //定时读
//注册定时：sh3001_int_callback()
#else //中断读
        //开中断
        sh3001_int_mode();
        log_info("int mode en!");
        /* port_wkup_enable(sh3001_int_pin, 0, sh3001_int_callback); //PA08-IO中断，0:上降沿触发，回调函数sh3001_int_callback*/
#ifdef CONFIG_CPU_BR23
        io_ext_interrupt_init(sh3001_int_pin, 1, sh3001_int_callback);
#elif defined(CONFIG_CPU_BR28)
        // br28外部中断回调函数，按照现在的外部中断注册方式
        // io配置在板级，定义在板级头文件，这里只是注册回调函数
        /* port_edge_wkup_set_callback_by_index(3, sh3001_int_callback); // 序号需要和板级配置中的wk_param对应上 */
        port_edge_wkup_set_callback(sh3001_int_callback);
#elif defined(CONFIG_CPU_BR27)
        port_edge_wkup_set_callback(sh3001_int_callback);
#endif

#endif
        init_flag = 1;
        imu_busy = 0;
        return 0;
    } else {
        log_error("sh3001 init fail!\n");
        imu_busy = 0;
        return -1;
    }
}


int sh3001_dev_ctl(u8 cmd, void *arg);
REGISTER_IMU_SENSOR(sh3001_sensor) = {
    .logo = "sh3001",
    .imu_sensor_init = sh3001_dev_init,
    .imu_sensor_check = NULL,
    .imu_sensor_ctl = sh3001_dev_ctl,
};

int sh3001_dev_ctl(u8 cmd, void *arg)
{
    int ret = -1;
    if (init_flag == 0) {
        log_error("sh3001 init fail!");
        return ret;//0:ok,,<0:err
    }
    if (imu_busy) {
        log_error("sh3001 busy!");
        return ret;//0:ok,,<0:err
    }
    imu_busy = 1;

    switch (cmd) {
    case IMU_GET_SENSOR_NAME:
        memcpy((u8 *)arg, &(sh3001_sensor.logo), 20);
        break;
    case IMU_SENSOR_ENABLE:
        /* cbuf_init(&hrsensor_cbuf, hrsensorcbuf, 24 * sizeof(int)); */
        SH3001_init();
        break;
    case IMU_SENSOR_DISABLE:
        /* cbuf_clear(&hrsensor_cbuf); */
        break;
    case IMU_SENSOR_RESET:
        break;
    case IMU_SENSOR_SLEEP:
        break;
    case IMU_SENSOR_WAKEUP:
        break;
    case IMU_SENSOR_INT_DET://传感器中断状态检查
        break;
    case IMU_SENSOR_DATA_READY://传感器数据准备就绪待读
        sh3001_int_callback();
        break;
    case IMU_SENSOR_CHECK_DATA://检查传感器缓存buf是否存满
        break;
    case IMU_SENSOR_READ_DATA://默认读传感器所有数据
        break;
    case IMU_GET_ACCEL_DATA://加速度数据
        break;
    case IMU_GET_GYRO_DATA://陀螺仪数据
        break;
    case IMU_GET_MAG_DATA://磁力计数据
        break;
    case IMU_SENSOR_SEARCH://检查传感器id
        SH3001_read(SH3001_ADDRESS, SH3001_CHIP_ID, 1, (u8 *)arg);
        if (*(u8 *)arg == 0x61) {
            ret = 0;
            log_info("sh3001 online!\n");
        } else {
            log_error("sh3001 offline!\n");
        }
        break;
    default:
        log_error("--cmd err!\n");
        break;
    }
    imu_busy = 0;
    return ret;

}



/***************************sh3001 test*******************************/

#if 0 //测试
static sh3001_param sh3001_info_test = {
#if (SH3001_USER_INTERFACE==0)
    .iic_hdl = 0,
    .iic_delay = 0,   //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
    // u8 iic_clk;      //iic_clk: <=400kHz
#else
    .spi_hdl = 1,     //SPIx (role:master)
    .spi_cs_pin = IO_PORTA_05, //
    .spi_work_mode = 1, //1:3wire(SPI_MODE_UNIDIR_1BIT) or 0:4wire(SPI_MODE_BIDIR_1BIT) (与spi结构体一样)
    // u8 port;         //SPIx group:A,B,C,D (spi结构体)
    // U8 spi_clk;      //spi_clk: <=1MHz (spi结构体)
#endif
};
void sh3001_test()
{
    short accData[3], gyroData[3];
    float TempData;
    if (SH3001_sensor_init(&sh3001_info_test)) { //no fifo no int
        log_info("sh3001 init success!\n");
        MDELAY(10);
#if (TCFG_SH3001_USER_INT_EN==0) //定时读
        SH3001_GetImuCompData(accData, gyroData);
        TempData = SH3001_GetTempData();
        log_info("sh3001 temp:%d.%d", (u16)TempData, (u16)(((u16)(TempData * 100)) % 100));
#else //中断读
        //开中断
        sh3001_int_mode();
        log_info("-------------------port wkup isr---------------------------");
        /* port_wkup_enable(sh3001_int_pin, 0, sh3001_int_callback); //PA08-IO中断，0:上降沿触发，回调函数sh3001_int_callback*/
#ifdef CONFIG_CPU_BR23
        io_ext_interrupt_init(sh3001_int_pin, 1, sh3001_int_callback);
#elif defined(CONFIG_CPU_BR28)
        // br28外部中断回调函数，按照现在的外部中断注册方式
        // io配置在板级，定义在板级头文件，这里只是注册回调函数
        /* port_edge_wkup_set_callback_by_index(3, sh3001_int_callback); // 序号需要和板级配置中的wk_param对应上 */
        port_edge_wkup_set_callback(sh3001_int_callback);
#elif defined(CONFIG_CPU_BR27)
        port_edge_wkup_set_callback(sh3001_int_callback);
#endif

#endif
    } else {
        log_error("sh3001 init fail!\n");
    }
}
#endif

#endif

