/*****************************************************************
>file name : spatial_audio.c
>create time : Mon 03 Jan 2022 01:58:51 PM CST
*****************************************************************/
#include "typedef.h"
#include "spatial_effect_imu.h"
#include "system/includes.h"
#include "audio_base.h"
#include "app_config.h"
#include "audio_codec_clock.h"
#include "spatial_effect_tws.h"

#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
#include "spatial_effect.h"

#if SPATIAL_AUDIO_EFFECT_ENABLE
#include "tech_lib/effect_surTheta_api.h"
#include "tech_lib/SpatialAudio_api.h"
#endif

const int  voicechange_fft_PLATFORM = 2;

/*T0模式下有效
0：环境声更明显
1：最左最右的时候音量差异更明显*/
const int OLD_V_FLAG = 1;/*0 or 1*/
/*角度的映射音量密度
OLD_V_FLAG = 1时有效*/
const int P360_MRATE = 128;/*0 ~ 128*/

/*空间音频模式使能控制变量
 * 0: 关闭音效，保留角度跟踪
 * 1: 打开音效和角度跟踪
 * 2: 轻量级音效和角度跟踪*/
const int P360_REVERB_NE = 1;

static struct spatial_audio_context *spatial_hdl = NULL;

struct sound360td_algo {
    int angle;
    const PointSound360TD_FUNC_API *ops;
    unsigned int run_buf[0];
};

struct spatial_calculator {
    void *privtate_data;
    u8 work_buf[0];
};
int  anglevolume = 150;
int  angleresetflag = 0;

static RP_PARM_CONIFG parmK = {
    .trackKIND = P360_T0,
    .ReverbKIND = P360_R1,
    .reverbance = 70,
    .dampingval = 70,
};
static u8 param_update_flag = 0;

#if TCFG_SPATIAL_EFFECT_ONLINE_ENABLE
void spatial_effect_online_updata(RP_PARM_CONIFG *params)
{
    memcpy(&parmK, params, sizeof(parmK));
    param_update_flag = 1;
}
void get_spatial_effect_reverb_params(RP_PARM_CONIFG *params)
{
    memcpy(params, &parmK, sizeof(parmK));
}

#endif /*TCFG_SPATIAL_EFFECT_ONLINE_ENABLE*/

static void *spatial_audio_effect_init(void)
{
    struct sound360td_algo *algo = NULL;

    PointSound360TD_FUNC_API *ops = get_PointSound360TD_func_api();
    PointSound360TD_PARM_SET params;
    RP_PARM_CONIFG  rp_obj;

    angleresetflag = 1;

    /*音效参数初始化*/
    memcpy(&rp_obj, &parmK, sizeof(parmK));
    param_update_flag = 0;

    params.theta = 0;
    params.volume = anglevolume;

    int buf_size = ops->need_buf(P360TD_REV_K1);

    algo = (struct sound360td_algo *)malloc(sizeof(struct sound360td_algo) + buf_size);
    if (!algo) {
        return NULL;
    }

    algo->ops = ops;
    algo->ops->open_config(algo->run_buf, &params, &rp_obj);
    algo->angle = 0;

    return algo;
}

static int spatial_audio_effect_handler(void *effect, void *data, int len)
{
    struct sound360td_algo *algo = (struct sound360td_algo *)effect;
    int frames = len >> 2;

#if TCFG_SPATIAL_EFFECT_ONLINE_ENABLE
    if (param_update_flag) {
        param_update_flag = 0;
        PointSound360TD_PARM_SET params;
        params.theta = 0;
        params.volume = anglevolume;
        algo->ops->open_config(algo->run_buf, &params, &parmK);
    }
#endif /*TCFG_SPATIAL_EFFECT_ONLINE_ENABLE*/

    algo->ops->run(algo->run_buf, data, data, frames);

    return len;
}

static void spatial_audio_effect_close(void *effect)
{
    struct sound360td_algo *algo = (struct sound360td_algo *)effect;

    free(algo);
}

/*
 * 空间效果参数配置接口
 */
static void spatial_audio_effect_params_setup(void *effect, int angle)
{
    struct sound360td_algo *algo = (struct sound360td_algo *)effect;

    if (angle != algo->angle) {
        PointSound360TD_PARM_SET params;
        //TODO
        params.theta = angle;
        params.volume = 100;
        params.volume = anglevolume;
        algo->ops->init(algo->run_buf, &params);
        algo->angle = angle;
    }

}

extern void put_float(double fv);
/*获取vm加速度计偏置*/
int read_vm_acc_cel_param(acc_cel_t *acc_cel)
{
    int ret = 0;
    ret = syscfg_read(CFG_IMU_ACC_OFFEST_ID, acc_cel, sizeof(acc_cel_t));
    if (ret != sizeof(acc_cel_t)) {
        printf("vm acc_cel read fail !!!");
        acc_cel->acc_offx = -48.231f;
        acc_cel->acc_offy = -57.035f;
        acc_cel->acc_offz = -84.097f;
    } else {
        printf("vm acc_cel read succ !!!");
    }
    printf("acc_offx : ");
    put_float(acc_cel->acc_offx);
    printf("acc_offy : ");
    put_float(acc_cel->acc_offy);
    printf("acc_offz : ");
    put_float(acc_cel->acc_offz);
    return 0;
}

/*获取vm陀螺仪偏置*/
int read_vm_gyro_cel_param(gyro_cel_t *gyro_cel)
{
    int ret = 0;
    ret = syscfg_read(CFG_IMU_GYRO_OFFEST_ID, gyro_cel, sizeof(gyro_cel_t));
    if (ret != sizeof(gyro_cel_t)) {
        printf("vm gyro_cel read fail !!!");
        gyro_cel->gyro_x = -1.694f;
        gyro_cel->gyro_y = -0.521f;
        gyro_cel->gyro_z = -0.078f;
    } else {
        printf("vm gyro_cel read succ !!!");
    }
    printf("gyro_x : ");
    put_float(gyro_cel->gyro_x);
    printf("gyro_y : ");
    put_float(gyro_cel->gyro_y);
    printf("gyro_z : ");
    put_float(gyro_cel->gyro_z);
    return 0;
}

/*配置传感器算法参数*/
int space_motion_param_init(info_spa_t *info_spa, spatial_config_t *conf, tranval_t *tranval)
{
#if TCFG_ICM42670P_ENABLE
    if (info_spa) {
        info_spa->fs = 99.f;
        info_spa->len = 6;
        info_spa->sensitivity = 16.4f;
        info_spa->range = 16;
    }
    if (conf) {
        conf->beta = 0.1f;
        conf->cel_val = 0.14f;
        conf->SerialTime = 0.2f;
        conf->time = 1.0f;
        conf->val = 0.02f;
        conf->sensval = 0.02f;
    }
    if (tranval) {
        tranval->trans_x[0] =  0;
        tranval->trans_x[1] =  1;
        tranval->trans_x[2] =  0;
        tranval->trans_y[0] =  0;
        tranval->trans_y[1] =  0;
        tranval->trans_y[2] = -1;
        tranval->trans_z[0] = -1;
        tranval->trans_z[1] =  0;
        tranval->trans_z[2] =  0;
    }

#elif TCFG_LSM6DSL_ENABLE
    if (info_spa) {
        info_spa->fs = 104.f;
        info_spa->len = 6;
        info_spa->sensitivity = 14.28f;
        info_spa->range = 16;
    }
    if (conf) {
        conf->beta = 0.1f;
        conf->cel_val = 0.12f;
        conf->SerialTime = 0.24f;
        conf->time = 1.f;
        conf->val = 0.02f;
        conf->sensval = 0.01f;
    }
    if (tranval) {
        tranval->trans_x[0] =  0;
        tranval->trans_x[1] =  1;
        tranval->trans_x[2] =  0;
        tranval->trans_y[0] =  0;
        tranval->trans_y[1] =  0;
        tranval->trans_y[2] = -1;
        tranval->trans_z[0] = -1;
        tranval->trans_z[1] =  0;
        tranval->trans_z[2] =  0;
    }

#elif TCFG_MPU6887P_ENABLE
    if (info_spa) {
        info_spa->fs = 100.f;
        info_spa->len = 6;
        info_spa->sensitivity = 16.4f;
        info_spa->range = 16;
    }
    if (conf) {
        conf->beta = 0.1f;
        conf->cel_val = 0.12f;
        conf->SerialTime = 0.2f;
        conf->time = 1.0f;
        conf->val = 0.07f;
        conf->sensval = 0.01f;
    }
    if (tranval) {
        tranval->trans_x[0] = -1;
        tranval->trans_x[1] =  0;
        tranval->trans_x[2] =  0;
        tranval->trans_y[0] =  0;
        tranval->trans_y[1] =  0;
        tranval->trans_y[2] = -1;
        tranval->trans_z[0] =  0;
        tranval->trans_z[1] = -1;
        tranval->trans_z[2] =  0;
    }

#else /*TCFG_MPU6050_EN*/
    if (info_spa) {
        info_spa->fs = 100.f;
        info_spa->len = 6;
        info_spa->sensitivity = 16.4f;
        info_spa->range = 16;
    }
    if (conf) {
        conf->beta = 0.1f;
        conf->cel_val = 0.08f;
        conf->SerialTime = 0.2f;
        conf->time = 1.0f;
        conf->val = 0.02f;
        conf->sensval = 0.01f;
    }
    if (tranval) {
        tranval->trans_x[0] = -1;
        tranval->trans_x[1] =  0;
        tranval->trans_x[2] =  0;
        tranval->trans_y[0] =  0;
        tranval->trans_y[1] =  0;
        tranval->trans_y[2] = -1;
        tranval->trans_z[0] =  0;
        tranval->trans_z[1] = -1;
        tranval->trans_z[2] =  0;
    }

#endif
    return 0;
}

static void *space_motion_calculator_open(void)
{
    struct spatial_calculator *c = NULL;

    //传感器算法参数初始化
    info_spa_t info_spa;
    spatial_config_t conf;
    tranval_t tranval;
    space_motion_param_init(&info_spa, &conf, &tranval);
    //陀螺仪偏置
    gyro_cel_t gyro_cel;
    read_vm_gyro_cel_param(&gyro_cel);
    //加速度偏置
    acc_cel_t acc_cel;
    read_vm_acc_cel_param(&acc_cel);

    int buf_size = get_Spatial_buf(info_spa.len);

    c = (struct spatial_calculator *)zalloc(sizeof(struct spatial_calculator) + buf_size);
    if (!c) {
        return NULL;
    }

    init_Spatial(c->work_buf, &info_spa, &tranval, &conf, &gyro_cel, &acc_cel);

    return c;
}

static void space_motion_calculator_close(void *calculator)
{
    if (calculator) {
        free(calculator);
    }
}

/*
 * 空间位置检测处理
 */
static int space_motion_detect(void *calculator, short *data, int len)
{
    struct spatial_calculator *c = (struct spatial_calculator *)calculator;
    int heading = 0;

    if (len == 0) {
        return 0;
    }
    /*
     * TODO : 这里是空间运动检测处理
     */
    if (angleresetflag) {
        Spatial_reset(c->work_buf);
        angleresetflag = 0;
    }

    int group_num = (len >> 1) / 6;
    while (group_num--) {
        Spatial_cacl(c->work_buf, data);
        data += 6;
        heading = get_Spa_angle(c->work_buf, TRACK_SENSITIVITY, ANGLE_RESET_SENSITIVITY);/*1 ~ 0.001，1：表示即时跟踪，数值越小跟踪越慢*/
        int flag = Spatial_stra(c->work_buf, ANGLE_RESET_TIME, 0.5f);
        if (flag == 1) {
            Spatial_reset(c->work_buf);
            flag = 0;
        }
    }
    /* printf("%d\n", heading); */
    return heading;

}

#if TCFG_USER_TWS_ENABLE
static void spatial_tws_data_handler(void *priv, void *data, int len)
{
    struct spatial_audio_context *ctx = (struct spatial_audio_context *)priv;
    memcpy(&ctx->tws_angle, data, sizeof(ctx->tws_angle));
}
#endif

extern u8 get_a2dp_spatial_audio_head_tracked(void);
void *spatial_audio_open(void)
{
    struct spatial_audio_context *ctx;

#if SPATIAL_AUDIO_EFFECT_ENABLE
    ctx = (struct spatial_audio_context *)zalloc(sizeof(struct spatial_audio_context) + 1024);
#else
    ctx = (struct spatial_audio_context *)zalloc(sizeof(struct spatial_audio_context));
#endif
    if (!ctx) {
        return NULL;
    }
    spatial_hdl = ctx;

    audio_codec_clock_set(SPATIAL_EFFECT_MODE, AUDIO_CODING_PCM, 0);

    ctx->head_tracked = get_a2dp_spatial_audio_head_tracked();
    if (ctx->head_tracked) {
        ctx->sensor = space_motion_detect_open();
    }
#if SPATIAL_AUDIO_EFFECT_ENABLE
    ctx->effect = spatial_audio_effect_init();
    ctx->calculator = space_motion_calculator_open();
#endif
#if TCFG_USER_TWS_ENABLE
    ctx->tws_conn = spatial_tws_create_connection(100, (void *)ctx, spatial_tws_data_handler);
#endif

#if SPATIAL_AUDIO_EXPORT_DATA
#if SPATIAL_AUDIO_EXPORT_MODE == 0
    extern void force_set_sd_online(char *sdx);
    force_set_sd_online("sd0");
    void *mnt = mount("sd0", "storage/sd0", "fat", 3, NULL);
    if (!mnt) {
        printf("sd0 mount fat failed.\n");
    }
#endif
    task_create(audio_export_task, (void *)ctx, "ftask");
    os_taskq_post_msg("ftask", 1, 0);
#endif

    /*空间音频正常的声道映射为左右声道*/
    ctx->mapping_channel = AUDIO_CH_LR;
    return ctx;
}

int spatial_audio_set_mapping_channel(void *sa, u8 channel)
{
    struct spatial_audio_context *ctx = (struct spatial_audio_context *)sa;

    ctx->mapping_channel = channel;

    return 0;
}

void spatial_audio_close(void *sa)
{
    struct spatial_audio_context *ctx = (struct spatial_audio_context *)sa;

    if (ctx->sensor) {
        space_motion_detect_close(ctx->sensor);
    }
#if SPATIAL_AUDIO_EFFECT_ENABLE
    if (ctx->effect) {
        spatial_audio_effect_close(ctx->effect);
    }
    if (ctx->calculator) {
        space_motion_calculator_close(ctx->calculator);
    }
#endif

#if TCFG_USER_TWS_ENABLE
    if (ctx->tws_conn) {
        spatial_tws_delete_connection(ctx->tws_conn);
    }
#endif

#if SPATIAL_AUDIO_EXPORT_DATA
    OS_SEM *sem = (OS_SEM *)malloc(sizeof(OS_SEM));
    os_sem_create(sem, 0);
    os_taskq_post_msg("ftask", 2, 1, (int)sem);
    os_sem_pend(sem, 0);
    free(sem);
    task_kill("ftask");
#endif
    free(ctx);
    spatial_hdl = NULL;
    audio_codec_clock_del(SPATIAL_EFFECT_MODE);
}

static int spatial_audio_remapping_data_handler(struct spatial_audio_context *ctx, void *data, int len)
{
    int pcm_frames = (len >> 2);
    s16 *pcm_buf = (s16 *)data;
    int i = 0;
    int tmp = 0;

    switch (ctx->mapping_channel) {
    case AUDIO_CH_L:
        for (i = 0; i < pcm_frames; i++) {
            pcm_buf[i] = pcm_buf[i * 2];
        }
        len /= 2;
        break;
    case AUDIO_CH_R:
        for (i = 0; i < pcm_frames; i++) {
            pcm_buf[i] = pcm_buf[i * 2 + 1];
        }
        len /= 2;
        break;
    case AUDIO_CH_MIX_MONO:
        for (i = 0; i < pcm_frames; i++) {
            tmp = pcm_buf[i * 2] + pcm_buf[i * 2 + 1];
            pcm_buf[i] = tmp / 2;
        }
        len /= 2;
        break;
    default:
        break;
    }

    return len;
}

int spatial_audio_space_data_read(void *data)
{
    int data_len = 0;
    if (spatial_hdl) {
        void *sensor = spatial_hdl->sensor;
        data_len = space_motion_data_read(sensor, data, 1024);
    }
    return data_len;
}

extern int aud_spatial_sensor_get_data_len();
extern int aud_spatial_sensor_data_read(s16 *data, int len);
static int spatial_audio_data_handler(struct spatial_audio_context *ctx, void *data, int len)
{
    int data_len = 0;
    if (ctx->sensor) {
#if TCFG_SENSOR_DATA_READ_IN_DEC_TASK
        data_len = aud_spatial_sensor_get_data_len();
        if (data_len) {
            data_len = aud_spatial_sensor_data_read(ctx->data, data_len);
        }

#else
#if !SPATIAL_AUDIO_EXPORT_DATA
        data_len = space_motion_data_read(ctx->sensor, ctx->data, 1024);
#endif /*SPATIAL_AUDIO_EXPORT_DATA*/
#endif /*TCFG_SENSOR_DATA_READ_IN_DEC_TASK*/
    }

    if (!ctx->calculator) {
        return 0;
    }

    int angle = 0;
    if (ctx->head_tracked == 0) {
        angle = 0;
    } else {
#if SPATIAL_AUDIO_EXPORT_DATA
        angle = space_motion_detect(ctx->calculator, (s16 *)ctx->space_fifo_buf, ctx->space_data_single_len);
#else
        angle = space_motion_detect(ctx->calculator, (s16 *)ctx->data, data_len);
#endif
    }

#if TCFG_USER_TWS_ENABLE
    if (ctx->tws_conn && data_len) {
        spatial_tws_audio_data_sync(ctx->tws_conn, (void *)&angle, sizeof(angle));
    }
#endif

    if (ctx->effect) {
        spatial_audio_effect_handler(ctx->effect, data, len);
#if (TCFG_SPATIAL_EFFECT_ONLINE_ENABLE)
        extern int set_bt_media_imu_angle(int angle);
#if TCFG_USER_TWS_ENABLE
        set_bt_media_imu_angle(ctx->tws_angle);
#else
        if (data_len) {
            set_bt_media_imu_angle(angle);
        }
#endif /*TCFG_USER_TWS_ENABLE*/
#endif /*TCFG_SPATIAL_EFFECT_ONLINE_ENABLE*/
        if (ctx->sensor) { /*处理一只耳机有传感器一只耳机没有传感器的情况*/
            /*有传感器的时候需要判读有传感器数据才更新角度到算法*/
            if (data_len) {
#if TCFG_USER_TWS_ENABLE
                spatial_audio_effect_params_setup(ctx->effect, ctx->tws_angle);
#else
                spatial_audio_effect_params_setup(ctx->effect, angle);
#endif
            }
        } else {
            /*没有传感器的时候，直接使用对耳角度*/
#if TCFG_USER_TWS_ENABLE
            spatial_audio_effect_params_setup(ctx->effect, ctx->tws_angle);
#else
            spatial_audio_effect_params_setup(ctx->effect, angle);
#endif
        }
    }

    return spatial_audio_remapping_data_handler(ctx, data, len);
}

void spatial_audio_head_tracked_en(struct spatial_audio_context *ctx, u8 en)
{
    printf("head_tracked = %d\n", en);
    ctx->head_tracked = en;
}

u8 get_spatial_audio_head_tracked(struct spatial_audio_context *ctx)
{
    printf("head_tracked = %d\n", ctx->head_tracked);
    return ctx->head_tracked;
}

int spatial_audio_filter(void *sa, void *data, int len)
{
    struct spatial_audio_context *ctx = (struct spatial_audio_context *)sa;

    if (!len) {
        return 0;
    }

#if SPATIAL_AUDIO_EXPORT_DATA
    if (ctx->export) {
        struct data_export_header header;
        ctx->audio_data_len += len;

        header.magic = 0x5A;
        header.ch = 0;
        header.seqn = ctx->audio_seqn++;
        header.len = len;
        header.timestamp = jiffies;
        header.crc = CRC16(data, len);
        header.total_len = ctx->audio_data_len;

        if (cbuf_is_write_able(&ctx->audio_cbuf, sizeof(header) + len)) {
            cbuf_write(&ctx->audio_cbuf, &header, sizeof(header));
            cbuf_write(&ctx->audio_cbuf, data, len);
        } else {
            printf("--- audio data export error --\n");
        }

        if (ctx->sensor) {
            int data_len = 1024;
            /*putchar('1');*/
            data_len = space_motion_data_read(ctx->sensor, ctx->space_fifo_buf, data_len);
            /*putchar('2');*/
            if (data_len) {
                ASSERT(data_len % 12 == 0, " , data_len : %d\n", data_len);
                ctx->space_data_len += data_len;
                ctx->space_data_single_len = data_len;

                header.magic = 0x5A;
                header.ch = 1;
                header.seqn = ctx->space_seqn++;
                header.len = data_len;
                header.timestamp = jiffies;
                header.crc = CRC16(ctx->space_fifo_buf, data_len);
                header.total_len = ctx->space_data_len;
#if 0
                if (cbuf_is_write_able(&ctx->space_cbuf, sizeof(header) + data_len)) {
                    cbuf_write(&ctx->space_cbuf, &header, sizeof(header));
                    cbuf_write(&ctx->space_cbuf, ctx->space_fifo_buf, data_len);
                } else {
                    printf("--- space data export error --\n");
                }
#else
                if (cbuf_is_write_able(&ctx->audio_cbuf, sizeof(header) + data_len)) {
                    cbuf_write(&ctx->audio_cbuf, &header, sizeof(header));
                    cbuf_write(&ctx->audio_cbuf, ctx->space_fifo_buf, data_len);
                    /*
                    s16 *print_data = ctx->space_fifo_buf;
                    data_len /= 2;
                    printf("[%d, %d, %d]\n", print_data[data_len - 3], print_data[data_len - 2], print_data[data_len - 1]);
                    */
                } else {
                    printf("--- space data export error --\n");
                }
#endif
            }
        }
#if SPATIAL_AUDIO_EFFECT_ENABLE	//增加处理后的数据
        spatial_audio_data_handler(ctx, data, len);
        header.magic = 0x5A;
        header.ch = 2;
        header.seqn = ctx->audio_out_seqn++;
        header.len = len;
        header.timestamp = jiffies;
        header.crc = CRC16(data, len);
        header.total_len = ctx->audio_data_len;

        if (cbuf_is_write_able(&ctx->audio_cbuf, sizeof(header) + len)) {
            cbuf_write(&ctx->audio_cbuf, &header, sizeof(header));
            cbuf_write(&ctx->audio_cbuf, data, len);
        } else {
            printf("--- audio data export error --\n");
        }
#endif/*SPATIAL_AUDIO_EFFECT_ENABLE*/
        os_taskq_post_msg("ftask", 1, 2);
    }
#elif SPATIAL_AUDIO_EFFECT_ENABLE
    return spatial_audio_data_handler(ctx, data, len);
#endif
    return len;
}

#if SPATIAL_AUDIO_EXPORT_DATA

static int spatial_audio_export_init(struct spatial_audio_context *ctx)
{
    int audio_buf_size = 40 * 1024;
    int space_buf_size = 8 * 1024;
    ctx->space_fifo_buf = (u8 *)malloc(1024);
    ctx->audio_buf = (u8 *)malloc(audio_buf_size);
    /*ctx->space_buf = (u8 *)malloc(space_buf_size);*/
    cbuf_init(&ctx->audio_cbuf, ctx->audio_buf, audio_buf_size);
    /*cbuf_init(&ctx->space_cbuf, ctx->space_buf, space_buf_size);*/

#if SPATIAL_AUDIO_EXPORT_MODE == 0
    ctx->audio_file = fopen("storage/sd0/C/spatial_audio/aud***.raw", "w+");
    /*ctx->space_file = fopen("storage/sd0/C/spatial_audio/spa***.raw", "w+"); */
#elif SPATIAL_AUDIO_EXPORT_MODE == 1
    aec_uart_init();
#endif
    ctx->export = 1;
    return 0;
}

static void spatial_audio_export_release(struct spatial_audio_context *ctx)
{
    ctx->export = 0;

    if (ctx->audio_buf) {
        free(ctx->audio_buf);
        ctx->audio_buf = NULL;
    }

    if (ctx->space_buf) {
        free(ctx->space_buf);
        ctx->space_buf = NULL;
    }

    if (ctx->space_fifo_buf) {
        free(ctx->space_fifo_buf);
        ctx->space_fifo_buf = NULL;
    }

#if SPATIAL_AUDIO_EXPORT_MODE == 0
    if (ctx->audio_file) {
        fclose(ctx->audio_file);
    }

    if (ctx->space_file) {
        fclose(ctx->space_file);
    }
#elif SPATIAL_AUDIO_EXPORT_MODE == 1
    aec_uart_close();
#endif
}

static int audio_data_export_handler(struct spatial_audio_context *ctx)
{
    int err = -EINVAL;
#if SPATIAL_AUDIO_EXPORT_MODE == 0
    if (ctx->audio_file) {
        if (cbuf_get_data_len(&ctx->audio_cbuf) >= 512) {
            fwrite(ctx->audio_file, cbuf_get_readptr(&ctx->audio_cbuf), 512);
            cbuf_read_updata(&ctx->audio_cbuf, 512);
            err = 0;
        } else {
            //TODO
        }
    }

#if 0
    if (ctx->space_file) {
        if (cbuf_get_data_len(&ctx->space_cbuf) >= 512) {
            fwrite(ctx->space_file, cbuf_get_readptr(&ctx->space_cbuf), 512);
            cbuf_read_updata(&ctx->space_cbuf, 512);
            err = 0;
        } else {
            //TODO
        }
    }
#endif
#elif SPATIAL_AUDIO_EXPORT_MODE == 1
    u8 send = 0;
    if (cbuf_get_data_len(&ctx->audio_cbuf) >= 512) {
        aec_uart_fill(0, cbuf_get_readptr(&ctx->audio_cbuf), 512);
        cbuf_read_updata(&ctx->audio_cbuf, 512);
        err = 0;
        send = 1;
    }

    if (!send && cbuf_get_data_len(&ctx->space_cbuf) >= 512) {
        aec_uart_fill(0, cbuf_get_readptr(&ctx->space_cbuf), 512);
        cbuf_read_updata(&ctx->space_cbuf, 512);
        err = 0;
        send = 1;
    }

    if (send) {
        putchar('t');
        aec_uart_write();
    }
#endif

    return err;
}

void audio_export_task(void *arg)
{
    int msg[16];
    int res;
    int pend_taskq = 1;
    struct spatial_audio_context *ctx = (struct spatial_audio_context *)arg;

    while (1) {
        if (pend_taskq) {
            res = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        } else {
            res = os_taskq_accept(ARRAY_SIZE(msg), msg);
        }

        if (res == OS_TASKQ) {
            switch (msg[1]) {
            case 0:
                spatial_audio_export_init(ctx);
                break;
            case 1:
                spatial_audio_export_release(ctx);
                os_sem_post((OS_SEM *)msg[2]);
                pend_taskq = 1;
                break;
            case 2: //音频数据
                break;
            case 3: //空间移动信息
                break;
            }
        }

        if (ctx->export) {
            if (0 != audio_data_export_handler(ctx)) {
                pend_taskq = 1;
            } else {
                pend_taskq = 0;
            }
        }

    }
}
#endif

#endif/*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/
