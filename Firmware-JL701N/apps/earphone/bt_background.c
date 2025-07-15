#include "system/includes.h"
#include "classic/tws_api.h"
#include "btstack/avctp_user.h"
#include "btstack/a2dp_media_codec.h"

#include "app_config.h"
#include "app_main.h"
#include "app_action.h"

#include "app_task.h"
#include "audio_config.h"

static u16 clear_to_seqn = 0;
static u16 slience_timer;
static u8 unmute_packet_cnt;
static u8 g_bt_background = 0;
static u8 energy_check_stop;

u8 bt_media_is_running();
void a2dp_dec_close();
void bt_switch_to_foreground(int action, bool exit_curr_app);

#define TWS_A2DP_CLEAR_ID     TWS_FUNC_ID('A', 2, 'C', 'R')


static void __tws_a2dp_clear_to_seqn(void *data, u16 len, bool rx)
{
    if (rx) {
        clear_to_seqn = *(u16 *)data;
    }
}

REGISTER_TWS_FUNC_STUB(tws_a2dp_clear_to_seqn) = {
    .func_id = TWS_A2DP_CLEAR_ID,
    .func = __tws_a2dp_clear_to_seqn,
};

bool bt_in_background()
{
    return g_bt_background;
}

void a2dp_slience_detect(void *p)
{
    int len;
    u8 *packet;
    int ingore_packet_num = (int)p;
    int role = tws_api_get_role();

    u32 media_type;
    putchar('^');
    while (1) {
        media_type = a2dp_media_get_codec_type();
        len = a2dp_media_try_get_packet(&packet);
        if (len <= 0) {
            break;
        }

        u16 seqn = (packet[2] << 8) | packet[3];

        if (role == TWS_ROLE_MASTER) {
            if (clear_to_seqn == 0) {
                clear_to_seqn = seqn + ingore_packet_num;
                if (clear_to_seqn == 0) {
                    clear_to_seqn = 1;
                }
                a2dp_media_free_packet(packet);
                a2dp_media_clear_packet_before_seqn(clear_to_seqn);
                break;
            }

            //能量检测
            if (energy_check_stop == 0) {
                int energy;
                if (media_type == 0) {
                    energy = sbc_energy_check(packet, len);
                } else {
                    energy = aac_energy_check(packet, len);

                }

                printf("energy:media_type= %d,%d, %d, %d\n", media_type, seqn, energy, unmute_packet_cnt);

                if (energy >= 10) {
                    if (++unmute_packet_cnt >= 20) {
                        unmute_packet_cnt = 0;
                        energy_check_stop = 1;

                        //能量检测结束,通知从机丢到指定包号数据，然后开始A2DP解码
                        clear_to_seqn = seqn + 10;
                        if (clear_to_seqn == 0) {
                            clear_to_seqn = 1;
                        }
                        tws_api_send_data_to_slave(&clear_to_seqn, 2, TWS_A2DP_CLEAR_ID);
                    }
                } else {
                    unmute_packet_cnt >>= 1;
                }
                a2dp_media_free_packet(packet);
                continue;
            }
        }

        printf("seqn: %d, %d\n", seqn, clear_to_seqn);

        a2dp_media_free_packet(packet);

        if (clear_to_seqn && seqn_after(seqn, clear_to_seqn)) {
            clear_to_seqn = 0;
            sys_timer_del(slience_timer);
            slience_timer = 0;
            if (!bt_in_background()) {
                //先点击手机播放再立马按键切模式时，此处需要打开a2dp解码
                g_bt_background = 1;
                bt_switch_to_foreground(ACTION_A2DP_START, 0);
            } else {
                app_task_switch_to(APP_BT_TASK, ACTION_A2DP_START);
            }
            break;
        }
    }
}

void bt_start_a2dp_slience_detect(int ingore_packet_num)
{
    if (slience_timer) {
        sys_timer_del(slience_timer);
    }
    clear_to_seqn = 0;
    unmute_packet_cnt = 0;
    energy_check_stop = 0;
    slience_timer = sys_timer_add((void *)ingore_packet_num, a2dp_slience_detect, 40);
    g_printf("bt_start_a2dp_slience_detect=%d\n", slience_timer);
}

void bt_stop_a2dp_slience_detect()
{
    if (slience_timer) {
        sys_timer_del(slience_timer);
        g_printf("bt_stop_a2dp_slience_detect");
        slience_timer = 0;
    }
}

int bt_switch_to_background()
{
    int a2dp_state;
    int esco_state;
    struct intent it;

    if (app_var.siri_stu && app_var.siri_stu != 3 && get_esco_busy_flag()) {
        // siri不退出
        return -EINVAL;
    }

    esco_state = get_call_status();
    if (esco_state == BT_CALL_OUTGOING  ||
        esco_state == BT_CALL_ALERT     ||
        esco_state == BT_CALL_INCOMING  ||
        esco_state == BT_CALL_ACTIVE) {
        // 通话不退出
        return -EINVAL;
    }



    a2dp_state = a2dp_get_status();
    r_printf("a2dp_state=%d\n", a2dp_state);
    if ((tws_api_get_role() == TWS_ROLE_MASTER) && (a2dp_state == BT_MUSIC_STATUS_STARTING)) {
        user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PAUSE, 0, NULL);
    }
    if (bt_media_is_running()) {
        a2dp_dec_close();
        a2dp_media_clear_packet_before_seqn(0);
        bt_start_a2dp_slience_detect(30);
    }

    g_bt_background = 1;

    return 0;
}

void bt_switch_to_foreground(int action, bool exit_curr_app)
{
    struct intent it;

    if (g_bt_background) {
        g_bt_background = 0;

        /* 退出当前模式 */
        if (exit_curr_app) {
            init_intent(&it);
            it.action = ACTION_BACK;
            start_app(&it);
        }
    }

    if (action) {
        init_intent(&it);
        it.name = "earphone";
        it.action = action;
        start_app(&it);
    }
}

int bt_background_event_probe_handler(struct bt_event *bt)
{
    int len;

    switch (bt->event) {
    case BT_STATUS_INIT_OK:
        len = syscfg_read(CFG_HAVE_MASS_STORAGE, &app_var.have_mass_storage, 1);
        if (len != 1) {
            if (dev_online("sd0")) {
                app_var.have_mass_storage = 1;
                syscfg_write(CFG_HAVE_MASS_STORAGE, &app_var.have_mass_storage, 1);
            }
        }
        printf("have_mass_storage: %d\n", app_var.have_mass_storage);
        break;
    case BT_STATUS_A2DP_MEDIA_START:
        puts("BT_STATUS_A2DP_MEDIA_START\n");
        if (bt_in_background()) {
            bt_start_a2dp_slience_detect(30);
            return -EINVAL;
        } else {
            bt_stop_a2dp_slience_detect();
        }
        break;
    case BT_STATUS_A2DP_MEDIA_STOP:
        puts("BT_STATUS_A2DP_MEDIA_STOP\n");
        if (bt_in_background()) {
            bt_stop_a2dp_slience_detect();
            extern void free_a2dp_using_decoder_conn();
            free_a2dp_using_decoder_conn();
            return -EINVAL;
        }
        break;
    case BT_STATUS_VOICE_RECOGNITION:
        if (bt->value && bt->value != 3) {
        } else {
            break;
        }
    case BT_STATUS_PHONE_INCOME:
    case BT_STATUS_PHONE_OUT:
    case BT_STATUS_PHONE_ACTIVE:
        bt_switch_to_foreground(ACTION_DO_NOTHING, 0);
        break;
    }

    return 0;
}



