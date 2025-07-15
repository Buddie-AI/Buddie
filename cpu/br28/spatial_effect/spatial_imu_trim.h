#ifndef _SPATIAL_IMU_TRIM_
#define _SPATIAL_IMU_TRIM_

#include "generic/typedef.h"

/*开始校准*/
int spatial_imu_trim_start();

/*关闭校准*/
int spatial_imu_trim_stop();

/*创建校准任务*/
int spatial_imu_trim_task_start();

/*删除校准任务和校准资源*/
int spatial_imu_trim_task_stop();

#endif
