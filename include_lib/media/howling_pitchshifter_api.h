
#ifndef HOWLING_pitchshifer_api_h__
#define HOWLING_pitchshifer_api_h__

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
    EFFECT_HOWLING_PS        = 0x01,              //1.5《=》12 ms
    EFFECT_HOWLING_HE       = 0x02,
    EFFECT_HOWLING_FS       = 0x04,
};

enum {
    PLATFORM_BR30 = 0,
    PLATFORM_BR36 = 1
};

typedef struct HOWLING_PITCHSHIFT_PARM_ {
    s16 ps_parm;
    s16 fe_parm;
    u32 effect_v;
} HOWLING_PITCHSHIFT_PARM;



typedef struct _HOWLING_PITCHSHIFT_FUNC_API_ {
    u32(*need_buf)(int flag);
    void (*open)(void *ptr, u32 sr, HOWLING_PITCHSHIFT_PARM *pitchshift_obj);       //中途改变参数，可以调init
    void (*run)(void *ptr, short *indata, short *outdata, int len);   //len是多少个byte
} HOWLING_PITCHSHIFT_FUNC_API;

extern HOWLING_PITCHSHIFT_FUNC_API *get_howling_ps_func_api();

#endif // reverb_api_h__

