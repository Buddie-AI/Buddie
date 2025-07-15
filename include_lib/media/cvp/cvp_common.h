#ifndef _CVP_COMMON_H_
#define _CVP_COMMON_H_

/*DMS版本定义*/
#define DMS_GLOBAL_V100    0xB1
#define DMS_GLOBAL_V200    0xB2

/*
 * V200新算法回声消除nlp模式
 * JLSP_NLP_MODE1: 模式1为单独的NLP回声抑制，回声压制会偏过，该模式下NLP模块可以单独开启
 * JLSP_NLP_MODE2: 模式2下回声信号会先经过AEC线性压制，然后进行NLP非线性压制
 *                 此模式NLP不能单独打开需要同时打开AEC,使用AEC模块压制不够时，建议开启该模式
 */
#define JLSP_NLP_MODE1 0x01   //模式一（默认）
#define JLSP_NLP_MODE2 0x02   //模式二

/*
 * V200新算法风噪降噪模式
 * JLSP_WD_MODE1: 模式1为常规的降风噪模式，风噪残余会偏大些
 * JLSP_WD_MODE2: 模式2为神经网络降风噪，风噪抑制会比较干净，但是会需要多消耗31kb的flash
 */
#define JLSP_WD_MODE1 0x01   //常规降噪
#define JLSP_WD_MODE2 0x02   //nn降风噪，目前该算法启用会多31kflash

/*DMS输出选择*/
typedef enum {
    DMS_OUTPUT_SEL_DEFAULT = 0,	/*默认输出：dms处理后的数据*/
    DMS_OUTPUT_SEL_MASTER,		/*主mic原始数据*/
    DMS_OUTPUT_SEL_SLAVE,		/*副mic原始数据*/
    DMS_OUTPUT_SEL_FBMIC,		/*FB mic原始数据*/
} CVP_OUTPUT_ENUM;

/*
*********************************************************************
*                  			Audio CVP IOCTL
* Description: CVP功能配置
* Arguments  : cmd 		操作命令
*		       value 	操作数
*		       priv 	操作内存地址
* Return	 : 0 成功 其他 失败
* Note(s)    : (1)比如动态开关降噪NS模块:
*				aec_dms_ioctl(CVP_NS_SWITCH,1,NULL);	//降噪关
*				aec_dms_ioctl(CVP_NS_SWITCH,0,NULL);  //降噪开
*********************************************************************
*/
enum {
    CVP_AEC_SWITCH = 1,
    CVP_NLP_SWITCH,
    CVP_NS_SWITCH,
    CVP_AGC_SWITCH,
    CVP_ENC_SWITCH,
    CVP_AGC_MAX_LVL,
};
int aec_ioctl(int cmd, int value, void *priv);
int aec_dms_ioctl(int cmd, int value, void *priv);
int aec_dms_flexible_ioctl(int cmd, int value, void *priv);


#endif /*_CVP_COMMON_H_*/

