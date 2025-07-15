
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
#include "imuSensor_manage.h"
#include "qmi8658c.h"

// #include "spi1.h"
// #include "port_wkup.h"



#if TCFG_QMI8658_ENABLE

/*************Betterlife ic debug***********/
#undef LOG_TAG_CONST
#define LOG_TAG             "[QMI8658]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

void delay(volatile u32 t);
void udelay(u32 us);
#define   MDELAY(n)    mdelay(n)


static qmi8658_param *qmi8658_info;

// static qmi8658_data qmi8658_raw_data={0};

/******************************************************************
* Description:	I2C or SPI bus interface functions	and delay time function
*
* Parameters:
*   devAddr: I2C device address, modify the macro(QMI8658_ADDRESS @qmi8658.h) based on your I2C function.
*            If SPI interface, please ingnore the parameter.
*   regAddr: register address
*   readLen: data length to read
*   *readBuf: data buffer to read
*   writeLen: data length to write
*   *writeBuf: data buffer to write
*
******************************************************************/

#if (QMI8658_USER_INTERFACE==QMI8658_USE_I2C)

#if TCFG_QMI8658_USER_IIC_TYPE
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
//起止信号间隔：standard mode(100khz):4.7us; fast mode(400khz):1.3us
//return: readLen:ok, other:fail
u16 qmi8658_I2C_Read_NBytes(unsigned char devAddr,
                            unsigned char regAddr,
                            unsigned char *readBuf,
                            u16 readLen)
{
    u16 i = 0;
    local_irq_disable();
    iic_start(qmi8658_info->iic_hdl);
    if (0 == iic_tx_byte(qmi8658_info->iic_hdl, devAddr)) {
        log_error("qmi8658 iic read err1");
        goto __iic_exit_r;
    }
    delay(qmi8658_info->iic_delay);
    /* if (0 == iic_tx_byte(qmi8658_info->iic_hdl, regAddr |0x80)) {//|0x80地址自动递增 */
    if (0 == iic_tx_byte(qmi8658_info->iic_hdl, regAddr)) {//|0x80地址自动递增
        log_error("qmi8658 iic read err2");
        goto __iic_exit_r;
    }
    delay(qmi8658_info->iic_delay);

    iic_start(qmi8658_info->iic_hdl);
    if (0 == iic_tx_byte(qmi8658_info->iic_hdl, devAddr + 1)) {
        log_error("qmi8658 iic read err3");
        goto __iic_exit_r;
    }
    for (i = 0; i < readLen; i++) {
        delay(qmi8658_info->iic_delay);
        if (i == (readLen - 1)) {
            *readBuf++ = iic_rx_byte(qmi8658_info->iic_hdl, 0);
        } else {
            *readBuf++ = iic_rx_byte(qmi8658_info->iic_hdl, 1);
        }
        /* if(i%100==0)wdt_clear(); */
    }
__iic_exit_r:
    iic_stop(qmi8658_info->iic_hdl);
    local_irq_enable();

    return i;
}

//起止信号间隔：standard mode(100khz):4.7us; fast mode(400khz):1.3us
//return:1:ok, 0:fail
unsigned char qmi8658_I2C_Write_Byte(unsigned char devAddr,  //芯片支持1byte写
                                     unsigned char regAddr,
                                     unsigned char byte)
{
    u8 ret = 1;
    local_irq_disable();
    iic_start(qmi8658_info->iic_hdl);
    if (0 == iic_tx_byte(qmi8658_info->iic_hdl, devAddr)) {
        log_error("qmi8658 iic write err1");
        ret = 0;
        goto __iic_exit_w;
    }
    delay(qmi8658_info->iic_delay);
    if (0 == iic_tx_byte(qmi8658_info->iic_hdl, regAddr)) {
        log_error("qmi8658 iic write err2");
        ret = 0;
        goto __iic_exit_w;
    }
    delay(qmi8658_info->iic_delay);
    if (0 == iic_tx_byte(qmi8658_info->iic_hdl, byte)) {
        log_error("qmi8658 iic write err3");
        ret = 0;
        goto __iic_exit_w;
    }
__iic_exit_w:
    iic_stop(qmi8658_info->iic_hdl);
    local_irq_enable();
    return ret;
}
//起止信号间隔：standard mode(100khz):4.7us; fast mode(400khz):1.3us
//return: readLen:ok, other:fail
unsigned char qmi8658_I2C_Write_NBytes(unsigned char devAddr,  //芯片支持1byte写
                                       unsigned char regAddr,
                                       unsigned char *writeBuf,
                                       u16 writeLen)
{
    u16 i;
    for (i = 0; i < writeLen; i++) {
        if (qmi8658_I2C_Write_Byte(devAddr, regAddr + i, writeBuf[i]) == 0) {
            log_error("qmi8658_I2C_Write_NBytes error N:%d", i);
            return i;
        }
        udelay(5);//delay5us
    }
    return i;
}
IMU_read    Qmi8658_read     = qmi8658_I2C_Read_NBytes;
IMU_write   Qmi8658_write    = qmi8658_I2C_Write_Byte;

#elif (QMI8658_USER_INTERFACE==QMI8658_USE_SPI)

#define spi_cs_init() \
    do { \
        gpio_write(qmi8658_info->spi_cs_pin, 1); \
        gpio_set_direction(qmi8658_info->spi_cs_pin, 0); \
        gpio_set_die(qmi8658_info->spi_cs_pin, 1); \
    } while (0)
#define spi_cs_uninit() \
    do { \
        gpio_set_die(qmi8658_info->spi_cs_pin, 0); \
        gpio_set_direction(qmi8658_info->spi_cs_pin, 1); \
        gpio_set_pull_up(qmi8658_info->spi_cs_pin, 0); \
        gpio_set_pull_down(qmi8658_info->spi_cs_pin, 0); \
    } while (0)
#define spi_cs_h()                  gpio_write(qmi8658_info->spi_cs_pin, 1)
#define spi_cs_l()                  gpio_write(qmi8658_info->spi_cs_pin, 0)

#define spi_read_byte()             spi_recv_byte(qmi8658_info->spi_hdl, NULL)
#define spi_write_byte(x)           spi_send_byte(qmi8658_info->spi_hdl, x)
#define spi_dma_read(x, y)          spi_dma_recv(qmi8658_info->spi_hdl, x, y)
#define spi_dma_write(x, y)         spi_dma_send(qmi8658_info->spi_hdl, x, y)
#define spi_set_width(x)            spi_set_bit_mode(qmi8658_info->spi_hdl, x)
#define spi_init()              spi_open(qmi8658_info->spi_hdl)
#define spi_closed()            spi_close(qmi8658_info->spi_hdl)
#define spi_suspend()           hw_spi_suspend(qmi8658_info->spi_hdl)
#define spi_resume()            hw_spi_resume(qmi8658_info->spi_hdl)

u16 qmi8658_SPI_readNBytes(unsigned char devAddr,
                           unsigned char regAddr,
                           unsigned char *readBuf,
                           u16 readLen)
{
    spi_cs_l();
    spi_write_byte(regAddr | 0x80);//| 0x80:read mode
    spi_dma_read(readBuf, readLen);
    spi_cs_h();
    //SPIRead((regAddr | 0x80), readBuf, readLen);
    return (readLen);
}
unsigned char qmi8658_SPI_writeByte(unsigned char devAddr,
                                    unsigned char regAddr,
                                    unsigned char writebyte)
{
    spi_cs_l();
    spi_write_byte((regAddr) & 0x7F);
    spi_write_byte(writebyte);
    spi_cs_h();
    udelay(5);//delay5us
    return (1);
}
unsigned char qmi8658_SPI_writeNBytes(unsigned char devAddr,
                                      unsigned char regAddr,
                                      unsigned char *writeBuf,
                                      u16 writeLen)
{
#if 0 //多字节dma写
    spi_cs_l();
    spi_write_byte(regAddr & 0x7F);
    spi_dma_write(writeBuf, writeLen);
    spi_cs_h();
#else
    u16 i = 0;
    for (; i < writeLen; i++) {
        qmi8658_SPI_writeByte(devAddr, regAddr + i, writeBuf[i]);
    }
#endif
    //SPIWrite((regAddr & 0x7F), writeBuf, writeLen);
    return (i);
}

IMU_read 	Qmi8658_read	= qmi8658_SPI_readNBytes;
IMU_write	Qmi8658_write	= qmi8658_SPI_writeByte;
#else
//I3C
#endif

void Qmi8658_delay(int ms)
{
    //your delay code(mSecond: millisecond):
    MDELAY(ms);
}

static unsigned short acc_lsb_div = 0;
static unsigned short gyro_lsb_div = 0;
static unsigned short ae_q_lsb_div = (1 << 14);
static unsigned short ae_v_lsb_div = (1 << 10);
static unsigned int imu_timestamp = 0;
static struct Qmi8658Config qmi8658_config;

enum {
    AXIS_X = 0,
    AXIS_Y = 1,
    AXIS_Z = 2,

    AXIS_TOTAL
};
#if 0
typedef struct {
    short 				sign[AXIS_TOTAL];
    unsigned short 		map[AXIS_TOTAL];
} qst_imu_layout;

static qst_imu_layout imu_map;

void Qmi8658_set_layout(short layout)
{
    if (layout == 0) {
        imu_map.sign[AXIS_X] = 1;
        imu_map.sign[AXIS_Y] = 1;
        imu_map.sign[AXIS_Z] = 1;
        imu_map.map[AXIS_X] = AXIS_X;
        imu_map.map[AXIS_Y] = AXIS_Y;
        imu_map.map[AXIS_Z] = AXIS_Z;
    } else if (layout == 1) {
        imu_map.sign[AXIS_X] = -1;
        imu_map.sign[AXIS_Y] = 1;
        imu_map.sign[AXIS_Z] = 1;
        imu_map.map[AXIS_X] = AXIS_Y;
        imu_map.map[AXIS_Y] = AXIS_X;
        imu_map.map[AXIS_Z] = AXIS_Z;
    } else if (layout == 2) {
        imu_map.sign[AXIS_X] = -1;
        imu_map.sign[AXIS_Y] = -1;
        imu_map.sign[AXIS_Z] = 1;
        imu_map.map[AXIS_X] = AXIS_X;
        imu_map.map[AXIS_Y] = AXIS_Y;
        imu_map.map[AXIS_Z] = AXIS_Z;
    } else if (layout == 3) {
        imu_map.sign[AXIS_X] = 1;
        imu_map.sign[AXIS_Y] = -1;
        imu_map.sign[AXIS_Z] = 1;
        imu_map.map[AXIS_X] = AXIS_Y;
        imu_map.map[AXIS_Y] = AXIS_X;
        imu_map.map[AXIS_Z] = AXIS_Z;
    } else if (layout == 4) {
        imu_map.sign[AXIS_X] = -1;
        imu_map.sign[AXIS_Y] = 1;
        imu_map.sign[AXIS_Z] = -1;
        imu_map.map[AXIS_X] = AXIS_X;
        imu_map.map[AXIS_Y] = AXIS_Y;
        imu_map.map[AXIS_Z] = AXIS_Z;
    } else if (layout == 5) {
        imu_map.sign[AXIS_X] = 1;
        imu_map.sign[AXIS_Y] = 1;
        imu_map.sign[AXIS_Z] = -1;
        imu_map.map[AXIS_X] = AXIS_Y;
        imu_map.map[AXIS_Y] = AXIS_X;
        imu_map.map[AXIS_Z] = AXIS_Z;
    } else if (layout == 6) {
        imu_map.sign[AXIS_X] = 1;
        imu_map.sign[AXIS_Y] = -1;
        imu_map.sign[AXIS_Z] = -1;
        imu_map.map[AXIS_X] = AXIS_X;
        imu_map.map[AXIS_Y] = AXIS_Y;
        imu_map.map[AXIS_Z] = AXIS_Z;
    } else if (layout == 7) {
        imu_map.sign[AXIS_X] = -1;
        imu_map.sign[AXIS_Y] = -1;
        imu_map.sign[AXIS_Z] = -1;
        imu_map.map[AXIS_X] = AXIS_Y;
        imu_map.map[AXIS_Y] = AXIS_X;
        imu_map.map[AXIS_Z] = AXIS_Z;
    } else {
        imu_map.sign[AXIS_X] = 1;
        imu_map.sign[AXIS_Y] = 1;
        imu_map.sign[AXIS_Z] = 1;
        imu_map.map[AXIS_X] = AXIS_X;
        imu_map.map[AXIS_Y] = AXIS_Y;
        imu_map.map[AXIS_Z] = AXIS_Z;
    }
}
#endif

void Qmi8658_config_acc(enum Qmi8658_AccRange range, enum Qmi8658_AccOdr odr, enum Qmi8658_LpfConfig lpfEnable, enum Qmi8658_StConfig stEnable)
{
    unsigned char ctl_dada;

    switch (range) {
    case Qmi8658AccRange_2g:
        acc_lsb_div = (1 << 14);
        break;
    case Qmi8658AccRange_4g:
        acc_lsb_div = (1 << 13);
        break;
    case Qmi8658AccRange_8g:
        acc_lsb_div = (1 << 12);
        break;
    case Qmi8658AccRange_16g:
        acc_lsb_div = (1 << 11);
        break;
    default:
        range = Qmi8658AccRange_8g;
        acc_lsb_div = (1 << 12);
    }
    if (stEnable == Qmi8658St_Enable) {
        ctl_dada = (unsigned char)range | (unsigned char)odr | 0x80;
    } else {
        ctl_dada = (unsigned char)range | (unsigned char)odr;
    }

    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl2, ctl_dada);
// set LPF & HPF
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl5, &ctl_dada, 1);
    ctl_dada &= 0xf0;
    if (lpfEnable == Qmi8658Lpf_Enable) {
        ctl_dada |= A_LSP_MODE_3;
        ctl_dada |= 0x01;
    } else {
        ctl_dada &= ~0x01;
    }
    /* ctl_dada = 0x00; */
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl5, ctl_dada);
// set LPF & HPF
}

void Qmi8658_config_gyro(enum Qmi8658_GyrRange range, enum Qmi8658_GyrOdr odr, enum Qmi8658_LpfConfig lpfEnable, enum Qmi8658_StConfig stEnable)
{
    // Set the CTRL3 register to configure dynamic range and ODR
    unsigned char ctl_dada;

    // Store the scale factor for use when processing raw data
    switch (range) {
    case Qmi8658GyrRange_16dps:
        gyro_lsb_div = 2048;
        break;
    case Qmi8658GyrRange_32dps:
        gyro_lsb_div = 1024;
        break;
    case Qmi8658GyrRange_64dps:
        gyro_lsb_div = 512;
        break;
    case Qmi8658GyrRange_128dps:
        gyro_lsb_div = 256;
        break;
    case Qmi8658GyrRange_256dps:
        gyro_lsb_div = 128;
        break;
    case Qmi8658GyrRange_512dps:
        gyro_lsb_div = 64;
        break;
    case Qmi8658GyrRange_1024dps:
        gyro_lsb_div = 32;
        break;
    case Qmi8658GyrRange_2048dps:
        gyro_lsb_div = 16;
        break;
//		case Qmi8658GyrRange_4096dps:
//			gyro_lsb_div = 8;
//			break;
    default:
        range = Qmi8658GyrRange_512dps;
        gyro_lsb_div = 64;
        break;
    }

    if (stEnable == Qmi8658St_Enable) {
        ctl_dada = (unsigned char)range | (unsigned char)odr | 0x80;
    } else {
        ctl_dada = (unsigned char)range | (unsigned char)odr;
    }
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl3, ctl_dada);

// Conversion from degrees/s to rad/s if necessary
// set LPF & HPF
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl5, &ctl_dada, 1);
    ctl_dada &= 0x0f;
    if (lpfEnable == Qmi8658Lpf_Enable) {
        ctl_dada |= G_LSP_MODE_3;
        ctl_dada |= 0x10;
    } else {
        ctl_dada &= ~0x10;
    }
    /* ctl_dada = 0x00; */
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl5, ctl_dada);
// set LPF & HPF
}

void Qmi8658_config_mag(enum Qmi8658_MagDev device, enum Qmi8658_MagOdr odr)
{
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl4, device | odr);
}

void Qmi8658_config_ae(enum Qmi8658_AeOdr odr)
{
    //Qmi8658_config_acc(Qmi8658AccRange_8g, AccOdr_1000Hz, Lpf_Enable, St_Enable);
    //Qmi8658_config_gyro(Qmi8658GyrRange_2048dps, GyrOdr_1000Hz, Lpf_Enable, St_Enable);
    Qmi8658_config_acc(qmi8658_config.accRange, qmi8658_config.accOdr, Qmi8658Lpf_Enable, Qmi8658St_Disable);
    Qmi8658_config_gyro(qmi8658_config.gyrRange, qmi8658_config.gyrOdr, Qmi8658Lpf_Enable, Qmi8658St_Disable);
    Qmi8658_config_mag(qmi8658_config.magDev, qmi8658_config.magOdr);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl6, odr);
}

void Qmi8658_send_ctl9cmd(enum Qmi8658_Ctrl9Command cmd)    //cmd=0x0d
{
    unsigned char	status1 = 0x00;
    unsigned short count = 0;
    //void* pi2c = qmi8658_i2c_init();
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl9, (unsigned char)cmd);	// write commond to ctrl9
#if defined(QMI8658_HANDSHAKE_NEW)
#if defined(QMI8658_HANDSHAKE_TO_STATUS)
    unsigned char status_reg = Qmi8658Register_StatusInt;
    unsigned char cmd_done = 0x80;
#else
    unsigned char status_reg = Qmi8658Register_Status1;// Qmi8658Register_Status1
    unsigned char cmd_done = 0x01;
#endif
    //count = 0;
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, status_reg, &status1, 1);
    /* log_info("status1=%x\n",status1); */
    while (((status1 & cmd_done) != cmd_done) && (count++ < 200)) {	// read statusINT until bit7 is 1
        Qmi8658_delay(1);
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, status_reg, &status1, 1);
        //log_info("first %d status_reg: 0x%x\n", count,status1);
    }
    /* log_info("Qmi8658_config_fifo ctrl9 done-1: %d\n", count); */
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl9, Qmi8658_Ctrl9_Cmd_NOP);	// write commond  0x00 to ctrl9
    count = 0;
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, status_reg, &status1, 1);
    while (((status1 & cmd_done) == cmd_done) && (count++ < 200)) {	// read statusINT until bit7 is 0
        Qmi8658_delay(1);
        /* log_info("second %d status_reg: 0x%x\n", count,status1); */
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, status_reg, &status1, 1);
    }
    /* log_info("ctrl9 done-2: %d\n", count); */
#else
    while (((status1 & QMI8658_STATUS1_CMD_DONE) == 0) && (count++ < 200)) {
        Qmi8658_delay(2);
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Status1, &status1, sizeof(status1));
    }
    //log_info("fifo rst done : %d\n", count);
#endif

    //qmi8658_i2c_deinit(pi2c);

}


#if (QMI8658_USE_FIFO_EN)
/*
 * if mode=Qmi8658_Fifo_Stream then watermark <= size 否则不会输出中断信息
 * */
void Qmi8658_config_fifo(uint8_t watermark, enum Qmi8658_FifoSize size, enum Qmi8658_FifoMode mode, enum Qmi8658_fifo_format format)
{
    /* Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, 0x00); */
    Qmi8658_delay(1);
#if (QMI8658_NEW_HANDSHAKE == 1)
    u8 temp_data;
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl1, &temp_data, 1);
#if (QMI8658_FIFO_INT_OMAP_INT1 == 1)
    temp_data |= BIT(2); //
#else
    temp_data &= ~BIT(2); //
#endif
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl1, temp_data);
#endif

    qmi8658_config.fifo_format = format;		//QMI8658_FORMAT_12_BYTES;
    qmi8658_config.fifo_ctrl = (uint8_t)size | (uint8_t)mode;

    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_FifoCtrl, qmi8658_config.fifo_ctrl);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_FifoWmkTh, (unsigned char)watermark);
    Qmi8658_send_ctl9cmd(Qmi8658_Ctrl9_Cmd_Rst_Fifo);

    /* if(format == QMI8658_FORMAT_ACCEL_6_BYTES) */
    /* 	Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, 0x01); */
    /* else if(format == QMI8658_FORMAT_GYRO_6_BYTES) */
    /* 	Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, 0x02); */
    /* else if(format == QMI8658_FORMAT_12_BYTES) */
    /* 	Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, 0x03); */
}

/*
 *sensor data formal:
 *      3 sensor:acc, gyro, mag
 *       AX_L[0] AX_H[0] AY_L[0] AY_H[0] AZ_L[0] AZ_H[0]
 *       GX_L[0] GX_H[0] GY_L[0] GY_H[0] GZ_L[0] GZ_H[0]
 *       MX_L[0] MX_H[0] MY_L[0] MY_H[0] MZ_L[0] MZ_H[0]
 *       AX_L[1] AX_H[1] ......
 *      2 sensor:acc, gyro
 *       AX_L[0] AX_H[0] AY_L[0] AY_H[0] AZ_L[0] AZ_H[0]
 *       GX_L[0] GX_H[0] GY_L[0] GY_H[0] GZ_L[0] GZ_H[0]
 *       AX_L[1] AX_H[1] ......
 *      1 sensor:acc or gyro
 *       AX_L[0] AX_H[0]] AY_L[0] AY_H[0] AZ_L [0] AZ_H[0] AX_L[1] ......
 * */
unsigned short Qmi8658_read_fifo(unsigned char *data)
{
    unsigned char fifo_status[2] = {0, 0};
    unsigned short fifo_bytes = 0;
    unsigned short fifo_level = 0;
    //unsigned short i;

    Qmi8658_send_ctl9cmd(Qmi8658_Ctrl9_Cmd_Req_Fifo);

    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_FifoCount, fifo_status, 2);
    fifo_bytes = (unsigned short)(((fifo_status[1] & 0x03) << 8) | fifo_status[0]);
    fifo_bytes *= 2;
    if ((qmi8658_config.fifo_format == QMI8658_FORMAT_ACCEL_6_BYTES) || (qmi8658_config.fifo_format == QMI8658_FORMAT_GYRO_6_BYTES) || (qmi8658_config.fifo_format == QMI8658_FORMAT_MAG_6_BYTES)) {
        fifo_level = fifo_bytes / 6; // one sensor
        fifo_bytes = fifo_level * 6;
    } else if (qmi8658_config.fifo_format == QMI8658_FORMAT_12_BYTES) {
        fifo_level = fifo_bytes / 12; // two sensor
        fifo_bytes = fifo_level * 12;
    } else if (qmi8658_config.fifo_format == QMI8658_FORMAT_18_BYTES) {
        fifo_level = fifo_bytes / 18; // three sensor
        fifo_bytes = fifo_level * 18;
    }
    /* log_info("fifo byte=%d level=%d\n", fifo_bytes, fifo_level); */
    if (fifo_level > 0) {
#if 0
        for (i = 1; i < fifo_level; i++) {
            Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_FifoData, data + i * 12, 12);
        }
#else
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_FifoData, data, fifo_bytes);
#endif
    }
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_FifoCtrl, qmi8658_config.fifo_ctrl);

    return fifo_bytes;
}


void Qmi8658_get_fifo_format(enum Qmi8658_fifo_format *format)
{
    if (format) {
        *format = qmi8658_config.fifo_format;
    }
}

#endif




/* status0:46(0x2e)
 * BIT:  7:4        3            2     1    0
 *      reserved   AE(new data) mag  gyro  acc
 */
unsigned char Qmi8658_readStatus0(void)
{
    unsigned char status[2];

    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Status0, status, sizeof(status));
    log_info("status[0:0x%x	1:0x%x]", status[0], status[1]);

    return status[0];
}
/*!status1:47(0x2f)
 * \brief Blocking read of data status register 1 (::Qmi8658Register_Status1).
 * bit7~bit1: reserved
 * bit0:      Used to indicate ctrl9 Command was done.
 * \returns Status byte \see STATUS1 for flag definitions.
 */
unsigned char Qmi8658_readStatus1(void)
{
    unsigned char status;

    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Status1, &status, sizeof(status));
    /* log_info("status1[0x%x]\n",status); */

    return status;
}

/*
 * Sample time stamp. Count incremented by one for each sample
(x, y, z data set) from sensor with highest ODR (circular register
0x0-0xFFFFFF)
 */
unsigned char Qmi8658_read_time_stamp_reg(void)
{
    unsigned char time_stamp[3];
    u32 sample_time = 0;

    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Timestamp_L, time_stamp, sizeof(time_stamp));
    sample_time = (u32)(time_stamp[2] << 16) | (u32)(time_stamp[1] << 8) | time_stamp[0];
    /* log_info("Sample time stamp =[0x%x]\n",sample_time); */
    return sample_time;
}
/************************0x59地址文档未介绍*****************************/
unsigned char Qmi8658_read_temptap_reg(void)
{
    unsigned char status;

    Qmi8658_read(QMI8658_SLAVE_ADDRESS, 0x59, &status, sizeof(status));
    log_info("tap_temp_reg =[0x%x]\n", status);

    return status;
}

float Qmi8658_readTemp(void)
{
    unsigned char buf[2];
    short temp = 0;
    float temp_f = 0;

    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Tempearture_L, buf, 2);
    temp = ((short)buf[1] << 8) | buf[0];
    temp_f = (float)temp / 256.0f;

    return temp_f;
}

void Qmi8658_read_acc_xyz(float acc_xyz[3])
{
    unsigned char	buf_reg[6];
    short 			raw_acc_xyz[3];

    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ax_L, buf_reg, 6); 	// 0x35, 53
    raw_acc_xyz[0] = (short)((unsigned short)(buf_reg[1] << 8) | (buf_reg[0]));
    raw_acc_xyz[1] = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
    raw_acc_xyz[2] = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));

    acc_xyz[0] = (raw_acc_xyz[0] * ONE_G) / acc_lsb_div;
    acc_xyz[1] = (raw_acc_xyz[1] * ONE_G) / acc_lsb_div;
    acc_xyz[2] = (raw_acc_xyz[2] * ONE_G) / acc_lsb_div;

    //log_info("fis210x acc:	%f	%f	%f\n", acc_xyz[0], acc_xyz[1], acc_xyz[2]);
}

void Qmi8658_read_gyro_xyz(float gyro_xyz[3])
{
    unsigned char	buf_reg[6];
    short 			raw_gyro_xyz[3];

    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Gx_L, buf_reg, 6);  	// 0x3b, 59
    raw_gyro_xyz[0] = (short)((unsigned short)(buf_reg[1] << 8) | (buf_reg[0]));
    raw_gyro_xyz[1] = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
    raw_gyro_xyz[2] = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));

    /* log_info("gyro:	%d	%d	%d\n", raw_gyro_xyz[0], raw_gyro_xyz[1], raw_gyro_xyz[2]); */

    gyro_xyz[0] = (raw_gyro_xyz[0] * 1.0f) / gyro_lsb_div;
    gyro_xyz[1] = (raw_gyro_xyz[1] * 1.0f) / gyro_lsb_div;
    gyro_xyz[2] = (raw_gyro_xyz[2] * 1.0f) / gyro_lsb_div;

    //log_info("fis210x gyro:	%f	%f	%f\n", gyro_xyz[0], gyro_xyz[1], gyro_xyz[2]);
}

/******
 * tim_count: Sample time stamp. Count incremented by one for each sample
(x, y, z data set) from sensor with highest ODR (circular register
0x0-0xFFFFFF).
*****/
void Qmi8658_read_xyz(float acc[3], float gyro[3], unsigned int *tim_count)
{
    unsigned char	buf_reg[12];
    short 			raw_acc_xyz[3];
    short 			raw_gyro_xyz[3];
//	float acc_t[3];
//	float gyro_t[3];

    if (tim_count) {
        unsigned char	buf[3];
        unsigned int timestamp;
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Timestamp_L, buf, 3);	// 0x18	24
        timestamp = (unsigned int)(((unsigned int)buf[2] << 16) | ((unsigned int)buf[1] << 8) | buf[0]);
        if (timestamp > imu_timestamp) {
            imu_timestamp = timestamp;
        } else {
            imu_timestamp = (timestamp + 0x1000000 - imu_timestamp);
        }

        *tim_count = imu_timestamp;
    }

    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ax_L, buf_reg, 12); 	// 0x19, 25
    raw_acc_xyz[0] = (short)((unsigned short)(buf_reg[1] << 8) | (buf_reg[0]));
    raw_acc_xyz[1] = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
    raw_acc_xyz[2] = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));

    raw_gyro_xyz[0] = (short)((unsigned short)(buf_reg[7] << 8) | (buf_reg[6]));
    raw_gyro_xyz[1] = (short)((unsigned short)(buf_reg[9] << 8) | (buf_reg[8]));
    raw_gyro_xyz[2] = (short)((unsigned short)(buf_reg[11] << 8) | (buf_reg[10]));

#if (QMI8658_UINT_MG_DPS)
    // mg
    acc[AXIS_X] = (float)(raw_acc_xyz[AXIS_X] * 1000.0f) / acc_lsb_div;
    acc[AXIS_Y] = (float)(raw_acc_xyz[AXIS_Y] * 1000.0f) / acc_lsb_div;
    acc[AXIS_Z] = (float)(raw_acc_xyz[AXIS_Z] * 1000.0f) / acc_lsb_div;
#else
    // m/s2
    acc[AXIS_X] = (float)(raw_acc_xyz[AXIS_X] * ONE_G) / acc_lsb_div;
    acc[AXIS_Y] = (float)(raw_acc_xyz[AXIS_Y] * ONE_G) / acc_lsb_div;
    acc[AXIS_Z] = (float)(raw_acc_xyz[AXIS_Z] * ONE_G) / acc_lsb_div;
#endif
//	acc[AXIS_X] = imu_map.sign[AXIS_X]*acc_t[imu_map.map[AXIS_X]];
//	acc[AXIS_Y] = imu_map.sign[AXIS_Y]*acc_t[imu_map.map[AXIS_Y]];
//	acc[AXIS_Z] = imu_map.sign[AXIS_Z]*acc_t[imu_map.map[AXIS_Z]];

#if (QMI8658_UINT_MG_DPS)
    // dps
    gyro[0] = (float)(raw_gyro_xyz[0] * 1.0f) / gyro_lsb_div;
    gyro[1] = (float)(raw_gyro_xyz[1] * 1.0f) / gyro_lsb_div;
    gyro[2] = (float)(raw_gyro_xyz[2] * 1.0f) / gyro_lsb_div;
#else
    // rad/s
    gyro[AXIS_X] = (float)(raw_gyro_xyz[AXIS_X] * 0.01745f) / gyro_lsb_div;		// *pi/180
    gyro[AXIS_Y] = (float)(raw_gyro_xyz[AXIS_Y] * 0.01745f) / gyro_lsb_div;
    gyro[AXIS_Z] = (float)(raw_gyro_xyz[AXIS_Z] * 0.01745f) / gyro_lsb_div;
#endif
//	gyro[AXIS_X] = imu_map.sign[AXIS_X]*gyro_t[imu_map.map[AXIS_X]];
//	gyro[AXIS_Y] = imu_map.sign[AXIS_Y]*gyro_t[imu_map.map[AXIS_Y]];
//	gyro[AXIS_Z] = imu_map.sign[AXIS_Z]*gyro_t[imu_map.map[AXIS_Z]];
}

/******
 * tim_count: Sample time stamp. Count incremented by one for each sample
(x, y, z data set) from sensor with highest ODR (circular register
0x0-0xFFFFFF).
*****/
void Qmi8658_read_xyz_raw(short raw_acc_xyz[3], short raw_gyro_xyz[3], unsigned int *tim_count)
{
    unsigned char	buf_reg[12];

    if (tim_count) {
        unsigned char	buf[3];
        unsigned int timestamp;
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Timestamp_L, buf, 3);	// 0x18	24
        timestamp = (unsigned int)(((unsigned int)buf[2] << 16) | ((unsigned int)buf[1] << 8) | buf[0]);
        if (timestamp > imu_timestamp) {
            imu_timestamp = timestamp;
        } else {
            imu_timestamp = (timestamp + 0x1000000 - imu_timestamp);
        }

        *tim_count = imu_timestamp;
    }
    if (raw_acc_xyz && raw_gyro_xyz) {
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ax_L, buf_reg, 12); 	// 0x19, 25
        raw_acc_xyz[0] = (short)((unsigned short)(buf_reg[1] << 8) | (buf_reg[0]));
        raw_acc_xyz[1] = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
        raw_acc_xyz[2] = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));
        raw_gyro_xyz[0] = (short)((unsigned short)(buf_reg[7] << 8) | (buf_reg[6]));
        raw_gyro_xyz[1] = (short)((unsigned short)(buf_reg[9] << 8) | (buf_reg[8]));
        raw_gyro_xyz[2] = (short)((unsigned short)(buf_reg[11] << 8) | (buf_reg[10]));
    } else if (raw_acc_xyz) {
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ax_L, buf_reg, 6); 	// 0x19, 25
        raw_acc_xyz[0] = (short)((unsigned short)(buf_reg[1] << 8) | (buf_reg[0]));
        raw_acc_xyz[1] = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
        raw_acc_xyz[2] = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));
    } else if (raw_gyro_xyz) {
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Gx_L, buf_reg, 6); 	// 0x19, 25
        raw_gyro_xyz[0] = (short)((unsigned short)(buf_reg[1] << 8) | (buf_reg[0]));
        raw_gyro_xyz[1] = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
        raw_gyro_xyz[2] = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));
    }
}

void Qmi8658_read_raw_acc_xyz(void *acc_data)
{
    unsigned char	buf_reg[8];
    imu_axis_data_t *raw_acc_xyz = (imu_axis_data_t *)acc_data;
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ax_L, buf_reg, 6); 	// 53
    raw_acc_xyz->x = (short)((unsigned short)(buf_reg[1] << 8) | (buf_reg[0]));
    raw_acc_xyz->y = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
    raw_acc_xyz->z = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));

    /* log_info("Qmi8658 acc: %d %d %d\n", raw_acc_xyz->x, raw_acc_xyz->y, raw_acc_xyz->z); */
}
void Qmi8658_read_raw_gyro_xyz(void *gyro_data)
{
    unsigned char	buf_reg[6];
    imu_axis_data_t *raw_gyro_xyz = (imu_axis_data_t *)gyro_data;
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Gx_L, buf_reg, 6); 	// 53
    raw_gyro_xyz->x = (short)((unsigned short)(buf_reg[1] << 8) | (buf_reg[0]));
    raw_gyro_xyz->y = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
    raw_gyro_xyz->z = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));

    /* log_info("Qmi8658 gyro: %d %d %d\n", raw_gyro_xyz->x, raw_gyro_xyz->y, raw_gyro_xyz->z); */
}
void Qmi8658_read_raw_acc_gyro_xyz(void *raw_data)
{
    unsigned char	buf_reg[14];
    short temp = 0;
    imu_sensor_data_t *raw_sensor_data = (imu_sensor_data_t *)raw_data;
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Tempearture_L, buf_reg, 14);  	// 0x3b, 59
    temp = ((short)buf_reg[1] << 8) | buf_reg[0];
    raw_sensor_data->acc.x = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
    raw_sensor_data->acc.y = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));
    raw_sensor_data->acc.z = (short)((unsigned short)(buf_reg[7] << 8) | (buf_reg[6]));

    raw_sensor_data->gyro.x = (short)((unsigned short)(buf_reg[9] << 8) | (buf_reg[8]));
    raw_sensor_data->gyro.y = (short)((unsigned short)(buf_reg[11] << 8) | (buf_reg[10]));
    raw_sensor_data->gyro.z = (short)((unsigned short)(buf_reg[13] << 8) | (buf_reg[12]));
    raw_sensor_data->temp_data = (float)temp / 256.0f;
    /* log_info("Qmi8658 raw:acc_x:%d acc_y:%d acc_z:%d gyro_x:%d gyro_y:%d gyro_z:%d", raw_sensor_data->acc.x, raw_sensor_data->acc.y, raw_sensor_data->acc.z, raw_sensor_data->gyro.x, raw_sensor_data->gyro.y, raw_sensor_data->gyro.z); */
    /* log_info("Qmi8658 temp:%d.%d\n", (u16)(raw_sensor_data->temp_data), (u16)(((u16)((raw_sensor_data->temp_data)* 100)) % 100)); */
}


void Qmi8658_read_ae(float quat[4], float velocity[3])
{
    unsigned char	buf_reg[14];
    short 			raw_q_xyz[4];
    short 			raw_v_xyz[3];

    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Q1_L, buf_reg, 14);
    raw_q_xyz[0] = (short)((unsigned short)(buf_reg[1] << 8) | (buf_reg[0]));
    raw_q_xyz[1] = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
    raw_q_xyz[2] = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));
    raw_q_xyz[3] = (short)((unsigned short)(buf_reg[7] << 8) | (buf_reg[6]));

    raw_v_xyz[1] = (short)((unsigned short)(buf_reg[9] << 8) | (buf_reg[8]));
    raw_v_xyz[2] = (short)((unsigned short)(buf_reg[11] << 8) | (buf_reg[10]));
    raw_v_xyz[2] = (short)((unsigned short)(buf_reg[13] << 8) | (buf_reg[12]));

    quat[0] = (float)(raw_q_xyz[0] * 1.0f) / ae_q_lsb_div;
    quat[1] = (float)(raw_q_xyz[1] * 1.0f) / ae_q_lsb_div;
    quat[2] = (float)(raw_q_xyz[2] * 1.0f) / ae_q_lsb_div;
    quat[3] = (float)(raw_q_xyz[3] * 1.0f) / ae_q_lsb_div;

    velocity[0] = (float)(raw_v_xyz[0] * 1.0f) / ae_v_lsb_div;
    velocity[1] = (float)(raw_v_xyz[1] * 1.0f) / ae_v_lsb_div;
    velocity[2] = (float)(raw_v_xyz[2] * 1.0f) / ae_v_lsb_div;
}

void Qmi8658_enableWakeOnMotion(enum Qmi8658_Interrupt int_set, enum Qmi8658_WakeOnMotionThreshold threshold, unsigned char blankingTime)
{
    unsigned char cal1_1_reg = (unsigned char)threshold;
    unsigned char cal1_2_reg  = (unsigned char)int_set | (blankingTime & 0x3F);
    unsigned char status1 = 0;
    int count = 0;

    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, QMI8658_CTRL7_DISABLE_ALL);
    Qmi8658_config_acc(Qmi8658AccRange_8g, Qmi8658AccOdr_LowPower_21Hz, Qmi8658Lpf_Disable, Qmi8658St_Disable);

    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_L, cal1_1_reg);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_H, cal1_2_reg);
    // ctrl9 wom setting
    Qmi8658_send_ctl9cmd(Qmi8658_Ctrl9_Cmd_WoM_Setting);
    // ctrl9 wom setting
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, QMI8658_CTRL7_ACC_ENABLE);
}

void Qmi8658_disableWakeOnMotion(void)
{
    unsigned char status1 = 0;
    int count = 0;

    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, QMI8658_CTRL7_DISABLE_ALL);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_L, 0);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_H, 0);

    Qmi8658_send_ctl9cmd(Qmi8658_Ctrl9_Cmd_WoM_Setting);
}

void Qmi8658_enableSensors(unsigned char enableFlags)
{

    if (enableFlags & QMI8658_CONFIG_AE_ENABLE) {
        enableFlags |= QMI8658_CTRL7_ACC_ENABLE | QMI8658_CTRL7_GYR_ENABLE;
    }
#if (QMI8658_SYNC_SAMPLE_MODE)
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, enableFlags | 0x80);
#else
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, enableFlags & QMI8658_CTRL7_ENABLE_MASK);
#endif
}


void Qmi8658_enable_acc_Sensors(unsigned char enableFlags)
{
    unsigned char ctrl7 = 0;
    if (enableFlags) {
        // Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Timestamp_L, &ctrl7, 1);//
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, &ctrl7, 1);
        ctrl7 |= QMI8658_CTRL7_ACC_ENABLE;
        Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, ctrl7 & QMI8658_CTRL7_ENABLE_MASK);
    } else { // close acc
        // Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Timestamp_L, &ctrl7, 1);
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, &ctrl7, 1);
        ctrl7 &= (~QMI8658_CTRL7_ACC_ENABLE);
        Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, ctrl7 & QMI8658_CTRL7_ENABLE_MASK);
    }

}

void Qmi8658_enable_gyro_Sensors(unsigned char enableFlags)
{
    unsigned char ctrl7 = 0;
    if (enableFlags) {
        // Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Timestamp_L, &ctrl7, 1);
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, &ctrl7, 1);
        ctrl7 |= QMI8658_CTRL7_GYR_ENABLE;
        Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, ctrl7 & QMI8658_CTRL7_ENABLE_MASK);
    } else { // close gyro
        // Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Timestamp_L, &ctrl7, 1);
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, &ctrl7, 1);
        ctrl7 &= (~QMI8658_CTRL7_GYR_ENABLE);
        Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, ctrl7 & QMI8658_CTRL7_ENABLE_MASK);
    }

}

/*************************************************
*************************************************
		模式切换具体时间查数据手册(差异较大)
*************************************************
*************************************************/
//15ms 后完成
void Qmi8658_reset(void)//
{
    log_info("qmi8658_soft_reset \n");
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Reset, 0xb0);//
    Qmi8658_delay(20);	// delay
}

//如果是spi3wire模式，执行powerdown后, 必须配置spi为3wire(Qmi8658Register_Ctrl1[bit7=1])
/* void Qmi8658_power_down(void)//??? */
/* { */
/* 	Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl1, 0);//clk disable??? */
/* 	Qmi8658_enableSensors(0); */
/* } */
//进入: 只能Power-On Default 进入该模式
//退出: 先soft reset再init
u8 Qmi8658_power_down(void)//
{
    u8 temp_data = 0;
    /* Qmi8658_reset(); */
    log_info("qmi8658_power_down\n");
//sensorDisable
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl1, &temp_data, 1);
    temp_data |= BIT(0); //sensorDisable
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl1, temp_data);//
//close acc,gyro,,
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, &temp_data, 1);
    temp_data &= ~0x0f;
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, temp_data);//

    Qmi8658_delay(10);	// delay
    return 1;
}

//进入: 只能Power-On Default, Low Power Accel Only进入该模式
//退出: 先soft reset再init
void Qmi8658_low_power(void)//
{
    u8 temp_data = 0;
    /* Qmi8658_reset(); */
    log_info("qmi8658_low_power\n");
//sensorenable
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl1, &temp_data, 1);
    temp_data &= ~BIT(0); //sensor enable
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl1, temp_data);//
//close acc,gyro,,
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, &temp_data, 1);
    temp_data &= ~0x0f;
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, temp_data);//
//aODR = 11xx
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl2, &temp_data, 1);
    temp_data |= 0x0f;
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl2, temp_data);//

    Qmi8658_delay(10);	// delay
}


/*
 * spi set:
 * ctrl1: bit7: 3_wire_en
 * 		  bit6: iic/spi_addr_auto_inc
 * 		  bit5: spi_read_big_endian_en
 * 		  4:1 : reserved
 * 		  bit0: disable  internal 2 MHz oscillator
 */
void Qmi8658_spi_mode_set(u8 spi_3_wire_en)//仅上电初始化调用
{
    unsigned char ctrl1 = 0;
    if (spi_3_wire_en) {
        ctrl1 = 0xa0;   //默认开大端模式
    } else {
        ctrl1 = 0x20;
    }
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl1, ctrl1);
}
void Qmi8658_spi_mode_switch(u8 spi_3_wire_en)
{
    unsigned char ctrl1 = 0;
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl1, &ctrl1, 1);
    if (spi_3_wire_en) {
        ctrl1 |= 0xa0;   //默认开大端模式
    } else {
        ctrl1 &= ~0x80;
        ctrl1 |= 0x20;
    }
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl1, ctrl1);
}





void Qmi8658_Config_apply(struct Qmi8658Config const *config)
{
    unsigned char fisSensors = config->inputSelection;

    Qmi8658_enableSensors(QMI8658_CTRL7_DISABLE_ALL);
    if (fisSensors & QMI8658_CONFIG_AE_ENABLE) {
        Qmi8658_config_ae(config->aeOdr);
    } else {
        if (config->inputSelection & QMI8658_CONFIG_ACC_ENABLE) {
            Qmi8658_config_acc(config->accRange, config->accOdr, Qmi8658Lpf_Disable, Qmi8658St_Disable);
        }
        if (config->inputSelection & QMI8658_CONFIG_GYR_ENABLE) {
            Qmi8658_config_gyro(config->gyrRange, config->gyrOdr, Qmi8658Lpf_Enable, Qmi8658St_Disable);
        }
    }

    if (config->inputSelection & QMI8658_CONFIG_MAG_ENABLE) {
        Qmi8658_config_mag(config->magDev, config->magOdr);
    }

    Qmi8658_enableSensors(fisSensors);
}


#if (QMI8658_USE_STEP_EN)
unsigned char stepConfig(unsigned short odr)  //11hz=0x0 ???
{
    unsigned short ped_fix_peak, ped_sample_cnt, ped_time_up, ped_fix_peak2peak;
    unsigned char ped_time_low, ped_time_cnt_entry, ped_fix_precision, ped_sig_count, basedODR;
    //void* pi2c = qmi8658_i2c_init();

    float finalRate;
    //finalRate = 100.0/odr;
    finalRate = 100.0 / odr; //14.285
    ped_sample_cnt = (unsigned short)(0x0032 / finalRate) ;//6;//(unsigned short)(0x0032 / finalRate) ;
    ped_fix_peak2peak = 0x00CC;
    ped_fix_peak = 0x00CC;
    ped_time_up = (unsigned short)(200.0 / finalRate);
    ped_time_low = (unsigned char)(20 / finalRate) ;
    ped_time_cnt_entry = 1;
    ped_fix_precision = 0;
    ped_sig_count = 1;//?????


    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_L, ped_sample_cnt & 0xFF);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_H, (ped_sample_cnt >> 8) & 0xFF);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_L, ped_fix_peak2peak & 0xFF);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_H, (ped_fix_peak2peak >> 8) & 0xFF);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_L, ped_fix_peak & 0xFF);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_H, (ped_fix_peak >> 8) & 0xFF);
    //Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal4_L, 0x01);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal4_H, 0x01);
    log_info("+");


    Qmi8658_send_ctl9cmd(Qmi8658_Ctrl9_Cmd_EnablePedometer);
    log_info(".");
    ///////////////////////////////
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_L, ped_time_up & 0xFF);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_H, (ped_time_up >> 8) & 0xFF);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_L, ped_time_low);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_H, ped_time_cnt_entry);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_L, ped_fix_precision);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_H, ped_sig_count);
    //Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal4_L, 0x02 );
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal4_H, 0x02);
    log_info("-");
    Qmi8658_send_ctl9cmd(Qmi8658_Ctrl9_Cmd_EnablePedometer);
    log_info("*");
//	qmi8658_i2c_deinit(pi2c);


}


uint32_t  Qmi8658_step_read_stepcounter(uint32_t *step)
{
    uint8_t reg_data[3] = {0};
    uint8_t	reg_47 = 0;
    uint32_t rslt;

#if 1
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_AccEl_X, reg_data, 3);
#else
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_AccEl_X, reg_data, 1);
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_AccEl_Y, &reg_data[1], 1);
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_AccEl_Z, &reg_data[2], 1);
#endif

    rslt = reg_data[2] << 16 | reg_data[1] << 8 | reg_data[0];
    *step = 	rslt;

    return rslt;
}

void step_disable(void)
{
    unsigned char ctrl8_data = 0x00;

    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl8, &ctrl8_data, 1);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl8, ctrl8_data & (~0x10)); // 0xD0



    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_L, 0x00);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_H, 0x00);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_L, 0x00);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_H, 0x00);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_L, 0x00);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_H, 0x00);
    //Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal4_L, 0x01);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal4_H, 0x01);
    log_info("+");


    Qmi8658_send_ctl9cmd(Qmi8658_Ctrl9_Cmd_Reset_Pedometer);
    log_info(".");
    ///////////////////////////////
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_L, 0x00);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_H, 0x00);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_L, 0x00);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_H, 0x00);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_L, 0x00);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_H, 0x00);
    //Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal4_L, 0x02 );
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal4_H, 0x02);
    log_info("-");
    Qmi8658_send_ctl9cmd(Qmi8658_Ctrl9_Cmd_Reset_Pedometer);
    log_info("*");


}


#endif


/*****************************
***********  any motion  config ctrl9
First CTRL9 Command:
write to CAL1_L    the value of AnyMotionXThr.
write to CAL1_H   the value of AnyMotionYThr.
write to CAL2_L    the value of AnyMotionZThr.
write to CAL2_H   the value of NoMotionXThr.
write to CAL3_L    the value of  NoMotionYThr.
write to CAL3_H   the value of   NoMotionZThr.
write to CAL4_L   the value of MOTION_MODE_CTRL
write to CAL4_H   the value of   0x01
write to CTRL9     the value of   0x0E   // CTRL_CMD_CONFIGURE_MOTION = 0x0E

Second CTRL9 Command:
write to CAL1_L    the value of AnyMotionWindow.
write to CAL1_H   the value of NoMotionWindow
write to CAL2_L    the value  of SigMotionWaitWindow[7:0]
write to CAL2_H   the value of SigMotionWaitWindow [15:8]
write to CAL3_L    the value of SigMotionConfirmWindow[7:0]
write to CAL3_H   the value of SigMotionConfirmWindow[15:8]
write to CAL4_H   the value of   0x02
write to CTRL9      the value of   0x0E   // CTRL_CMD_CONFIGURE_MOTION    0x0E

************************************/
#if (QMI8658_USE_ANYMOTION_EN)
/*******/
/*motion_g_th  is bit5~bit7 ,  1 bit is   1G .  such as  :  001 is  1G, 111 is  7G .  */
/*motion_mg_th  is  bit0 ~bit4,   1bit is   1/32G = 32.25mg;    00001 is  32.25mg ,   00002 is  32.25*2mg .  motion_mg_th   set range is  0~ 31     */
/*****/
unsigned char anymotion_Config(enum Qmi8658_anymotion_G_TH motion_g_th, unsigned char  motion_mg_th)   //11hz=0x0 ?????
{

    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_L, (motion_g_th + motion_mg_th)); //0x03   // any motion X threshold U 3.5 first three bit(uint 1g)  last five bit (uint 1/32 g)
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_H, (motion_g_th + motion_mg_th)); //0x03  //the value of AnyMotionYThr.
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_L, (motion_g_th + motion_mg_th)); //0x03  //the value of AnyMotionZThr.
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_H, 0x03);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_L, 0x03);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_H, 0x03);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal4_L, 0x07);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal4_H, 0x01);
    log_info("+");


    Qmi8658_send_ctl9cmd(Qmi8658_Ctrl9_Cmd_Motion);
    log_info(".");
    ///////////////////////////////
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_L, 0x03); //0x05
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_H, 0x1a);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_L, 0x2c);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_H, 0x01);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_L, 0x64);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_H, 0x00);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal4_H, 0x02);
    log_info("-");
    Qmi8658_send_ctl9cmd(Qmi8658_Ctrl9_Cmd_Motion);
    log_info("*");
    return 1;
}


void anymotion_lowpwr_config(void)
{
    qmi8658_config.inputSelection = QMI8658_CONFIG_ACC_ENABLE;//QMI8658_CONFIG_ACCGYR_ENABLE;
    qmi8658_config.accRange = Qmi8658AccRange_8g;
    qmi8658_config.accOdr = Qmi8658AccOdr_LowPower_11Hz;//Qmi8658AccOdr_LowPower_3Hz;//Qmi8658AccOdr_LowPower_11Hz;
    qmi8658_config.gyrRange = Qmi8658GyrRange_1024dps;		//Qmi8658GyrRange_2048dps   Qmi8658GyrRange_1024dps
    qmi8658_config.gyrOdr = Qmi8658GyrOdr_250Hz;

    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl8, 0xc2);  // 0xC2
    anymotion_Config(Qmi8658_1G, 3) ;  //  th = 1.1g;

    //Qmi8658_Config_apply(&qmi8658_config);
}


void anymotion_high_odr_enable(void)
{
    unsigned char ctrl8_data = 0x00;

    /*
    	qmi8658_config.inputSelection = QMI8658_CONFIG_ACC_ENABLE;//QMI8658_CONFIG_ACCGYR_ENABLE;
    	qmi8658_config.accRange = Qmi8658AccRange_8g;
    	qmi8658_config.accOdr = Qmi8658AccOdr_125Hz;
    	qmi8658_config.gyrRange = Qmi8658GyrRange_1024dps;		//Qmi8658GyrRange_2048dps   Qmi8658GyrRange_1024dps
    	qmi8658_config.gyrOdr = Qmi8658GyrOdr_125Hz;
    //	qmi8658_config.magOdr = Qmi8658MagOdr_125Hz;
    //	qmi8658_config.magDev = MagDev_AKM09918;
    //	qmi8658_config.aeOdr = Qmi8658AeOdr_128Hz;
    */


    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl8, &ctrl8_data, 1);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl8, ctrl8_data | 0xC2); // 0xC2
    anymotion_Config(Qmi8658_1G, 3) ; ////  th = 1.1g;

    //Qmi8658_Config_apply(&qmi8658_config);
}


void anymotion_disable(void)
{
    unsigned char ctrl8_data = 0x00;

    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl8, &ctrl8_data, 1);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl8, ctrl8_data & (~0x02)); // 0xC2

    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_L, 0x23); //0x03   // any motion X threshold U 3.5 first three bit(uint 1g)  last five bit (uint 1/32 g)
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_H, 0x23); //0x03  //the value of AnyMotionYThr.
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_L, 0x23); //0x03  //the value of AnyMotionZThr.
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_H, 0x03);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_L, 0x03);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_H, 0x03);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal4_L, 0x00);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal4_H, 0x01);
    log_info("+");


    Qmi8658_send_ctl9cmd(Qmi8658_Ctrl9_Cmd_Motion);
    log_info(".");
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_L, 0x03); //0x05
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_H, 0x1a);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_L, 0x2c);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_H, 0x01);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_L, 0x64);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_H, 0x00);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal4_H, 0x02);
    log_info("-");
    Qmi8658_send_ctl9cmd(Qmi8658_Ctrl9_Cmd_Motion);
    log_info("*");


}

#endif
/***********************************
First CTRL9 Command:
write to CAL1_L    the value of peakWindow.     //30表示，  30 ms at 1000 Hz  .就是ODR的个数。
write to CAL1_H   the value of priority.
write to CAL2_L    the value of TapWindow low byte.       //100表示，  100 ms at 1000 Hz  就是ODR的个数。
write to CAL2_H   the value of  TapWindow high byte.
write to CAL3_L    the value of  DTapWindow low byte.
write to CAL3_H   the value of   DTapWindow high byte.
write to CAL4_H   the value of   0x01
write to CTRL9     the value of   0x0C       // CTRL_CMD_CONFIGURE_TAP = 0x0C

Second CTRL9 Command:
write to CAL1_L    the value of alpha.      //更新下个数据的百分比
write to CAL1_H   the value of gamma.   //差值数据更新的百分比
write to CAL2_L    the value  of peakMagThr low byte.   xyz 模值的阀值。 敲的力度阀值
write to CAL2_H   the value of peakMagThr high byte.
write to CAL3_L    the value of UDMThr low byte.    //停止敲击的，阀值，gamma与其有关。值设置越大，越容易满足条件。
write to CAL3_H   the value of UDMThr high byte.
write to CAL4_H   the value of   0x02
write to CTRL9      the value of   0x0C   // CTRL_CMD_CONFIGURE_TAP    0x0C


*************************************************/

#if (QMI8658_USE_TAP_EN)
unsigned char tap_Config(void)  //11hz=0x0e ????
{
#if  1  //alpha  tap  param
    uint8_t  peakWindow = 0x0e;  //0x1e  30ms at 1000hz ODR.
    uint8_t  priority   = 0x04;
    uint16_t  TapWindow = 0x000c;   //0x0016 //0x0064  100MS  at 100hz odr
    uint16_t  DTapWindow = 0x0058; //
    uint8_t  alpha = 0x40; //0x40; //0x08;  //0.065  //alpha  百分比 =alpha[7] + alpha[6~0]* (1/128)
    uint8_t  gamma =  0x20; //0x10; //0x20;//0x20;  //0.25  //gamma  百分比 =gamma[7] + gamma[6~0]* (1/128)
    uint16_t  peakMagThr = 0x0069;//0x0200;//0x0599//0x0599;    // 1.4g
    uint16_t  UDMThr = 0x0049; //0x0100; //0x0199;     // 0.4g
#endif //


    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_L, peakWindow); //0x1E
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_H, priority); //0x03
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_L, TapWindow & 0xff); //0x64
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_H, (TapWindow >> 8) & 0xff);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_L, DTapWindow & 0xff);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_H, (DTapWindow >> 8) & 0xff);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal4_H, 0x01);
    log_info("+");


    Qmi8658_send_ctl9cmd(Qmi8658_Ctrl9_Cmd_EnableTap);
    log_info(".");
    ///////////////////////////////
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_L, alpha);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_H, gamma);  //0x20
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_L, peakMagThr & 0xff);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_H, (peakMagThr >> 8) & 0xff); //0x05
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_L, UDMThr & 0xff);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_H, (UDMThr >> 8) & 0xff); //0x01
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal4_H, 0x02);
    log_info("-");
    Qmi8658_send_ctl9cmd(Qmi8658_Ctrl9_Cmd_EnableTap);
    log_info("*");

}

void tap_enable(void)
{
    unsigned char ctrl8_data = 0x00;

    /*
    	qmi8658_config.inputSelection = QMI8658_CONFIG_ACCGYR_ENABLE;//QMI8658_CONFIG_ACCGYR_ENABLE;
    	qmi8658_config.accRange = Qmi8658AccRange_2g;
    	qmi8658_config.accOdr = Qmi8658AccOdr_125Hz;
    	qmi8658_config.gyrRange = Qmi8658GyrRange_1024dps;		//Qmi8658GyrRange_2048dps   Qmi8658GyrRange_1024dps
    	qmi8658_config.gyrOdr = Qmi8658GyrOdr_125Hz;
    	//qmi8658_config.magOdr = Qmi8658MagOdr_125Hz;
    	//qmi8658_config.magDev = MagDev_AKM09918;
    	//qmi8658_config.aeOdr = Qmi8658AeOdr_128Hz;
    	*/


    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl8, &ctrl8_data, 1);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl8, ctrl8_data | 0xc1); // 0xC2
    tap_Config() ;

    //Qmi8658_Config_apply(&qmi8658_config);
}
#endif
#if (QMI8658_NEW_HANDSHAKE)
void qmi8658_on_demand_cali(void)
{
    unsigned char cod_status = 0x00;
    unsigned char cod_data[6];

    log_info("qmi8658_on_demand_cali start");
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl9, (unsigned char)Qmi8658_Ctrl9_Cmd_On_Demand_Cali);
    Qmi8658_delay(2200);	// delay 2000ms above
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl9, 0x00);//qmi8658_Ctrl9_Cmd_Ack
    Qmi8658_delay(10);		// delay
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, 0x70/* Qmi8658Register_Cod_Status */, &cod_status, 1);
    if (cod_status) {
        log_info("qmi8658_on_demand_cali fail! status=0x%x", cod_status);
    } else {
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Dvx_L, &cod_data[0], 6);//qmi8658_get_gyro_gain
        log_info("qmi8658_on_demand_cali done! cod[%d %d %d]",
                 (unsigned short)(cod_data[1] << 8 | cod_data[0]),
                 (unsigned short)(cod_data[3] << 8 | cod_data[2]),
                 (unsigned short)(cod_data[5] << 8 | cod_data[4]));
    }
}
void qmi8658_apply_gyr_gain(unsigned char cod_data[6])
{
    u8 cod_data_temp[8];

    /* Qmi8658_enableSensors(QMI8658_CTRL7_DISABLE_ALL); */
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_L, cod_data[0]);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal1_H, cod_data[1]);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_L, cod_data[2]);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal2_H, cod_data[3]);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_L, cod_data[4]);
    Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Cal3_H, cod_data[5]);

    Qmi8658_send_ctl9cmd(0xaa);//(qmi8658_Ctrl9_Cmd_Apply_Gyro_Gain);
    /* Qmi8658_enableSensors(QMI8658_CONFIG_ACCGYR_ENABLE); */
}
#endif
unsigned char qmi8658_init(u8 demand_cali_en)
{
    unsigned char qmi8658_chip_id = 0x00;
    unsigned char qmi8658_revision_id = 0x00;
    unsigned char iCount = 0;

    unsigned char fw_version[3] = {0};
    unsigned char usid_version[6] = {0};
#if (QMI8658_USER_INTERFACE==QMI8658_USE_SPI)
    if (qmi8658_info->spi_work_mode) {
        Qmi8658_spi_mode_set(1);//使能spi_3_wire
    } else {
        Qmi8658_spi_mode_set(0);//使能spi_4_wire
    }
#endif
    while ((qmi8658_chip_id == 0x00) && (iCount < 10)) {
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_WhoAmI, &qmi8658_chip_id, 1);
        if (qmi8658_chip_id == 0x05) {
            break;
        }
        iCount++;
        mdelay(1);
    }

    /* Qmi8658_read(QMI8658_SLAVE_ADDRESS, 0x49, fw_version, 3); */
    /* log_info("Qmi8658_fw_version(Quaternion Increment dQW?)=0x%x, 0x%x, 0x%x  \n", fw_version[0],fw_version[1],fw_version[2]); */
    Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Revision, &qmi8658_revision_id, 1);

    if (qmi8658_chip_id == 0x05) {
        log_info("Qmi8658_init slave=0x%x  Qmi8658Register_WhoAmI=0x%x 0x%x\n", QMI8658_SLAVE_ADDRESS, qmi8658_chip_id, qmi8658_revision_id);

        /* Qmi8658_read(QMI8658_SLAVE_ADDRESS, 0x51, usid_version, 6); */
        /* log_info("Qmi8658_usid_version(Velocity Increment along XYZ?)=0x%x, 0x%x, 0x%x ,0x%x, 0x%x, 0x%x \n", usid_version[0],usid_version[1],usid_version[2] ,usid_version[3],usid_version[4],usid_version[5]); */

#if (QMI8658_NEW_HANDSHAKE==0)
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl1, &iCount, 1);
        iCount |= 0x60;
        iCount &= ~ 0x01;
        /* iCount |= 0x04; */
        Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl1, iCount);//bit7:1:spi_3_wire, bit6:addr_auto_inc, bit5:spi_read_big_endian(大端),bit0:0:clkosc
#else
        /* Qmi8658_reset(); */
        Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl1, 0x60 | 0x08 | 0x10); // 另一种qmi8658
        if (demand_cali_en) {
            qmi8658_on_demand_cali();
            /* printf("set cod"); */
        } else {
            /* qmi8658_apply_gyr_gain(cod_data_copy); */
            /* printf("restore cod"); */
        }
        Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, 0x00);
        Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl8, 0xc0);
#endif
        qmi8658_config.inputSelection = QMI8658_CONFIG_ACCGYR_ENABLE;//QMI8658_CONFIG_ACCGYR_ENABLE;
        qmi8658_config.accRange = Qmi8658AccRange_16g;
        qmi8658_config.accOdr = Qmi8658AccOdr_125Hz;
        qmi8658_config.gyrRange = Qmi8658GyrRange_2048dps;		//Qmi8658GyrRange_2048dps   Qmi8658GyrRange_1024dps
        qmi8658_config.gyrOdr = Qmi8658GyrOdr_125Hz;

#if (QMI8658_USE_TAP_EN)
        tap_enable();
#endif

#if (QMI8658_USE_ANYMOTION_EN)
        //anymotion_high_odr_enable();
        anymotion_lowpwr_config();
#endif

#if (QMI8658_USE_FIFO_EN)
        Qmi8658_config_fifo(128, Qmi8658_Fifo_128, Qmi8658_Fifo_Stream, QMI8658_FORMAT_12_BYTES);
#endif

#if (QMI8658_USE_STEP_EN)
        qmi8658_config.inputSelection = QMI8658_CONFIG_ACC_ENABLE;
        qmi8658_config.accRange = Qmi8658AccRange_8g;
        qmi8658_config.accOdr = Qmi8658AccOdr_LowPower_21Hz;//Qmi8658AccOdr_62_5Hz;//Qmi8658AccOdr_LowPower_21Hz;
        qmi8658_config.gyrRange = Qmi8658GyrRange_1024dps;		//Qmi8658GyrRange_2048dps   Qmi8658GyrRange_1024dps
        qmi8658_config.gyrOdr = Qmi8658GyrOdr_500Hz;

        Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, 0x00);		//
        Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl8, 0xD0);
        stepConfig(21);  //ODR =  21hz
        //stepConfig(62);   //ODR = 62.5HZ
#endif

        Qmi8658_Config_apply(&qmi8658_config);
#if 0
        unsigned char read_data = 0x00;
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl1, &read_data, 1);
        log_info("Qmi8658Register_Ctrl1=0x%x \n", read_data);
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl2, &read_data, 1);
        log_info("Qmi8658Register_Ctrl2=0x%x \n", read_data);
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl3, &read_data, 1);
        log_info("Qmi8658Register_Ctrl3=0x%x \n", read_data);
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl4, &read_data, 1);
        log_info("Qmi8658Register_Ctrl4=0x%x \n", read_data);
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl5, &read_data, 1);
        log_info("Qmi8658Register_Ctrl5=0x%x \n", read_data);
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl6, &read_data, 1);
        log_info("Qmi8658Register_Ctrl6=0x%x \n", read_data);
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, &read_data, 1);
        log_info("Qmi8658Register_Ctrl7=0x%x \n", read_data);
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, 77, &qmi8658_revision_id, 1);
        log_info("teset:0x%x", qmi8658_revision_id);
#endif
//		Qmi8658_set_layout(2);
        return 1;
    } else {
        log_error("Qmi8658_init fail\n");
        qmi8658_chip_id = 0;
        return 0;
    }
}

static u8 qmi8658_int_pin1 = IO_PORTB_04;
static u8 qmi8658_int_pin2 = IO_PORTB_03;
//return:0:fail, 1:ok
u8 qmi8658_sensor_init(void *priv)
{
    if (priv == NULL) {
        log_error("qmi8658 init fail(no param)\n");
        return 0;
    }
    qmi8658_info = (qmi8658_param *)priv;

#if (QMI8658_USER_INTERFACE==QMI8658_USE_I2C) //iic interface
    iic_init(qmi8658_info->iic_hdl);
#elif (QMI8658_USER_INTERFACE==QMI8658_USE_SPI)//spi interface
    spi_cs_init();
    spi_init();
#else
    //I3C
#endif
    gpio_set_die(qmi8658_int_pin1, 1);
    gpio_set_direction(qmi8658_int_pin1, 1);
    // gpio_set_pull_up(qmi8658_int_pin1,1);
    gpio_set_die(qmi8658_int_pin2, 1);
    gpio_set_direction(qmi8658_int_pin2, 1);
    // gpio_set_pull_up(qmi8658_int_pin2,1);
    return qmi8658_init(1);
}







static volatile u32 qmi8658_init_flag ;
static u8 imu_busy = 0;
static qmi8658_param qmi8658_info_data;
/* #define SENSORS_MPU_BUFF_LEN 14 */
/* u8 read_qmi8658_buf[SENSORS_MPU_BUFF_LEN]; */
#if (QMI8658_USE_FIFO_EN)
u8 fifo_data[1024];//size:Qmi8658_FifoSize * 6 * Number_of_enabled_sensors
#endif
void qmi8658_int_callback()
{
    if (qmi8658_init_flag == 0) {
        log_error("qmi8658 init fail!");
        return ;
    }
    if (imu_busy) {
        log_error("qmi8658 busy!");
        return ;
    }
    imu_busy = 1;

    imu_sensor_data_t raw_sensor_datas;
    float TempData = 0.0;
    u8 status_temp = 0;
#if (QMI8658_USE_FIFO_EN)
    u16 fifo_level = 0;
    fifo_level = Qmi8658_read_fifo(fifo_data);
    TempData = Qmi8658_readTemp();
    /* log_info("qmi8658 raw:acc_x:%d acc_y:%d acc_z:%d gyro_x:%d gyro_y:%d gyro_z:%d, tim_count:%d", (s16)(fifo_data[1]<<8|fifo_data[0]), (s16)(fifo_data[3]<<8|fifo_data[2]), (s16)(fifo_data[5]<<8|fifo_data[4]), (s16)(fifo_data[7]<<8|fifo_data[6]), (s16)(fifo_data[9]<<8|fifo_data[8]), (s16)(fifo_data[11]<<8|fifo_data[10]),fifo_level);//共fifo_level组数据, 只打印第一组数据 */
#else
    status_temp = Qmi8658_readStatus0();
    /* log_info("status:0x%x", status_temp); */
    if (status_temp & 0x03) {
        Qmi8658_read_raw_acc_gyro_xyz(&raw_sensor_datas);//获取原始值
        /* TempData=Qmi8658_readTemp(); */
        TempData = raw_sensor_datas.temp_data;
        /* log_info("qmi8658 raw:acc_x:%d acc_y:%d acc_z:%d gyro_x:%d gyro_y:%d gyro_z:%d", raw_sensor_datas.acc.x, raw_sensor_datas.acc.y, raw_sensor_datas.acc.z, raw_sensor_datas.gyro.x, raw_sensor_datas.gyro.y, raw_sensor_datas.gyro.z); */
    }
#endif
    /* log_info("qmi8658 temp:%d.%d\n", (u16)TempData, (u16)(((u16)(TempData * 100)) % 100)); */
    imu_busy = 0;
}

s8 qmi8658_dev_init(void *arg)
{
    if (arg == NULL) {
        log_error("qmi8658 init fail(no arg)\n");
        return -1;
    }
#if (QMI8658_USER_INTERFACE==QMI8658_USE_I2C)
    qmi8658_info_data.iic_hdl = ((struct imusensor_platform_data *)arg)->peripheral_hdl;
    qmi8658_info_data.iic_delay = ((struct imusensor_platform_data *)arg)->peripheral_param0;   //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
    // u8 iic_clk;      //iic_clk: <=400kHz
#elif (QMI8658_USER_INTERFACE==QMI8658_USE_SPI)
    qmi8658_info_data.spi_hdl = ((struct imusensor_platform_data *)arg)->peripheral_hdl,     //SPIx (role:master)
                      qmi8658_info_data.spi_cs_pin = ((struct imusensor_platform_data *)arg)->peripheral_param0; //IO_PORTA_05
    qmi8658_info_data.spi_work_mode = ((struct imusensor_platform_data *)arg)->peripheral_param1; //1:3wire(SPI_MODE_UNIDIR_1BIT) or 0:4wire(SPI_MODE_BIDIR_1BIT) (与spi结构体一样)
    // u8 port;         //SPIx group:A,B,C,D (spi结构体)
    // U8 spi_clk;      //spi_clk: <=15MHz (spi结构体)
#else
    //I3C
#endif
    qmi8658_int_pin2 = ((struct imusensor_platform_data *)arg)->imu_sensor_int_io;
    qmi8658_int_pin1 = qmi8658_int_pin2;
    if (imu_busy) {
        log_error("qmi8658 busy!");
        return -1;
    }
    imu_busy = 1;
    if (qmi8658_sensor_init(&qmi8658_info_data)) {
#if (QMI8658_USE_INT_EN) //中断模式,暂不支持
        log_info("int mode en!");
        /* port_wkup_enable(qmi8658_int_pin2, 1, qmi8658_int_callback); //PA08-IO中断，1:下降沿触发，回调函数qmi8658_int_callback*/
#ifdef CONFIG_CPU_BR23
        io_ext_interrupt_init(qmi8658_int_pin2, 1, qmi8658_int_callback);
#elif defined(CONFIG_CPU_BR28)
        // br28外部中断回调函数，按照现在的外部中断注册方式
        // io配置在板级，定义在板级头文件，这里只是注册回调函数
        /* port_edge_wkup_set_callback_by_index(3, qmi8658_int_callback); // 序号需要和板级配置中的wk_param对应上 */
        port_edge_wkup_set_callback(qmi8658_int_callback);
#elif defined(CONFIG_CPU_BR27)
        port_edge_wkup_set_callback(qmi8658_int_callback);
#endif
#else //定时

#endif
        qmi8658_init_flag = 1;
        imu_busy = 0;
        log_info("qmi8658 Device init success!%d\n", qmi8658_init_flag);
        return 0;
    } else {
        log_info("qmi8658 Device init fail!\n");
        imu_busy = 0;
        return -1;
    }
}


int qmi8658_dev_ctl(u8 cmd, void *arg);
REGISTER_IMU_SENSOR(qmi8658_sensor) = {
    .logo = "qmi8658",
    .imu_sensor_init = qmi8658_dev_init,
    .imu_sensor_check = NULL,
    .imu_sensor_ctl = qmi8658_dev_ctl,
};

int qmi8658_dev_ctl(u8 cmd, void *arg)
{
    int ret = -1;
    u8 status_temp = 0;
    if (qmi8658_init_flag == 0) {
        log_error("qmi8658 init fail!");
        return ret;//0:ok,,<0:err
    }
    if (imu_busy) {
        log_error("qmi8658 busy!");
        return ret;//0:ok,,<0:err
    }
    imu_busy = 1;
    switch (cmd) {
    case IMU_GET_SENSOR_NAME:
        memcpy((u8 *)arg, &(qmi8658_sensor.logo), 20);
        ret = 0;
        break;
    case IMU_SENSOR_ENABLE: //enable demand cali
        /* cbuf_init(&hrsensor_cbuf, hrsensorcbuf, 24 * sizeof(int)); */
        qmi8658_init(1);
        ret = 0;
        break;
    case IMU_SENSOR_DISABLE:
        /* cbuf_clear(&hrsensor_cbuf); */
        //close power
        Qmi8658_power_down();
        ret = 0;
        break;
    case IMU_SENSOR_RESET://reset后必须enable
        Qmi8658_reset();
        ret = 0;
        break;
    case IMU_SENSOR_SLEEP:
        if (Qmi8658_power_down()) {
            log_info("qmi8658 enter sleep ok!");
            ret = 0;
        } else {
            log_error("qmi8658 enter sleep fail!");
        }
        break;
    case IMU_SENSOR_WAKEUP: //disable demand cali
        if (qmi8658_init(0)) {
            log_info("qmi8658 wakeup ok!");
            ret = 0;
        } else {
            log_error("qmi8658 wakeup fail!");
        }
        break;
    case IMU_SENSOR_INT_DET://传感器中断状态检查
        break;
    case IMU_SENSOR_DATA_READY://传感器数据准备就绪待读
        break;
    case IMU_SENSOR_CHECK_DATA://检查传感器缓存buf是否存满
        break;
    case IMU_SENSOR_READ_DATA://默认读传感器所有数据
        status_temp = Qmi8658_readStatus0();
        /* log_info("status:0x%x", status_temp); */
        if (status_temp & 0x03) {
            Qmi8658_read_raw_acc_gyro_xyz(arg);//获取原始值
            ret = 0;
        }
        break;
    case IMU_GET_ACCEL_DATA://加速度数据
        /* float TempData = 0.0; */
        status_temp = Qmi8658_readStatus0();
        /* log_info("status:0x%x", status_temp); */
        if (status_temp & 0x01) {
            Qmi8658_read_raw_acc_xyz(arg);//获取原始值
            ret = 0;
            /* TempData = Qmi8658_readTemp(); */
        }
        break;
    case IMU_GET_GYRO_DATA://陀螺仪数据
        status_temp = Qmi8658_readStatus0();
        /* log_info("status:0x%x", status_temp);  */
        if (status_temp & 0x02) {
            Qmi8658_read_raw_gyro_xyz(arg);//获取原始值
            ret = 0;
        }
        break;
    case IMU_GET_MAG_DATA://磁力计数据
        log_error("qmi8658 have no mag!\n");
        break;
    case IMU_SENSOR_SEARCH://检查传感器id
        Qmi8658_read(QMI8658_SLAVE_ADDRESS, Qmi8658Register_WhoAmI, (u8 *)arg, 1);
        if (*(u8 *)arg == 0x05) {
            ret = 0;
            log_info("qmi8658 online!\n");
        } else {
            log_error("qmi8658 offline!\n");
        }
        break;
    case IMU_GET_SENSOR_STATUS://获取传感器状态
        status_temp = Qmi8658_readStatus0();
        *(u8 *)arg = status_temp;
        ret = 0;
        break;
    case IMU_SET_SENSOR_FIFO_CONFIG://配置传感器FIFO
#if (QMI8658_USE_FIFO_EN)
        u8 *tmp = (u8 *)arg;
        u8 wm_th = tmp[0];
        u8 fifo_size = tmp[1];//Qmi8658_Fifo_64
        u8 fifo_mode = tmp[2];//Qmi8658_Fifo_Stream
        u8 fifo_format = tmp[3];//QMI8658_FORMAT_12_BYTES
        Qmi8658_config_fifo(wm_th, fifo_size, fifo_mode, fifo_format);
        ret = 0;
#endif
        break;
    case IMU_GET_SENSOR_READ_FIFO://读取传感器FIFO数据
#if (QMI8658_USE_FIFO_EN)
        ret = Qmi8658_read_fifo((u8 *)arg);
#endif
        break;
    case IMU_SET_SENSOR_TEMP_DISABLE://温度传感器无法控制
        u8 temp = *(u8 *)arg;
        ret = 0;
        break;
    default:
        log_error("--cmd err!\n");
        break;
    }
    imu_busy = 0;
    return ret;//0:ok,,<0:err

}



#endif

