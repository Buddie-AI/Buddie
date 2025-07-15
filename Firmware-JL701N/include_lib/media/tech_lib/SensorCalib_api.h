#ifndef SENSORCALIB_API_H
#define SENSORCALIB_API_H
#include "tech_lib/SpatialAudio_api.h"

//校准阈值
typedef struct {
    float thv1;//加速度计校准阈值1
    float thv2;//
} Thval_t;

enum {
    type_0 = 0,
    type_1,
    type_2
};
enum {
    mode_0 = 0,
    mode_1
};

int GroCelBuff();
void GroCelInit(void *buf, info_spa_t *para, int time);
int Gro_Calibration(void *ptr, short *data, info_spa_t *para, tranval_t *, int mode, gyro_cel_t *agv);

int AccCelBuff();
void AccCelInit(void *ptr, info_spa_t *para, int time);
int Acc_Calibration(void *ptr, short *data, info_spa_t *para, acc_cel_t *ac, Thval_t *tv);

#endif // !1


