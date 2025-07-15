#ifndef effectSUR_api_h__
#define effectSUR_api_h__

/*单耳输入时，channel设置*/
enum {
    EFFECT_CH_L = 0x10,                 //单声道输入，输出左声道
    EFFECT_CH_R = 0x20,                 //单声道输入，输出右声道
    EFFECT_CH2_L = 0x30,                  //双声道输入，输出2个左声道
    EFFECT_CH2_R = 0x40,                  //双声道输入，输出2个右声道
};

enum {
    EFFECT_3D_TYPE0 = 0x01,
    EFFECT_3D_TYPE1 = 0x02,            //这2个2选1  ：如果都置上，优先用EFFECT_3D_TYPE1
    EFFECT_3D_LRDRIFT = 0x04,
    EFFECT_3D_ROTATE = 0x08,           //这2个2选1 : 如果都置上，优先用EFFECT_3D_ROTATE
    EFFECT_3D_TYPE2 = 0x10,
    EFFECT_3D_LRDRIFT2 = 0x20,
};


typedef struct __SUR_FUNC_API_ {
    unsigned int (*need_buf)(int flag);
    unsigned int (*open)(unsigned int *ptr, int effectflag, int nch);
    unsigned int (*init)(unsigned int *ptr, int rotatestep, int damping, int feedback, int roomsize);
    unsigned int (*run)(unsigned int *ptr, short *inbuf, int len);             // len是对点
    unsigned int (*switch_effect)(unsigned int *ptr, int effectflag, int rotatestep, int damping, int feedback, int roomsize);//新增的切换音效参数接口
    unsigned int (*switch_nch)(unsigned int *ptr, int nch);//新增的声道修改接口
} SUR_FUNC_API;


extern SUR_FUNC_API *get_sur_func_api();



#if 0
因为复用其他音效的参数接口，所以还是用了4个参数，EFFECT_3D_TYPE2具体参数对应名称应该是 ： {
    int rotatestep;                 //无效参数
    int rot60_100ms;              //范围0到150
    int wetgain ;                   //范围0到100
    int delay ;                      //范围0到100
}

默认参数可以用 FFECT_3D_TYPE2  {2, 90, 70, 100},

// 由于原本预留参数有些没留出来，为了可以调节频响之类的多效果，所以在原基础上通过把参数列表指针当做其中一个参数传进来
// 如果 rot60_100ms小于0，wetgain=100的话， 那最后一个参数就是参数列表，这样就可以支持配置更多参数。

设置如下：

flag = EFFECT_3D_TYPE2;
{ {2, -1, 100, surmode_present[MODE_SUR_KTV]},
    {2, -1, 100, surmode_present[MODE_SUR_3DTHE]},
    {2, -1, 100, surmode_present[MODE_SUR_HALL]},
    {2, -1, 100, surmode_present[MODE_SUR_VOICE]},
    {2, -1, 100, surmode_present[MODE_SUR_SUR]}
}


//其中参数示例：

enum {
    MODE_SUR_KTV = 0,       //ktv模式
    MODE_SUR_3DTHE,         //3D影院
    MODE_SUR_HALL,          //音乐厅
    MODE_SUR_VOICE,         //清澈人声
    MODE_SUR_SUR,            //全景环绕
    MODE_SUR_MAX
};

//dry,wet,delay,rot60,Erwet,Erfactor,Ewidth,Ertolate,predelay,width,diffusion,dampinglpf,basslpf,bassB
const static  int  surmode_present[MODE_SUR_MAX][14] = {
    {80, 100, 35, 9000, 80, 80, 80, 80, 20, 100, 70, 6500, 4000, 18},
    {80, 100, 90, 10030, 90, 90, 90, 90, 18, 100, 75, 7000, 1000, 29},
    {80, 100, 100, 13000, 100, 100, 100, 100, 20, 100, 72, 9000, 50, 3},
    {80, 100, 20, 5010, 100, 70, 70, 100, 15, 100, 70, 5500, 3500, 60},
    {80, 100, 100, 10000, 80, 80, 80, 90, 10, 100, 72, 8000, 500, 15},
};


//dry,wet,delay,Erwet,Erfactor,Ewitdh,Ertolate,width,diffusion 范围都是 0到 100
//rot60 范围是 0到15000, 单位ms
//basslpf: 低频增强频率，范围 0到10000
//dampinglpf: 高频衰减频率，范围 0到 10000
//bassB: 低频增强比例


混响调参说明：
1.dry： 干声增益
2.wet:
整体效果声增益
3.delay： 反射时间间隔，影响整体空间大小
4.rot60:
衰减60dB需要的时间, 即衰减速度，如果空间里面的物体吸声厉害，或者比较空旷，衰减比较快
5.Erwet：早反射声的增益， 早反射声的存在 会让混响听起来更真实。但是不同的增益会影响听感上距离感，空间构造
6.Erfactor： 早反射声的一个密集程度。 这个值大，早反射声之间间隔比较大。构造的空间感也会相对较大。 较小的话，就反射比较密集。空间感变小。
7.Ewidth：影响早反射声左右声道的声音差异
8.Ertolate:
影响漫反射声音反馈的大小
9.predelay：干声跟效果声的时间间隔。也会影响整体空间感。
10.width：左右声道的差异，左右声道的差异，会产生立体感。（某些版本的sdk上该参数无效）
11.diffusion： 发散程度, 影响 漫反射声音的变形程度。
12.dampinglpf：高频衰减过度频率。反射声的一个频率衰减，这是一个斜着往下掉的曲线，影响了反射声中的高频分量。高频分量越多，整体音色越亮，但是成分太多，声音有些乱
13.basslpf：反射声中的低频增强分量过度频率
14.bassB:
低频增强比例。 这里的低频增强，其实是压了高频。为了使整体频响不超过1，不引起反馈发散。所以加大了这个值感觉混响弱了。可以适量加大rot60或者Ertolate
#endif
#endif // reverb_api_h__
