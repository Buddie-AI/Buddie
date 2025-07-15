/*****************************************************************
>file name : apps/common/sine_make.c
>author : lichao
>create time : Sun 05 May 2019 08:37:35 PM CST
*****************************************************************/
#include "system/includes.h"
#include "sine_make.h"

#define SINE_USE_MALLOC			1

struct audio_sin_maker {
    u8 open;
    u8 id;
    u8 sin_num;
    u8 channel;
    u8 repeat;
    u8 next_param;
    u16 fade_points;
    /*int sample_rate;*/
    int volume;
    int points;
    int sin_index;
    int win_sin_index;
    const struct sin_param *sin;
};

#ifndef SINE_USE_MALLOC
static struct audio_sin_maker sin_maker_handle;
#endif

void *sin_tone_open(const struct sin_param *param, int num, u8 channel, u8 repeat)
{
    if (!param || !num) {
        return NULL;
    }

#ifdef SINE_USE_MALLOC
    struct audio_sin_maker *sin = zalloc(sizeof(struct audio_sin_maker));
#else
    struct audio_sin_maker *sin = &sin_maker_handle;
#endif

    if (!sin) {
        return NULL;
    }
    if (sin->open) {
        return NULL;
    }

    sin->sin = param;
    sin->sin_num = num;
    sin->open = 1;
    sin->points = param[0].points;
    sin->id = 0;
    sin->channel = channel;
    sin->repeat = repeat;
#if (defined CONFIG_CPU_BR18 || \
     defined CONFIG_CPU_BR21)
    /*这里如果加多fade points，会导致结束的时候有噪声,不是在淡出尾部*/
    sin->fade_points = 0;
#else
    sin->fade_points = 480;
#endif
    sin->next_param = 1;

    return (void *)sin;
}
#if !defined(SINE_MAKE_IN_MASK)
const int sf_sin_tab1[513] = {
    0x00000000, 0x0000c910, 0x0001921f, 0x00025b2d, 0x0003243a, 0x0003ed45, 0x0004b64e, 0x00057f53,
    0x00064855, 0x00071154, 0x0007da4e, 0x0008a343, 0x00096c33, 0x000a351d, 0x000afe00, 0x000bc6dd,
    0x000c8fb3, 0x000d5881, 0x000e2147, 0x000eea03, 0x000fb2b7, 0x00107b61, 0x00114401, 0x00120c96,
    0x0012d521, 0x00139d9f, 0x00146611, 0x00152e77, 0x0015f6d0, 0x0016bf1b, 0x00178758, 0x00184f87,
    0x001917a7, 0x0019dfb7, 0x001aa7b7, 0x001b6fa7, 0x001c3786, 0x001cff53, 0x001dc70f, 0x001e8eb8,
    0x001f564e, 0x00201dd1, 0x0020e541, 0x0021ac9b, 0x002273e2, 0x00233b13, 0x0024022e, 0x0024c933,
    0x00259021, 0x002656f8, 0x00271db7, 0x0027e45f, 0x0028aaed, 0x00297163, 0x002a37bf, 0x002afe01,
    0x002bc429, 0x002c8a35, 0x002d5026, 0x002e15fb, 0x002edbb4, 0x002fa14f, 0x003066ce, 0x00312c2e,
    0x0031f170, 0x0032b694, 0x00337b98, 0x0034407c, 0x00350540, 0x0035c9e4, 0x00368e66, 0x003752c6,
    0x00381705, 0x0038db21, 0x00399f19, 0x003a62ef, 0x003b26a0, 0x003bea2c, 0x003cad94, 0x003d70d7,
    0x003e33f3, 0x003ef6e9, 0x003fb9b8, 0x00407c60, 0x00413ee0, 0x00420138, 0x0042c367, 0x0043856d,
    0x0044474a, 0x004508fc, 0x0045ca83, 0x00468be0, 0x00474d11, 0x00480e16, 0x0048ceef, 0x00498f9a,
    0x004a5019, 0x004b1069, 0x004bd08b, 0x004c907f, 0x004d5043, 0x004e0fd8, 0x004ecf3c, 0x004f8e70,
    0x00504d72, 0x00510c43, 0x0051cae3, 0x0052894f, 0x00534789, 0x0054058f, 0x0054c362, 0x00558100,
    0x00563e6a, 0x0056fb9e, 0x0057b89d, 0x00587565, 0x005931f7, 0x0059ee52, 0x005aaa76, 0x005b6662,
    0x005c2215, 0x005cdd8f, 0x005d98d0, 0x005e53d8, 0x005f0ea5, 0x005fc937, 0x0060838f, 0x00613dab,
    0x0061f78b, 0x0062b12e, 0x00636a95, 0x006423be, 0x0064dcaa, 0x00659557, 0x00664dc6, 0x006705f5,
    0x0067bde5, 0x00687595, 0x00692d05, 0x0069e433, 0x006a9b21, 0x006b51cc, 0x006c0836, 0x006cbe5c,
    0x006d7440, 0x006e29e0, 0x006edf3d, 0x006f9454, 0x00704927, 0x0070fdb5, 0x0071b1fd, 0x007265ff,
    0x007319ba, 0x0073cd2f, 0x0074805c, 0x00753341, 0x0075e5dd, 0x00769831, 0x00774a3c, 0x0077fbfe,
    0x0078ad75, 0x00795ea2, 0x007a0f84, 0x007ac01a, 0x007b7065, 0x007c2064, 0x007cd016, 0x007d7f7c,
    0x007e2e93, 0x007edd5d, 0x007f8bd9, 0x00803a06, 0x0080e7e4, 0x00819573, 0x008242b1, 0x0082ef9f,
    0x00839c3d, 0x00844889, 0x0084f484, 0x0085a02c, 0x00864b82, 0x0086f686, 0x0087a136, 0x00884b92,
    0x0088f59b, 0x00899f4e, 0x008a48ad, 0x008af1b7, 0x008b9a6b, 0x008c42c9, 0x008cead0, 0x008d9281,
    0x008e39da, 0x008ee0db, 0x008f8784, 0x00902dd5, 0x0090d3cd, 0x0091796b, 0x00921eb0, 0x0092c39a,
    0x0093682a, 0x00940c5f, 0x0094b039, 0x009553b7, 0x0095f6d9, 0x0096999f, 0x00973c07, 0x0097de12,
    0x00987fc0, 0x0099210f, 0x0099c200, 0x009a6293, 0x009b02c6, 0x009ba299, 0x009c420c, 0x009ce11f,
    0x009d7fd1, 0x009e1e22, 0x009ebc12, 0x009f599f, 0x009ff6cb, 0x00a09393, 0x00a12ff9, 0x00a1cbfb,
    0x00a26799, 0x00a302d3, 0x00a39da9, 0x00a4381a, 0x00a4d225, 0x00a56bcb, 0x00a6050a, 0x00a69de3,
    0x00a73656, 0x00a7ce61, 0x00a86605, 0x00a8fd41, 0x00a99415, 0x00aa2a80, 0x00aac082, 0x00ab561b,
    0x00abeb4a, 0x00ac800f, 0x00ad1469, 0x00ada859, 0x00ae3bde, 0x00aecef7, 0x00af61a5, 0x00aff3e6,
    0x00b085bb, 0x00b11722, 0x00b1a81d, 0x00b238aa, 0x00b2c8c9, 0x00b3587a, 0x00b3e7bc, 0x00b4768f,
    0x00b504f3, 0x00b592e7, 0x00b6206c, 0x00b6ad7f, 0x00b73a23, 0x00b7c655, 0x00b85216, 0x00b8dd65,
    0x00b96842, 0x00b9f2ac, 0x00ba7ca4, 0x00bb0629, 0x00bb8f3b, 0x00bc17d9, 0x00bca003, 0x00bd27b8,
    0x00bdaef9, 0x00be35c5, 0x00bebc1b, 0x00bf41fc, 0x00bfc767, 0x00c04c5c, 0x00c0d0da, 0x00c154e1,
    0x00c1d870, 0x00c25b89, 0x00c2de29, 0x00c36051, 0x00c3e200, 0x00c46337, 0x00c4e3f5, 0x00c56439,
    0x00c5e403, 0x00c66354, 0x00c6e22a, 0x00c76085, 0x00c7de65, 0x00c85bca, 0x00c8d8b3, 0x00c95521,
    0x00c9d112, 0x00ca4c87, 0x00cac77f, 0x00cb41fa, 0x00cbbbf8, 0x00cc3578, 0x00ccae79, 0x00cd26fd,
    0x00cd9f02, 0x00ce1689, 0x00ce8d90, 0x00cf0417, 0x00cf7a1f, 0x00cfefa8, 0x00d064af, 0x00d0d937,
    0x00d14d3d, 0x00d1c0c2, 0x00d233c6, 0x00d2a649, 0x00d31849, 0x00d389c7, 0x00d3fac3, 0x00d46b3b,
    0x00d4db31, 0x00d54aa4, 0x00d5b993, 0x00d627fe, 0x00d695e5, 0x00d70348, 0x00d77026, 0x00d7dc7f,
    0x00d84853, 0x00d8b3a1, 0x00d91e6a, 0x00d988ad, 0x00d9f26a, 0x00da5ba0, 0x00dac450, 0x00db2c79,
    0x00db941a, 0x00dbfb34, 0x00dc61c7, 0x00dcc7d1, 0x00dd2d53, 0x00dd924d, 0x00ddf6be, 0x00de5aa6,
    0x00debe05, 0x00df20db, 0x00df8327, 0x00dfe4e9, 0x00e04621, 0x00e0a6cf, 0x00e106f2, 0x00e1668a,
    0x00e1c598, 0x00e2241a, 0x00e28210, 0x00e2df7b, 0x00e33c5a, 0x00e398ac, 0x00e3f473, 0x00e44fac,
    0x00e4aa59, 0x00e50479, 0x00e55e0b, 0x00e5b710, 0x00e60f88, 0x00e66771, 0x00e6becc, 0x00e71599,
    0x00e76bd8, 0x00e7c187, 0x00e816a8, 0x00e86b39, 0x00e8bf3c, 0x00e912ae, 0x00e96591, 0x00e9b7e4,
    0x00ea09a7, 0x00ea5ad9, 0x00eaab7b, 0x00eafb8c, 0x00eb4b0c, 0x00eb99fb, 0x00ebe858, 0x00ec3624,
    0x00ec835e, 0x00ecd007, 0x00ed1c1d, 0x00ed67a1, 0x00edb293, 0x00edfcf2, 0x00ee46be, 0x00ee8ff8,
    0x00eed89e, 0x00ef20b0, 0x00ef6830, 0x00efaf1b, 0x00eff573, 0x00f03b37, 0x00f08066, 0x00f0c501,
    0x00f10908, 0x00f14c7a, 0x00f18f57, 0x00f1d19f, 0x00f21352, 0x00f25470, 0x00f294f8, 0x00f2d4eb,
    0x00f31447, 0x00f3530e, 0x00f3913f, 0x00f3ced9, 0x00f40bdd, 0x00f4484b, 0x00f48422, 0x00f4bf62,
    0x00f4fa0b, 0x00f5341d, 0x00f56d97, 0x00f5a67b, 0x00f5dec6, 0x00f6167a, 0x00f64d97, 0x00f6841b,
    0x00f6ba07, 0x00f6ef5b, 0x00f72417, 0x00f7583a, 0x00f78bc5, 0x00f7beb7, 0x00f7f110, 0x00f822d1,
    0x00f853f8, 0x00f88486, 0x00f8b47b, 0x00f8e3d6, 0x00f91298, 0x00f940c0, 0x00f96e4e, 0x00f99b43,
    0x00f9c79d, 0x00f9f35e, 0x00fa1e84, 0x00fa4910, 0x00fa7302, 0x00fa9c59, 0x00fac516, 0x00faed37,
    0x00fb14be, 0x00fb3bab, 0x00fb61fc, 0x00fb87b2, 0x00fbaccd, 0x00fbd14d, 0x00fbf531, 0x00fc187a,
    0x00fc3b28, 0x00fc5d3a, 0x00fc7eb0, 0x00fc9f8a, 0x00fcbfc9, 0x00fcdf6c, 0x00fcfe73, 0x00fd1cdd,
    0x00fd3aac, 0x00fd57de, 0x00fd7474, 0x00fd906e, 0x00fdabcc, 0x00fdc68c, 0x00fde0b1, 0x00fdfa38,
    0x00fe1324, 0x00fe2b72, 0x00fe4323, 0x00fe5a38, 0x00fe70b0, 0x00fe868b, 0x00fe9bc9, 0x00feb069,
    0x00fec46d, 0x00fed7d4, 0x00feea9d, 0x00fefcc9, 0x00ff0e58, 0x00ff1f49, 0x00ff2f9d, 0x00ff3f54,
    0x00ff4e6d, 0x00ff5ce9, 0x00ff6ac7, 0x00ff7808, 0x00ff84ab, 0x00ff90b1, 0x00ff9c18, 0x00ffa6e3,
    0x00ffb10f, 0x00ffba9e, 0x00ffc38f, 0x00ffcbe2, 0x00ffd397, 0x00ffdaaf, 0x00ffe129, 0x00ffe705,
    0x00ffec43, 0x00fff0e3, 0x00fff4e6, 0x00fff84a, 0x00fffb11, 0x00fffd39, 0x00fffec4, 0x00ffffb1,
    0x00ffffff,
};
#else
extern const int sf_sin_tab1[513] ;
#endif


#define  SINE_INT_ZOOM   16384
#define  SINE_INT_ZBIT   14
#define __int64	long long
/*软件索引实现sin生成*/
static int get_sine_value(int mx_idx)
{
    int ret = 0;
    int idx = 0;
    int tm_idx = mx_idx & 0x1FFFFFF;   //2^25
    int phase = tm_idx & 0x3FFF;
    int tp_idx0 = tm_idx >> 14;
    int tp_idx1 = tp_idx0 + 1;
    int sign = 1, dt0, dt1;

    if (tp_idx0 > 1024) {
        sign = -1;
        tp_idx0 = 2048 - tp_idx0;
    }
    if (tp_idx0 < 513) {
        dt0 = sf_sin_tab1[tp_idx0];
    } else {
        dt0 = sf_sin_tab1[1024 - tp_idx0];
    }

    if (tp_idx1 > 1024) {
        sign = -1;
        tp_idx1 = 2048 - tp_idx1;
    }
    if (tp_idx1 < 513) {
        dt1 = sf_sin_tab1[tp_idx1];
    } else {
        dt1 = sf_sin_tab1[1024 - tp_idx1];
    }

    ret = ((__int64)dt0 * (SINE_INT_ZOOM - phase) + (__int64)dt1 * phase) >> SINE_INT_ZBIT;

    ret *= sign;
    return ret;
}

static void hw_sin_value(int a, int *sin_res, u8 precision)
{
    u64 s64 = a;

    *sin_res = __asm_sine(s64, precision);
}

int sin_tone_make(void *_maker, void *data, int len)
{
    struct audio_sin_maker *maker = (struct audio_sin_maker *)_maker;
    s16 *pcm = (s16 *)data;
    int sin_value = 0;
    int win_sin_value = 0;
    int add_idx = 0, sub_vol = 0, win_add_idx = 0;
    int offset = 0;
    u8 id = maker->id;
    u8 sin_num = maker->sin_num;
    u8 repeat = maker->repeat;
    u8 channel = maker->channel;
    int sin_index;
    int win_sin_index;
    int volume;
    u32 reamin_points = len / 2 / channel;

    do {
        if (maker->next_param) {
            maker->volume = SINE_TOTAL_VOLUME;
            maker->sin_index = 0;
            maker->win_sin_index = 0;
            maker->next_param = 0;
        }

        u8 win = maker->sin[id].win;
        add_idx = ((u64)(1 << 25) * maker->sin[id].freq / DEFAULT_SINE_SAMPLE_RATE) >> 9;
        if (win) {
            win_add_idx = ((u64)(1 << 25) * maker->sin[id].decay / DEFAULT_SINE_SAMPLE_RATE) >> 9;
            /*sub_vol = 0;*/
        } else {
            sub_vol = maker->sin[id].decay;
        }

        sin_index = maker->sin_index;
        win_sin_index = maker->win_sin_index;
        volume = maker->volume;
        u32 points = 0;
        if (maker->fade_points) {
            points = maker->fade_points > reamin_points ? reamin_points : maker->fade_points;
            sub_vol = 0;
            maker->fade_points -= points;
        } else {
            points = maker->points > reamin_points ? reamin_points : maker->points;
            maker->points -= points;
        }
        reamin_points -= points;
        while (points--) {
            /*hw_sin_value(sin_index, &sin_value, 0);*/
            sin_value = __asm_sine((s64)sin_index, 2);
            if (win) {
                /*hw_sin_value(win_sin_index, &win_sin_value, 0);*/
                win_sin_value = __asm_sine((s64)win_sin_index, 2);
#if ((defined CONFIG_CPU_BR36) || (defined CONFIG_CPU_BR28))
                sin_value = ((s64)sin_value * (s64)win_sin_value) >> 44;
#else
                sin_value = ((s64)sin_value * (s64)win_sin_value) >> 34;
#endif
                win_sin_index += win_add_idx;
                win_sin_index &= 0x1ffffff;
            } else {
#if ((defined CONFIG_CPU_BR36) || (defined CONFIG_CPU_BR28))
                sin_value = ((s64)volume * sin_value) >> 39;
#else
                sin_value = ((s64)volume * sin_value) >> 34;
#endif
            }
            sin_index += add_idx;
            sin_index &= 0x1ffffff;

            volume -= sub_vol;
            if (volume < 0) {
                volume = 0;
            }

            *pcm++ = sin_value;
            if (channel == 2) {
                *pcm++ = sin_value;
            } else if (channel == 4) {
                *pcm++ = sin_value;
                *pcm++ = sin_value;
                *pcm++ = sin_value;
            }
        }

        maker->volume = volume;
        maker->sin_index = sin_index;
        maker->win_sin_index = win_sin_index;
        if (!maker->points) {
            if (++id >= sin_num) {
                if (!repeat) {
                    break;
                }
                id = 0;
            }
            maker->points = maker->sin[id].points;
            maker->id = id;
            maker->next_param = 1;
        }

        if (!reamin_points) {
            break;
        }
    } while (1);

    return len - (reamin_points * 2 * channel);
}

int sin_tone_points(void *_maker)
{
    struct audio_sin_maker *maker = (struct audio_sin_maker *)_maker;
    int points = 0;
    u8 i = 0;

    for (i = 0; i < maker->sin_num; i++) {
        points += maker->sin[i].points;
    }

    return points;
}

void sin_tone_close(void *_maker)
{
    struct audio_sin_maker *maker = (struct audio_sin_maker *)_maker;

#ifdef SINE_USE_MALLOC
    if (maker) {
        free(maker);
    }
#else
    if (sin_maker_handle.open) {
        sin_maker_handle.open = 0;
    }
#endif

}

#include "asm/math_fast_function.h"

#ifndef DATA16
#define DATA16		32767
#endif

/*********************************
 * 	 fc 		: 正弦波中心频率
 * 	 fs 		: 采样频率
 * FrameSize	：每次计算输出点数
 *   idx   		:当前计算起始indix
 *   rst    	：结果存放地址
 * *******************************/
void SinWave_Generator(int fc, int fs, int FrameSize, int idx, short *rst)
{
    float tmp0, tmp1, tmp2;
    tmp0 = (float)fc / fs;
    for (int i = 0; i < FrameSize; i++) {
        tmp1 = (i + idx) * tmp0 * 2;
        tmp2 = sin_float(tmp1);
        *rst = (short)(tmp2 * DATA16);
        rst++;
    }
}
/*********************************
 * 	 fs 		: 采样频率
 * FrameSize	：每次计算输出点数
 *   idx   		:当前计算起始indix
 *   ts   		:期望一次扫频所持续时间
 *   rst    	：结果存放地址
 * *******************************/
void SweepSin_Generator(int fs, int FrameSize, int idx, float ts, short *rst)
{
    float fp, fc, tmp1, tmp2;
    int Ncnt, NPoint, DPoint;

    NPoint = fs * ts;
    // fp = (fs/2)/(fs*ts);
    fp = 1 / (2 * 2 * ts);

    for (int i = 0; i < FrameSize; i++) {
        Ncnt = (i + idx) / NPoint;
        DPoint = (i + idx) - Ncnt * NPoint;
        //printf("idx:%d \n",DPoint);
        tmp1 = ((DPoint * fp) / fs) * DPoint * 2;
        tmp2 = sin_float(tmp1);
        //printf("Sin[%d]:%d",i,(int)(tmp2*1000));
        *rst = (short)(tmp2 * DATA16);
        rst++;
    }
}

/*
*********************************************************************
* Description: 正弦信号生成
* Arguments  : fc               信号中心频率
*              fs               信号采样率
*              buf				数据输出地址
*			   len				数据生成长度，单位是bytes
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
int sin_idx = 0;
void sin_pcm_fill(int fc, int fs, void *buf, u32 len)
{
    SinWave_Generator(fc, fs, len / 2, sin_idx, buf);
    sin_idx += len / 2;
}

/*
*********************************************************************
* Description: 扫频信号生成
* Arguments  : fs               信号采样率
*              buf				数据输出地址
*			   len				数据生成长度，单位是bytes
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
int sweep_sin_idx = 0;
void sweepsin_pcm_fill(int fs, void *buf, u32 len)
{
    SweepSin_Generator(fs, len / 2, sweep_sin_idx, 25, buf);
    sweep_sin_idx += len / 2;
}
