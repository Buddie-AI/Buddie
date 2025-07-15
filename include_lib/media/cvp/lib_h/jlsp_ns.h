#ifndef __JLSP_NS_H__
#define __JLSP_NS_H__

//出现单个麦掩蔽参数
typedef struct {
    float detect_time;//检测时间
    float detect_eng_diff_thr; /*0~-90 dB 两个mic能量差异持续大于此阈值超过检测时间则会检测为故障*/
    float detect_eng_lowerbound;// 0~-90 dB start detect when mic energy lower than this
    int MalfuncDet_MaxFrequency;//检测频率上限
    int MalfuncDet_MinFrequency;//检测频率下限
    int OnlyDetect;// 0 -> 故障切换到单mic模式， 1-> 只检测不切换
} JLSP_dms_micsel_cfg;

//beamforming参数设置

typedef struct {
    int UseExWeight;    //挂空
    int *ENC_WeightA;   //挂空
    int *ENC_WeightB;   //挂空
    int ENC_Weight_Q;   //挂空

    int ENC_Process_MaxFrequency;       //设定enc处理的最大频率，设定范围（3000~8000），默认8000
    int ENC_Process_MinFrequency;       //设定enc处理的最小频率，设定范围（0~1000），默认为0
    int SIR_MaxFrequency;               //设定enc信噪比处理最大频率，设定范围（1000~8000），默认3000
    float Mic_Distance;                 //设定主副mic的距离，单位（m），范围（0.015~35），默认0.015
    float Target_Signal_Degradation;    //根据主副麦能量的diff值，设定范围（0~1），默认1.0
    float AggressFactor;                //设置enc压制强度，设定范围（0~4），默认4
    float minSuppress;                  //设定enc抑制的最小值，设定范围（0~0.1），默认0.09

    float compen_db;                    //补偿增益控制，单位（db），具体调节配合麦的增益协同调节，一般设定范围为（0~20）
    int bf_enhance;                     //该值默认设置为0，设为1时，高频成分会丰富些，但是低信噪比下高频噪声也会偏大
    int lowFreqHoldEn;
    float bfSupressFactor;
} JLSP_dms_bf_cfg;

/*3麦降噪参数*/
typedef struct {
    int Tri_CutTh;	//fb麦统计截止频率
    float Tri_SnrThreshold0;  //sir设定阈值
    float Tri_SnrThreshold1;  //sir设定阈值
    float *Tri_TransferFunc; //fb -> main传递函数
    float Tri_FbAlignedDb;	//fb和主副麦之间需要补偿增益
    float Tri_FbCompenDb; //fb补偿增益
    int Tri_TfEqSel; //eq,传递函数的选择：0选择eq，1选择传递函数，2选择传递函数和eq
    //int Tri_TransferMode; //传递函数模式选择，0：基于幅度的传递函数，1：基于幅度和相位的传递函数


    int Tri_Process_MaxFrequency;
    int Tri_Process_MinFrequency;
    int Tri_SIR_MaxFrequency;
    float Tri_Mic_Distance;
    float Tri_Target_Signal_Degradation;
    float Tri_AggressFactor;
    float Tri_MinSuppress;

    float Tri_CompenDb; //mic增益补偿
    int Tri_Bf_Enhance;
    int Tri_LowFreqHoldEn;
    float Tri_SupressFactor;

} JLSP_tri_cfg;

//回声消除设置参数
typedef struct {
    int AEC_Process_MaxFrequency;    //设定回声消除处理的最大频率，设定范围（3000~8000），默认值为8000
    int AEC_Process_MinFrequency;    //设定回声消除处理的最小频率，设定范围（0~1000），默认值为0
    int AF_Length; //挂空
    float muf;                      //设定aec滤波器的学习速率，默认0.02，设定范围（0.0001~0.5）
} JLSP_dms_aec_cfg;


//非线性回声压制设置参数
typedef struct {
    int NLP_Process_MaxFrequency;    //设定回声抑制的最大频率，设定范围（3000~8000），默认值为8000
    int NLP_Process_MinFrequency;    //设定回声抑制的最小频率，设定范围（0~1000），默认值为0
    float OverDrive;                 //定设压制强度，越大压制越强，设定范围（0~5）默认为1.0
} JLSP_dms_nlp_cfg;


//dns参数设置
typedef struct {
    float AggressFactor;    //降噪强度，设置范围（0.3~6.0），默认值：1.0
    float minSuppress;      //增益最小值控制，越小降噪越强，设置范围（0.0~1.0），默认为0.1
    float init_noise_lvl;   //初始噪声水平
    float high_gain;        //单麦高频增强，默认设置为1.0，设置范围为（1.0~3.0）
    float rb_rate;          //单麦混响增强，默认设置为0.1，设置范围为（0~0.9）
    int enhance_flag;       //是否开启双麦高频增强，默认为1
} JLSP_dms_dns_cfg;

typedef struct {
    int min_mag_db_level;           //语音能量放大下限阈值（单位db,默认-50，范围（-90db~-35db））
    int max_mag_db_level;           //语音能量放大上限阈值（单位db,默认-3，范围（-90db~0db））
    int addition_mag_db_level;      //语音补偿能量值（单位db,默认0，范围（0db~20db））
    int clip_mag_db_level;          //语音最大截断能量值（单位db,默认-3，范围（-10db~db））
    int floor_mag_db_level;         //语音最小截断能量值（单位db,默认-70，范围（-90db~-35db）
} JLSP_agc_cfg;

//风噪检测参数设置
typedef struct {
    float wn_msc_th;    //双麦非相关性阈值 (0-1)
    float ms_th;        //麦增益能量阈值(0-255)
    int wn_inty1;
    int wn_inty2;
    float wn_gain1;
    float wn_gain2;

    int t1_bot;         //低风噪等级能量下限阈值
    int t1_top;         //低风噪等级能量上限阈值
    int t2_bot;         //强风噪等级能量下限阈值
    int t2_top;         //强风噪等级能量上限阈值
    int offset;         //风噪能量增益偏移阈值

    int t1_bot_cnt_limit;   //风强变到弱风等级连续帧数计数,位宽16bit
    int t1_top_cnt_limit;   //风强由弱风变到中风等级连续帧数计数，位宽16bit
    int t2_bot_cnt_limit;   //风强由强风变到中风连续帧数计数，位宽16bit
    int t2_top_cnt_limit;   //风强变到强风连续帧数计数，位宽16bit
} JLSP_dms_wind_cfg;

/*
gain_floor: 增益的最小值控制,范围0~1,建议值(0~0.2)之间
over_drive: 控制降噪强度:
0 < over_drive < 1，越小降噪强度越轻，太小噪声会很大；
over_drive = 1,正常降噪
over_drive > 1,降噪强度加强，越大降噪强度越强，太大会吃音
建议调节范围0.3~3之间来控制降噪强度的强弱
high_gain: 控制声音的宏亮度,范围(1.0f~3.5f),越大声音越宏亮,太大可能会使噪声增加, 为1.0f表示不做增强, 建议设置2.0f左右
rb_rate: 混响强度,设置范围（0.0f~0.9f）,越大混响越强, 为0.0f表示不增强, 建议默认设置0.5f
*/

void *JLSP_ns_init(char *private_buffer, char *shared_buffer, float gain_floor, float over_drive, float high_gain, float rb_rate, int samplerate, const int is_dual);
int JLSP_ns_get_heap_size(int *private_size, int *shared_size, int samplerate, int is_dual);

int JLSP_ns_reset(void *m);//单双麦共用
void JLSP_ns_update_shared_buffer(void *m, char *shared_buffer);//单双麦共用
int JLSP_ns_process(void *m, void *input, void *output, int *outsize);
int JLSP_ns_free(void *m);//单双麦共用

void JLSP_ns_set_noiselevel(void *m, float noise_level_init);//单双麦共用


/*功能：初始化snr相关参数
* stop_freq: 高通滤波器的截止频率
* snr_th: 人声阈值设定,即大于该值才会进行snr压制处理, 采用db进行设置, 默认-25db
* commpress_params: 压制系数
*	0db一下分为了5个区间进行压缩,压缩系数范围(0~3),越大压制越强,建议范围(0~1.0f)
*	snr < -10, 压制系数compress_params[4]
*	snr < -8, 压制系数compress_params[3]
*	snr < -5, 压制系数compress_params[2]
*	snr < -3, 压制系数compress_params[1]
*	snr < 0, 压制系数compress_params[0]
*/
void JLSP_init_snr(void *m, int stop_freq, int snr_th,  float *compress_params);

/*功能： 获取当前帧snr
* 返回：当前帧snr值
*/
float JLSP_get_snr(void *m);




/*
 * 双麦降噪接口
 */
void JLSP_wd_single(unsigned char wd_act);
/*
 *wd_flag: 0: no wind 1: wind
 *wd_val:  wind strong bit(16)
 *wd_lev:  wind level 0:弱风，1:中风, 2:强风
 */
int JLSP_get_wd_info(void *handle, int *wd_flag, int *wd_val, int *wd_lev);

/*设置使用的模块*/
void JLSP_enable_module(int Enablebit);//仅双麦使用

/*初始化*/
void *JLSP_DualMicSystem_Init(char *private_buf,
                              char *share_buf,
                              void *aec_cfg,
                              void *nlp_cfg,
                              void *bf_cfg,
                              void *dns_cfg,
                              void *wn_cfg,
                              void *micsel_cfg,
                              int samplerate,
                              int EnableBit);

/*数据处理，返回风噪大小*/
int JLSP_DualMicSystem_Process(void *handle,
                               short *near,
                               short *near_ref,
                               short *far,
                               short *out,
                               int points);

//检测各个麦是否被堵住或者破坏相关信息
/*获取单双麦切换状态
 * 0: 正常双麦 ;
 * 1: 副麦坏了，触发故障
 * -1: 主麦坏了，触发故障
 */
int JLSP_DualMicSystem_GetMicState(void *handle);
/*
 * 获取mic的能量值，开了MFDT_EN才能用
 * mic: 0 获取主麦能量
 * mic：1 获取副麦能量
 * return：返回能量值，[0~90.3]
 */
float JLSP_DualMicSystem_GetMicEnergy(void *handle, unsigned char mic);
/*设置初始使用单双麦切换状态
 * 0: 正常双麦 ;
 * 1: 副麦坏了，触发故障
 * -1: 主麦坏了，触发故障
 */
void JLSP_DualMicSystem_SetMicState(void *handle, int state);

//int JLSP_DualMicSystem_GetWdInfo(void* handle, int* wd_flag, int* wd_val, int* wd_lev);
//int JLSP_DualMicSystem_Get_Size(int* private_size, int* share_size, int samplerate, const int is_dual);
//int JLSP_DualMicSystem_Reset(void* m);
//void JLSP_DualMicSystem_Update_Shared_Buffer(void* m, char* shared_buffer);
//int JLSP_DualMicSystem_Free(void* m);
//void JLSP_DualMicSystem_Set_Noiselevel(void* m, float noise_level_init);

/*
* 三麦降噪接口
*/
/*设置使用的模块*/
void JLSP_TriMicSystem_EnableModule(int Enablebit, int Enablebit_Ex);

/*获取风噪信息*/
int JLSP_TriMicSystem_GetWdInfo(void *handle, int *wd_flag, int *wd_val, int *wd_lev);

/*获取buf大小*/
int JLSP_TriMicSystem_Get_Size(int *private_size, int *share_size, int samplerate, const int is_dual);

/*初始化*/
void *JLSP_TriMicSystem_Init(char *private_buf,
                             char *share_buf,
                             void *aec_cfg,
                             void *nlp_cfg,
                             void *tri_cfg,
                             void *dns_cfg,
                             void *wn_cfg,
                             void *agc_cfg,
                             int samplerate,
                             int EnableBit);

/*数据处理，返回风噪大小*/
int JLSP_TriMicSystem_Process(void *handle,
                              short *near,
                              short *near_ref,
                              short *fb,
                              short *far,
                              short *out,
                              int InDataLen);

/*tri_en: 1 正常3MIC降噪
 *        0 变成2MIC降噪，不使用 FB MIC数据*/
void JLSP_TriMicSystem_ModeChoose(void *handle, unsigned char tri_en);
#endif
