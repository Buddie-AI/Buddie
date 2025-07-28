//
//  QCloudRealTimeResponse.h
//  QCloudSDK
//
//  Created by Sword on 2019/4/3.
//  Copyright © 2019 Tencent. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <QCloudRealTime/QCloudConfig.h>


NS_ASSUME_NONNULL_BEGIN


//客户端错误码
typedef NS_ENUM(NSInteger, QCloudRealTimeClientErrCode) {
    QCloudRealTimeClientErrCode_Success = 0,                    //成功
    QCloudRealTimeClientErrCode_NetworkError = -100,         //无网络
    QCloudRealTimeClientErrCode_Timeout = -101,     //手机网路存在问题，请求超时
    QCloudRealTimeClientErrCode_MicError = -102,  //录音过程音频通道被占用，录音失败，比如电话
    QCloudRealTimeClientErrCode_AudioInitError = -103  //音频源初始化失败（麦克风启动失败，权限拒绝等，如果使用自定义音频源start方法返回错误也会触发）
};



/**
 * 识别结果类
 */
@interface QCloudRealTimeResult : NSObject

/*客户端错误码 QCloudRealTimeClientErrCode */
@property (nonatomic, assign) NSInteger clientErrCode;
/** clientErrCode对应的描述信息 */
@property (nonatomic, copy) NSString *clientErrMessage;


/*未解析的json原文本，如有需求，可拿到后按业务需求自定义处理，
 注：以下结果仅当QCloudRealTimeClientErrCode==QCloudRealTimeClientErrCode_Success时不为nil*/
@property (nonatomic, copy) NSString *jsonText;

/*----------以下是jsonText解析出来的内容--------------------*/
/** 后台错误码Code = 0时表示成功，其他表示为失败 */
//后台错误码见https://cloud.tencent.com/document/product/1093/48982
@property (nonatomic, assign) NSInteger code;
/** code对应的描述信息 */
@property (nonatomic, copy) NSString *message;
/** 语音流的识别id */
@property (nonatomic, copy) NSString *voiceId;
/** 当前语音流的识别结果 */
@property (nonatomic, copy) NSString *text;
/** 语音包序列号，注意:不是语音流序列号*/
@property (nonatomic, assign) NSInteger seq;

/** result_list */
@property (nonatomic, copy) NSArray *resultList;
/** 识别到的总文本 */
@property (nonatomic, copy) NSString *recognizedText;

@property (nonatomic, assign) NSInteger finalN;

/** 本 message 唯一 id */
@property (nonatomic, copy) NSString *messageId;
@property (nonatomic, assign) NSInteger messageNo;
/*----------以上为解析好的内容--------------------*/

/*——————————————以下参数非后端返回————————————————*/
/** 记录语音流请求参数 */
@property (nonatomic, strong) NSDictionary *requestParameters;
/** 表示后面的 result_list 里面有几段结果，如果是0表示没有结果，遇到中间是静音。如果是1表示 result_list 有一个结果， 在发给服务器分片很大的情况下可能会出现多个结果，正常情况下都是1个结果。*/
@property (nonatomic, assign) NSInteger resultNumber;

/*————————————————————————————————————————————————*/

- (instancetype)initWithDictionary:(NSDictionary *)dic requestParameters:(NSDictionary *)requestParameters;




@end

/**
* 语音识别请求回包的result_list
*/
@interface QCloudRealTimeResultResponse : NSObject

/** 返回分片类型标记， 0表示一小段话开始，1表示在小段话的进行中，2表示小段话的结束 */
@property (nonatomic, assign) NSInteger sliceType;
/** 表示第几段话 */
@property (nonatomic, assign) NSInteger index;
/** 这个分片在整个音频流中的开始时间 */
@property (nonatomic, assign) NSInteger startTime;
/** 这个分片在整个音频流中的结束时间 */
@property (nonatomic, assign) NSInteger endTime;
/** 识别结果 */
@property (nonatomic, strong) NSString *voiceTextStr;

/** word_list */
@property (nonatomic, copy) NSArray *wordList;

- (instancetype)initWithDictionary:(NSDictionary *)dic;

@end

NS_ASSUME_NONNULL_END

