#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "app_config.h"
#include "audio_config.h"
/* #include "audio_dec.h" */
#include "application/audio_dec_app.h"
#include "tone_player.h"
#include "sine_make.h"

#if TCFG_KEY_TONE_EN

// 按键提示音任务名
#define AUDIO_KEY_TONE_TASK_NAME				"key_tone"

// 按键提示音数字音量控制
#if (SYS_DIGVOL_GROUP_EN && defined(CONFIG_MEDIA_DEVELOP_ENABLE))
#define AUDIO_KEY_TONE_DIGVOL_EN				0
#define AUDIO_KEY_TONE_DIGVOL_NAME				"key_tone"
#else
#define AUDIO_KEY_TONE_DIGVOL_EN				0
#endif


//////////////////////////////////////////////////////////////////////////////

/*
 * 使用audio_dec_app.h里面的接口，可以实现普通文件、正弦波文件、正弦波数组播放
 */

// 参数
struct __key_tone_play_info {
    const char *file_name;			// 文件名
    struct audio_sin_param *sin;	// 正弦波数组
    u8 sin_num;						// 正弦波数组长度
    u8 preemption;					// 1-抢占，0-叠加
    void (*evt_handler)(void *priv, int flag);	// 事件回调
    void *evt_priv;					// 事件回调私有句柄
};

// 按键提示音
struct __key_tone {
    volatile u8	busy;		// 任务工作中
    volatile u8	start;		// 运行标志
    volatile u8 play;
#if AUDIO_KEY_TONE_DIGVOL_EN
    u8 volume;				// 音量
#endif
    struct __key_tone_play_info play_info;		// 输入参数
    struct audio_dec_sine_app_hdl *dec_sin;		// 文件播放句柄
    struct audio_dec_file_app_hdl *dec_file;	// sine播放句柄
    void (*evt_handler)(void *priv, int flag);	// 事件回调
    void *evt_priv;			// 事件回调私有句柄
    const char *evt_owner;	// 事件回调所在任务
};

//////////////////////////////////////////////////////////////////////////////

extern struct audio_mixer mixer;

static struct __key_tone *key_tone = NULL;

//////////////////////////////////////////////////////////////////////////////

/*
*********************************************************************
*                  Audio Key Tone Get File Ext Name
* Description: 获取文件名后缀
* Arguments  : *name		文件名
* Return	 : 后缀
* Note(s)    : None.
*********************************************************************
*/
static char *get_file_ext_name(char *name)
{
    int len = strlen(name);
    char *ext = (char *)name;

    while (len--) {
        if (*ext++ == '.') {
            break;
        }
    }
    if (len <= 0) {
        ext = name + (strlen(name) - 3);
    }

    return ext;
}

/*
*********************************************************************
*                  Audio Key Tone Post Event
* Description: 解码事件
* Arguments  : *ktone		按键提示音解码句柄
*              end_flag		结束标志
* Return	 : true		成功
* Note(s)    : None.
*********************************************************************
*/
static int key_tone_dec_event_handler(struct __key_tone *ktone, u8 end_flag)
{
    int argv[4];
    if (!ktone->evt_handler) {
        log_i("evt_handler null\n");
        return false;
    }
    argv[0] = (int)ktone->evt_handler;
    argv[1] = 2;
    argv[2] = (int)ktone->evt_priv;
    argv[3] = (int)end_flag; //0正常关闭，1被打断关闭

    int ret = os_taskq_post_type(ktone->evt_owner, Q_CALLBACK, 4, argv);
    if (ret) {
        return false;
    }
    return true;
}

/*
*********************************************************************
*                  Audio Key Tone Reelease
* Description: 提示音解码器释放
* Arguments  : *ktone		按键提示音解码句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
static void key_tone_dec_hdl_release(struct __key_tone *ktone)
{
    if (ktone->dec_file) {
        audio_dec_file_app_close(ktone->dec_file);
        ktone->dec_file = NULL;
    }
    if (ktone->dec_sin) {
        audio_dec_sine_app_close(ktone->dec_sin);
        ktone->dec_sin = NULL;
    }
#if (!defined(CONFIG_MEDIA_DEVELOP_ENABLE))
    app_audio_state_exit(APP_AUDIO_STATE_WTONE);
#endif
    /* clock_remove_set(DEC_TONE_CLK); */
}

/*
*********************************************************************
*                  Audio Key Tone Init OK
* Description: 解码初始化完成
* Arguments  : *ktone		按键提示音解码句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
static void key_tone_dec_hdl_init_ok(struct __key_tone *ktone)
{
#if (!defined(CONFIG_MEDIA_DEVELOP_ENABLE))
    if (audio_mixer_get_ch_num(&mixer) <= 1) {
#ifdef TCFG_WTONT_ONCE_VOL
        extern u8 get_tone_once_vol(void);
        app_audio_state_switch(APP_AUDIO_STATE_WTONE, get_tone_once_vol());
#else
        app_audio_state_switch(APP_AUDIO_STATE_WTONE, get_tone_vol());
#endif
    }
#endif
    /* clock_add_set(DEC_TONE_CLK); */
}

/*
*********************************************************************
*                  Audio Key Tone Close
* Description: 关闭提示音解码
* Arguments  : *ktone		按键提示音解码句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
static void key_tone_play_close(struct __key_tone *ktone)
{
    /* log_i("ktone:0x%x, sin:0x%x, file:0x%x \n", ktone, ktone->dec_file, ktone->dec_file); */
    if (ktone->dec_file || ktone->dec_sin) {
        key_tone_dec_hdl_release(ktone);
        key_tone_dec_event_handler(ktone, 1);
    }
}

/*
*********************************************************************
*                  Audio Key Tone Event Callback
* Description: 解码回调处理
* Arguments  : *ktone		按键提示音解码句柄
*              event		事件
*              *param		事件参数
* Return	 : 0	正常
* Note(s)    : None.
*********************************************************************
*/
static int key_tone_dec_app_evt_cb(struct __key_tone *ktone, enum audio_dec_app_event event, int *param)
{
    switch (event) {
    case AUDIO_DEC_APP_EVENT_START_INIT_OK:
        log_i("key_tone start init ok\n");
        key_tone_dec_hdl_init_ok(ktone);
        break;
    case AUDIO_DEC_APP_EVENT_PLAY_END:
        log_i("key_tone play end\n");
        key_tone_dec_hdl_release(ktone);
        key_tone_dec_event_handler(ktone, 0);
        break;

#if defined(CONFIG_MEDIA_DEVELOP_ENABLE)
    case AUDIO_DEC_APP_EVENT_STREAM_OPEN: {
#if AUDIO_KEY_TONE_DIGVOL_EN
        struct audio_dec_stream_entries_hdl *entries_hdl = (struct audio_dec_stream_entries_hdl *)param;
        void *dvol_entry = NULL;
        audio_dig_vol_param temp_digvol_param = {
            .vol_start = ktone->volume,//get_max_sys_vol(),
            .vol_max =  get_max_sys_vol(),
            .ch_total = audio_output_channel_num(),
            .fade_en = 0,
            .fade_points_step = 5,
            .fade_gain_step = 10,
            .vol_list = NULL,
        };
        dvol_entry = sys_digvol_group_ch_open(AUDIO_KEY_TONE_DIGVOL_NAME, -1, &temp_digvol_param);
        (entries_hdl->entries_addr)[entries_hdl->entries_cnt++] = dvol_entry;
#endif // SYS_DIGVOL_GROUP_EN
    }
    break;

    case AUDIO_DEC_APP_EVENT_STREAM_CLOSE: {
    }
    break;

#endif /*#if defined(CONFIG_MEDIA_DEVELOP_ENABLE)*/

    case AUDIO_DEC_APP_EVENT_DEC_CLOSE:

#if AUDIO_KEY_TONE_DIGVOL_EN
        sys_digvol_group_ch_close(AUDIO_KEY_TONE_DIGVOL_NAME);
#endif // SYS_DIGVOL_GROUP_EN

        break;

    default :
        break;
    }

    return 0;
}

/*
*********************************************************************
*                  Audio Key Tone Sin Event Callback
* Description: sine提示音解码回调
* Arguments  : *priv		私有句柄
*              event		事件
*              *param		事件参数
* Return	 : 0	正常
* Note(s)    : None.
*********************************************************************
*/
static int key_tone_dec_sine_app_evt_cb(void *priv, enum audio_dec_app_event event, int *param)
{
    struct audio_dec_sine_app_hdl *sine_dec = priv;
    struct __key_tone *ktone = sine_dec->priv;
    switch (event) {
    case AUDIO_DEC_APP_EVENT_DEC_PROBE:
        if (sine_dec->sin_maker) {
            break;
        }
        audio_dec_sine_app_probe(sine_dec);
        if (!sine_dec->sin_maker) {
            return -ENOENT;
        }
        break;

    default :
        return key_tone_dec_app_evt_cb(ktone, event, param);
    }

    return 0;
}

/*
*********************************************************************
*                  Audio Key Tone File Event Callback
* Description: file提示音解码回调
* Arguments  : *priv		私有句柄
*              event		事件
*              *param		事件参数
* Return	 : 0	正常
* Note(s)    : None.
*********************************************************************
*/
static int key_tone_dec_file_app_evt_cb(void *priv, enum audio_dec_app_event event, int *param)
{
    struct audio_dec_file_app_hdl *file_dec = priv;
    struct __key_tone *ktone = file_dec->priv;
    switch (event) {
    case AUDIO_DEC_APP_EVENT_DEC_PROBE:
        break;
    default :
        return key_tone_dec_app_evt_cb(ktone, event, param);
    }

    return 0;
}

/*
*********************************************************************
*                  Audio Key Tone Run
* Description: 按键提示音运行
* Arguments  : *ktone		按键提示音解码句柄
* Return	 : 0	正常
* Note(s)    : None.
*********************************************************************
*/
static int key_tone_play_run(struct __key_tone *ktone)
{
    if (!ktone->start) {
        return -1;
    }

    struct __key_tone_play_info info = {0};
    local_irq_disable();
    if (!ktone->play) {
        local_irq_enable();
        return 0;
    }
    // 保存播放参数
    memcpy(&info, &ktone->play_info, sizeof(struct __key_tone_play_info));
    ktone->play = 0;
    local_irq_enable();

    // 关闭现有play
    key_tone_play_close(ktone);

    // 设置参数
    ktone->evt_handler = info.evt_handler;
    ktone->evt_priv = info.evt_priv;
    ktone->evt_owner = "app_core";

    // 正弦波数组播放
    if (info.sin && info.sin_num) {
        ktone->dec_sin = audio_dec_sine_app_create_by_parm(info.sin, info.sin_num, !info.preemption);
        if (ktone->dec_sin == NULL) {
            return -1;
        }
        ktone->dec_sin->dec->evt_cb = key_tone_dec_sine_app_evt_cb;
        ktone->dec_sin->priv = ktone;
        /* audio_dec_sine_app_open(ktone->dec_sin); */
        ktone->dec_sin->dec->wait.only_del = 1;
        audio_dec_app_start(ktone->dec_sin->dec);
        return 0;
    }

    // 判断文件名后缀
    u8 file_name[16];
    char *format = NULL;
    FILE *file = fopen(info.file_name, "r");
    if (!file) {
        return -1;
    }
    fget_name(file, file_name, 16);
    format = get_file_ext_name((char *)file_name);
    fclose(file);

    // 正弦波文件播放
    if (ASCII_StrCmpNoCase(format, "sin", 3) == 0) {
        ktone->dec_sin = audio_dec_sine_app_create(info.file_name, !info.preemption);
        if (ktone->dec_sin == NULL) {
            return -1;
        }
        ktone->dec_sin->dec->evt_cb = key_tone_dec_sine_app_evt_cb;
        ktone->dec_sin->priv = ktone;
        /* audio_dec_sine_app_open(ktone->dec_sin); */
        ktone->dec_sin->dec->wait.only_del = 1;
        audio_dec_app_start(ktone->dec_sin->dec);
        return 0;
    }

    // 普通文件播放
    ktone->dec_file = audio_dec_file_app_create(info.file_name, !info.preemption);
    if (ktone->dec_file == NULL) {
        return -1;
    }
    if (ktone->dec_file->dec->dec_type == AUDIO_CODING_SBC) {
        audio_dec_app_set_frame_info(ktone->dec_file->dec, 0x4e, ktone->dec_file->dec->dec_type);
    }
    ktone->dec_file->dec->evt_cb = key_tone_dec_file_app_evt_cb;
    ktone->dec_file->priv = ktone;
    /* audio_dec_file_app_open(ktone->dec_file); */
    ktone->dec_file->dec->wait.only_del = 1;
    audio_dec_app_start(ktone->dec_file->dec);
    return 0;
}

/*
*********************************************************************
*                  Audio Key Tone Task Run
* Description: 按键提示音任务处理
* Arguments  : *p		按键提示音解码句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
static void key_tone_task_deal(void *p)
{
    int res = 0;
    int msg[16];
    struct __key_tone *ktone = (struct __key_tone *)p;
    ktone->start = 1;
    ktone->busy = 1;
    while (1) {
        os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        res = key_tone_play_run(ktone);
        if (res) {
            ///等待删除线程
            key_tone_play_close(ktone);
            ktone->busy = 0;
            while (1) {
                os_time_dly(10000);
            }
        }
    }
}

/*
*********************************************************************
*                  Audio Key Tone Destroy
* Description: 注销按键提示音播放
* Arguments  : None.
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_key_tone_destroy(void)
{
    if (!key_tone) {
        return ;
    }

    key_tone->start = 0;

    while (key_tone->busy) {
        os_taskq_post_msg(AUDIO_KEY_TONE_TASK_NAME, 1, 0);
        os_time_dly(1);
    }

    task_kill(AUDIO_KEY_TONE_TASK_NAME);

    local_irq_disable();
    free(key_tone);
    key_tone = NULL;
    local_irq_enable();
}

/*
*********************************************************************
*                  Audio Key Tone Init
* Description: 按键提示音初始化
* Arguments  : None.
* Return	 : true		成功
* Note(s)    : None.
*********************************************************************
*/
int audio_key_tone_init(void)
{
    int err = 0;
    if (key_tone) {
        return true;
    }
    struct __key_tone *ktone = zalloc(sizeof(struct __key_tone));
    err = task_create(key_tone_task_deal, (void *)ktone, AUDIO_KEY_TONE_TASK_NAME);
    if (err != OS_NO_ERR) {
        log_e("%s creat fail %x\n", __FUNCTION__,  err);
        free(ktone);
        return false;
    }
#if AUDIO_KEY_TONE_DIGVOL_EN
    ktone->volume = get_max_sys_vol();
#endif
    key_tone = ktone;
    return true;
}

//////////////////////////////////////////////////////////////////////////////

// 正弦波序号
#define KTONE_SINE_NORAML           0

// 按键提示音序号
enum {
    KTONE_IDEX_NORAML = 0,
    KTONE_IDEX_NUM_0,
    KTONE_IDEX_NUM_1,

    KTONE_IDEX_MAX,
};

// 序号对应表
static const char *const key_tone_index[] = {
    [KTONE_IDEX_NORAML] 	= DEFAULT_SINE_TONE(KTONE_SINE_NORAML),
    [KTONE_IDEX_NUM_0] 		= SDFILE_RES_ROOT_PATH"tone/0.*",
    [KTONE_IDEX_NUM_1] 		= SDFILE_RES_ROOT_PATH"tone/1.*",
};

/*
 * 正弦波参数配置:
 * freq : 实际频率 * 512
 * points : 正弦波点数
 * win : 正弦窗
 * decay : 衰减系数(百分比), 正弦窗模式下为频率设置：频率*512
 */
static const struct sin_param sine_16k_normal[] = {
    /*{0, 1000, 0, 100},*/
    {200 << 9, 4000, 0, 100},
};


//////////////////////////////////////////////////////////////////////////////

/*
*********************************************************************
*                  Audio Key Tone Get Sine Param Data
* Description: 获取正弦波数组
* Arguments  : id		正弦波序号
*              *num		正弦波数组长度
* Return	 : 正弦波数组
* Note(s)    : None.
*********************************************************************
*/
static const struct sin_param *get_sine_param_data(u8 id, u8 *num)
{
    const struct sin_param *param_data = NULL;
    switch (id) {
    case KTONE_SINE_NORAML:
        param_data = sine_16k_normal;
        *num = ARRAY_SIZE(sine_16k_normal);
        break;
    default:
        return NULL;
    }
    return param_data;
};

/*
*********************************************************************
*                  Audio Key Tone Open With Callback
* Description: 打开文件播放
* Arguments  : *file_name		文件名
*              *sin				正弦波数组
*              sin_num			正弦波数组长度
*              preemption		1-抢断播放；0-叠加播放
*              evt_handler		事件回调
*              evt_priv			事件回调参数
* Return	 : 0	正常
* Note(s)    : None.
*********************************************************************
*/
static int ktone_play_open_with_callback_base(const char *file_name, struct audio_sin_param *sin, u8 sin_num, u8 preemption, void (*evt_handler)(void *priv, int flag), void *evt_priv)
{
    struct __key_tone *ktone = key_tone;
    if ((!ktone) || (!ktone->start)) {
        return -1;
    }

    struct __key_tone_play_info info = {0};
    info.preemption = preemption;
    info.evt_handler = evt_handler;
    info.evt_priv = evt_priv;
    if (sin && sin_num) {
        info.sin = sin;
        info.sin_num = sin_num;
    } else {
        info.file_name = file_name;
        if (IS_DEFAULT_SINE(file_name)) {
            info.sin = get_sine_param_data(DEFAULT_SINE_ID(file_name), &info.sin_num);
        }
    }

    local_irq_disable();
    memcpy(&ktone->play_info, &info, sizeof(struct __key_tone_play_info));
    ktone->play = 1;
    local_irq_enable();

    os_taskq_post_msg(AUDIO_KEY_TONE_TASK_NAME, 1, 0);

    return 0;
}

/*
*********************************************************************
*                  Audio Key Tone Play Sine
* Description: 播放正弦波数组
* Arguments  : *sin				正弦波数组
*              sin_num			正弦波数组长度
*              preemption		1-抢断播放；0-叠加播放
* Return	 : 0	正常
* Note(s)    : None.
*********************************************************************
*/
int audio_key_tone_play_sin(struct audio_sin_param *sin, u8 sin_num, u8 preemption)
{
    /* y_printf("n:0x%x \n", name); */
    return ktone_play_open_with_callback_base(NULL, sin, sin_num, preemption, NULL, NULL);
}

/*
*********************************************************************
*                  Audio Key Tone Play File
* Description: 播放文件
* Arguments  : *name			文件名
*              preemption		1-抢断播放；0-叠加播放
* Return	 : 0	正常
* Note(s)    : None.
*********************************************************************
*/
int audio_key_tone_play_name(const char *name, u8 preemption)
{
    /* y_printf("n:0x%x \n", name); */
    return ktone_play_open_with_callback_base(name, NULL, 0, preemption, NULL, NULL);
}

/*
*********************************************************************
*                  Audio Key Tone Play Index
* Description: 按序号播放文件
* Arguments  : index			序号
*              preemption		1-抢断播放；0-叠加播放
* Return	 : 0	正常
* Note(s)    : 使用key_tone_index[]数组
*********************************************************************
*/
int audio_key_tone_play_index(u8 index, u8 preemption)
{
    if (index >= KTONE_IDEX_MAX) {
        return -1;
    }
    return audio_key_tone_play_name(key_tone_index[index], preemption);
}

/*
*********************************************************************
*                  Audio Key Tone Play
* Description: 按键提示音播放
* Arguments  : None.
* Return	 : None.
* Note(s)    : 默认播放
*********************************************************************
*/
void audio_key_tone_play(void)
{
    audio_key_tone_play_index(KTONE_IDEX_NORAML, 0);
    /* audio_key_tone_play_index(KTONE_IDEX_NUM_0, 0); */
    /* audio_key_tone_play_sin(sine_16k_normal, ARRAY_SIZE(sine_16k_normal), 0); */
}

/*
*********************************************************************
*                  Audio Key Tone Check Play
* Description: 检测按键提示音是否在播放
* Arguments  : None.
* Return	 : true		正在播放
* Note(s)    : None.
*********************************************************************
*/
int audio_key_tone_is_play(void)
{
    if ((!key_tone) || (!key_tone->start)) {
        return false;
    }
    if (key_tone->dec_file || key_tone->dec_sin) {
        return true;
    }
    return false;
}

/*
*********************************************************************
*                  Audio Key Tone Set Digvol
* Description: 设置按键提示音的音量
* Arguments  : volume		音量
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_key_tone_digvol_set(u8 volume)
{
#if AUDIO_KEY_TONE_DIGVOL_EN
    log_i("vol:%d \n", volume);
    if (!key_tone) {
        return ;
    }
    key_tone->volume = volume;
    void *vol_hdl = audio_dig_vol_group_hdl_get(sys_digvol_group, AUDIO_KEY_TONE_DIGVOL_NAME);
    if (vol_hdl == NULL) {
        return;
    }
    audio_dig_vol_set(vol_hdl, AUDIO_DIG_VOL_ALL_CH, volume);
#endif
}


#endif

