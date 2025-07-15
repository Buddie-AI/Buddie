#ifndef SPATIALAUDIO_API_H
#define SPATIALAUDIO_API_H

typedef struct TRANVAL {
    int trans_x[3];
    int trans_y[3];
    int trans_z[3];

} tranval_t;

typedef struct COMMON_INFO {
    float fs;//采样率
    int len;//一包数据长度
    float sensitivity;//陀螺仪灵敏度
    int range;//加速度计量程（正）
} info_spa_t;

typedef struct {
    float beta;
    float val;//陀螺仪参考阈值
    float cel_val;//动态校准参考阈值
    float time;//动态校准时长
    float SerialTime;//动态校准窗长
    float sensval;//角度灵敏度，越小越灵敏，范围：0.01~0.1，默认0.1
} spatial_config_t;

//陀螺仪偏置
typedef struct {
    float gyro_x;
    float gyro_y;
    float gyro_z;
} gyro_cel_t;

//加速度计偏置
typedef struct {
    float acc_offx;
    float acc_offy;
    float acc_offz;
} acc_cel_t;

extern inline float root_float(float x);
extern inline float angle_float(float x, float y);
int get_Spatial_buf(int len);
void init_Spatial(void *ptr, info_spa_t *, tranval_t *, spatial_config_t *, gyro_cel_t *, acc_cel_t *ac);
void Spatial_cacl(void *ptr, short *data);
int get_Spa_angle(void *ptr, float alpha, float voloct);
void Spatial_reset(void *ptr);
int Spatial_stra(void *ptr, int time, float val1);
int get_test_angle(void *ptr);
//int get_Pitch_angle(void* ptr);

#endif // !1


