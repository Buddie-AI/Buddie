#include "spatial_imu_trim.h"
#include "spatial_effect_imu.h"
/* #include "imuSensor_manage.h" */
#include "tech_lib/SpatialAudio_api.h"
#include "tech_lib/SensorCalib_api.h"
#include "system/includes.h"
#include "asm/timer.h"
#include "app_config.h"
#include "task.h"
#include "event.h"

#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE

/*校准时间配置*/
#define IMU_TRIM_TIME 10

#if TCFG_MPU6887P_ENABLE
/*icm42607p*/
static info_spa_t info_spa = {
    .fs = 99.f,
    .len = 6,
    .sensitivity = 16.4f,
    .range = 16,//+-16g量程
};

static spatial_config_t conf = {
    .beta = 0.1f,
    .cel_val = 0.14f,
    .SerialTime = 0.2f,
    .time = 1.0f,
    .val = 0.02f,
    .sensval = 0.01f,
};
#elif TCFG_MPU6887P_ENABLE
/*mpu6887*/
static info_spa_t info_spa = {
    .fs = 100.f,
    .len = 6,
    .sensitivity = 16.4f,
    .range = 16,//+-16g量程
};

static spatial_config_t conf = {
    .beta = 0.1f,
    .cel_val = 0.08f,
    .SerialTime = 0.2f,
    .time = 1.0f,
    .val = 0.02f,
    .sensval = 0.01f,
};
#else
/*lsm6dsl*/
static info_spa_t info_spa = {
    .fs = 104.f,
    .len = 6,
    .sensitivity = 14.28f,
    .range = 16,
};

static spatial_config_t conf = {
    .beta = 0.1f,
    .cel_val = 0.12f,
    .SerialTime = 0.24f,
    .time = 1.f,
    .val = 0.02f,
    .sensval = 0.01f
};
#endif

static Thval_t Thval = {
    .thv1 = 0.85f,
    .thv2 = 1.2f,
};
//以上参数取值以实际为例

typedef struct {
    acc_cel_t acc_cel;
    char *Accbuf;
    gyro_cel_t gyro_cel;
    char *Gyrobuf;
    tranval_t tranval;
    s16 in_buf[512];
    void *sensor;
} imu_trim_hdl_t;
static imu_trim_hdl_t *imu_trim_hdl = NULL;

/*获取校准传感器的状态
 *  0：未校准
 *  1：校准成功
 * -1：校准中
 * -2：校准失败
 */
static int global_imu_trim_state = 0;
int get_imu_trim_state()
{
    return global_imu_trim_state;
}

typedef struct {
    u8 cmd;           //命令
    u8 magic_code[4]; //"csmr"
    u8 data_len;      //LTV结构总长度
    u8 len0;          //L : 数据长度
    u8 type0;         //T : 数据类型
    u8 diaplay[20];   //V : 数据内容
    u8 len1;          //L : 数据长度
    u8 type1;         //T : 数据类型
    u8 notice;        //V : 数据内容
} testbox_data_hdl_t;

static testbox_data_hdl_t testbox_data  = {

    .cmd = 0x01,
    /* .magic_code[] = "csmr",  */
    .data_len = 25,
    .len0 = 21,
    .type0 = 0x01,   //显示内容
    /* .diaplay[20] = " ", */
    .len1 = 2,
    .type1 = 0x02,   //提示音
    .notice = 0,
};

static u8 notice_flag = 1;
int testbox_imu_trim_run(u8 *send_buf)
{
    int state = get_imu_trim_state();
    char csmr[] = "csmr";
    char imu_trim_start[] = " trim start !";
    char imu_trim_fail[] = " trim fail !";
    char imu_trim_succ[] = " trim succ !";
    char imu_trim_run[]  = " trim run...";

    /* printf("state %d", state); */
    memcpy(testbox_data.magic_code, csmr, 4);
    if (state == 0) {
        /*还没有校准时，校准一遍*/
        spatial_imu_trim_start();
        memcpy(testbox_data.diaplay, imu_trim_start, sizeof(imu_trim_start));
        testbox_data.notice = 1; //开始校准 ‘di’一声
    } else if (state == 1) {
        /*校准成功了，不再做校准了*/
        memcpy(testbox_data.diaplay, imu_trim_succ, sizeof(imu_trim_succ));
        if (notice_flag) {
            notice_flag = 0;
            testbox_data.notice = 2; //校准成功 ‘didi’两声
        } else {
            testbox_data.notice = 0;
        }

    } else if (state == -1) {
        /*正在校准中*/
        putchar('.');
        memcpy(testbox_data.diaplay, imu_trim_run, sizeof(imu_trim_run));
        testbox_data.notice = 0;
    } else if (state == -2) {
        /*校准失败*/
        memcpy(testbox_data.diaplay, imu_trim_fail, sizeof(imu_trim_fail));
        if (notice_flag) {
            notice_flag = 0;
            testbox_data.notice = 3; //校准失败 ‘dididi’三声
        } else {
            testbox_data.notice = 0;
        }
    }

    memcpy(&send_buf[2], &testbox_data, sizeof(testbox_data_hdl_t));
    return sizeof(testbox_data_hdl_t);
}

void testbox_trim_flag_reset()
{
    global_imu_trim_state = 0;
    notice_flag = 1;
}


/*校准资源初始化*/
static int spatial_imu_trim_init()
{

    printf("%sn", __func__);
    if (imu_trim_hdl) {
        printf("imu trim is alreadly init !!!");
        return -1;
    }
    imu_trim_hdl = zalloc(sizeof(imu_trim_hdl_t));
    if (imu_trim_hdl == NULL) {
        printf("imu_trim_hdl zalloc fail !!!");
        return -1;
    }

    int Accsize = AccCelBuff();//加速度计校准buf大小
    printf("AccCelBuff size %d", Accsize);
    imu_trim_hdl->Accbuf = (char *)zalloc(Accsize);
    AccCelInit(imu_trim_hdl->Accbuf, &info_spa, IMU_TRIM_TIME);//加速度计校准参数初始化

    int Gyrosize = GroCelBuff();//陀螺仪计校准buf大小
    printf("GroCelBuff size %d", Accsize);
    imu_trim_hdl->Gyrobuf = (char *)zalloc(Gyrosize);
    GroCelInit(imu_trim_hdl->Gyrobuf, &info_spa, IMU_TRIM_TIME);//校准参数初始化

    /*imu初始化*/
    if (imu_trim_hdl->sensor == NULL) {
        imu_trim_hdl->sensor = space_motion_detect_open();
    }
    if (imu_trim_hdl->sensor == NULL) {
        printf("sensor open fail !!!");
        return -1;
    }

    global_imu_trim_state = -1;//校准中
    return 0;
}

/*重启传感器，等待数据稳定 mpu6887p*/
static int spatial_imu_restart()
{
    if (imu_trim_hdl) {
        if (imu_trim_hdl->sensor) {
            os_time_dly(10);
            space_motion_detect_close(imu_trim_hdl->sensor);
            imu_trim_hdl->sensor = NULL;
        }
        /*imu初始化*/
        if (imu_trim_hdl->sensor == NULL) {
            os_time_dly(10);
            imu_trim_hdl->sensor = space_motion_detect_open();
        }
    }

    return 0;
}

/*释放校准资源*/
static int spatial_imu_trim_exit()
{
    printf("%s\n", __func__);
    if (imu_trim_hdl) {
        if (imu_trim_hdl->Accbuf) {
            free(imu_trim_hdl->Accbuf);
            imu_trim_hdl->Accbuf = NULL;
        }

        if (imu_trim_hdl->Gyrobuf) {
            free(imu_trim_hdl->Gyrobuf);
            imu_trim_hdl->Gyrobuf = NULL;
        }
        if (imu_trim_hdl->sensor) {
            space_motion_detect_close(imu_trim_hdl->sensor);
            imu_trim_hdl->sensor = NULL;
        }
        free(imu_trim_hdl);
        imu_trim_hdl = NULL;
    }
    return 0;
}

extern void put_float(double fv);
/*校准传感器*/
static int spatial_imu_trim_acc_gyro()
{
    printf("%s\n", __func__);

    if (imu_trim_hdl == NULL) {
        return 0;
    }
    int data_len = 0;
    int i = 0;
    int flag_A = 0;
    int flag_G = 0;
    int mode = 0;
    global_imu_trim_state = -1;//校准中
    do {
        i = 0;
        os_time_dly(10);
        data_len = space_motion_data_read(imu_trim_hdl->sensor, imu_trim_hdl->in_buf, 1024);
        /* printf("data_len %d", data_len); */
        putchar('.');
        while (data_len) {
            data_len -= 12;
            flag_A = Acc_Calibration(imu_trim_hdl->Accbuf,
                                     &imu_trim_hdl->in_buf[i],
                                     &info_spa,
                                     &imu_trim_hdl->acc_cel,
                                     &Thval);
            if (flag_A == 1) {
                printf("imu_trim_acc succ!!!");
                printf("acc_cel:x:");
                put_float(imu_trim_hdl->acc_cel.acc_offx);
                printf("acc_cel:y:");
                put_float(imu_trim_hdl->acc_cel.acc_offy);
                printf("acc_cel:z:");
                put_float(imu_trim_hdl->acc_cel.acc_offz);
                int ret = syscfg_write(CFG_IMU_ACC_OFFEST_ID, &imu_trim_hdl->acc_cel, sizeof(acc_cel_t));
                if (ret != sizeof(acc_cel_t)) {
                    printf("acc_cel write vm fail %d !!!", ret);
                }
                /* return 0; */
            } else if (flag_A == -2) {
                printf("imu_trim_acc fail!!!");
                global_imu_trim_state = -2;//校准失败
                return -2;
            }

            flag_G = Gro_Calibration(imu_trim_hdl->Gyrobuf,
                                     &imu_trim_hdl->in_buf[i],
                                     &info_spa,
                                     &imu_trim_hdl->tranval,
                                     mode,
                                     &imu_trim_hdl->gyro_cel);//计算mode=0；完成陀螺仪校准，完成返回0，否则返回-1

            if (flag_G == mode) {
                printf("imu_trim_gyro succ!!!");
                printf("gyro_cel:x:");
                put_float(imu_trim_hdl->gyro_cel.gyro_x);
                printf("gyro_cel:y:");
                put_float(imu_trim_hdl->gyro_cel.gyro_y);
                printf("gyro_cel:z:");
                put_float(imu_trim_hdl->gyro_cel.gyro_z);

                int ret = syscfg_write(CFG_IMU_GYRO_OFFEST_ID, &imu_trim_hdl->gyro_cel, sizeof(gyro_cel_t));
                if (ret != sizeof(gyro_cel_t)) {
                    printf("gyro_cel write vm fail %d !!!", ret);
                }

                global_imu_trim_state = 1;//校准成功
                return 0;
            } else if (flag_G == -2) {
                printf("imu_trim_gyro fail!!!");
                global_imu_trim_state = -2;//校准失败
                return -2;
            }
            global_imu_trim_state = -1;//校准中

            i += 6;
        }
    } while (1);
    return 0;
}

/*开始校准*/
int spatial_imu_trim_start()
{
    struct sys_event e;
    e.type = SYS_AUD_EVENT;
    e.arg  = (void *)AUDIO_EVENT_TRIM_IMU_START;
    sys_event_notify(&e);
}

/*关闭校准*/
int spatial_imu_trim_stop()
{
    struct sys_event e;
    e.type = SYS_AUD_EVENT;
    e.arg  = (void *)AUDIO_EVENT_TRIM_IMU_STOP;
    sys_event_notify(&e);
}

/*校准任务*/
static void spatial_imu_trim_task(void *p)
{
    int ret = 0;
    /*初始化传感器*/
    ret = spatial_imu_trim_init();
    if (ret != 0) {
        global_imu_trim_state = -2;//校准失败
        goto err0;
    }
    /*等待传感器数据稳定*/
    spatial_imu_restart();
    /*校准传感器*/
    ret = spatial_imu_trim_acc_gyro();
    /*判断是否校准成功，0：校准成功，-2：校准失败*/
    if (ret == -2) {
        global_imu_trim_state = -2;//校准失败
    }
err0 :
    /*关闭传感器*/
    spatial_imu_trim_exit();
    /*关闭校准任务*/
    spatial_imu_trim_stop();
}

/*创建校准任务*/
int spatial_imu_trim_task_start()
{
    int err = 0;
    err = task_create(spatial_imu_trim_task, imu_trim_hdl, "imu_trim");
    if (err != OS_NO_ERR) {
        printf("imu_trim task create fail : %d !!!", err);
    }
    return err;
}

/*删除校准任务和校准资源*/
int spatial_imu_trim_task_stop()
{
    int err = 0;
    err = task_kill("imu_trim");
    if (err != OS_NO_ERR) {
        printf("imu_trim task kill fail : %d !!!", err);
    }
    /*释放校准资源*/
    spatial_imu_trim_exit();
    return err;
}

static u8 spatial_imu_trim_idle_query(void)
{
    return ((imu_trim_hdl == NULL) ? 1 : 0);
}

REGISTER_LP_TARGET(spatial_imu_trim_lp_target) = {
    .name = "spatial_imu_trim",
    .is_idle = spatial_imu_trim_idle_query,
};
#endif /*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/
