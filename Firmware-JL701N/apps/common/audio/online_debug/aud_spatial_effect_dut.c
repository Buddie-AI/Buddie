#include "aud_spatial_effect_dut.h"
#include "audio_online_debug.h"
#include "config/config_interface.h"
#include "app_config.h"
#include "os/os_api.h"
#include "classic/tws_api.h"
#include "timer.h"

#if (defined(TCFG_AUDIO_SPATIAL_EFFECT_ENABLE) && TCFG_AUDIO_SPATIAL_EFFECT_ENABLE && TCFG_SPATIAL_EFFECT_ONLINE_ENABLE)

#define IMU_DATA_SEND_INTERVAL  100
// 配置参数
typedef struct _RP_PARM_CONIFG {
    int trackKIND;      //角度合成算法选择: P360_T0或者P360_T1
    int ReverbKIND;     //混响算法选择：P360_R0或者P360_R1
    int reverbance;     //混响值 : 0到100
    int dampingval;     //高频decay：0到80
} RP_PARM_CONIFG;
static RP_PARM_CONIFG parmK;

extern void spatial_effect_online_updata(RP_PARM_CONIFG *params);
extern void get_spatial_effect_reverb_params(RP_PARM_CONIFG *params);

static u32 send_timer = 0;
static int global_angle = 0;

int set_bt_media_imu_angle(int angle)
{
    global_angle = angle;
    return global_angle;
}

void spatial_effect_imu_dut_data_timer(void *p)
{
    int tmp_angle = 0;
    extern u8 bt_media_is_running(void);
#if TCFG_USER_TWS_ENABLE
    /*TWS连接的时候*/
    if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
        /*主机发送角度*/
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            /*播歌的时候*/
            if (bt_media_is_running()) {
                tmp_angle = global_angle;
                if (tmp_angle > 180) {
                    tmp_angle = tmp_angle - 360;
                }
                int buf[2];
                buf[0] = SPATIAL_EFFECT_IMU_DUT_DATA;
                buf[1] = tmp_angle;
                /* app_online_db_send(DB_PKT_TYPE_SPATIAL_EFFECT, buf, sizeof(buf)); */
                app_online_db_send_more(DB_PKT_TYPE_SPATIAL_EFFECT, buf, sizeof(buf));
            }
        }
    } else
#endif /*TCFG_USER_TWS_ENABLE*/
    {
        /*没有tws连接的时候*/
        if (bt_media_is_running()) {/*发送播歌角度*/
            tmp_angle = global_angle;
            if (tmp_angle > 180) {
                tmp_angle = tmp_angle - 360;
            }
            int buf[2];
            buf[0] = SPATIAL_EFFECT_IMU_DUT_DATA;
            buf[1] = tmp_angle;
            /* app_online_db_send(DB_PKT_TYPE_SPATIAL_EFFECT, buf, sizeof(buf)); */
            app_online_db_send_more(DB_PKT_TYPE_SPATIAL_EFFECT, buf, sizeof(buf));
        }
    }
}

int spatial_effect_imu_dut_start()
{

    if (send_timer == 0) {
        send_timer = usr_timer_add(NULL, spatial_effect_imu_dut_data_timer, IMU_DATA_SEND_INTERVAL, 1);
    }
    return 0;
}

int spatial_effect_imu_dut_stop()
{
    if (send_timer) {
        usr_timer_del(send_timer);
        send_timer = 0;
    }
    return 0;
}


int spatial_effect_online_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size)
{
    put_buf(packet, size);
    int res_data = 0;
    int err = 0;
    u8 parse_seq = ext_data[1];
    int cmd;
    memcpy(&cmd, packet, sizeof(cmd));
    u8 *data = &packet[4];

    printf("[SPATIAL EFFECT]cmd:0x%x\n", cmd);
    switch (cmd) {
    case SPATIAL_EFFECT_REVERB_INFO_QUERY:
        printf("SPATIAL_EFFECT_REVERB_INFO_QUERY\n");
        get_spatial_effect_reverb_params(&parmK);
        err = app_online_db_ack(parse_seq, (u8 *)&parmK, sizeof(parmK));
        break;

    case SPATIAL_EFFECT_REVERB_PARAM:
        printf("SPATIAL_EFFECT_REVERB_PARAM\n");
        memcpy(&parmK, data, sizeof(parmK));

        spatial_effect_online_updata(&parmK);

        printf("%s\n trackKIND:%d, ReverbKIND:%d, reverbance:%d, dampingval:%d\n", __func__,
               parmK.trackKIND, parmK.ReverbKIND, parmK.reverbance, parmK.dampingval);
        err = app_online_db_ack(parse_seq, (u8 *)"OK", 2);
        break;
    case SPATIAL_EFFECT_IMU_TRIM:
        printf("SPATIAL_EFFECT_IMU_TRIM\n");
        static int tmp = -2;

        extern int spatial_imu_trim_start();
        spatial_imu_trim_start();

        /*测试代码*/
        res_data = 0;
        err = app_online_db_ack(parse_seq, (u8 *)&res_data, 4);
        break;
    case SPATIAL_EFFECT_IMU_DUT_START:
        printf("SPATIAL_EFFECT_IMU_DUT_START\n");
        spatial_effect_imu_dut_start();
        err = app_online_db_ack(parse_seq, (u8 *)"OK", 2);
        break;
    case SPATIAL_EFFECT_IMU_DUT_STOP:
        printf("SPATIAL_EFFECT_IMU_DUT_STOP\n");
        spatial_effect_imu_dut_stop();
        err = app_online_db_ack(parse_seq, (u8 *)"OK", 2);
        break;
    default :
        break;
    }
    printf("err:%d\n", err);

    return 0;
}

int aud_spatial_effect_dut_open()
{
    app_online_db_register_handle(DB_PKT_TYPE_SPATIAL_EFFECT, spatial_effect_online_parse);
    return 0;
}

int aud_spatial_effect_dut_close()
{
    return 0;
}

#endif /*(defined(TCFG_SPATIAL_EFFECT_ONLINE_ENABLE) && (TCFG_SPATIAL_EFFECT_ONLINE_ENABLE == 1))*/
