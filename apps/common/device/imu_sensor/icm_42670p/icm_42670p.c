#include "inv_imu_transport.h"
#include "app_config.h"
#include "icm_42670p.h"
#include "generic/typedef.h"
#include "gpio.h"
#include "inv_imu_defs.h"
#include "inv_imu_driver.h"
#include "imuSensor_manage.h"
#include "asm/iic_hw.h"
#include "asm/iic_soft.h"
#include "asm/timer.h"
/* #include "inv_imu_extfunc.h" */
/* #include "inv_imu_regmap.h" */

#if TCFG_ICM42670P_ENABLE

/*************Betterlife ic debug***********/
#undef LOG_TAG_CONST
#define LOG_TAG             "[ICM42670P]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

void delay(volatile u32 t);
void udelay(u32 us);
#define   MDELAY(n)    mdelay(n)

static icm42670p_param *icm42670p_info;

#if (ICM42670P_USER_INTERFACE==ICM42670P_USE_I2C)

#if TCFG_ICM42670P_USER_IIC_TYPE
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
//起止信号间隔：:>1.3us
//return: readLen:ok, other:fail
u32 icm42670p_I2C_Read_NBytes(unsigned char devAddr,
                              unsigned char regAddr,
                              unsigned char *readBuf,
                              u32 readLen)
{
    u32 i = 0;
    local_irq_disable();
    iic_start(icm42670p_info->iic_hdl);
    if (0 == iic_tx_byte(icm42670p_info->iic_hdl, devAddr)) {
        log_error("icm42670p iic read err1");
        goto __iic_exit_r;
    }
    delay(icm42670p_info->iic_delay);
    /* if (0 == iic_tx_byte(icm42670p_info->iic_hdl, regAddr |0x80)) {//|0x80地址自动递增 */
    if (0 == iic_tx_byte(icm42670p_info->iic_hdl, regAddr)) {//|0x80地址自动递增
        log_error("icm42670p iic read err2");
        goto __iic_exit_r;
    }
    delay(icm42670p_info->iic_delay);

    iic_start(icm42670p_info->iic_hdl);
    if (0 == iic_tx_byte(icm42670p_info->iic_hdl, devAddr + 1)) {
        log_error("icm42670p iic read err3");
        goto __iic_exit_r;
    }
    for (i = 0; i < readLen; i++) {
        delay(icm42670p_info->iic_delay);
        if (i == (readLen - 1)) {
            *readBuf++ = iic_rx_byte(icm42670p_info->iic_hdl, 0);
        } else {
            *readBuf++ = iic_rx_byte(icm42670p_info->iic_hdl, 1);
        }
        /* if(i%100==0)wdt_clear(); */
    }
__iic_exit_r:
    iic_stop(icm42670p_info->iic_hdl);
    local_irq_enable();

    return i;
}

//起止信号间隔：:>1.3us
//return:writeLen:ok, other:fail
u32 icm42670p_I2C_Write_NBytes(unsigned char devAddr,
                               unsigned char regAddr,
                               unsigned char *writeBuf,
                               u32 writeLen)
{
    u32 i = 0;
    local_irq_disable();
    iic_start(icm42670p_info->iic_hdl);
    if (0 == iic_tx_byte(icm42670p_info->iic_hdl, devAddr)) {
        log_error("icm42670p iic write err1");
        goto __iic_exit_w;
    }
    delay(icm42670p_info->iic_delay);
    if (0 == iic_tx_byte(icm42670p_info->iic_hdl, regAddr)) {
        log_error("icm42670p iic write err2");
        goto __iic_exit_w;
    }

    for (i = 0; i < writeLen; i++) {
        delay(icm42670p_info->iic_delay);
        if (0 == iic_tx_byte(icm42670p_info->iic_hdl, writeBuf[i])) {
            log_error("icm42670p iic write err3:%d", i);
            goto __iic_exit_w;
        }
    }
__iic_exit_w:
    iic_stop(icm42670p_info->iic_hdl);
    local_irq_enable();
    return i;
}
IMU_read    icm42670p_read     = icm42670p_I2C_Read_NBytes;
IMU_write   icm42670p_write    = icm42670p_I2C_Write_NBytes;

#elif (ICM42670P_USER_INTERFACE==ICM42670P_USE_SPI)
// only support 4-wire mode
#define spi_cs_init() \
    do { \
        gpio_write(icm42670p_info->spi_cs_pin, 1); \
        gpio_set_direction(icm42670p_info->spi_cs_pin, 0); \
        gpio_set_die(icm42670p_info->spi_cs_pin, 1); \
    } while (0)
#define spi_cs_uninit() \
    do { \
        gpio_set_die(icm42670p_info->spi_cs_pin, 0); \
        gpio_set_direction(icm42670p_info->spi_cs_pin, 1); \
        gpio_set_pull_up(icm42670p_info->spi_cs_pin, 0); \
        gpio_set_pull_down(icm42670p_info->spi_cs_pin, 0); \
    } while (0)
#define spi_cs_h()                  gpio_write(icm42670p_info->spi_cs_pin, 1)
#define spi_cs_l()                  gpio_write(icm42670p_info->spi_cs_pin, 0)

#define spi_read_byte()             spi_recv_byte(icm42670p_info->spi_hdl, NULL)
#define spi_write_byte(x)           spi_send_byte(icm42670p_info->spi_hdl, x)
#define spi_dma_read(x, y)          spi_dma_recv(icm42670p_info->spi_hdl, x, y)
#define spi_dma_write(x, y)         spi_dma_send(icm42670p_info->spi_hdl, x, y)
#define spi_set_width(x)            spi_set_bit_mode(icm42670p_info->spi_hdl, x)
#define spi_init()              spi_open(icm42670p_info->spi_hdl)
#define spi_closed()            spi_close(icm42670p_info->spi_hdl)
#define spi_suspend()           hw_spi_suspend(icm42670p_info->spi_hdl)
#define spi_resume()            hw_spi_resume(icm42670p_info->spi_hdl)

u32 icm42670p_SPI_readNBytes(unsigned char devAddr,
                             unsigned char regAddr,
                             unsigned char *readBuf,
                             u32 readLen)
{
    spi_cs_l();
    spi_write_byte(regAddr | 0x80);//| 0x80:read mode
    spi_dma_read(readBuf, readLen);
    spi_cs_h();
    //SPIRead((regAddr | 0x80), readBuf, readLen);
    return (readLen);
}
unsigned char icm42670p_SPI_writeByte(unsigned char devAddr,
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
u32 icm42670p_SPI_writeNBytes(unsigned char devAddr,
                              unsigned char regAddr,
                              unsigned char *writeBuf,
                              u32 writeLen)
{
#if 0 //多字节dma写
    spi_cs_l();
    spi_write_byte(regAddr & 0x7F);
    spi_dma_write(writeBuf, writeLen);
    spi_cs_h();
#else
    u32 i = 0;
    spi_cs_l();
    spi_write_byte((regAddr) & 0x7F);
    for (; i < writeLen; i++) {
        spi_write_byte(writeBuf[i]);//能否连续写??????????????????????????????????????????
    }
    spi_cs_h();
    // for(;i<writeLen;i++){
    // 	icm42670p_SPI_writeByte(devAddr, regAddr+i,writeBuf[i]);
    // }
#endif
    //SPIWrite((regAddr & 0x7F), writeBuf, writeLen);
    return (writeLen);
}

IMU_read 	icm42670p_read	= icm42670p_SPI_readNBytes;
IMU_write	icm42670p_write	= icm42670p_SPI_writeNBytes;
#endif

#define ICM_TIMER JL_TIMER2
#define     STD_12M  5
#define     DIV4        0b0001
void icm42670p_timer_init()//仅初始化使用500ms左右,数据稳定后不再使用
{
    ICM_TIMER->CON = BIT(14);
    ICM_TIMER->CNT = 0;
    ICM_TIMER->PWM = 0;
    ICM_TIMER->PRD = 0xffffffff;
    ICM_TIMER->CON = (DIV4 << 4) | (STD_12M << 10) | BIT(0);
}
/* Sleep implementation */
void inv_imu_sleep_us(uint32_t us)
{
    udelay(us);
}
/* Get time implementation */
uint64_t inv_imu_get_time_us(void)
{
    return ICM_TIMER->CNT / 3;//inv_timer_get_counter(TIMEBASE_TIMER);
}

int inv_io_hal_read_reg(struct inv_imu_serif *serif, uint8_t reg, uint8_t *rbuffer, uint32_t rlen)
{
    u32 rc;
    rc = icm42670p_read(ICM42670P_SLAVE_ADDRESS, reg, rbuffer, rlen);
    if (rc == rlen) {
        return 0;
    } else {
        return -1;
    }
}
int inv_io_hal_write_reg(struct inv_imu_serif *serif, uint8_t reg, const uint8_t *wbuffer, uint32_t wlen)
{
    u32 rc;
    icm42670p_write(ICM42670P_SLAVE_ADDRESS, reg, wbuffer, wlen);
    if (rc == wlen) {
        return 0;
    } else {
        return -1;
    }
}







/* For SmartMotion */
static int32_t icm_mounting_matrix[9] = {(1 << 30),  0,       0,
                                         0, (1 << 30),  0,
                                         0,       0, (1 << 30)
                                        };
/*
 * IMU interrupt handler.
 * Function is executed when an IMU interrupt rises on MCU.
 * This function get a timestamp and store it in the timestamp buffer.
 * Note that this function is executed in an interrupt handler and thus no protection
 * are implemented for shared variable timestamp_buffer.
 */
static volatile int irq_from_device = 0;

static void ext_interrupt_cb(void *context, unsigned int int_num)
{
    (void)context;

#if !USE_FIFO
    /*
     * Read timestamp from the timer dedicated to timestamping
     */
    /* uint64_t timestamp = inv_timer_get_counter(TIMEBASE_TIMER); */
    /*  */
    /* if (int_num == INV_GPIO_INT1) { */
    /* 	if (!RINGBUFFER_FULL(&timestamp_buffer)) */
    /* 		RINGBUFFER_PUSH(&timestamp_buffer, &timestamp); */
    /* } */
#endif

    irq_from_device |= BIT(int_num);
}

/*
 * Print raw data or scaled data
 * 0 : print raw accel, gyro and temp data
 * 1 : print scaled accel, gyro and temp data in g, dps and degree Celsius
 */
static u8 icm42670p_int_pin1 = IO_PORTB_03;
static u8 icm42670p_int_pin2 = IO_PORTB_04;
struct inv_imu_serif icm_serif;
/* Just a handy variable to handle the IMU object */
static struct inv_imu_device icm_driver;
void imu_callback(inv_imu_sensor_event_t *event);
int configure_imu_device();
int icm42670p_sensor_init(void *priv)
{
    if (priv == NULL) {
        log_error("icm42670p init fail(no param)\n");
        return 0;
    }
    icm42670p_info = (icm42670p_param *)priv;

//init interrup io, Register the callback function.
    gpio_set_die(icm42670p_int_pin1, 1);
    gpio_set_direction(icm42670p_int_pin1, 1);
    /* gpio_set_pull_up(icm42670p_int_pin1, 1); */
    /* gpio_set_pull_down(icm42670p_int_pin1, 0); */
    gpio_set_die(icm42670p_int_pin2, 1);
    gpio_set_direction(icm42670p_int_pin2, 1);
    /* gpio_set_pull_up(icm42670p_int_pin2, 1); */
    /* gpio_set_pull_down(icm42670p_int_pin2, 0); */

    /* Initialize serial interface between MCU and IMU */
    icm_serif.context   = 0;        /* no need */
    icm_serif.read_reg  = inv_io_hal_read_reg;
    icm_serif.write_reg = inv_io_hal_write_reg;
    icm_serif.max_read  = 1024 * 32; /* maximum number of bytes allowed per serial read */
    icm_serif.max_write = 1024 * 32; /* maximum number of bytes allowed per serial write */
    icm_serif.serif_type = SERIF_TYPE;
//iic or spi init
    switch (icm_serif.serif_type) {
    case UI_SPI4: {
#if (ICM42670P_USER_INTERFACE==ICM42670P_USE_SPI)
        spi_cs_init();
        spi_init();
        /* uint8_t dummy = 0; */
        /* To avoid SPI disturbance on IMU DB, on-chip IMU is forced to SPI by doing a dummy-write*/
        /* Write to register MPUREG_WHO_AM_I */
        /* inv_spi_master_write_register(INV_SPI_ONBOARD_REVB, 0x75, 1, &dummy); */
#endif
        break;
    }

    case UI_I2C:
#if (ICM42670P_USER_INTERFACE==ICM42670P_USE_I2C)
        /* Force AD0 to be driven as output level low or high*/
        iic_init(icm42670p_info->iic_hdl);
#endif
        break;
    default:
        return -1;
    }

    return configure_imu_device();
}

int configure_imu_device()
{
    int rc = 0;
    uint8_t who_am_i;

    /* Init device */
    rc = inv_imu_init(&icm_driver, &icm_serif, imu_callback);
    if (rc != INV_ERROR_SUCCESS) {
        log_error("icm42670p Failed to initialize IMU!");
        return rc;
    }

    /* Check WHOAMI */
    rc = inv_imu_get_who_am_i(&icm_driver, &who_am_i);
    if (rc != INV_ERROR_SUCCESS) {
        log_error("icm42670p Failed to read whoami!");
        return rc;
    }

    if (who_am_i != ICM_WHOAMI) {
        log_error("icm42670p Bad WHOAMI value!  Read 0x%02x, expected 0x%02x", who_am_i, ICM_WHOAMI);
        return INV_ERROR;
    }
    log_info("icm42670p WHOAMI value:0x%x, 0x%x", who_am_i, ICM_WHOAMI);

#if !USE_FIFO
    /* RINGBUFFER_CLEAR(&timestamp_buffer); */
#endif


    icm42670p_timer_init();
    if (!USE_FIFO) {
        rc |= inv_imu_configure_fifo(&icm_driver, INV_IMU_FIFO_DISABLED);
    }

    if (USE_HIGH_RES_MODE) {
        rc |= inv_imu_enable_high_resolution_fifo(&icm_driver);
    } else {
        rc |= inv_imu_set_accel_fsr(&icm_driver, ACCEL_CONFIG0_FS_SEL_16g);
        rc |= inv_imu_set_gyro_fsr(&icm_driver, GYRO_CONFIG0_FS_SEL_2000dps);
    }

    if (USE_LOW_NOISE_MODE) {
        rc |= inv_imu_set_accel_frequency(&icm_driver, ACCEL_CONFIG0_ODR_100_HZ);
        rc |= inv_imu_set_gyro_frequency(&icm_driver, GYRO_CONFIG0_ODR_100_HZ);
        rc |= inv_imu_enable_accel_low_noise_mode(&icm_driver);
        /* rc |= inv_imu_set_accel_ln_bw(&icm_driver,ACCEL_CONFIG1_ACCEL_FILT_BW_16); */
    } else {
        rc |= inv_imu_set_accel_frequency(&icm_driver, ACCEL_CONFIG0_ODR_100_HZ);
        rc |= inv_imu_set_gyro_frequency(&icm_driver, GYRO_CONFIG0_ODR_100_HZ);
        rc |= inv_imu_enable_accel_low_power_mode(&icm_driver);
        /* rc |= inv_imu_set_accel_lp_avg(&icm_driver, ACCEL_CONFIG1_ACCEL_FILT_AVG_2); */
    }

    rc |= inv_imu_enable_gyro_low_noise_mode(&icm_driver);
    /* rc |= inv_imu_set_gyro_ln_bw(&icm_driver,GYRO_CONFIG1_GYRO_FILT_BW_16); */

    if (!USE_FIFO) {
        inv_imu_sleep_us(GYR_STARTUP_TIME_US);
    }

    return rc;
}


#if SCALED_DATA_G_DPS
static void get_accel_and_gyr_fsr(uint16_t *accel_fsr_g, uint16_t *gyro_fsr_dps)
{
    ACCEL_CONFIG0_FS_SEL_t accel_fsr_bitfield;
    GYRO_CONFIG0_FS_SEL_t gyro_fsr_bitfield;

    inv_imu_get_accel_fsr(&icm_driver, &accel_fsr_bitfield);
    switch (accel_fsr_bitfield) {
    case ACCEL_CONFIG0_FS_SEL_2g:
        *accel_fsr_g = 2;
        break;
    case ACCEL_CONFIG0_FS_SEL_4g:
        *accel_fsr_g = 4;
        break;
    case ACCEL_CONFIG0_FS_SEL_8g:
        *accel_fsr_g = 8;
        break;
    case ACCEL_CONFIG0_FS_SEL_16g:
        *accel_fsr_g = 16;
        break;
    default:
        *accel_fsr_g = -1;
    }

    inv_imu_get_gyro_fsr(&icm_driver, &gyro_fsr_bitfield);
    switch (gyro_fsr_bitfield) {
    case GYRO_CONFIG0_FS_SEL_250dps:
        *gyro_fsr_dps = 250;
        break;
    case GYRO_CONFIG0_FS_SEL_500dps:
        *gyro_fsr_dps = 500;
        break;
    case GYRO_CONFIG0_FS_SEL_1000dps:
        *gyro_fsr_dps = 1000;
        break;
    case GYRO_CONFIG0_FS_SEL_2000dps:
        *gyro_fsr_dps = 2000;
        break;
    default:
        *gyro_fsr_dps = -1;
    }
}
#endif

/* --------------------------------------------------------------------------------------
 *  Static functions definition
 * -------------------------------------------------------------------------------------- */
static void apply_mounting_matrix(const int32_t matrix[9], int32_t raw[3])
{
    unsigned i;
    int64_t data_q30[3];

    for (i = 0; i < 3; i++) {
        data_q30[i] = ((int64_t)matrix[3 * i + 0] * raw[0]);
        data_q30[i] += ((int64_t)matrix[3 * i + 1] * raw[1]);
        data_q30[i] += ((int64_t)matrix[3 * i + 2] * raw[2]);
    }
    raw[0] = (int32_t)(data_q30[0] >> 30);
    raw[1] = (int32_t)(data_q30[1] >> 30);
    raw[2] = (int32_t)(data_q30[2] >> 30);
}
static u8 *cpy_addr = NULL;
static u16 cpy_data_len = 0;
void imu_callback(inv_imu_sensor_event_t *event)
{
    uint64_t timestamp;
    int32_t accel[3], gyro[3];
#if ICM42670P_FIFO_DATA_FIT
    short accel_tmp[3], gyro_tmp[3];
#endif /*ICM42670P_FIFO_DATA_FIT*/

#if SCALED_DATA_G_DPS
    float accel_g[3];
    float gyro_dps[3];
    float temp_degc;
    uint16_t accel_fsr_g, gyro_fsr_dps;
#endif

#if USE_FIFO
    static uint64_t last_fifo_timestamp = 0;
    static uint32_t rollover_num = 0;

    // Handle rollover
    if (last_fifo_timestamp > event->timestamp_fsync) {
        rollover_num++;
    }
    last_fifo_timestamp = event->timestamp_fsync;

    // Compute timestamp in us
    timestamp = event->timestamp_fsync + rollover_num * UINT16_MAX;
    timestamp *= inv_imu_get_fifo_timestamp_resolution_us_q24(&icm_driver);
    timestamp /= (1UL << 24);

    if (icm_driver.fifo_highres_enabled) {
        accel[0] = (((int32_t)event->accel[0] << 4)) | event->accel_high_res[0];
        accel[1] = (((int32_t)event->accel[1] << 4)) | event->accel_high_res[1];
        accel[2] = (((int32_t)event->accel[2] << 4)) | event->accel_high_res[2];

        gyro[0] = (((int32_t)event->gyro[0] << 4)) | event->gyro_high_res[0];
        gyro[1] = (((int32_t)event->gyro[1] << 4)) | event->gyro_high_res[1];
        gyro[2] = (((int32_t)event->gyro[2] << 4)) | event->gyro_high_res[2];

    } else {
        accel[0] = event->accel[0];
        accel[1] = event->accel[1];
        accel[2] = event->accel[2];

        gyro[0] = event->gyro[0];
        gyro[1] = event->gyro[1];
        gyro[2] = event->gyro[2];
    }
#else
    /* inv_disable_irq(); */
    /* if (!RINGBUFFER_EMPTY(&timestamp_buffer)) */
    /* 	RINGBUFFER_POP(&timestamp_buffer, &timestamp); */
    /* inv_enable_irq(); */

    accel[0] = event->accel[0];
    accel[1] = event->accel[1];
    accel[2] = event->accel[2];

    gyro[0] = event->gyro[0];
    gyro[1] = event->gyro[1];
    gyro[2] = event->gyro[2];

    // Force sensor_mask so it gets displayed below
    event->sensor_mask |= (1 << INV_SENSOR_TEMPERATURE);
    event->sensor_mask |= (1 << INV_SENSOR_ACCEL);
    event->sensor_mask |= (1 << INV_SENSOR_GYRO);
#endif

    /* apply_mounting_matrix(icm_mounting_matrix, accel); */
    /* apply_mounting_matrix(icm_mounting_matrix, gyro); */

#if SCALED_DATA_G_DPS
    /*
     * Convert raw data into scaled data in g and dps
    */
    get_accel_and_gyr_fsr(&accel_fsr_g, &gyro_fsr_dps);
    accel_g[0]  = (float)(accel[0] * accel_fsr_g)  / INT16_MAX;
    accel_g[1]  = (float)(accel[1] * accel_fsr_g)  / INT16_MAX;
    accel_g[2]  = (float)(accel[2] * accel_fsr_g)  / INT16_MAX;
    gyro_dps[0] = (float)(gyro[0]  * gyro_fsr_dps) / INT16_MAX;
    gyro_dps[1] = (float)(gyro[1]  * gyro_fsr_dps) / INT16_MAX;
    gyro_dps[2] = (float)(gyro[2]  * gyro_fsr_dps) / INT16_MAX;
    if (USE_HIGH_RES_MODE || !USE_FIFO) {
        temp_degc = 25 + ((float)event->temperature / 128);
    } else {
        temp_degc = 25 + ((float)event->temperature / 2);
    }

    /*
     * Output scaled data on UART link
     */
    /* log_info("accel:%d,%d,%d,gyro:%d,%d,%d,accel_fsr_g:%d,gyro_fsr_dps:%d",accel[0],accel[1],accel[2],gyro[0],gyro[1],gyro[2],accel_fsr_g,gyro_fsr_dps); */
    if (event->sensor_mask & (1 << INV_SENSOR_ACCEL) && event->sensor_mask & (1 << INV_SENSOR_GYRO)) {
        /* log_info("cpy_addr:%d",cpy_addr); */
        memcpy(cpy_addr, (u8 *)&timestamp, sizeof(timestamp));
        cpy_addr += sizeof(timestamp);
        memcpy(cpy_addr, (u8 *)accel_g, sizeof(accel_g));
        cpy_addr += sizeof(accel_g);
        memcpy(cpy_addr, (u8 *)gyro_dps, sizeof(gyro_dps));
        cpy_addr += sizeof(gyro_dps);
        memcpy(cpy_addr, (u8 *) & (temp_degc), sizeof(temp_degc));
        cpy_addr += sizeof(temp_degc);
        cpy_data_len++;
        /* log_info("%u: accel_g*10:%d, %d, %d, temp_degc*10:%d, gyro_dps*10:%d, %d, %d", */
        /*          (uint32_t)timestamp, */
        /*          (s32)(accel_g[0] * 10), (s32)(accel_g[1] * 10), (s32)(accel_g[2] * 10), */
        /*          (s32)(temp_degc * 10), */
        /*          (s32)(gyro_dps[0] * 10), (s32)(gyro_dps[1] * 10), (s32)(gyro_dps[2] * 10)); */

    } else if (event->sensor_mask & (1 << INV_SENSOR_GYRO)) {
        /* memcpy(cpy_addr,(u8*)&timestamp,sizeof(timestamp)); */
        /* cpy_addr += sizeof(timestamp); */
        /* memcpy(cpy_addr,(u8*)gyro_dps,sizeof(gyro_dps)); */
        /* cpy_addr += sizeof(gyro_dps); */
        /* memcpy(cpy_addr,(u8*)&(temp_degc),sizeof(temp_degc)); */
        /* cpy_addr += sizeof(temp_degc); */
        log_info("%u: accel_g: NA, NA, NA, temp_degc*10:%d, gyro_dps*10:%d, %d, %d",
                 (uint32_t)timestamp,
                 (s32)(temp_degc * 10),
                 (s32)(gyro_dps[0] * 10), (s32)(gyro_dps[1] * 10), (s32)(gyro_dps[2] * 10));

    } else if (event->sensor_mask & (1 << INV_SENSOR_ACCEL)) {
        /* memcpy(cpy_addr,(u8*)&timestamp,sizeof(timestamp)); */
        /* cpy_addr += sizeof(timestamp); */
        /* memcpy(cpy_addr,(u8*)accel_g,sizeof(accel_g)); */
        /* cpy_addr += sizeof(accel_g); */
        /* memcpy(cpy_addr,(u8*)&(temp_degc),sizeof(temp_degc)); */
        /* cpy_addr += sizeof(temp_degc); */
        log_info("%u: accel_g*10:%d, %d, %d, temp_degc*10:%d, gyro_dps: NA, NA, NA",
                 (uint32_t)timestamp,
                 (s32)(accel_g[0] * 10), (s32)(accel_g[1] * 10), (s32)(accel_g[2] * 10),
                 (s32)(temp_degc * 10));
    }
#else

    /*
     * Output raw data on UART link
     */
    if (event->sensor_mask & (1 << INV_SENSOR_ACCEL) && event->sensor_mask & (1 << INV_SENSOR_GYRO)) {
#if ICM42670P_FIFO_DATA_FIT
        accel_tmp[0] = (short)accel[0];
        accel_tmp[1] = (short)accel[1];
        accel_tmp[2] = (short)accel[2];
        gyro_tmp[0] = (short)gyro[0];
        gyro_tmp[1] = (short)gyro[1];
        gyro_tmp[2] = (short)gyro[2];
        memcpy(cpy_addr, (u8 *)accel_tmp, sizeof(accel_tmp));
        cpy_addr += sizeof(accel_tmp);
        memcpy(cpy_addr, (u8 *)gyro_tmp, sizeof(gyro_tmp));
        cpy_addr += sizeof(gyro_tmp);
#else /*ICM42670P_FIFO_DATA_FIT == 0*/
        memcpy(cpy_addr, (u8 *)&timestamp, sizeof(timestamp));
        cpy_addr += sizeof(timestamp);
        memcpy(cpy_addr, (u8 *)accel, sizeof(accel));
        cpy_addr += sizeof(accel);
        memcpy(cpy_addr, (u8 *)gyro, sizeof(gyro));
        cpy_addr += sizeof(gyro);
        memcpy(cpy_addr, (u8 *) & (event->temperature), sizeof(event->temperature));
        cpy_addr += sizeof(event->temperature);
#endif /*ICM42670P_FIFO_DATA_FIT*/
        cpy_data_len++;
        /* log_info("%u: accel:%d, %d, %d, temperature:%d, gyro:%d, %d, %d", */
        /*          (uint32_t)timestamp, */
        /*          accel[0], accel[1], accel[2], */
        /*          event->temperature, */
        /*          gyro[0], gyro[1], gyro[2]); */
    } else if (event->sensor_mask & (1 << INV_SENSOR_GYRO)) {
        /* memcpy(cpy_addr,(u8*)&timestamp,sizeof(timestamp)); */
        /* cpy_addr += sizeof(timestamp); */
        /* memcpy(cpy_addr,(u8*)gyro,sizeof(gyro)); */
        /* cpy_addr += sizeof(gyro); */
        /* memcpy(cpy_addr,(u8*)&(event->temperature),sizeof(event->temperature)); */
        /* cpy_addr += sizeof(event->temperature); */
        log_info("%u: NA, NA, NA, temperature:%d, gyro:%d, %d, %d",
                 (uint32_t)timestamp,
                 event->temperature,
                 gyro[0], gyro[1], gyro[2]);
    } else if (event->sensor_mask & (1 << INV_SENSOR_ACCEL)) {
        /* memcpy(cpy_addr,(u8*)&timestamp,sizeof(timestamp)); */
        /* cpy_addr += sizeof(timestamp); */
        /* memcpy(cpy_addr,(u8*)accel,sizeof(accel)); */
        /* cpy_addr += sizeof(accel); */
        /* memcpy(cpy_addr,(u8*)&(event->temperature),sizeof(event->temperature)); */
        /* cpy_addr += sizeof(event->temperature); */
        log_info("%u: accel:%d, %d, %d, temperature:%d, NA, NA, NA",
                 (uint32_t)timestamp,
                 accel[0], accel[1], accel[2],
                 event->temperature);
    }
#endif
}
u16 get_package_size()
{
#if ICM42670P_FIFO_DATA_FIT
    return 12;
#else /*ICM42670P_FIFO_DATA_FIT == 0*/
#if SCALED_DATA_G_DPS
    return 36;
#else /*SCALED_DATA_G_DPS== 0*/
    return 34;
#endif /*SCALED_DATA_G_DPS*/
#endif /*ICM42670P_FIFO_DATA_FIT*/
}
int get_imu_data(void *buf)
{
#if USE_FIFO
//解码后数据: 8(timer[u64])+12(acc[float])+12(gyro[float])+4(temp[float])
//原始数据  : 8(timer[u64])+12(acc[int32])+12(gyro[int32])+2(temp[int16])
    cpy_addr = (u8 *)buf;
    return inv_imu_get_data_from_fifo(&icm_driver);
#else
    return inv_imu_get_data_from_registers(&icm_driver);
#endif
}







static u8 init_flag = 0;
static u8 imu_busy = 0;
static icm42670p_param icm42670p_info_data;
volatile u8 icm42670p_int_flag = 0;
/* #define SENSORS_MPU_BUFF_LEN 14 */
/* u8 read_icm42670p_buf[SENSORS_MPU_BUFF_LEN]; */
#if (ICM42670P_USE_INT_EN) //中断模式
static u8 data_buf[36 * 10]; //(8+12+12+4)*60=

void icm42670p_int_callback()
{
    if (init_flag == 0) {
        log_error("icm42670p init fail!");
        return ;
    }
    if (imu_busy) {
        log_error("icm42670p busy!");
        return ;
    }
    imu_busy = 1;

    icm42670p_int_flag = 1;
    imu_sensor_data_t raw_sensor_datas;
    float TempData = 0.0;
    u8 status_temp = 0;

    get_imu_data(data_buf);
    /* icm42670p_read_raw_acc_gyro_xyz(&raw_sensor_datas);//获取原始值 */
    /* TempData = raw_sensor_datas.temp_data; */
    /* log_info("icm42670p raw:acc_x:%d acc_y:%d acc_z:%d gyro_x:%d gyro_y:%d gyro_z:%d", raw_sensor_datas.acc.x, raw_sensor_datas.acc.y, raw_sensor_datas.acc.z, raw_sensor_datas.gyro.x, raw_sensor_datas.gyro.y, raw_sensor_datas.gyro.z); */
    /* log_info("icm42670p temp:%d.%d\n", (u16)TempData, (u16)(((u16)(TempData * 100)) % 100)); */
    imu_busy = 0;
}
#endif

s8 icm42670p_dev_init(void *arg)
{
    if (arg == NULL) {
        log_error("icm42670p init fail(no arg)\n");
        return -1;
    }
#if (ICM42670P_USER_INTERFACE==ICM42670P_USE_I2C)
    icm42670p_info_data.iic_hdl = ((struct imusensor_platform_data *)arg)->peripheral_hdl;
    icm42670p_info_data.iic_delay = ((struct imusensor_platform_data *)arg)->peripheral_param0;   //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
    // u8 iic_clk;      //iic_clk: <=400kHz
#elif (ICM42670P_USER_INTERFACE==ICM42670P_USE_SPI)
    icm42670p_info_data.spi_hdl = ((struct imusensor_platform_data *)arg)->peripheral_hdl,     //SPIx (role:master)
                        icm42670p_info_data.spi_cs_pin = ((struct imusensor_platform_data *)arg)->peripheral_param0; //IO_PORTA_05
    // u8 port;         //SPIx group:A,B,C,D (spi结构体)
    // U8 spi_clk;      //spi_clk: <=1MHz (spi结构体)
#else
    //I3C
#endif
    icm42670p_int_pin1 = ((struct imusensor_platform_data *)arg)->imu_sensor_int_io;
    icm42670p_int_pin2 = icm42670p_int_pin1;
    if (imu_busy) {
        log_error("icm42670p busy!");
        return -1;
    }
    imu_busy = 1;
    if (icm42670p_sensor_init(&icm42670p_info_data) == 0) {
        log_info("icm42670p Device init success!\n");
#if (ICM42670P_USE_INT_EN) //中断模式,暂不支持
        log_info("int mode en!");
        /* port_wkup_enable(icm42670p_int_pin1, 1, icm42670p_int_callback); //PA08-IO中断，1:下降沿触发，回调函数icm42670p_int_callback*/
#ifdef CONFIG_CPU_BR23
        io_ext_interrupt_init(icm42670p_int_pin1, 1, icm42670p_int_callback);
#elif defined(CONFIG_CPU_BR28)
        // br28外部中断回调函数，按照现在的外部中断注册方式
        // io配置在板级，定义在板级头文件，这里只是注册回调函数
        /* port_edge_wkup_set_callback_by_index(3, icm42670p_int_callback); // 序号需要和板级配置中的wk_param对应上 */
        port_edge_wkup_set_callback(icm42670p_int_callback);
#elif defined(CONFIG_CPU_BR27)
        port_edge_wkup_set_callback(icm42670p_int_callback);
#endif
#else //定时

#endif
        init_flag = 1;
        imu_busy = 0;
        return 0;
    } else {
        log_info("icm42670p Device init fail!\n");
        imu_busy = 0;
        return -1;
    }
}


int icm42670p_dev_ctl(u8 cmd, void *arg);
REGISTER_IMU_SENSOR(icm42670p_sensor) = {
    .logo = "icm42670p",
    .imu_sensor_init = icm42670p_dev_init,
    .imu_sensor_check = NULL,
    .imu_sensor_ctl = icm42670p_dev_ctl,
};

int icm42670p_dev_ctl(u8 cmd, void *arg)
{
    int ret = -1;
    u8 status_temp[2] = {0, 0};
    if (init_flag == 0) {
        log_error("icm42670p init fail!");
        return ret;//0:ok,,<0:err
    }
    if (imu_busy) {
        log_error("icm42670p busy!");
        return ret;//0:ok,,<0:err
    }
    imu_busy = 1;
    switch (cmd) {
    case IMU_GET_SENSOR_NAME:
        memcpy((u8 *)arg, &(icm42670p_sensor.logo), 20);
        ret = 0;
        break;
    case IMU_SENSOR_ENABLE:
        /* cbuf_init(&hrsensor_cbuf, hrsensorcbuf, 24 * sizeof(int)); */
        configure_imu_device();
        ret = 0;
        break;
    case IMU_SENSOR_DISABLE:
        /* cbuf_clear(&hrsensor_cbuf); */
        inv_imu_device_reset(&icm_driver);
        ret = 0;
        break;
    case IMU_SENSOR_RESET:
        inv_imu_device_reset(&icm_driver);
        ret = 0;
        break;
    case IMU_SENSOR_SLEEP:
        if (inv_imu_enter_sleep(&icm_driver) == 0) {
            log_info("icm42670p enter sleep ok!");
            ret = 0;
        } else {
            log_error("icm42670p enter sleep fail!");
        }
        break;
    case IMU_SENSOR_WAKEUP:
        if (inv_imu_exit_sleep(&icm_driver) == 0) { //
            log_info("icm42670p wakeup ok!");
            ret = 0;
        } else {
            log_error("icm42670p wakeup fail!");
        }
        break;
    case IMU_SENSOR_INT_DET://传感器中断状态检查
        break;
    case IMU_SENSOR_DATA_READY://传感器数据准备就绪待读
        /* icm42670p_int_callback(); */
        break;
    case IMU_SENSOR_CHECK_DATA://检查传感器缓存buf是否存满
        break;
    case IMU_SENSOR_READ_DATA://默认读传感器所有数据
        /* status_temp = mpu6887p_read_status(); */
        /* #<{(| log_info("status:0x%x", status_temp); |)}># */
        /* if (status_temp & 0x01) { */
        /*     mpu6887p_read_raw_acc_gyro_xyz(arg);//获取原始值 */
        /*     ret = 0; */
        /* } */
        break;
    case IMU_GET_ACCEL_DATA://加速度数据
        /* #<{(| float TempData = 0.0; |)}># */
        /* status_temp = mpu6887p_read_status(); */
        /* #<{(| log_info("status:0x%x", status_temp); |)}># */
        /* if (status_temp & 0x01) { */
        /*     mpu6887p_read_raw_acc_xyz(arg);//获取原始值 */
        /*     ret = 0; */
        /*     #<{(| TempData = mpu6887p_readTemp(); |)}># */
        /* } */
        break;
    case IMU_GET_GYRO_DATA://陀螺仪数据
        /* status_temp = mpu6887p_read_status(); */
        /* #<{(| log_info("status:0x%x", status_temp); |)}># */
        /* if (status_temp & 0x01) { */
        /*     mpu6887p_read_raw_gyro_xyz(arg);//获取原始值 */
        /*     ret = 0; */
        /* } */
        break;
    case IMU_GET_MAG_DATA://磁力计数据
        log_error("icm42670p have no mag!\n");
        break;
    case IMU_SENSOR_SEARCH://检查传感器id
        int rc = 0;
        rc = inv_imu_get_who_am_i(&icm_driver, (u8 *)arg);
        if (rc != INV_ERROR_SUCCESS) {
            log_error("icm42670p offline!\n");
        }
        if (*(u8 *)arg == ICM_WHOAMI) {
            ret = 0;
            log_info("icm42670p online!\n");
        } else {
            log_error("icm42670p offline!\n");
        }
        break;
    case IMU_GET_SENSOR_STATUS://获取传感器状态
        inv_imu_read_reg(&icm_driver, INT_STATUS_DRDY, 2, status_temp);
        *(u8 *)arg = status_temp[0];
        *((u8 *)arg + 1) = status_temp[1];
        ret = 0;
        break;
    case IMU_SET_SENSOR_FIFO_CONFIG://配置传感器FIFO
        /* u8 *tmp = (u8 *)arg; */
        /* u16 wm_th = tmp[0] | (tmp[1] << 8); */
        /* u8 fifo_mode = tmp[2]; */
        /* u8 acc_en = tmp[3]; */
        /* u8 gyro_en = tmp[4]; */
        /* mpu6887p_config_fifo(wm_th, fifo_mode, acc_en, gyro_en); */
        /* ret = 0; */
        break;
    case IMU_GET_SENSOR_READ_FIFO://读取传感器FIFO数据
        u16 total_packet_count = 0;
        cpy_addr = (u8 *)arg;
//解码后数据: 8(timer[u64])+12(acc[float])+12(gyro[float])+4(temp[float])
//原始数据  : 8(timer[u64])+12(acc[int32])+12(gyro[int32])+2(temp[int16])
        total_packet_count = inv_imu_get_data_from_fifo(&icm_driver);
#if ICM42670P_FIFO_DATA_FIT
        total_packet_count = cpy_data_len * 12; //数据长度
#else /*ICM42670P_FIFO_DATA_FIT*/
        total_packet_count = cpy_data_len; //包
#endif /*ICM42670P_FIFO_DATA_FIT*/
        cpy_data_len = 0;
        /* uint16_t packet_size = FIFO_16BYTES_PACKET_SIZE; */
        /* 		if (icm_driver.fifo_highres_enabled){ */
        /* 			packet_size = FIFO_20BYTES_PACKET_SIZE; */
        /* 		} */
        /* #if 1 //原数据 */
        /* 		memcpy((u8 *)arg,(u8 *)(icm_driver.fifo_data),packet_size *total_packet_count); */
        /* #endif */
        ret = total_packet_count;//
        break;
    case IMU_SET_SENSOR_TEMP_DISABLE://关闭温度传感器
        log_info("cannot disable temp!\n");
        ret = 0;
        break;
    default:
        log_error("--cmd err!\n");
        break;
    }
    imu_busy = 0;
    return ret;//0:ok,,<0:err

}









/***************************ICM42670P test*******************************/
#if 0 //测试
static icm42670p_param icm42670p_info_test = {
#if (ICM42670P_USER_INTERFACE==ICM42670P_USE_I2C)
    .iic_hdl = 0,
    .iic_delay = 0,   //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
#elif (ICM42670P_USER_INTERFACE==ICM42670P_USE_SPI)
    .spi_hdl = 1,     //SPIx (role:master)
    .spi_cs_pin = IO_PORTA_05, //
#else
    //I3C
#endif
};
u8 test_buf[36 * 20]; //(8+12+12+4)*60=
/********************int test*******************/
void icm42670p_test()
{
    imu_sensor_data_t raw_sensor_datas;
    u8 status_temp = 0;
    float TempData;
    if (icm42670p_sensor_init(&icm42670p_info_test) == 0) { //no fifo no int
        log_info("icm42670p init success!\n");
        /* MDELAY(10); */
#if (ICM42670P_USE_INT_EN==0) //定时
        JL_PORTG->OUT &= ~BIT(5);
        JL_PORTG->DIR &= ~BIT(5);
        JL_PORTG->OUT &= ~BIT(4);
        JL_PORTG->DIR &= ~BIT(4);
        while (1) {
            MDELAY(50);
            /* status_temp = icm42670p_read_status(); */
            log_info("status0:0x%x", status_temp);
            /* if (status_temp & 0x01) { */
            /* icm42670p_read_raw_acc_gyro_xyz(&raw_sensor_datas);//获取原始值 */
            /* TempData = icm42670p_readTemp(); //温度 */
            /*     TempData = raw_sensor_datas.temp_data; */
            /*     log_info("icm42670p raw:acc_x:%d acc_y:%d acc_z:%d gyro_x:%d gyro_y:%d gyro_z:%d", raw_sensor_datas.acc.x, raw_sensor_datas.acc.y, raw_sensor_datas.acc.z, raw_sensor_datas.gyro.x, raw_sensor_datas.gyro.y, raw_sensor_datas.gyro.z); */
            /*     log_info("icm42670p temp:%d.%d\n", (u16)TempData, (u16)(((u16)(TempData * 100)) % 100)); */
            /* } */
            JL_PORTG->OUT |= BIT(5);
            get_imu_data(test_buf);
            JL_PORTG->OUT &= ~BIT(5);
            wdt_clear();
        }
#else //中断
        //开中断
        log_info("-------------------port wkup isr---------------------------");
        /* port_wkup_enable(icm42670p_int_pin1, 1, icm42670p_int_callback);*/
#ifdef CONFIG_CPU_BR23
        io_ext_interrupt_init(icm42670p_int_pin1, 1, icm42670p_int_callback);
        /* #elif defined(CONFIG_CPU_BR28)||defined(CONFIG_CPU_BR27) */
        /*         // br28外部中断回调函数，按照现在的外部中断注册方式 */
        /*         // io配置在板级，定义在板级头文件，这里只是注册回调函数 */
        /*         port_edge_wkup_set_callback(icm42670p_int_callback); */
        /* #endif */
#elif defined(CONFIG_CPU_BR28)
        // br28外部中断回调函数，按照现在的外部中断注册方式
        // io配置在板级，定义在板级头文件，这里只是注册回调函数
        /* port_edge_wkup_set_callback_by_index(3, icm42670p_int_callback); // 序号需要和板级配置中的wk_param对应上 */
        port_edge_wkup_set_callback(icm42670p_int_callback);
#elif defined(CONFIG_CPU_BR27)
        port_edge_wkup_set_callback(icm42670p_int_callback);
#endif
        while (1) {
            MDELAY(500);
            /* status_temp = icm42670p_read_status(); */
            log_info("int status0:0x%x", status_temp);
            wdt_clear();
        }
#endif
    } else {
        log_error("icm42670p init fail!\n");
    }
}
#endif
#endif

