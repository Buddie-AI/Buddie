/*****************************************************************
>file name : kws_event.c
>author : lichao
>create time : Mon 01 Nov 2021 11:34:00 AM CST
*****************************************************************/
#include "system/includes.h"
#include "kws_event.h"
#include "jl_kws.h"

extern int config_audio_kws_event_enable;

static const int kws_wake_word_event[] = {
    KWS_EVENT_NULL,
    KWS_EVENT_HEY_KEYWORD,
};

static const int kws_multi_command_event[] = {
    KWS_EVENT_NULL,
    KWS_EVENT_NULL,
    KWS_EVENT_XIAOJIE,
    KWS_EVENT_XIAOJIE,
    KWS_EVENT_PLAY_MUSIC,
    KWS_EVENT_STOP_MUSIC,
    KWS_EVENT_PAUSE_MUSIC,
    KWS_EVENT_VOLUME_UP,
    KWS_EVENT_VOLUME_DOWN,
    KWS_EVENT_PREV_SONG,
    KWS_EVENT_NEXT_SONG,
    KWS_EVENT_ANC_ON,
    KWS_EVENT_ANC_OFF,
    KWS_EVENT_TRANSARENT_ON,
};

static const int kws_call_command_event[] = {
    KWS_EVENT_NULL,
    KWS_EVENT_NULL,
    KWS_EVENT_CALL_ACTIVE,
    KWS_EVENT_CALL_HANGUP,
};

static const int *kws_model_events[3] = {
    kws_wake_word_event,
    kws_multi_command_event,
    kws_call_command_event,
};

int smart_voice_kws_event_handler(u8 model, int kws)
{
    if (!config_audio_kws_event_enable || kws < 0) {
        return 0;
    }
    int event = KWS_EVENT_NULL;
    struct sys_event e;

    event = kws_model_events[model][kws];

    if (event == KWS_EVENT_NULL) {
        return -EINVAL;
    }

    e.type = SYS_KEY_EVENT;
    e.u.key.event = event;
    e.u.key.value = 'V';
    e.u.key.type = KEY_DRIVER_TYPE_VOICE;//区分按键类型

    sys_event_notify(&e);

    return 0;
}

static const char *kws_dump_words[] = {
    "no words",
    "hey siri",
    "xiao jie",
    "xiao du",
    "bo fang yin yue",
    "ting zhi bo fang",
    "zan ting bo fang",
    "zeng da yin liang",
    "jian xiao yin liang",
    "shang yi shou",
    "xia yi shou",
    "jie ting dian hua",
    "gua duan dian hua",
    "anc on",
    "anc off",
    "transarent on",
};

struct kws_result_context {
    u16 timer;
    u32 result[0];
};

static void smart_voice_kws_dump_timer(void *arg)
{
    struct kws_result_context *ctx = (struct kws_result_context *)arg;
    int i = 0;

    int kws_num = ARRAY_SIZE(kws_dump_words);
    printf("\n===============================================\nResults:\n");
    for (i = 1; i < kws_num; i++) {
        printf("%s : %u\n", kws_dump_words[i], ctx->result[i]);
    }
    printf("\n===============================================\n");
}

void *smart_voice_kws_dump_open(int period_time)
{
    if (!config_audio_kws_event_enable) {
        return NULL;
    }

    struct kws_result_context *ctx = NULL;
    ctx = zalloc(sizeof(struct kws_result_context) + (sizeof(u32) * ARRAY_SIZE(kws_dump_words)));

    if (ctx) {
        ctx->timer = sys_timer_add(ctx, smart_voice_kws_dump_timer, period_time);
    }
    return ctx;
}

void smart_voice_kws_dump_result_add(void *_ctx, u8 model, int kws)
{
    if (!config_audio_kws_event_enable || kws < 0) {
        return;
    }

    struct kws_result_context *ctx = (struct kws_result_context *)_ctx;
    int event = kws_model_events[model][kws];
    ctx->result[event]++;
}

void smart_voice_kws_dump_close(void *_ctx)
{
    struct kws_result_context *ctx = (struct kws_result_context *)_ctx;
    if (config_audio_kws_event_enable) {
        if (ctx->timer) {
            sys_timer_del(ctx->timer);
            free(ctx);
        }
    }
}
