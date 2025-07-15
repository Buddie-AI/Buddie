#ifndef effect_surTheta_h__
#define effect_surTheta_h__

typedef struct _PointSound360TD_PARM_SET {
    int theta;
    int volume;
} PointSound360TD_PARM_SET;

//混响参数
typedef struct  _RP_PARM_CONIFG {
    int trackKIND;    //角度合成算法用哪一种 :0或者1
    int ReverbKIND;   //2或者3
    int reverbance;   //湿声比例 : 0~100
    int dampingval;   //高频decay  ：0~80
} RP_PARM_CONIFG;

typedef struct _PointSound360Reverb_PARM_SET {
    unsigned char reverbance;     //0-100: //reverb parm
    unsigned char dampingval;        //0-100% //reverb parm
    unsigned char voltrack;        // 0-1  // vol track
    char doangle;                    // 0 or  1

    char diangle;                 // 0-360 : 2 channel angle
    unsigned char wet;            //0-100://backgroud
    unsigned char dry;            //0-100 ://voice part
    char reverb_kind;             //reverb kind

    unsigned short k1_w;             //0-2000: output wetgain
    unsigned short k1_d;              //0-2000: output drygain

    char reverb_part;            // 0-128   // mix  wet  and  wet_dry reverb

    char reserved0;
    short dest_vol;           // dest vol;

} PointSound360Reverb_PARM_SET;

//角度合成算法定义
enum {
    P360_T0 = 0,
    P360_T1 = 1
};

//混响算法定义
enum {
    P360_R0 = 2,
    P360_R1 = 3
};

enum {
    PSOUND_BOTH = 0,
    PSOUND_L = 1,
    PSOUND_R = 2
};

enum {
    P360TD_NO_REV = 0,
    P360TD_REV_K0 = 1,
    P360TD_REV_K1 = 2,
    P360TD_REV_K3 = 3,
    P360TD_REV_K4 = 4
};




typedef struct __PointSound360TD_FUNC_API_ {
    unsigned int (*need_buf)(int flag);
    unsigned int (*open)(unsigned int *ptr, PointSound360TD_PARM_SET *sound360_parm, PointSound360Reverb_PARM_SET *reverb_parm, int ch_mode);
    unsigned int (*init)(unsigned int *ptr, PointSound360TD_PARM_SET *sound360_parm);
    unsigned int (*run)(unsigned int *ptr, short *inbuf, short *out, int len);             // len是sizePerChannel
    unsigned int (*open_config)(unsigned int *ptr, PointSound360TD_PARM_SET *sound360_parm, RP_PARM_CONIFG *sound360_configparm);
} PointSound360TD_FUNC_API;



extern PointSound360TD_FUNC_API *get_PointSound360TD_func_api();


#endif // reverb_api_h__
