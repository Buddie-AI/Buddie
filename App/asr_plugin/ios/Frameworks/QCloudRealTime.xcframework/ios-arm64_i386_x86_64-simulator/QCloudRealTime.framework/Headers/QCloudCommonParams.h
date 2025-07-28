//
//  QCloudCommonParamters.h
//  QCloudSDK
//
//  Created by Sword on 2019/2/26.
//  Copyright © 2019 Tencent. All rights reserved.
//

#import <Foundation/Foundation.h>


NS_ASSUME_NONNULL_BEGIN

@interface QCloudCommonParams : NSObject

@property (nonatomic, strong) NSString *appid;
@property (nonatomic, assign) NSInteger projectId;   //optional 腾讯云项目 ID，可填 0，总长度不超过 1024 字节。
@property (nonatomic, strong) NSString *action;
@property (nonatomic, strong) NSString *region;
@property (nonatomic, assign) NSInteger timestamp;
@property (nonatomic, assign) NSInteger nonce;
@property (nonatomic, strong) NSString *secretId;
@property (nonatomic, strong) NSString *secretKey;
@property (nonatomic, strong) NSString *token; //临时鉴权token
@property (nonatomic, strong) NSString *signature;
@property (nonatomic, strong) NSString *version;
@property (nonatomic, strong) NSString *signatureMethod;


+ (instancetype)defaultRequestParams;

/**
 V3鉴权通用参数
 @return V3通用参数
 */
- (NSDictionary *)commonParamsForV3Authentication;

/**
 V1鉴权通用参数

 @return V1通用参数
 */
- (NSDictionary *)commonParamsForV1Authentication;

/**
 是否使用V3接口鉴权，默认NO

 @return 返回一个Bool值表示是否使用V3鉴权
 */
- (BOOL)usingV3Authentication;

@end

NS_ASSUME_NONNULL_END
