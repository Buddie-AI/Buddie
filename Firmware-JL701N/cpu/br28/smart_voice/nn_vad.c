/*****************************************************************
>file name : nn_vad.c
>create time : Thu 16 Dec 2021 07:19:26 PM CST
*****************************************************************/
#include "typedef.h"
#include "media/tech_lib/jlsp_vad.h"

struct audio_nn_vad_context {
    void *algo;
    u8 *share_buffer;
    u8 algo_mem[0];
};

void *audio_nn_vad_open(void)
{
    struct audio_nn_vad_context *ctx;

    int model_size = 0;
    int share_size = 10 * 1024;
    int lib_heap_size = JLSP_vad_get_heap_size(NULL, &model_size);
    ctx = (struct audio_nn_vad_context *)zalloc(sizeof(struct audio_nn_vad_context) + lib_heap_size + share_size);
    if (!ctx) {
        return NULL;
    }

    ctx->share_buffer = ctx->algo_mem + lib_heap_size;

    ctx->algo = (void *)JLSP_vad_init((char *)ctx->algo_mem,
                                      lib_heap_size,
                                      (char *)ctx->share_buffer,
                                      share_size,
                                      NULL,
                                      model_size);

    if (!ctx->algo) {
        goto err;
    }
    JLSP_vad_reset(ctx->algo);

    return ctx;
err:
    if (ctx) {
        free(ctx);
    }
    return NULL;
}

int audio_nn_vad_data_handler(void *vad, void *data, int len)
{
    struct audio_nn_vad_context *ctx = (struct audio_nn_vad_context *)vad;
    int out_flag;

    if (ctx) {
        return JLSP_vad_process(ctx->algo, data, &out_flag);
    }

    return 0;
}

void audio_nn_vad_close(void *vad)
{
    struct audio_nn_vad_context *ctx = (struct audio_nn_vad_context *)vad;

    if (ctx->algo) {
        JLSP_vad_free(ctx->algo);
    }
    free(ctx);
}

