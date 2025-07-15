
#ifndef pitchshifer_api_h__
#define pitchshifer_api_h__

#ifndef u8
#define u8  unsigned char
#endif

#ifndef u16
#define u16 unsigned short
#endif

#ifndef s16
#define s16 short
#endif


#ifndef u32
#define u32 unsigned int
#endif


#ifndef s32
#define s32 int
#endif

#ifndef s16
#define s16 signed short
#endif

/*#define  EFFECT_OLD_RECORD          0x01
#define  EFFECT_MOYIN               0x0*/
//#define  EFFECT_ROBORT_FLAG          0X04

enum {
    EFFECT_PITCH_SHIFT       = 0x00,
    EFFECT_VOICECHANGE_KIN0,
    EFFECT_VOICECHANGE_KIN1,
    EFFECT_VOICECHANGE_KIN2,
    EFFECT_ROBORT,

    EFFECT_AUTOTUNE = 0xfe,
    EFFECT_FUNC_NULL = 0xff,
};

typedef struct PITCH_SHIFT_PARM_ {
    u32 sr;                          //input  audio samplerate
    u32 shiftv;                      //pitch rate:  <100(pitch up), >100(pitch down)
    u32 effect_v;
    u32 formant_shift;
} PITCH_SHIFT_PARM;

/******
效果配置说明：
1.EFFECT_PITCH_SHIFT  移频，变调不变速，init_parm.shiftv调节有效，init_parm.formant_shift调节无效
2.EFFECT_VOICECHANGE_KIN0  变声，可以调节不同的 init_parm.shiftv  跟 init_parm.formant_shift ，调出更多的声音效果
3.EFFECT_VOICECHANGE_KIN1  变声，同EFFECT_VOICECHANGE_KIN0类似的，但是2者由于运算的不同，会有区别。
4.EFFECT_ROBORT 机器音效果，类似 喜马拉雅那样的
5.EFFECT_AUTOTUNE  电音效果
*******/

typedef struct _PITCHSHIFTER_DSP_RUNFUNP_ {
    int *buf;
    int *inlen;
} PITCHSHIFTER_RUNFUNP;


#define  C_MAJOR                                8192 //c大调
#define  Cshop_MAJOR                            8875//升c大调
#define  D_MAJOR                                9557
#define  Dshop_MAJOR                            10240
#define  E_MAJOR                                10923
#define  F_MAJOR                                11605
#define  Fshop_MAJOR                            12288
#define  G_MAJOR                                12971
#define  Gshop_MAJOR                            13653
#define  A_MAJOR                                14336
#define  Ashop_MAJOR                            15019
#define  B_MAJOR                                15701


typedef struct _PITCHSHIFT_FUNC_API_ {
    u32(*need_buf)(void *ptr, PITCH_SHIFT_PARM *pitchshift_obj);
    void (*open)(void *ptr, PITCH_SHIFT_PARM *pitchshift_obj);        //中途改变参数，可以调init
    void (*run)(void *ptr, short *indata, short *outdata, int len);   //len是多少个byte
    void (*init)(void *ptr, PITCH_SHIFT_PARM *pitchshift_obj);        //中途改变参数，可以调init
} PITCHSHIFT_FUNC_API;

extern PITCHSHIFT_FUNC_API *get_pitchshift_func_api();

#endif // reverb_api_h__

