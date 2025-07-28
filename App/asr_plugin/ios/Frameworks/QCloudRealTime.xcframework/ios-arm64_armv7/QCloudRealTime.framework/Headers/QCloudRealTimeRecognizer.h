//
//  QCloudRealTimeRecognizer.h
//  QCloudSDK
//
//  Created by Sword on 2019/3/28.
//  Copyright © 2019 Tencent. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <QCloudRealTime/QCloudRealTimeRecognizer.h>
#import <QCloudRealTime/QCloudConfig.h>



NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, QCloudASRRealTimeRecognizeState) {
    QCloudASRRealTimeRecognizeStateNone = 0,
    QCloudASRRealTimeRecognizeStateStart,
    QCloudASRRealTimeRecognizeStateStop,
    QCloudASRRealTimeRecognizeStateRecognizing
};


@class QCloudConfig;
@class QCloudRealTimeResult;
@protocol QCloudAudioDataSource;
@protocol QCloudRealTimeRecognizerDelegate;

@interface QCloudRealTimeRecognizer : NSObject

@property (nonatomic, assign, readonly) QCloudASRRealTimeRecognizeState state;
@property (nonatomic, weak) id<QCloudRealTimeRecognizerDelegate>delegate;

@property (nonatomic, strong, readonly) QCloudConfig *config;

/**
 * 初始化方法，使用内置录音器采集音频
 * @param config 配置参数，详见QCloudConfig定义
 */
- (instancetype)initWithConfig:(QCloudConfig *)config;

/**
 * 初始化方法，使用自定义音频源
 * @param config 配置参数，详见QCloudConfig定义
 */
- (instancetype)initWithConfig:(QCloudConfig *)config dataSource:(id<QCloudAudioDataSource>)dataSource;

/**
 * 通过appId secretId secretKey初始化
 * @param appid     腾讯云appId        基本概念见https://cloud.tencent.com/document/product/441/6194
 * @param secretId  腾讯云secretId     基本概念见https://cloud.tencent.com/document/product/441/6194
 * @param secretKey 腾讯云secretKey    基本概念见https://cloud.tencent.com/document/product/441/6194
 */
- (instancetype)initWithAppId:(NSString *)appid secretId:(NSString *)secretId secretKey:(NSString *)secretKey;

/**
 * 通过appId secretId secretKey初始化, 临时鉴权
 * @param appid     腾讯云appId        基本概念见https://cloud.tencent.com/document/product/441/6194
 * @param secretId  腾讯云secretId     基本概念见https://cloud.tencent.com/document/product/441/6194
 * @param secretKey 腾讯云secretKey    基本概念见https://cloud.tencent.com/document/product/441/6194
 */
- (instancetype)initWithAppId:(NSString *)appid secretId:(NSString *)secretId secretKey:(NSString *)secretKey token:(NSString *)token;


/**
 * 通过内置录音器采集音频开始实时语音识别，配合stop使用
 */
- (void)start;
/*
 * 停止实时语音识别
 */
- (void)stop;

/*
 * 关闭麦克风
 * keepMicrophoneRecording 开启后 调用此方法关闭麦克风
 */
- (void)stopMicrophone;

-(void)EnableDebugLog:(BOOL)enable;

@end


@protocol QCloudRealTimeRecognizerDelegate <NSObject>

@required
/**
 * 每个语音包分片识别结果
 * @param result 语音分片的识别结果（非稳态结果，会持续修正）
 */
- (void)realTimeRecognizerOnSliceRecognize:(QCloudRealTimeRecognizer *)recognizer result:(QCloudRealTimeResult *)result;


/**
 * 语音流的识别结果
 * 一次识别中可以包括多句话，这里持续返回的每句话的识别结果
 * @param recognizer 实时语音识别实例
 * @param result 语音分片的识别结果 （稳态结果）
 */
- (void)realTimeRecognizerOnSegmentSuccessRecognize:(QCloudRealTimeRecognizer *)recognizer result:(QCloudRealTimeResult *)result;


@optional
/**
 * 一次识别成功回调
 @param recognizer 实时语音识别实例
 @param result 一次识别出的总文本, 实际是由SDK本地处理，将本次识别的realTimeRecognizerOnSegmentSuccessRecognize 识别结果拼接后一次性返回
 */
- (void)realTimeRecognizerDidFinish:(QCloudRealTimeRecognizer *)recognizer result:(NSString *)result;
/**
 * 一次识别失败回调
 * @param recognizer 实时语音识别实例
 * @param result 识别结果信息，错误信息详情看QCloudRealTimeResponse内错误码
 */
- (void)realTimeRecognizerDidError:(QCloudRealTimeRecognizer *)recognizer result:(QCloudRealTimeResult *)result;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * 开始录音回调
 * @param recognizer 实时语音识别实例
 * @param error 开启录音失败，错误信息
 */
- (void)realTimeRecognizerDidStartRecord:(QCloudRealTimeRecognizer *)recognizer error:(NSError * _Nullable )error;
/**
 * 结束录音回调
 * @param recognizer 实时语音识别实例
 */
- (void)realTimeRecognizerDidStopRecord:(QCloudRealTimeRecognizer *)recognizer;
/**
 * 录音音量实时回调用,建议使用realTimeRecognizerDidUpdateVolumeDB来替代
 * @param recognizer 实时语音识别实例
 * @param volume 声音音量，取值范围（-40-0)
 */
- (void)realTimeRecognizerDidUpdateVolume:(QCloudRealTimeRecognizer *)recognizer volume:(float)volume __attribute__((deprecated("This method is deprecated. Use realTimeRecognizerDidUpdateVolumeDB instead.")));
/**
 * 录音音量实时回调用
 * @param recognizer 实时语音识别实例
 * @param volume 声音音量，计算方式如下$A_{i}$为采集音频振幅值
 * $$A_{mean} = \frac{1}{n} \sum_{i=1}^{n} A_{i}^{2}$$
 * $$volume=\max (10*\log_{10}(A_{mean}), 0)$$
 */
- (void)realTimeRecognizerDidUpdateVolumeDB:(QCloudRealTimeRecognizer *)recognizer volume:(float)volume;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * 语音流的开始识别
 * @param recognizer 实时语音识别实例
 * @param voiceId 语音流对应的voiceId，唯一标识
 * @param seq flow的序列号
 */
- (void)realTimeRecognizerOnFlowRecognizeStart:(QCloudRealTimeRecognizer *)recognizer voiceId:(NSString *)voiceId seq:(NSInteger)seq;
/**
 * 语音流的结束识别
 * @param recognizer 实时语音识别实例
 * @param voiceId 语音流对应的voiceId，唯一标识
 * @param seq flow的序列号
 */
- (void)realTimeRecognizerOnFlowRecognizeEnd:(QCloudRealTimeRecognizer *)recognizer voiceId:(NSString *)voiceId seq:(NSInteger)seq;


/**
 * 录音停止后回调一次，再次开始录音会清空上一次保存的文件
 * @param recognizer 实时语音识别实例
 * @param audioFilePath 音频文件路径
 */
- (void)realTimeRecognizerDidSaveAudioDataAsFile:(QCloudRealTimeRecognizer *)recognizer
                                   audioFilePath:(NSString *)audioFilePath;

/**
 * 触发静音事件时会回调
 */
-(void)realTimeRecognizerOnSliceDetectTimeOut;
/**
 * 日志输出
 * @param log 日志
 */
- (void)realTimeRecgnizerLogOutPutWithLog:(NSString *)log;

@end

NS_ASSUME_NONNULL_END
