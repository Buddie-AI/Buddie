#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "system/timer.h"
#include "system/task.h"
#include "app_protocol_api.h"
#include "btstack/avctp_user.h"
#include "syscfg_id.h"
#include "system/includes.h"
#include "bt_tws.h"
#include "app_protocol_ota.h"
#include "btstack/third_party/app_protocol_libs_api.h"
#include "audio_config.h"

#if AI_APP_PROTOCOL
static int app_protocol_demo_init(int match_id);
static int app_protocol_ama_init(int match_id);
static int app_protocol_gma_init(int match_id);
static int app_protocol_dma_init(int match_id);
static int app_protocol_tme_init(int match_id);
static int app_protocol_mma_init(int match_id);
static int app_protocol_gfps_init(int match_id);

#if 1
#define APP_PROTOCOL_LOG    printf
#define APP_PROTOCOL_DUMP   put_buf
#else
#define APP_PROTOCOL_LOG(...)
#define APP_PROTOCOL_DUMP(...)
#endif

typedef struct {
    // linked list - assert: first field
    void *offset_item;
    // data is contained in same memory
    u32        service_record_handle;
    u8         *service_record;
} app_protocal_item_t;
#define AI_RECORD_REGISTER(handler) \
	const app_protocal_item_t  handler sec(.sdp_record_item)

struct app_protocol_info_t {
    /*配置类接口*/
    void (*set_product_id)(u32 pid);
    void (*set_vendor_id)(u32 vid);
    void (*set_local_version)(u32 vid);
    void (*set_special_info_group)(void *addr);
    void (*set_tws_sibling_mac)(void *mac);
};

struct app_ctrl_operation_t {
    int(*protocol_init)();
    int(*protocol_exit)();
    int(*adv_enable)(int enable);
    int(*ibeacon_adv)(int sw);
    int(*regist_wakeup_send)(void *priv, void *cbk);
    int(*regist_recieve_cbk)(void *priv, void *cbk);
    int(*latency_enable)(void *priv, u32 enable);
    int(*send_data)(void *priv, void *buf, u16 len);
    int(*send_voice_data)(void *buf, u16 len);
    int(*disconnect)(void *addr);
    int(*tws_receive_sync_data)(u8 *data, int len);
    int(*get_auth_state)(void);
    int(*start_voice_recognition)(int st);
    int(*update_tws_state_to_lib)(int sta);
};
struct callback_register_t {
    void(*message_handler_regedit)(int (*handler)(int id, int opcode, u8 *data, u32 len));
    void(*check_some_status_register)(int (*handler)(int state_flag));
    void(*check_tws_role_is_master_register)(bool (*handler)(void));
    void(*tx_resume)(void (*handler)(void));
    void(*rx_resume)(void (*handler)(void));
    void(*get_battery)(bool (*handler)(u8 battery_type, u8 *value));
};
typedef struct app_protocol {
    int    handler_id;
    int (*loop)(void);
    int (*tx_loop)(void);
    int (*special_message)(int id, int opcode, u8 *data, u32 len);
    struct app_protocol_info_t  *settings;
    struct app_ctrl_operation_t *app_ctrl;
    struct callback_register_t  *callback;
} app_protocol_interface_t;

#define MAX_NUMBER_RUN              2
struct app_protocol_task {
    app_protocol_interface_t app_p[MAX_NUMBER_RUN];
    u16 tick_timer;
    u8 running_protocol_number: 3;
    u8 run_flag: 1;
    u8 first_init_flag: 1;
    u8 reserve: 3;
};
struct app_protocol_task g_app;
#define __this (&g_app)
static app_protocol_interface_t *current_run_app = NULL;

#define list_for_each_app_protocol(p) \
    for (p = __this->app_p; p < __this->app_p + MAX_NUMBER_RUN; p++)


static app_protocol_interface_t *get_interface_for_id(u32 id)
{
    app_protocol_interface_t *app;
    list_for_each_app_protocol(app) {
        if (id == app->handler_id) {
            return app;
        }
    }
    return NULL;
}
static int free_interface_for_id(u32 id)
{
    app_protocol_interface_t *app;
    list_for_each_app_protocol(app) {
        if (id == app->handler_id) {
            app->handler_id = 0;
            app->loop = NULL;
            app->tx_loop = NULL;
            app->settings = NULL;
            app->app_ctrl = NULL;
            app->callback = NULL;
            return 1;
        }
    }
    return 0;
}
static app_protocol_interface_t *get_empty_interface()
{
    app_protocol_interface_t *app;
    list_for_each_app_protocol(app) {
        if (0 == app->handler_id) {
            return app;
        }
    }
    return NULL;
}

/**获取当前协议*/
u32 app_protocol_get_cur_handle_id()
{
    if (current_run_app) {
        return current_run_app->handler_id;
    }
    return 0;
}

/*设置产品的PID*/
void app_protocol_set_product_id(u32 handler_id, u32 pid)
{
    app_protocol_interface_t *app = get_interface_for_id(handler_id);
    if (app && app->settings) {
        if (app->settings->set_product_id) {
            app->settings->set_product_id(pid);
        }
    }
}
/*设置产品的VID*/
void app_protocol_set_vendor_id(u32 handler_id, u32 vid)
{
    app_protocol_interface_t *app = get_interface_for_id(handler_id);
    if (app && app->settings) {
        if (app->settings->set_vendor_id) {
            app->settings->set_vendor_id(vid);
        }
    }
}

/*设置产品的version*/
void app_protocol_set_local_version(u32 handler_id, u32 version)
{
    app_protocol_interface_t *app = get_interface_for_id(handler_id);
    if (app && app->settings) {
        if (app->settings->set_local_version) {
            app->settings->set_local_version(version);
        }
    }
}

/*配置协议更多信息，例如GMA的三元组地址*/
void app_protocol_set_info_group(u32 handler_id, void *addr)
{
    app_protocol_interface_t *app = get_interface_for_id(handler_id);
    if (app && app->settings) {
        if (app->settings->set_special_info_group) {
            app->settings->set_special_info_group(addr);
        }
    }
}
/*配置TWS的地址,app协议用于配置广播包或者命令告知app*/
void app_protocol_set_tws_sibling_mac(u8 *mac)
{
    app_protocol_interface_t *app;
    list_for_each_app_protocol(app) {
        if (app->settings) {
            if (app->settings->set_tws_sibling_mac) {
                app->settings->set_tws_sibling_mac(mac);
            }
        }
    }
}

void app_protocol_ibeacon_switch(int sw)
{
    app_protocol_interface_t *app;
    list_for_each_app_protocol(app) {
        if (app->app_ctrl) {
            if (app->app_ctrl->ibeacon_adv) {
                app->app_ctrl->ibeacon_adv(sw);
            }
        }
    }
}
void app_protocol_ble_adv_switch(int sw)
{
    app_protocol_interface_t *app;
    list_for_each_app_protocol(app) {
        if (app->app_ctrl) {
            if (app->app_ctrl->adv_enable) {
                app->app_ctrl->adv_enable(sw);
            }
        }
    }
}
void app_protocol_disconnect(void *addr)
{
    app_protocol_interface_t *app;
    list_for_each_app_protocol(app) {
        if (app->app_ctrl) {
            if (app->app_ctrl->disconnect) {
                app->app_ctrl->disconnect(addr);
            }
        }
    }
}
void app_protocol_get_tws_data_for_lib(u8 *data, u32 len)
{
    app_protocol_interface_t *app;
    list_for_each_app_protocol(app) {
        if (app->app_ctrl) {
            if (app->app_ctrl->tws_receive_sync_data) {
                app->app_ctrl->tws_receive_sync_data(data, len);
            }
        }
    }
}
void app_protocal_update_tws_state_to_lib(int state)
{
    app_protocol_interface_t *app;
    list_for_each_app_protocol(app) {
        if (app->app_ctrl) {
            if (app->app_ctrl->update_tws_state_to_lib) {
                app->app_ctrl->update_tws_state_to_lib(state);
            }
        }
    }
}
__attribute__((weak))
bool is_tws_master_role()
{
#if TCFG_USER_TWS_ENABLE
    return (tws_api_get_role() == TWS_ROLE_MASTER);
#endif
    return 1;
}

static int app_get_tws_side()
{
    // CHECK_STATUS_TWS_SIDE:     /*0是单耳，1是左耳，2是右耳*/
#if TCFG_USER_TWS_ENABLE
    if (tws_api_get_local_channel() == 'R') {
        return 2;
    } else if (tws_api_get_local_channel() == 'L') {
        return 1;
    }
#endif
    return 0;
}
int app_protocol_check_status(int status_flag)
{
    switch (status_flag) {
    case CHECK_STATUS_TWS_MASTER:
        return is_tws_master_role();
    case CHECK_STATUS_TWS_SLAVE:
        return !is_tws_master_role();
    case CHECK_STATUS_TWS_PAIR_STA: /*1是tws已经配对了，0是未配对*/
        if (get_bt_tws_connect_status()) {
            return true;
        } else {
            return false;
        }
    case CHECK_STATUS_TWS_SIDE:     /*0是单耳，1是左耳，2是右耳*/
        return app_get_tws_side();
    }
    return 0;
}
__attribute__((weak))
bool app_protocol_get_battery(u8 type, u8 *value)
{
    for (int i = 0; i < APP_PROTOCOL_BAT_T_MAX; i++) {
        if (type & BIT(i)) {
            value[i] = app_protocal_get_bat_by_type(i);
        }
    }
    return 0;
}
static void set_mic_rec_param(int id)
{
    switch (id) {
    case TME_HANDLER_ID:
        mic_rec_pram_init(AUDIO_CODING_OPUS, 1 << 6, app_protocol_send_voice_data, 4, 1024); //TME每4帧加一个头，每帧长度46
        break;
    case GMA_HANDLER_ID:
        mic_rec_pram_init(AUDIO_CODING_OPUS, 0, app_protocol_send_voice_data, 4, 1024);
        break;
    case MMA_HANDLER_ID:
        mic_rec_pram_init(AUDIO_CODING_SPEEX, 0, app_protocol_send_voice_data, 5, 1024); //小米每5帧加一个头，每帧长度42
        break;
    default:
        mic_rec_pram_init(AUDIO_CODING_OPUS, 0, app_protocol_send_voice_data, 4, 1024);
        break;

    }
}
static int app_protocol_message_handler(int id, int opcode, u8 *data, u32 len)
{
    static app_protocol_interface_t *message_app = NULL;
    APP_PROTOCOL_LOG("\napp_protocol_message_handler opcode = %d\n", opcode);
    int ret = 0;
    switch (opcode) {
    case APP_PROTOCOL_CONNECTED_SPP:
    case APP_PROTOCOL_CONNECTED_BLE:
        //连接上断开消息的
        app_protocol_post_bt_event(AI_EVENT_APP_CONNECT, NULL);
        /* void bt_check_exit_sniff(); */
        /* bt_check_exit_sniff(); */
        user_send_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);
        current_run_app = get_interface_for_id(id);
#if APP_PROTOCOL_SPEECH_EN
        set_mic_rec_param(id);
#endif
        printf("APP_PROTOCOL_CONNECT\n");
        break;
    case APP_PROTOCOL_DISCONNECT:
        app_protocol_post_bt_event(AI_EVENT_APP_DISCONNECT, NULL);
        app_protocol_ota_interrupt_by_disconnect(id);
        current_run_app = NULL;
        printf("APP_PROTOCOL_disconnect\n");
        if (app_protocol_update_success_flag()) {
            cpu_reset();
        }
        break;
    case APP_PROTOCOL_AUTH_PASS:
        break;
    case APP_PROTOCOL_SPEECH_ENCODER_TYPE:
        break;
#if APP_PROTOCOL_SPEECH_EN
    case APP_PROTOCOL_SPEECH_START:
        APP_PROTOCOL_LOG("app_protocol_speech_start by app\n");
        ret = app_protocol_start_speech_by_app(1);
        break;
    case APP_PROTOCOL_SPEECH_STOP:
        ret = app_protocol_stop_speech_by_app();
        break;
#endif
    case APP_PROTOCOL_SET_VOLUME:
        app_protocol_set_volume(data[0]);
        break;
    case APP_PROTOCOL_GET_VOLUME:
        data[0] = app_audio_get_volume(APP_AUDIO_STATE_MUSIC);
        break;
    case APP_PROTOCOL_LIB_TWS_DATA_SYNC:
        app_protocol_tws_send_to_sibling(APP_PROTOCOL_TWS_FOR_LIB, data, len);
        break;
    default:
        ret = app_protocol_ota_message_handler(id, opcode, data, len);
        break;
    }
    message_app = get_interface_for_id(id);

    if (message_app) {
        if (message_app->special_message) {
            message_app->special_message(id, opcode, data, len);
        }
    }
    return ret;
}

static void app_protocol_resume(void)
{
    if (__this->run_flag == 0) {
        return ;
    }
    os_taskq_post_type(APP_PROTOCOL_TASK_NAME, APP_PROTOCOL_RX_DATA_EVENT, 0, NULL);
}
static void app_protocol_send_resume(void)
{
    if (__this->run_flag == 0) {
        return ;
    }
    os_taskq_post_type(APP_PROTOCOL_TASK_NAME, APP_PROTOCOL_TX_DATA_EVENT, 0, NULL);
}
static void app_protocol_tick_resume(void)
{
    app_protocol_resume();///500ms resume for send retry
}
static void app_protocol_loop_process(void *parm)
{
    int msg[8];
    int ret;
    app_protocol_interface_t *app = NULL;
    __this->tick_timer = sys_timer_add(NULL, app_protocol_tick_resume, 1000);
    __this->run_flag = 1;
    APP_PROTOCOL_LOG("app_protocol_loop_process\n");
    while (1) {
        ret = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        if (ret != OS_TASKQ) {
            continue;
        }
        switch (msg[0]) {
        case APP_PROTOCOL_RX_DATA_EVENT:
            list_for_each_app_protocol(app) {
                if (app->handler_id) {
                    if (app->loop) {
                        app->loop();
                    }
                }
            }
            break;
        case APP_PROTOCOL_TX_DATA_EVENT:
            list_for_each_app_protocol(app) {
                if (app->handler_id) {
                    if (app->tx_loop) {
                        app->tx_loop();
                    }
                }
            }
            break;
        case APP_PROTOCOL_TASK_EXIT:
            os_sem_post((OS_SEM *)msg[1]);
            os_time_dly(10000);
            break;
        default:
            APP_PROTOCOL_LOG("err app protocal msg\n");
            break;
        }
    }
}
//可以动态加载选择某个APP协议
void app_protocol_init(int handler_id)
{
    APP_PROTOCOL_LOG("app_protocol_init\n");
    //如果是第二次调用这个函数，就只起注册协议的作用，不需要重新建立任务
    if (__this->running_protocol_number < MAX_NUMBER_RUN) {
#if APP_PROTOCOL_DEMO_CODE
        if (app_protocol_demo_init(handler_id)) {
            __this->running_protocol_number++;
        }
#endif
#if APP_PROTOCOL_AMA_CODE
        if (app_protocol_ama_init(handler_id)) {
            __this->running_protocol_number++;
        }
#endif
#if APP_PROTOCOL_GMA_CODE
        if (app_protocol_gma_init(handler_id)) {
            __this->running_protocol_number++;
        }
#endif
#if APP_PROTOCOL_DMA_CODE
        if (app_protocol_dma_init(handler_id)) {
            __this->running_protocol_number++;
        }
#endif
#if APP_PROTOCOL_MMA_CODE
        if (app_protocol_mma_init(handler_id)) {
            __this->running_protocol_number++;
        }
#endif

#if APP_PROTOCOL_TME_CODE
        if (app_protocol_tme_init(handler_id)) {
            __this->running_protocol_number++;
        }
#endif
#if APP_PROTOCOL_GFPS_CODE
        if (app_protocol_gfps_init(handler_id)) {
            __this->running_protocol_number++;
        }
#endif
    }
    if ((__this->running_protocol_number > 0) && __this->first_init_flag == 0) {
        APP_PROTOCOL_LOG("app_protocol_task create\n");
        __this->first_init_flag = 1;
        task_create(app_protocol_loop_process, NULL, APP_PROTOCOL_TASK_NAME);
    }
}
void app_protocol_all_init()
{
    app_protocol_init(DEMO_HANDLER_ID);
    app_protocol_init(DMA_HANDLER_ID);
    app_protocol_init(MMA_HANDLER_ID);
    app_protocol_init(GMA_HANDLER_ID);
    app_protocol_init(TME_HANDLER_ID);
    app_protocol_init(GFPS_HANDLER_ID);
}
/*为了适应以前的参数格式，自定义了一个函数发消息*/
int app_protocol_post_msg(const char *name, int argc, ...)
{
    int ret = 0;
    int argv[8];
    APP_PROTOCOL_LOG("mm_task_post_msg\n");
    va_list argptr;
    ASSERT(argc < 8);
    va_start(argptr, argc);
    for (int i = 0; i < argc; i++) {
        argv[i] = va_arg(argptr, int);
    }
    va_end(argptr);
    if (argc == 1) {
        ret =  os_taskq_post_type(name, argv[0], 0, NULL);
    } else {
        ret =  os_taskq_post_type(name, argv[0], argc - 1, &argv[1]);
    }
    return ret;
}
void app_protocol_exit(int handler_id)
{
    //handler_id为0时退出所有，有具体handler值是退出一个
    OS_SEM sem;
    app_protocol_interface_t *app;
    if (handler_id == 0) {
        list_for_each_app_protocol(app) {
            if (app->handler_id) {
                if (app->app_ctrl) {
                    if (app->app_ctrl->protocol_exit) {
                        app->app_ctrl->protocol_exit();
                    }
                }
                if (free_interface_for_id(app->handler_id)) {
                    __this->running_protocol_number--;
                }
            }
        }
    } else {
        app = get_interface_for_id(handler_id);
        if (app) {
            if (app->app_ctrl) {
                if (app->app_ctrl->protocol_exit) {
                    app->app_ctrl->protocol_exit();
                }
            }
            if (free_interface_for_id(handler_id)) {
                __this->running_protocol_number--;
            }
        }
    }
    APP_PROTOCOL_LOG("app_protocol_exit\n");
    if (__this->run_flag && (!__this->running_protocol_number)) {
        __this->run_flag = 0;
        if (__this->tick_timer) {
            sys_timer_del(__this->tick_timer);
            __this->tick_timer = 0;
        }
        os_sem_create(&sem, 0);
        os_taskq_post_type(APP_PROTOCOL_TASK_NAME, APP_PROTOCOL_TASK_EXIT, 1, (int *)&sem);
        os_sem_pend(&sem, 0);

        task_kill(APP_PROTOCOL_TASK_NAME);
        __this->first_init_flag = 0;

        APP_PROTOCOL_LOG("app_protocol_delete task\n");
    }
}

int app_protocol_send_voice_data(uint8_t *voice_buf, uint16_t voice_len)
{
    int error = 0;
    app_protocol_interface_t *app = current_run_app;
    if (app) {
        if (app->app_ctrl) {
            if (app->app_ctrl->send_voice_data) {
                error = app->app_ctrl->send_voice_data(voice_buf, voice_len);
            }
        }
        return error;
    }
    return -1;
}
int app_protocol_check_connect_success()
{
    int flag = 0;
    app_protocol_interface_t *app = current_run_app;
    if (app) {
        if (app->app_ctrl) {
            //一般认为只有真正加密完才能说连接完成
            if (app->app_ctrl->get_auth_state) {
                flag = app->app_ctrl->get_auth_state();
            }
        }
    }
    return flag;
}
static int app_protocol_start_voice_recognition(int state)
{
    int flag = 0;
    app_protocol_interface_t *app = current_run_app;
    if (!app) {
        //做案子的时候百度有时候没连接也有状态更新到蓝牙库
        app = get_interface_for_id(DMA_HANDLER_ID);
    }
    if (app) {
        if (app->app_ctrl) {
            //一般认为只有真正加密完才能说连接完成
            if (app->app_ctrl->start_voice_recognition) {
                flag = app->app_ctrl->start_voice_recognition(state);
            }
        }
    }
    return flag;
}
int app_protocol_start_speech_cmd()
{
    return app_protocol_start_voice_recognition(1);
}
int app_protocol_stop_speech_cmd()
{
    return app_protocol_start_voice_recognition(0);
}
int app_protocol_left_start_speech_cmd()
{
    printf("left click");
    return app_protocol_start_voice_recognition(2);
}
int app_protocol_right_start_speech_cmd()
{
    printf("right click");
    return app_protocol_start_voice_recognition(3);
}

void app_protocol_common_init(app_protocol_interface_t *app)
{
#if USER_SUPPORT_PROFILE_SPP
    extern void user_spp_data_handler(u8 packet_type, u16 ch, u8 * packet, u16 size);
    spp_data_deal_handle_register(user_spp_data_handler);
#endif
#if TCFG_USER_TWS_ENABLE
    tws_api_auto_role_switch_disable();
#endif
    if (app->callback) {
        if (app->callback->message_handler_regedit) {
            app->callback->message_handler_regedit(app_protocol_message_handler);
        }
        if (app->callback->check_tws_role_is_master_register) {
            app->callback->check_tws_role_is_master_register(is_tws_master_role);
        }
        if (app->callback->check_some_status_register) {
            app->callback->check_some_status_register(app_protocol_check_status);
        }

        if (app->callback->tx_resume) {
            app->callback->tx_resume(app_protocol_send_resume);
        }
        if (app->callback->rx_resume) {
            app->callback->rx_resume(app_protocol_resume);
        }
        if (app->callback->get_battery) {
            app->callback->get_battery(app_protocol_get_battery);
        }
    }
    if (app->app_ctrl) {
        if (app->app_ctrl->protocol_init) {
            app->app_ctrl->protocol_init();
        }
    }

}
//**对接具体协议的接口**//
#if APP_PROTOCOL_DEMO_CODE
extern int demo_process();
extern int demo_all_init(void);
extern int demo_all_exit(void);
static const struct app_ctrl_operation_t demo_ctrl = {
    .protocol_init = demo_all_init,
    .protocol_exit = demo_all_exit,
};
static int app_protocol_demo_init(int match_id)
{
    if (DEMO_HANDLER_ID != match_id) {
        return 0;
    }
    APP_PROTOCOL_LOG("app_protocol_demo_init\n");
    app_protocol_interface_t *app = get_interface_for_id(DEMO_HANDLER_ID);
    if (app) {
        APP_PROTOCOL_LOG("DEMO_HANDLER_ID init again\n");
    } else {
        app = get_empty_interface();
        if (app) {
            app->handler_id = DEMO_HANDLER_ID;
            app->loop = demo_process;
            app->settings = NULL;
            app->app_ctrl = &demo_ctrl;
            app->callback = NULL;
            app_protocol_common_init(app);
            APP_PROTOCOL_LOG("DEMO_HANDLER_ID init OK\n");
            return 1;
        } else {
            APP_PROTOCOL_LOG("not support more app protocal\n");
        }
    }
    return 0;
}
#endif
#if APP_PROTOCOL_AMA_CODE
static int ama_run_loop(void)
{
    APP_PROTOCOL_LOG("ama_run_loop\n");
    return 0;
}
static int app_protocol_ama_init(int match_id)
{
    if (AMA_HANDLER_ID != match_id) {
        return 0;
    }
    APP_PROTOCOL_LOG("app_protocol_ama_init\n");
    app_protocol_interface_t *app = get_interface_for_id(AMA_HANDLER_ID);
    if (app) {
        APP_PROTOCOL_LOG("AMA_HANDLER_ID init again\n");
    } else {
        app = get_empty_interface();
        if (app) {
            app->handler_id = AMA_HANDLER_ID;
            app->loop = ama_run_loop;
            app->settings = NULL;
            app->app_ctrl = NULL;
            app->callback = NULL;
            APP_PROTOCOL_LOG("AMA_HANDLER_ID init OK\n");
            return 1;
        } else {
            APP_PROTOCOL_LOG("not support more app protocal\n");
        }
    }
    return 0;
}
#endif
#if APP_PROTOCOL_GMA_CODE
#if TCFG_USER_TWS_ENABLE//GMA_TWS_SUPPORTED
u8 gma_tws_support = 1;
#endif
extern const char *gma_tone_tab[];
extern struct app_protocol_private_handle_t gma_private_handle;
extern const u8 sdp_gma_spp_service_data[];
AI_RECORD_REGISTER(gma_record_item) = {
    .service_record = (u8 *)sdp_gma_spp_service_data,
    .service_record_handle = 0x00010035,
};
static const struct callback_register_t gma_callback = {
    .message_handler_regedit = gma_message_callback_register,
    .check_tws_role_is_master_register = gma_is_tws_master_callback_register,
    .tx_resume = gma_tx_resume_register,
    .rx_resume = gma_rx_resume_register,
    .get_battery = gma_get_battery_callback_register,
};
static const struct app_ctrl_operation_t gma_ctrl = {
    .protocol_init = gma_all_init,
    .protocol_exit = gma_all_exit,
    .send_voice_data = gma_opus_voice_mic_send,
    .get_auth_state = gma_connect_success,
    .start_voice_recognition = gma_start_voice_recognition,
    .adv_enable    = gma_ble_adv_enable,
    .ibeacon_adv  = gma_ble_ibeacon_adv,
    .disconnect   = gma_disconnect,
};
static const struct app_protocol_info_t gma_settings = {
    .set_special_info_group = gma_set_active_ali_para,
    .set_tws_sibling_mac = gma_set_sibling_mac_para,
};

#define GMA_MAIN_NUMBER				0X00
#define GMA_SECONDARY_NUMBER		0X00
#define GMA_REVISION_NUMBER			0X01

int app_protocol_gma_get_version(void)
{
    return (GMA_MAIN_NUMBER << 16 | GMA_SECONDARY_NUMBER << 8 | GMA_REVISION_NUMBER);
}

/*
 *code crc16
 * */
static unsigned short wCRCin = 0xFFFF;
static unsigned short wCPoly = 0x1021;

static unsigned short CRC16_CCITT_FALSE(u16 init_crc, unsigned char *data, unsigned int datalen)
{

    while (datalen--) {
        wCRCin ^= *(data++) << 8;
        for (int i = 0; i < 8; i++) {
            if (wCRCin & 0x8000) {
                wCRCin = (wCRCin << 1) ^ wCPoly;
            } else {
                wCRCin = wCRCin << 1;
            }
        }
    }
    return (wCRCin);

}

static void CRC16_CCITT_FALSE_INIT(void)
{
    wCRCin = 0xFFFF;
    wCPoly = 0x1021;
}

static int CRC16_CODE_RESULT(int calc_crc)
{
    return 0;
}

static int app_protocol_gma_init(int match_id)
{
    if (GMA_HANDLER_ID != match_id) {
        return 0;
    }
    APP_PROTOCOL_LOG("app_protocol_gma_init\n");
    app_protocol_interface_t *app = get_interface_for_id(GMA_HANDLER_ID);
    if (app) {
        APP_PROTOCOL_LOG("GMA_HANDLER_ID init again\n");
    } else {
        app = get_empty_interface();
        if (app) {
            app->handler_id = GMA_HANDLER_ID;
            app->loop = gma_rx_loop;
            app->tx_loop = tm_data_send_process_thread;
            app->settings = &gma_settings;
            app->app_ctrl = &gma_ctrl;
            app->callback = &gma_callback;
            gma_prev_init();
            app_protocol_tone_register(gma_tone_tab);
            app_protocol_handle_register(&gma_private_handle);
            app_protocol_common_init(app);
            app_protocol_ota_api api = {
                .ota_request_data  = gma_ota_requset_next_packet,
                .ota_report_result = gma_replay_ota_result,
                .ota_crc_init_hdl  = CRC16_CCITT_FALSE_INIT,
                .ota_crc_calc_hdl  = CRC16_CCITT_FALSE,
            };
            app_protocol_ota_init(&api, 0);
            /* app_protocol_data_api_register(gma_ota_requset_next_packet, gma_replay_ota_result); */
            /* app_protocol_ota_crc_handler_register( CRC16_CCITT_FALSE_INIT,  CRC16_CCITT_FALSE); */
            APP_PROTOCOL_LOG("GMA_HANDLER_ID init OK\n");
            return 1;
        } else {
            APP_PROTOCOL_LOG("not support more app protocal\n");
        }
    }
    return 0;
}
#endif

#if APP_PROTOCOL_DMA_CODE
//u8 dma_tws_support = 1;
extern const char *dma_notice_tab[];
extern const u8 sdp_dueros_spp_service_data[];
extern const u8 sdp_dueros_ota_service_data[];
AI_RECORD_REGISTER(dma_record_item) = {
    .service_record = (u8 *)sdp_dueros_spp_service_data,
    .service_record_handle = 0x00010036,
};
AI_RECORD_REGISTER(dma_ota_record_item) = {
    .service_record = (u8 *)sdp_dueros_ota_service_data,
    .service_record_handle = 0x00010037,
};

static const struct callback_register_t dma_callback = {
    .message_handler_regedit = dma_message_callback_register,
    .check_some_status_register = dma_check_status_callback_register,
    .tx_resume = dma_tx_resume_register,
    .rx_resume = dma_rx_resume_register,
    .get_battery = dma_get_battery_callback_register,
};

static const struct app_ctrl_operation_t dma_ctrl = {
    .protocol_init = dma_all_init,
    .protocol_exit = dma_all_exit,
    .adv_enable    = dma_ble_adv_enable,
    .send_voice_data = dma_speech_data_send,
    .get_auth_state = dma_pair_state,
    .start_voice_recognition = dma_start_voice_recognition,
    .disconnect   = dma_disconnect,
    .update_tws_state_to_lib = dma_update_tws_state_to_lib,
    .tws_receive_sync_data = dma_tws_data_deal,
    /*.send_voice_data = dma_opus_voice_mic_send,
    .adv_enable    = dma_ble_adv_enable,*/
};
static const struct app_protocol_info_t dma_settings = {
    .set_product_id = dma_set_pid,
    .set_special_info_group = dma_set_product_id_key,

};
static int dueros_special_message(int id, int opcode, u8 *data, u32 len)
{
    int ret = 0;
    switch (opcode) {
    case APP_PROTOCOL_DMA_SAVE_RAND:
        APP_PROTOCOL_LOG("APP_PROTOCOL_DMA_SAVE_RAND");
        //APP_PROTOCOL_DUMP(data, len);
        ret = syscfg_write(VM_DMA_RAND, data, len);
        //app_protocol_tws_send_to_sibling(DMA_TWS_ALL_INFO_SYNC, data, len);
        break;
    case APP_PROTOCOL_DMA_READ_RAND:
        APP_PROTOCOL_LOG("APP_PROTOCOL_DMA_READ_RAND");
        ret = syscfg_read(VM_DMA_RAND, data, len);
        //APP_PROTOCOL_DUMP(data, len);
        break;
    case APP_PROTOCOL_DMA_SAVE_OTA_INFO:
        APP_PROTOCOL_LOG("APP_PROTOCOL_DMA_SAVE_OTA_INFO");
        APP_PROTOCOL_DUMP(data, len);
        //不支持断点续传
        //ret = syscfg_write(VM_TME_AUTH_COOKIE, data, len);
        //app_protocol_tws_send_to_sibling(DMA_TWS_OTA_INFO_SYNC, data, len);
        break;
    case APP_PROTOCOL_DMA_READ_OTA_INFO:
        APP_PROTOCOL_LOG("APP_PROTOCOL_DMA_READ_OTA_INFO");
        //ret = syscfg_read(VM_TME_AUTH_COOKIE, data, len);
        //不支持断点续传
        memset(data, 0, len);
        APP_PROTOCOL_DUMP(data, len);
        break;
    case APP_PROTOCOL_DMA_TTS_TYPE:
        printf("app_protocol_dma_tone_play %d\n", data[0]);
        //app_protocol_dma_tone_play(data[0], 1);
        break;
    }
    return ret;
}
static int app_protocol_dma_init(int match_id)
{
    if (DMA_HANDLER_ID != match_id) {
        return 0;
    }
    APP_PROTOCOL_LOG("app_protocol_dma_init\n");
    app_protocol_interface_t *app = get_interface_for_id(DMA_HANDLER_ID);
    if (app) {
        APP_PROTOCOL_LOG("DMA_HANDLER_ID init again\n");
    } else {
        app = get_empty_interface();
        if (app) {
            app->handler_id = DMA_HANDLER_ID;
            app->loop = dueros_process;
            app->tx_loop = dueros_send_process;
            app->special_message = dueros_special_message;
            app->settings = &dma_settings;
            app->app_ctrl = &dma_ctrl;
            app->callback = &dma_callback;
            app_protocol_tone_register(dma_notice_tab);
            dueros_dma_manufacturer_info_init();
            app_protocol_common_init(app);
            APP_PROTOCOL_LOG("DMA_HANDLER_ID init OK\n");
            return 1;
        } else {
            APP_PROTOCOL_LOG("not support more app protocal\n");
        }
    }
    return 0;
}
#endif

#if APP_PROTOCOL_TME_CODE
extern const char *tme_notice_tab[APP_RROTOCOL_TONE_MAX];
extern const u8 sdp_tme_service_data[];
extern struct app_protocol_private_handle_t tme_private_handle;
AI_RECORD_REGISTER(tme_record_item) = {
    .service_record = (u8 *)sdp_tme_service_data,
    .service_record_handle = 0x00010037,
};

static const struct callback_register_t tme_callback = {
    .message_handler_regedit = tme_message_callback_register,
    .check_tws_role_is_master_register = tme_is_tws_master_callback_register,
    .tx_resume = tme_tx_resume_register,
    .rx_resume = tme_rx_resume_register,
    .get_battery = tme_get_battery_callback_register,
};
static const struct app_ctrl_operation_t tme_ctrl = {
    .protocol_init = tme_all_init,
    .protocol_exit = tme_all_exit,
    .adv_enable    = tme_ble_adv_enable,
    .send_voice_data = tme_send_voice_data,
    .get_auth_state = tme_connect_success,
    .start_voice_recognition = tme_start_voice_recognition,
    .disconnect = tme_protocol_disconnect,
//   .ibeacon_adv  = gma_ble_ibeacon_adv,
//   int(*regist_wakeup_send)(void *priv, void *cbk);
//   int(*regist_recieve_cbk)(void *priv, void *cbk);
//   int(*latency_enable)(void *priv, u32 enable);
//   int(*send_data)(void *priv, void *buf, u16 len);
};
static const struct app_protocol_info_t tme_settings = {
    .set_special_info_group = tme_set_configuration_info,
    .set_product_id = tme_set_pid,
    .set_vendor_id = tme_set_bid,
    //.set_tws_sibling_mac = gma_set_sibling_mac_para,
    //void (*set_left_battery)(u8 bat);
};

#define   TME_VERSION   {0x00,0x10}//版本号
static const char version[] = TME_VERSION;

int app_protocol_tme_get_version(void)
{
    return (version[0] << 8) | version[1];
}


static int app_protocol_tme_init(int match_id)
{
    if (TME_HANDLER_ID != match_id) {
        return 0;
    }
    APP_PROTOCOL_LOG("app_protocol_tme_init\n");
    app_protocol_interface_t *app = get_interface_for_id(TME_HANDLER_ID);
    if (app) {
        APP_PROTOCOL_LOG("TME_HANDLER_ID init again\n");
    } else {
        app = get_empty_interface();
        if (app) {
            app->handler_id = TME_HANDLER_ID;
            app->loop = TME_protocol_process;
            app->tx_loop = TME_send_packet_process;
            app->settings = &tme_settings;
            app->app_ctrl = &tme_ctrl;
            app->callback = &tme_callback;
            app_protocol_tone_register(tme_notice_tab);
            app_protocol_handle_register(&tme_private_handle);

            app_protocol_ota_api api = {
                .ota_request_data  =  TME_request_ota_data,
                .ota_report_result = NULL,
                .ota_crc_init_hdl  = NULL,
                .ota_crc_calc_hdl  = NULL,
                .ota_notify_file_size = TME_notify_file_size,
            };
            app_protocol_ota_init(&api, 0);

            app_protocol_common_init(app);
            APP_PROTOCOL_LOG("TME_HANDLER_ID init OK\n");
            return 1;
        } else {
            APP_PROTOCOL_LOG("not support more app protocal\n");
        }
    }
    return 0;
}
#endif

#if APP_PROTOCOL_MMA_CODE
extern void mma_prev_init();
extern const char *mma_notice_tab[APP_RROTOCOL_TONE_MAX];
extern struct app_protocol_private_handle_t mma_private_handle;

static const struct callback_register_t mma_callback = {
    .message_handler_regedit = mma_message_callback_register,
    .check_tws_role_is_master_register = mma_is_tws_master_callback_register,
    .tx_resume = mma_tx_resume_register,
    .rx_resume = mma_rx_resume_register,
    .get_battery = mma_get_battery_callback_register,
};

static const struct app_ctrl_operation_t mma_ctrl = {

    .protocol_init = mma_all_init,
    .protocol_exit = mma_all_exit,
    .adv_enable    = mma_ble_adv_enable,
    .send_voice_data = XM_speech_data_send,
    .get_auth_state = XM_protocal_auth_pass,
    .start_voice_recognition = mma_start_voice_recognition,
    .disconnect   = mma_disconnect,
    .tws_receive_sync_data = mma_tws_data_deal,
};
static const struct app_protocol_info_t mma_settings = {
    .set_product_id = mma_set_product_id,
    .set_vendor_id  = mma_set_verdor_id,
    .set_local_version = mma_set_local_version,
};
int mma_special_message(int id, int opcode, u8 *data, u32 len)
{
    int ret = 0;
    switch (opcode) {
    case APP_PROTOCOL_MMA_SAVE_INFO:
        APP_PROTOCOL_LOG("APP_PROTOCOL_MMA_SAVE_");
        APP_PROTOCOL_DUMP(data, len);
        ret = syscfg_write(VM_DMA_RAND, data, len);
        break;
    case APP_PROTOCOL_MMA_READ_INFO:
        APP_PROTOCOL_LOG("APP_PROTOCOL_mMA_READ_");
        ret = syscfg_read(VM_DMA_RAND, data, len);
        APP_PROTOCOL_DUMP(data, len);
        break;
    case APP_PROTOCOL_MMA_SAVE_ADV_COUNTER:
        syscfg_write(VM_TME_AUTH_COOKIE, data, len);
        break;
    case APP_PROTOCOL_MMA_READ_ADV_COUNTER:
        syscfg_read(VM_TME_AUTH_COOKIE, data, len);
        break;
    }
    return ret;
}
static int app_protocol_mma_init(int match_id)
{
    if (MMA_HANDLER_ID != match_id) {
        return 0;
    }
    APP_PROTOCOL_LOG("app_protocol_mma_init\n");
    app_protocol_interface_t *app = get_interface_for_id(MMA_HANDLER_ID);
    if (app) {
        APP_PROTOCOL_LOG("MMA_HANDLER_ID init again\n");
    } else {
        app = get_empty_interface();
        if (app) {
            app->handler_id = MMA_HANDLER_ID;
            app->loop = mma_protocol_loop_process;
            /*app->tx_loop = ;*/
            app->special_message = mma_special_message;
            app->settings = &mma_settings;
            app->app_ctrl = &mma_ctrl;
            app->callback = &mma_callback;
            app_protocol_tone_register(mma_notice_tab);

            app_protocol_ota_api api = {
                .ota_request_data  = mma_request_ota_data,
                .ota_report_result = mma_report_ota_status,
                .ota_crc_init_hdl  = NULL,
                .ota_crc_calc_hdl  = NULL,
                .ota_notify_file_size = mma_notify_file_size,
            };
            app_protocol_ota_init(&api, 1);     //mma需要等待APP来进行复位
            mma_prev_init();
            app_protocol_handle_register(&mma_private_handle);
            app_protocol_common_init(app);
            APP_PROTOCOL_LOG("mMA_HANDLER_ID init OK\n");
            return 1;
        } else {
            APP_PROTOCOL_LOG("not support more app protocal\n");
        }
    }
    return 0;
}
#endif
#if APP_PROTOCOL_GFPS_CODE

u8 gfp_debug_enable = 0x03; // 打印开关,BIT(0):printf BIT(1):put_buf

#define GFPS_ANC_TRANSPARENT_MODE   0x80
#define GFPS_OFF_MODE               0x20
#define GFPS_ANC_MODE               0x08
#define GFPS_ANC_ALL_MODE           (GFPS_ANC_TRANSPARENT_MODE | GFPS_OFF_MODE | GFPS_ANC_MODE)
extern void gfps_hearable_controls_enable(u8 enable);
extern void gfps_hearable_controls_update(u8 display_flag, u8 settable_flag, u8 current_state);

extern struct app_protocol_private_handle_t gfps_private_handle;
extern const u8 sdp_gfp_spp_service_data[];
AI_RECORD_REGISTER(gfps_record_item) = {
    .service_record = (u8 *)sdp_gfp_spp_service_data,
    .service_record_handle = 0x00010039,
};
static const struct callback_register_t gfps_callback = {
    .message_handler_regedit = gfps_message_callback_register,
    .check_tws_role_is_master_register = gfps_is_tws_master_callback_register,
    .tx_resume = NULL,
    .rx_resume = NULL,
    .get_battery = NULL,
};
static const struct app_ctrl_operation_t gfps_ctrl = {
    .protocol_init = gfps_all_init,
    .protocol_exit = gfps_all_exit,
    .adv_enable    = gfps_ble_adv_enable,
    .send_voice_data = NULL,
    .get_auth_state = NULL,
    .start_voice_recognition = NULL,
    .disconnect   = gfps_disconnect,
    .update_tws_state_to_lib = NULL,
    .tws_receive_sync_data = gfps_tws_data_deal,
    /*.send_voice_data = gfps_opus_voice_mic_send,
    .adv_enable    = gfps_ble_adv_enable,*/
};
static const struct app_protocol_info_t gfps_settings = {
    .set_product_id = NULL,
    .set_special_info_group = NULL,
};

static uint8_t google_model_id_used[3] = {0xF2, 0x63, 0x8D};
static char google_public_key_used[64] = {
    0x5f, 0x17, 0xf0, 0x5a, 0x22, 0x03, 0xe2, 0xdc, 0x93, 0x1d, 0x5c, 0x00, 0xe8, 0xa4, 0x06, 0x7c,
    0x3c, 0x09, 0xa0, 0xad, 0x91, 0x82, 0x3e, 0x45, 0xda, 0xe7, 0xe5, 0x2f, 0x5e, 0xe9, 0x80, 0x1b,
    0xa6, 0xe2, 0x21, 0x9f, 0x65, 0x85, 0xa8, 0xab, 0x65, 0xf1, 0x47, 0x7e, 0x46, 0x7a, 0xdb, 0xb2,
    0x4b, 0x46, 0x72, 0xb8, 0x35, 0x89, 0x0b, 0x99, 0x57, 0xb7, 0x0e, 0xa9, 0x45, 0xe1, 0x7a, 0xbb

};
static char google_private_key_used[32] = {
    0x86, 0x89, 0x57, 0x30, 0xFB, 0x95, 0x8A, 0x9E, 0x42, 0x57, 0x56, 0x97,
    0x39, 0x54, 0x77, 0xE7, 0x62, 0x44, 0x7A, 0xA1, 0x2B, 0xDF, 0x35, 0xFE,
    0xE9, 0xC5, 0x82, 0x28, 0x1B, 0xD7, 0xF5, 0xBE
};

static u8 gfps_special_name[2] = "JL";

//lib收到控制消息后，会传消息上来处理
extern int gfps_message_deal_handler(int id, int opcode, u8 *data, u32 len);

static int app_protocol_gfps_init(int match_id)
{
    if (GFPS_HANDLER_ID != match_id) {
        return 0;
    }
    APP_PROTOCOL_LOG("app_protocol_gfps_init\n");
    app_protocol_interface_t *app = get_interface_for_id(GFPS_HANDLER_ID);
    if (app) {
        APP_PROTOCOL_LOG("GFPS_HANDLER_ID init again\n");
    } else {
        app = get_empty_interface();
        if (app) {
            app->handler_id = GFPS_HANDLER_ID;
            app->loop = NULL;
            app->tx_loop = NULL;
            app->special_message = gfps_message_deal_handler;
            app->settings = &gfps_settings;
            app->app_ctrl = &gfps_ctrl;
            app->callback = &gfps_callback;
            //update_channel_map_do_in_irq_flag_set(1);
            gfps_set_anti_spoofing_public_key(google_public_key_used);
            gfps_set_anti_spoofing_private_key(google_private_key_used);
            gfps_set_model_id(google_model_id_used);
#if CONFIG_ANC_ENABLE
            gfps_hearable_controls_enable(1);
            gfps_hearable_controls_update(GFPS_ANC_ALL_MODE, GFPS_ANC_ALL_MODE, GFPS_OFF_MODE);
#endif
            app_protocol_handle_register(&gfps_private_handle);
            app_protocol_common_init(app);
            APP_PROTOCOL_LOG("GFPS_HANDLER_ID init OK\n");
            return 1;
        } else {
            APP_PROTOCOL_LOG("not support more app protocal\n");
        }
    }
    return 0;
}
#endif

int app_protocol_get_version_by_id(int id)
{
#if APP_PROTOCOL_GMA_CODE
    if (id == GMA_HANDLER_ID) {
        return app_protocol_gma_get_version();
    }
#endif
#if APP_PROTOCOL_TME_CODE
    if (id == TME_HANDLER_ID) {
        return app_protocol_tme_get_version();
    }
#endif
    return 0;
}

/* int app_protocol_reply_frame_check_result(int result){ */
/*     if( app_protocol_get_cur_handle_id() == GMA_HANDLER_ID){ */
/*         gma_replay_ota_result(result); */
/*     } */
/*     return 0; */
/* } */

//编译问题加几个空的
_WEAK_ void bt_ble_init(void)
{
}
_WEAK_ void bt_ble_exit(void)
{
}
_WEAK_ void bt_ble_adv_enable(void)
{
}

#else
void app_protocol_init(int handler_id)
{
}
void app_protocol_exit(int handler_id)
{
}
#endif

