//
//  QCloudAudioRecorderDataSource.h
//  QCloudSDK
//
//  Created by Sword on 2019/4/12.
//  Copyright © 2019 Tencent. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "QCloudAudioDataSource.h"
NS_ASSUME_NONNULL_BEGIN

@class QCloudConfig;

@interface QCloudAudioRecorderDataSource : NSObject<QCloudAudioDataSource>

@property (nonatomic, weak) QCloudConfig *config;

//获取当前data长度
- (NSInteger)dataLength;

@end

NS_ASSUME_NONNULL_END
