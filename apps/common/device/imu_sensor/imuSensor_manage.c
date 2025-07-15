#include "imuSensor_manage.h"
/* #include "asm/iic_hw.h" */
/* #include "asm/iic_soft.h" */

#if TCFG_IMUSENSOR_ENABLE
#undef LOG_TAG_CONST
#define LOG_TAG     "[imu]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

static struct imusensor_platform_data *platform_data;
IMU_SENSOR_INTERFACE *imuSensor_hdl = NULL;
u8 imu_sensor_cnt = 0;

#if 0/*{{{*/
u8 imusensor_write_nbyte(u8 w_chip_id, u8 register_address, u8 *buf, u8 data_len)
{
    u8 write_len = 0;
    u8 i;

    iic_start(imuSensor_info->iic_hdl);
    if (0 == iic_tx_byte(imuSensor_info->iic_hdl, w_chip_id)) {
        write_len = 0;
        r_printf("\n imuSensor iic w err 0\n");
        goto __wend;
    }

    delay(imuSensor_info->iic_delay);

    if (0 == iic_tx_byte(imuSensor_info->iic_hdl, register_address)) {
        write_len = 0;
        r_printf("\n imuSensor iic w err 1\n");
        goto __wend;
    }

    for (i = 0; i < data_len; i++) {
        delay(imuSensor_info->iic_delay);
        if (0 == iic_tx_byte(imuSensor_info->iic_hdl, buf[i])) {
            write_len = 0;
            r_printf("\n imuSensor iic w err 2\n");
            goto __wend;
        }
        write_len++;
    }

__wend:
    iic_stop(imuSensor_info->iic_hdl);
    return write_len;
}

u8 imusensor_read_nbyte(u8 r_chip_id, u8 register_address, u8 *buf, u8 data_len)
{
    u8 read_len = 0;

    iic_start(imuSensor_info->iic_hdl);
    if (0 == iic_tx_byte(imuSensor_info->iic_hdl, r_chip_id - 1)) {
        r_printf("\n imuSensor iic r err 0\n");
        read_len = 0;
        goto __rend;
    }

    delay(imuSensor_info->iic_delay);
    if (0 == iic_tx_byte(imuSensor_info->iic_hdl, register_address)) {
        r_printf("\n imuSensor iic r err 1\n");
        read_len = 0;
        goto __rend;
    }

    iic_start(imuSensor_info->iic_hdl);
    if (0 == iic_tx_byte(imuSensor_info->iic_hdl, r_chip_id)) {
        r_printf("\n imuSensor iic r err 2\n");
        read_len = 0;
        goto __rend;
    }

    delay(imuSensor_info->iic_delay);

    for (; data_len > 1; data_len--) {
        *buf++ = iic_rx_byte(imuSensor_info->iic_hdl, 1);
        read_len ++;
    }

    *buf = iic_rx_byte(imuSensor_info->iic_hdl, 0);
    read_len ++;

__rend:
    iic_stop(imuSensor_info->iic_hdl);
    delay(imuSensor_info->iic_delay);

    return read_len;
}
#endif/*}}}*/

extern char *strchrnul(const char *__s, int __c)
__THROW __attribute_pure__ __nonnull((1));
int imu_sensor_io_ctl(u8 *sensor_name, enum OPERATION_SENSOR cmd, void *arg)
{
    int retval = -1;
    list_for_each_imusensor(imuSensor_hdl) {
        /* log_info("%s?=%s", sensor_name, imuSensor_hdl->logo); */
        if ((sensor_name == NULL) && (cmd == IMU_GET_SENSOR_NAME)) {
            log_info("get name:%s", imuSensor_hdl->logo);
            retval = 1;
            continue;
        }
        if (!memcmp(imuSensor_hdl->logo, sensor_name, strlen(imuSensor_hdl->logo))) {
            /* if (!memcmp(imuSensor_hdl->logo, sensor_name, sizeof(imuSensor_hdl->logo))) { */
            if ((strchrnul(sensor_name, '\0') - (u32)sensor_name) != (strrchr(imuSensor_hdl->logo, '\0') - (u32)(imuSensor_hdl->logo))) {
                goto __imu_exit;
            }
            retval = imuSensor_hdl->imu_sensor_ctl(cmd, arg);
            return retval;//1:ok
        }
    }
__imu_exit:
    if (retval < 0) {
        log_e(">>>sensor_name(%s) io_ctl fail: sensor_name err\n", sensor_name);
    }
    return retval;//1:ok
}

int imu_sensor_init(void *_data, u16 _data_size)
{
    int retval = -1;
    u8 data_len = _data_size / sizeof(struct imusensor_platform_data);
    if (data_len < 1) {
        log_error("_data_size:%d\n", _data_size);
        return retval;
    }
    platform_data = (const struct imusensor_platform_data *)_data;
    /* log_info("-----imusensor_dev_begin:0x%x,imusensor_dev_end:0x%x", imusensor_dev_begin, imusensor_dev_end); */
    for (u8 i = 0; i < data_len; i++) {
        retval = -1;
        list_for_each_imusensor(imuSensor_hdl) {
            /* log_info("%s?=%s", platform_data[i].imu_sensor_name, imuSensor_hdl->logo); */
            if (!memcmp(imuSensor_hdl->logo, platform_data[i].imu_sensor_name, sizeof(platform_data[i].imu_sensor_name))) {

                retval = -1;
                if (imuSensor_hdl->imu_sensor_init(&platform_data[i]) == 0) {
                    g_printf(">>>>imuSensor(%s)_Init SUCC\n", platform_data[i].imu_sensor_name);
                    imu_sensor_cnt++;
                    retval = 1;
                } else {
                    g_printf(">>>>imuSensor(%s)_Init ERROR\n", platform_data[i].imu_sensor_name);
                    retval = 0;
                }
                break;
            }
        }
        if (retval < 0) {
            log_e(">>>imuSensor(%s) init fail: logo err\n", platform_data[i].imu_sensor_name);
        }
    }

    return retval;//1:ok
}





/**********************test***********************/
#if 0
u8 test_buf[36 * 20]; //(8+12+12+4)*60=
void imu_test()
{
    u8 name_test[20] = "sh3011";
    u8 arg_data;
    /* imu_sensor_init(); */
    imu_sensor_io_ctl(name_test, 0xff, NULL);
    imu_sensor_io_ctl(NULL, IMU_GET_SENSOR_NAME, name_test);
    memset(name_test, 0, 20);
    memcpy(name_test, "sh3001", 6);
    imu_sensor_io_ctl(name_test, 0xff, NULL);
    imu_sensor_io_ctl(name_test, IMU_SENSOR_SEARCH, &arg_data);
    memset(name_test, 0, 20);
    memcpy(name_test, "mpu9250", 7);
    imu_sensor_io_ctl(name_test, 0xff, NULL);
    imu_sensor_io_ctl(name_test, IMU_SENSOR_SEARCH, &arg_data);
    memset(name_test, 0, 20);
    memcpy(name_test, "mpu6887p", 8);
    imu_sensor_io_ctl(name_test, 0xff, NULL);
    imu_sensor_io_ctl(name_test, IMU_SENSOR_SEARCH, &arg_data);
    imu_sensor_io_ctl("mpu6887p1", IMU_SENSOR_SEARCH, &arg_data);
    memset(name_test, 0, 20);
    memcpy(name_test, "qmi8658", 7);
    imu_sensor_io_ctl(name_test, 0xff, NULL);
    imu_sensor_io_ctl(name_test, IMU_SENSOR_SEARCH, &arg_data);
    memset(name_test, 0, 20);
    memcpy(name_test, "lsm6dsl", 7);
    imu_sensor_io_ctl(name_test, IMU_SENSOR_SEARCH, &arg_data);

    memcpy(name_test, "icm42670p", 9);
    imu_sensor_io_ctl(name_test, 0xff, NULL);
    imu_sensor_io_ctl(name_test, IMU_SENSOR_SEARCH, &arg_data);
    imu_axis_data_t raw_acc_xyz;
    imu_axis_data_t raw_gyro_xyz;
    imu_sensor_data_t raw_sensor_datas;
    memset((u8 *)&raw_acc_xyz, 0, sizeof(imu_axis_data_t));
    memset((u8 *)&raw_gyro_xyz, 0, sizeof(imu_axis_data_t));
    memset((u8 *)&raw_sensor_datas, 0, sizeof(imu_sensor_data_t));
    uint64_t timestamp;
    float accel_g[3];
    float gyro_dps[3];
    float temp_degc;
    int pack_len = 0;
    while (1) {
        pack_len = imu_sensor_io_ctl(name_test, IMU_GET_SENSOR_READ_FIFO, test_buf);
        if (pack_len > 0) {
            memcpy((u8 *)&timestamp, test_buf, 8);
            memcpy((u8 *)accel_g, test_buf + 8, 12);
            memcpy((u8 *)gyro_dps, test_buf + 20, 12);
            memcpy((u8 *)&temp_degc, test_buf + 32, 4);
            log_info("pack_len:%d  first pack:%u: accel_g*10:%d, %d, %d, temp_degc*10:%d, gyro_dps*10:%d, %d, %d", pack_len,
                     (uint32_t)timestamp,
                     (s32)(accel_g[0] * 10), (s32)(accel_g[1] * 10), (s32)(accel_g[2] * 10),
                     (s32)(temp_degc * 10),
                     (s32)(gyro_dps[0] * 10), (s32)(gyro_dps[1] * 10), (s32)(gyro_dps[2] * 10));
        }

        /* imu_sensor_io_ctl(name_test, IMU_SENSOR_READ_DATA, &raw_sensor_datas); */
        /* log_info("qmi8658 raw:acc_x:%d acc_y:%d acc_z:%d gyro_x:%d gyro_y:%d gyro_z:%d", raw_sensor_datas.acc.x, raw_sensor_datas.acc.y, raw_sensor_datas.acc.z, raw_sensor_datas.gyro.x, raw_sensor_datas.gyro.y, raw_sensor_datas.gyro.z); */
        /* log_info("qmi8658 temp:%d.%d\n", (u16)raw_sensor_datas.temp_data, (u16)(((u16)(raw_sensor_datas.temp_data * 100)) % 100)); */
        /* mdelay(10); */
        /* imu_sensor_io_ctl(name_test, IMU_GET_ACCEL_DATA, &raw_acc_xyz); */
        /* log_info("qmi8658 acc: %d %d %d\n", raw_acc_xyz.x, raw_acc_xyz.y, raw_acc_xyz.z); */
        /* mdelay(10); */
        /* imu_sensor_io_ctl(name_test, IMU_GET_GYRO_DATA, &raw_gyro_xyz); */
        /* log_info("qmi8658 gyro: %d %d %d\n", raw_gyro_xyz.x, raw_gyro_xyz.y, raw_gyro_xyz.z); */

        mdelay(50);
        wdt_clear();
    }
}
#endif
#endif

