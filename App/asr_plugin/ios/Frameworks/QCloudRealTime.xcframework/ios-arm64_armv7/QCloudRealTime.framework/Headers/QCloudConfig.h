//
//  QCloudConfig.h
//  QCloudSDK
//
//  Created by Sword on 2019/3/29.
//  Copyright © 2019 Tencent. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <QCloudRealTime/QCloudRealTimeRecognizer.h>


NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, QCloudASRNetworkProtocol) {
    QCloudASRNetworkProtocolWSS = 0,//默认
    QCloudASRNetworkProtocolWS = 1,
};



@interface QCloudConfig : NSObject

@property (nonatomic, copy) NSString* extraUserAgent;
//通用配置参数
@property (nonatomic, strong, readonly) NSString *appId;        //腾讯云appId     基本概念见https://cloud.tencent.com/document/product/441/6194
@property (nonatomic, strong, readonly) NSString *secretId;     //腾讯云secretId  基本概念见https://cloud.tencent.com/document/product/441/6194
@property (nonatomic, strong, readonly) NSString *secretKey;    //腾讯云secretKey 基本概念见https://cloud.tencent.com/document/product/441/6194
@property (nonatomic, assign, readonly) NSInteger projectId;    //腾讯云projectId 基本概念见https://cloud.tencent.com/document/product/441/6194
@property (nonatomic, strong, readonly) NSString *token;    //腾讯云临时鉴权token
//实时语音识别相关参数

@property (nonatomic, assign) BOOL enableDetectVolume;                        //是否检测录音音量的变化, 开启后sdk会实时回调音量变化
@property (nonatomic, assign) BOOL endRecognizeWhenDetectSilence;             //是否识别静音，默认YES
@property (nonatomic, assign) BOOL endRecognizeWhenDetectSilenceAutoStop;     //识别到静音是否停止本次识别，默认YES
@property (nonatomic, assign) float silenceDetectDuration;                    //最大静音时间阈值, 超过silenceDetectDuration时间不说话则为静音, 单位:秒
@property (nonatomic, assign) NSInteger sliceTime;    //分片时间, 此参数影响语音分片长度, 单位:毫秒,必须为20的整倍数，如果不是，sdk内将自动调整为20的整倍数，例如77将被调整为60，如果您不了解此参数不建议更改
@property (nonatomic, assign) NSInteger requestTimeout;  //网络请求超时时间，单位:秒, 取值范围[5-60], 默认20

//是否压缩音频。默认压缩，压缩音频有助于优化弱网或网络不稳定时的识别速度及稳定性
//SDK历史版本均默认压缩且不提供配置开关，如无特殊需求，建议使用默认值
@property (nonatomic, assign) BOOL compression;

/*——————————————————————————————————————————————————————————————————————————————————*/
/*以下为后端识别参数配置，具体的取值见API 文档https://cloud.tencent.com/document/product/1093/48982 */
@property (nonatomic, copy) NSString* engineType;   //引擎识别类型,默认16k_zh
@property (nonatomic, assign) NSInteger filterDirty;  //是否过滤脏词，具体的取值见API文档的filter_dirty参数
@property (nonatomic, assign) NSInteger filterModal;  //过滤语气词具体的取值见API文档的filter_modal参数
@property (nonatomic, assign) NSInteger filterPunc;   //过滤句末的句号具体的取值见API文档的filter_punc参数
@property (nonatomic, assign) NSInteger convertNumMode;  //是否进行阿拉伯数字智能转换。具体的取值见API文档的convert_num_mode参数
@property (nonatomic, strong) NSString *hotwordId;   //热词id。具体的取值见API文档的hotword_id参数
@property (nonatomic, strong) NSString *customizationId;  //自学习模型id
@property (nonatomic, assign) NSInteger vadSilenceTime;                     
//语音断句检测阈值，静音时长超过该阈值会被认为断句（多用在智能客服场景，需配合 needvad = 1 使用），具体的取值见API文档vad_silence_time
@property (nonatomic, assign) NSInteger needvad;  //默认1 0：关闭 vad，1：开启 vad。 如果语音分片长度超过60秒，用户需开启 vad。
@property (nonatomic, assign) NSInteger wordInfo;
//是否显示词级别时间戳。0：不显示；1：显示，不包含标点时间戳，2：显示，包含标点时间戳。默认为0。
@property (nonatomic, assign) NSInteger reinforceHotword __attribute__((deprecated("该属性即将过期，可通过在控制台配置热词列表里的热词权重为100，并将热词列表ID通过hotwordID属性传入SDK，实现增强热词的功能"))); //热词增强功能 0: 关闭, 1: 开启 默认0.
@property (nonatomic, assign) float noiseThreshold; // 噪音参数阈值，默认为0，取值范围：[-1,1]
@property (nonatomic, assign) NSInteger maxSpeakTime; // 强制断句功能，取值范围 5000-90000(单位:毫秒），默认值0(不开启)。 在连续说话不间断情况下，该参数将实现强制断句（此时结果变成稳态，slice_type=2）。如：游戏解说场景，解说员持续不间断解说，无法断句的情况下，将此参数设置为10000，则将在每10秒收到 slice_type=2的回调。

/*——————————————————————————————————————————————————————————————————————————————————*/

@property (nonatomic, assign) BOOL keepMicrophoneRecording;
//默认关闭 开启后 需要调用 stopMicrophone 停止麦克风。使用场景：在停止识别后 需要麦克风继续录音一段时间 （录音不会上传服务器 不会识别 也不会保存）只支持内置录音设置


//shouldSaveAsFile：仅限使用SDK内置录音器有效，是否保存录音文件到本地 默认关闭
@property (nonatomic, assign) BOOL shouldSaveAsFile;
//SaveFilePath：开启shouldSaveAsFile后音频保存的路径，仅限使用SDK内置录音器有效，默认路径为[NSTemporaryDirectory() stringByAppendingPathComponent:@"recordaudio.wav"]
@property (nonatomic, copy) NSString *saveFilePath;


@property (nonatomic, assign) QCloudASRNetworkProtocol netWorkProtocol;       //默认wss

@property (nonatomic, strong, readonly) NSMutableDictionary* ApiParams;

/**
 * 初始化方法
 * @param appid     腾讯云appId     基本概念见https://cloud.tencent.com/document/product/441/6194
 * @param secretId  腾讯云secretId  基本概念见https://cloud.tencent.com/document/product/441/6194
 * @param secretKey 腾讯云secretKey 基本概念见https://cloud.tencent.com/document/product/441/6194
 * @param projectId 腾讯云projectId 基本概念见https://cloud.tencent.com/document/product/441/6194
 */
- (instancetype)initWithAppId:(NSString *)appid
                     secretId:(NSString *)secretId
                    secretKey:(NSString *)secretKey
                    projectId:(NSInteger)projectId;


- (instancetype)initWithAppId:(NSString *)appid
                     secretId:(NSString *)secretId
                    secretKey:(NSString *)secretKey
                    token:(NSString *)token
                    projectId:(NSInteger)projectId;

/// 设置自定义参数,可使用该方法控制请求时的参数
/// @param value nil时将删除参数,否则会在请求中添加参数
- (void)setApiParam:(NSString* _Nonnull)key value:(NSObject* _Nullable)value;

@end

NS_ASSUME_NONNULL_END
