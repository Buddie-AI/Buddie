#ifndef DRC_API_H
#define DRC_API_H

#ifdef WIN32

#define AT_DRC(x)
#define AT_DRC_CODE
#define AT_DRC_CONST
#define AT_DRC_SPARSE_CODE
#define AT_DRC_SPARSE_CONST

#else
#define AT_DRC(x)           __attribute((section(#x)))
#define AT_DRC_CODE         AT_DRC(.drc_code)
#define AT_DRC_CONST        AT_DRC(.drc_const)
#define AT_DRC_SPARSE_CODE  AT_DRC(.drc_sparse_code)
#define AT_DRC_SPARSE_CONST AT_DRC(.drc_sparse_const)
#endif


enum {
    DATA_IN_SHORT = 0,    //输入数据类型，输入short时，输出不能为int
    DATA_IN_INT,
};
enum {
    DATA_OUT_SHORT = 0, //输出数据类型，输入short时，输出不能为int
    DATA_OUT_INT
};

enum {
    PEAK = 0,	 //算法类型
    RMS
};

enum {
    PERPOINT = 0, //mode 模式
    TWOPOINT
};

typedef struct _DrcParam {
    /*
        int *attackTime;       //启动时间
        int *releaseTime;	   //释放时间
        float *threshold;	   //阈值{x1，y1,x2,y2}  最多5组十个数据
        int *ThresholdNum;	   //阈值的组数
        int *rmsTime;		   // rms时间  algorithm为RMS有效
        float *InputGain;	   // 输入增益（db）
        float *OutputGain;	   // 输出数据增益(db)
        unsigned char algorithm;		   //  PEAK或者RMS
        unsigned char mode;			   //  模式
        unsigned char intype;			  //  输入数据类型
        unsigned char outtype;		  //  输出数据类型
    	*/
    int channel;
    int sampleRate;
    int attackTime;       //启动时间
    int releaseTime;	   //释放时间
    float *threshold;	   //阈值{x1，y1,x2,y2}  最多5组十个数据
    int ThresholdNum;	   //阈值的组数
    int rmsTime;		   // rms时间  algorithm为RMS有效
    float InputGain;	   // 输入增益（db）
    float OutputGain;	   // 输出数据增益(db)
    unsigned char algorithm;		   //  PEAK或者RMS
    unsigned char mode;			   //  模式
    unsigned char intype;			  //  输入数据类型
    unsigned char outtype;		  //  输出数据类型
    int  IndataInc;				  // 输入数据同个通道下个点的步进 例如左右左右  步进为2
    int  OutdataInc;			  // 输出数据同个通道下个点的步进 例如左右左右  步进为2
} DrcParam;

//int GetDrcBuf(int algorithm, int *rmsTime, int channel, int sampleRate);
int GetDrcBuf(DrcParam *param); // bufsize 与 algorithm，rmsTime channel、 sampleRate有关
//void DrcInit(void *WorkBuf, DrcParam *param, int channel, int sampleRate);
void DrcInit(void *WorkBuf, DrcParam *param);
void DrcUpdate(void *WorkBuf, DrcParam *param);
int DrcRun(void *WorkBuf, void *indata, void *outdata, int per_channel_npoint);
/* InputGain   输入数据放大或者缩小的db数（正数放大，负数缩小） */
// inputgain 配2，表示总体增加2dB
/* OutputGain   输出数据放大或者缩小的db数（正数放大，负数缩小） */
#if 0
int GetDrcBuf(int algorithm, int *rmsTime, int channel, int sampleRate);
获取buf大小
参数说明：
algorithm：算法类型，PEAK或者RMS
rmsTime:
rms时间ms，算法类型为RMS时有效
channel：  通道数
sampleRate:
采样率

void DrcInit(void *WorkBuf, int *attackTime, int *releaseTime, float *threshold, int *ThresholdNum, int *rmsTime, int channel, int sampleRate, int algorithm, int mode, int intype, int outtype);
初始化
参数说明：
WorkBuf:
运行buf
attackTime:
启动时间ms
releaseTime:
释放时间ms
threshold:
信号输入与对应的输出[in1, out1, in2, out2 ***排列] 最多5组数据, 3个阈值，4段
ThresholdNum:
信号输入与对应的输出组数
rmsTime:
rms时间ms
channel:
通道
sampleRate：采样率
algorithm：算法类型
mode：模式
intype：输入数据类型，DATA_IN_SHORT或者DATA_IN_INT
outtype:
输出数据类型，DATA_OUT_SHOR或者DATA_OUT_INT；若输入数据类型为DATA_IN_SHORT，输出类型只能设置为DATA_OUT_SHOR

void DrcUpdate(void *WorkBuf, int *attackTime, int *releaseTime, float *threshold, int *ThresholdNum, int channel, int sampleRate);
更新参数：使用rms算法时，当rmsTime fs变化时因为buf会产生变化，只能初始化更新
参数说明：
WorkBuf:
运行buf
attackTime:
启动时间ms
releaseTime:
释放时间ms
threshold:
信号输入与对应的输出[in1, out1, in2, out2 ***排列] 最多5组数据, 3个阈值，4段
ThresholdNum:
信号输入与对应的输出组数

int DrcRun(void *WorkBuf, void *indata, void *outdata, int per_channel_npoint);
运行：输入输出数据类型，要跟初始化配置的数据类型对应
WorkBuf:
运行buf
indata：输入数据
outdata：输出数据
per_channel_npoint：每个通道的样点数
#endif

#endif // !DRC_API_H

