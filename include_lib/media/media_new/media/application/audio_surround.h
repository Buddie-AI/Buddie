
#ifndef _AUDIO_SURROUND_API_H_
#define _AUDIO_SURROUND_API_H_
#include "surround/effect_sur_api.h"
// #include "audio_config.h"

#define SURROUND_USE_AUDIO_STREAM   0 //是否使用audio_sream流节点

typedef struct _surround_update_parm {
    int surround_type;//音效类型
    int rotatestep;//旋转速度
    int damping;//高频衰减速度
    int feedback;//整体衰减速度
    int roomsize;//空间大小
} surround_update_parm;

typedef struct _surround_open_parm {
    u8 channel;//通道数立体声配2， 左声道配EFFECT_CH_L, 右声道配EFFECT_CH_R
    u8 surround_effect_type;//默认的环绕音效类型
} surround_open_parm;


typedef struct _surround_hdl {
    SUR_FUNC_API *ops;
    void *work_buf;
    OS_MUTEX mutex;
    u8 surround_en: 1;
    u8 update: 1;
    u8 nch_update: 1;

    surround_open_parm parm;

#if SURROUND_USE_AUDIO_STREAM
    struct audio_stream_entry entry;	// 音频流入口
#endif

} surround_hdl;

/*cmd*/
enum {
    EFFECT_3D_PANORAMA = 0,             //3d全景
    EFFECT_3D_ROTATES,                  //3d环绕
    EFFECT_FLOATING_VOICE,              //流动人声
    EFFECT_GLORY_OF_KINGS,              //王者荣耀
    EFFECT_FOUR_SENSION_BATTLEFIELD,    //四季战场

    EFFECT_SUR2,             //一种新的环绕声(仿某耳机的环绕音效)需将const_surround_en|BIT(2)
    /*如使用以下效果，需将const_surround_en|BIT(1), mips占用 55M ram33k*/
    EFFECT_3D_PANORAMA2,     //3d全景另一种效果
    EFFECT_KTV,       	      //ktv模式
    EFFECT_3DTHE,            //3D影院
    EFFECT_HALL,             //音乐厅
    EFFECT_VOICE,            //清澈人声
    EFFECT_SUR,              //全景环绕

    EFFECT_OFF2,               //音效开关(数据会经过模块，但不做处理,保证数据延续，避免由音效开关引起的哒哒音，)
    EFFECT_OFF,               //音效开关(数据直接不经过模块)
};


//{EFFECT_3D_TYPE1,  	2, 	120, 110, 128},//3D全景
//{EFFECT_3D_ROTATE, 	2,   60,  70, 128},//3D旋转， step旋转速度 建议1~8
//{EFFECT_3D_LRDRIFT,	140, 120,  95, 128}, //流动人声,step流动速度 建议40~400
//{EFFECT_3D_TYPE0,  	2, 	120,  95, 128},//王者荣耀
//{EFFECT_3D_TYPE1,  	2, 	 60, 100,  30},//四季战场,同全景，但把整体调小了
//以上是内置默认效果


#if 0
//如启用多效果调节，需将const_surround_en|BIT(1),调用update接口传参使用。
//{EFFECT_3D_TYPE2,  	2, 	 90, 100,  100},//3d全景的另一效果，该效果，mips占用 55M ram33k.
//第一个参数2是无效参数.
//第2个参数90调节反馈强度范围是1到150，大点的话空间感比较强。
//第3个参数是 湿声增益，可调范围是0到128。
//第4个参数调节空间大小，范围是0-100，是反馈参数，有效范围0到100。

//例如：
surround_update_parm parm = {0};
parm.surround_type = EFFECT_3D_TYPE2;
parm.rotatestep 	 = 2;
parm.damping       = 90;
parm.feedback      = 100;
parm.roomsize      = 100;
audio_surround_parm_update(hdl, EFFECT_3D_PANORAMA2, &parm);
#endif

#if 0
//如需要自定义 ktv模式、3d影院、音乐厅、清澈人声、全景环绕
//参数顺序
//dry,wet,delay,rot60,Erwet,Erfactor,Ewidth,Ertolate,predelay,width,diffusion,dampinglpf,basslpf,bassB
const static  int  surmode_present[][14] = {
    {55, 70, 35, 8000, 80, 80, 80, 80, 20, 100, 70, 6500, 4000, 18}, //EFFECT_KTV
    {60, 60, 90, 9030, 90, 90, 90, 90, 18, 100, 75, 7000, 1000, 29}, //EFFECT_3DTHE
    {50, 70, 100, 12000, 100, 100, 100, 100, 20, 100, 72, 9000, 50, 3}, //EFFECT_HALL
    {40, 70, 20, 2010, 100, 70, 70, 100, 15, 100, 70, 5500, 3500, 60}, //EFFECT_VOICE
    {60, 70, 100, 9000, 80, 80, 80, 90, 10, 100, 72, 8000, 500, 15}, //EFFECT_SUR
};

parm.surround_type = EFFECT_3D_TYPE2;
parm.rotatestep 	 = 2;
parm.damping       = -1;
parm.feedback      = 100;
parm.roomsize      = (int)surmode_present[0];
audio_surround_parm_update(hdl, EFFECT_KTV, &parm);

//如使用默认的 ktv模式、3d影院、音乐厅、清澈人声、全景环绕
audio_surround_parm_update(hdl, EFFECT_KTV, NULL);
audio_surround_parm_update(hdl, EFFECT_3DTHE, NULL);
audio_surround_parm_update(hdl, EFFECT_HALL, NULL);
audio_surround_parm_update(hdl, EFFECT_VOICE, NULL);
audio_surround_parm_update(hdl, EFFECT_SUR, NULL);
#endif


/*----------------------------------------------------------------------------*/
/**@brief   audio_surround_open  环绕音效打开
   @param    *_parm: 环绕音效始化参数，详见结构体surround_open_parm
   @return   句柄
   @note
*/
/*----------------------------------------------------------------------------*/
surround_hdl *audio_surround_open(surround_open_parm *parm);

/*----------------------------------------------------------------------------*/
/**@brief   audio_surround_parm_update 环绕音效参数更新
   @param    cmd:EFFECT_3D_PANORAMA,EFFECT_3D_ROTATES,EFFECT_FLOATING_VOICE,EFFECT_GLORY_OF_KINGS,EFFECT_FOUR_SENSION_BATTLEFIELD
   @param    *_parm:参数指针,NULL则使用默认德参数，否则传入自定义参数
   @return   0：成功  -1: 失败
   @note     对耳时，左右声道效果，须设置保持一致
*/
/*----------------------------------------------------------------------------*/
int audio_surround_parm_update(surround_hdl *_hdl, u32 cmd, surround_update_parm *_parm);

/* surround_update_parm parm = {0}; */
// parm.surround_type = EFFECT_3D_TYPE1;
// parm.rotatestep = 2;//旋转速度
// parm.damping = 120;//高频衰减速度
// parm.feedback = 110;//整体衰减速度
// parm.roomsize = 128;//空间大小
/* audio_surround_parm_update(hdl, EFFECT_3D_PANORAMA, &parm); */
/*----------------------------------------------------------------------------*/
/**@brief   audio_surround_close 环绕音效关闭处理
   @param    _hdl:句柄
   @return  0:成功  -1：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_surround_close(surround_hdl *_hdl);

/*----------------------------------------------------------------------------*/
/**@brief   audio_surround_run 环绕音效处理
   @param    _hdl:句柄
   @param    data:需要处理的数据
   @param    len:数据长度
   @return  0:成功  -1：失败
   @note    无数据流节点时，直接使用改接口进行环绕音效的处理
*/
/*----------------------------------------------------------------------------*/
int audio_surround_run(surround_hdl *_hdl, void *data, u32 len);


/*----------------------------------------------------------------------------*/
/**@brief   audio_surround_switch_nch  通道切换
   @param    _hdl:句柄
   @param    nch:通道数 2 4 或者EFFECT_CH_L EFFECT_CH_R
   @return  0:成功  -1：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_surround_switch_nch(surround_hdl *_hdl, int nch);
#endif
