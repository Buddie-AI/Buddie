//
//  QCloudAudioDataSource.h
//  QCloudSDK
//
//  Created by Sword on 2019/4/12.
//  Copyright © 2019 Tencent. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * 语音数据数据源，如果调用者需要自己提供语音数据需要, 调用者实现此协议中所有方法
 * 提供符合以下要求的语音数据:
 * 采样率：16k
 * 音频格式：pcm
 * 编码：16bit位深的单声道
 */
@protocol QCloudAudioDataSource <NSObject>

@required

/**
 * 标识data source是否开始工作，执行完start后需要设置成YES， 执行完stop后需要设置成NO
 */
@property (nonatomic, assign) BOOL running;

@property (nonatomic, copy, readonly) NSString *audioFilePath;

/**
 * 标识QCloudAudioRecorder是否正在录音
 */
@property (nonatomic, assign, readonly) BOOL recording;

/**
 * SDK会调用start方法，实现此协议的类需要初始化数据源。didStart 是否开始  YES 往下执行，NO不会往下执行
 */
- (void)start:(void(^)(BOOL didStart, NSError *error))completion;
/**
 * SDK会调用stop方法，实现此协议的类需要停止提供数据
 */
- (void)stop;
/**
 * SDK会调用实现此协议的对象的此方法读取语音数据, 如果语音数据不足expectLength，read线程进入休眠。
 * @param expectLength 期望读取的字节数，如果返回的NSData不足expectLength个字节，SDK会抛出异常。
 */
- (nullable NSData *)readData:(NSInteger)expectLength;

@end

NS_ASSUME_NONNULL_END
