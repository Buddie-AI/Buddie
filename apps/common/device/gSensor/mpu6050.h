#ifndef __MPU6050_H_
#define __MPU6050_H_

#include "mpu6050_reg.h"

#define MPU6050_GYRO_OUT_RATE   1000    //1K或8K
#define MPU6050_SAMPLE_RATE     125

enum {
    ACCEL_RANGE_2G,
    ACCEL_RANGE_4G,
    ACCEL_RANGE_8G,
    ACCEL_RANGE_16G,
};

#endif
