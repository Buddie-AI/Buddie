//
// AsrPlugin.m
// asr_plugin
// 
// Created by tbolpcao on 2023/5/5
// Copyright (c) 2023 Tencent. All rights reserved.
//

#import "AsrPlugin.h"
#import "QCloudRealTime/QCloudRealTimeRecognizer.h"
#import "QCloudRealTime/QCloudRealTimeResult.h"
#import "QCloudRealTime/QCloudAudioDataSource.h"
#import <AVFoundation/AVFoundation.h>

/**
  将OC中消息回调到Flutter
 */
@interface ASRObserver : NSObject<QCloudRealTimeRecognizerDelegate>

@end

@implementation ASRObserver{
    int _observer_id;
    FlutterMethodChannel* _channel;
}

- (instancetype)init:(int)observer_id channel:(FlutterMethodChannel*) channel {
    _observer_id = observer_id;
    _channel = channel;
    return self;
}

- (void)realTimeRecognizerOnSliceRecognize:(nonnull QCloudRealTimeRecognizer *)recognizer result:(nonnull QCloudRealTimeResult *)result {
    __weak typeof(self) weakSelf = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        __strong typeof(self) strongSelf = weakSelf;
        if(strongSelf == nil) {
            return;
        }
        QCloudRealTimeResultResponse *currentResult = [result.resultList firstObject];
        if(currentResult == nil){
            return;
        }
        NSDictionary* args = @{
            @"id": @(strongSelf->_observer_id),
            @"sentence_id": @(currentResult.index),
            @"sentence_text": currentResult.voiceTextStr
        };
        [strongSelf->_channel invokeMethod:@"onSliceSuccess" arguments:args];
    });
}

- (void)realTimeRecognizerOnSegmentSuccessRecognize:(nonnull QCloudRealTimeRecognizer *)recognizer result:(nonnull QCloudRealTimeResult *)result {
    __weak typeof(self) weakSelf = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        __strong typeof(self) strongSelf = weakSelf;
        if(strongSelf == nil) {
            return;
        }
        QCloudRealTimeResultResponse *currentResult = [result.resultList firstObject];
        if(currentResult == nil){
            return;
        }
        NSDictionary* args = @{
            @"id": @(strongSelf->_observer_id),
            @"sentence_id": @(currentResult.index),
            @"sentence_text": currentResult.voiceTextStr
        };
        [strongSelf->_channel invokeMethod:@"onSegmentSuccess" arguments:args];
    });
}

- (void)realTimeRecognizerDidFinish:(QCloudRealTimeRecognizer *)recognizer result:(NSString *)result {
    __weak typeof(self) weakSelf = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        __strong typeof(self) strongSelf = weakSelf;
        if(strongSelf == nil) {
            return;
        }
        NSString* tmp = result;
        if(tmp == nil){
            tmp = @"";
        }
        NSDictionary* args = @{
            @"id": @(strongSelf->_observer_id),
            @"text": tmp
        };
        [strongSelf->_channel invokeMethod:@"onSuccess" arguments:args];
    });
}


- (void)realTimeRecognizerDidError:(QCloudRealTimeRecognizer *)recognizer result:(QCloudRealTimeResult *)result {
    __weak typeof(self) weakSelf = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        __strong typeof(self) strongSelf = weakSelf;
        if(strongSelf == nil) {
            return;
        }
        NSDictionary* args = nil;
        if(result.clientErrCode != QCloudRealTimeClientErrCode_Success) {
            args = @{
                @"id": @(strongSelf->_observer_id),
                @"code": @(result.clientErrCode),
                @"message": result.clientErrMessage,
                @"response": result.jsonText == nil ? @"" : result.jsonText,
            };
        }else{
            NSString* message = @"";
            if(result.message == nil && result.jsonText != nil) {
                NSDictionary* dict = [NSJSONSerialization JSONObjectWithData:[result.jsonText dataUsingEncoding:NSUTF8StringEncoding] options:kNilOptions error:nil];
                if(dict != nil && dict[@"message"] != nil) {
                    message = dict[@"message"];
                }
            }
            args = @{
                @"id": @(strongSelf->_observer_id),
                @"code": @(-1),
                @"message": message,
                @"response": result.jsonText == nil ? @"" : result.jsonText,
            };
        }
        [strongSelf->_channel invokeMethod:@"onFailed" arguments:args];
    });
}

- (void)realTimeRecognizerDidStartRecord:(QCloudRealTimeRecognizer *)recognizer error:(NSError *)error {
    __weak typeof(self) weakSelf = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        __strong typeof(self) strongSelf = weakSelf;
        if(strongSelf == nil) {
            return;
        }
        NSDictionary* args = @{
            @"id": @(strongSelf->_observer_id)
        };
        [strongSelf->_channel invokeMethod:@"onStartRecord" arguments:args];
    });
}

- (void)realTimeRecognizerDidStopRecord:(QCloudRealTimeRecognizer *)recognizer {
    __weak typeof(self) weakSelf = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        __strong typeof(self) strongSelf = weakSelf;
        if(strongSelf == nil) {
            return;
        }
        NSDictionary* args = @{
            @"id": @(strongSelf->_observer_id)
        };
        [strongSelf->_channel invokeMethod:@"onStopRecord" arguments:args];
    });
}

- (void)realTimeRecognizerDidSaveAudioDataAsFile:(QCloudRealTimeRecognizer *)recognizer audioFilePath:(NSString *)audioFilePath {
    __weak typeof(self) weakSelf = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        __strong typeof(self) strongSelf = weakSelf;
        if(strongSelf == nil) {
            return;
        }
        NSDictionary* args = @{
            @"id": @(strongSelf->_observer_id),
            @"code": @(0),
            @"message": audioFilePath,
        };
        [strongSelf->_channel invokeMethod:@"onAudioFile" arguments:args];
    });
}

@end

@interface ASRDataSource : NSObject<QCloudAudioDataSource>

@end

@implementation ASRDataSource{
    int _observer_id;
    FlutterMethodChannel* _channel;
}

- (instancetype)init:(int)observer_id channel:(FlutterMethodChannel*) channel {
    _observer_id = observer_id;
    _channel = channel;
    return self;
}

@synthesize audioFilePath;

@synthesize recording;

@synthesize running;

- (nullable NSData *)readData:(NSInteger)expectLength {
    __weak typeof(self) weakSelf = self;
    NSCondition* condition = [[NSCondition alloc] init];
    __block NSInteger code = -1;
    NSMutableData* ret_data = [[NSMutableData alloc] init];
    dispatch_async(dispatch_get_main_queue(), ^{
        __strong typeof(self) strongSelf = weakSelf;
        if(strongSelf == nil) {
            return;
        }
        NSDictionary* args = @{
            @"id": @(strongSelf->_observer_id),
            @"size": @(expectLength)
        };
        [strongSelf->_channel invokeMethod:@"read" arguments:args result:^(id  _Nullable result) {
            FlutterStandardTypedData* data = (FlutterStandardTypedData*)result;
            code = data.data.length;
            [ret_data appendData:data.data];
            [condition signal];
        }];
    });
    while (code == -1){
        [condition wait];
    }
    [condition unlock];
    return ret_data;
}

- (void)start:(nonnull void (^)(BOOL, NSError * _Nonnull))completion {
    running = YES;
    completion(YES, nil);
}

- (void)stop {
    running = NO;
}

@end

@interface ASRController : NSObject

@end

@implementation ASRController{
    QCloudConfig* _config;
    QCloudRealTimeRecognizer* _recognizer;
    id<QCloudRealTimeRecognizerDelegate> _observer;
    id<QCloudAudioDataSource> _source;
}

- (instancetype)init:(QCloudConfig*)val {
    self = [super init];
    self->_config = val;
    return self;
}

- (void)setObserver:(id<QCloudRealTimeRecognizerDelegate>)val {
    self->_observer = val;
}

- (void)setDataSource:(id<QCloudAudioDataSource>)val {
    self->_source = val;
}

- (void)start {
    if(_source != nil){
        _recognizer = [[QCloudRealTimeRecognizer alloc] initWithConfig:_config dataSource:_source];
    }else{
        NSError* error = nil;
        [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryRecord error:&error];
        [[AVAudioSession sharedInstance] setActive:YES error:nil];
        _recognizer = [[QCloudRealTimeRecognizer alloc] initWithConfig:_config];
    }
    _recognizer.delegate = _observer;
    [_recognizer EnableDebugLog:YES];
    [_recognizer start];
}

- (void)stop {
    [_recognizer stop];
}

@end

@implementation AsrPlugin{
    unsigned int key;
    NSMutableDictionary<NSNumber*, NSObject*>* instance_mgr;
    FlutterMethodChannel* _channel;
}


+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar {
  FlutterMethodChannel* channel = [FlutterMethodChannel
      methodChannelWithName:@"asr_plugin"
            binaryMessenger:[registrar messenger]];
  AsrPlugin* instance = [[AsrPlugin alloc] init:channel];
  [registrar addMethodCallDelegate:instance channel:channel];
}

- (instancetype)init:(FlutterMethodChannel*) channel {
    key = 0;
    instance_mgr = [[NSMutableDictionary alloc] init];
    _channel = channel;
    return self;
}

- (unsigned int) addInstance:(id)val {
    while([instance_mgr objectForKey:@(key)]){
        key++;
    }
    [instance_mgr setObject:val forKey:@(key)];
    return key;
}

- (void) removeInstance:(unsigned int)val {
    [instance_mgr removeObjectForKey:@(val)];
}

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result {
  if ([@"ASRController.new" isEqualToString:call.method]) {
      int appID = [call.arguments[@"appID"] integerValue];
      NSString* secretID = call.arguments[@"secretID"];
      NSString* secretKey = call.arguments[@"secretKey"];
      NSString* token = call.arguments[@"token"];
      NSString* engine_model_type = call.arguments[@"engine_model_type"];
      int filter_dirty = [call.arguments[@"filter_dirty"] integerValue];
      int filter_modal = [call.arguments[@"filter_modal"] integerValue];
      int filter_punc = [call.arguments[@"filter_punc"] integerValue];
      int convert_num_mode = [call.arguments[@"convert_num_mode"] integerValue];
      NSString* hotword_id = call.arguments[@"hotword_id"];
      NSString* customization_id = call.arguments[@"customization_id"];
      int vad_silence_time = [call.arguments[@"vad_silence_time"] integerValue];
      int needvad = [call.arguments[@"needvad"] integerValue];
      int word_info = [call.arguments[@"word_info"] integerValue];
      int reinforce_hotword = [call.arguments[@"reinforce_hotword"] integerValue];
      float noise_threshold = [call.arguments[@"noise_threshold"] floatValue];
      
      bool is_compress = [call.arguments[@"is_compress"] boolValue];
      bool silence_detect = [call.arguments[@"silence_detect"] boolValue];
      int silence_detect_duration = [call.arguments[@"silence_detect_duration"] integerValue];
      bool is_save_audio_file = [call.arguments[@"is_save_audio_file"] boolValue];
      NSString* audio_file_path = call.arguments[@"audio_file_path"];
      
      QCloudConfig* config = nil;
      if(token == nil || token == [NSNull null]) {
          config = [[QCloudConfig alloc] initWithAppId:@(appID).stringValue secretId:secretID secretKey:secretKey projectId:0];
      }else{
          config = [[QCloudConfig alloc] initWithAppId:@(appID).stringValue secretId:secretID secretKey:secretKey token:token projectId:0];
      }
      config.engineType = engine_model_type;
      config.filterDirty = filter_dirty;
      config.filterModal = filter_modal;
      config.filterPunc = filter_punc;
      config.convertNumMode = convert_num_mode;
      config.hotwordId = hotword_id;
      config.customizationId = customization_id;
      if (vad_silence_time != 0) {
          config.vadSilenceTime = vad_silence_time;
      }
      config.needvad = needvad;
      config.wordInfo = word_info;
      config.reinforceHotword = reinforce_hotword;
      config.noiseThreshold = noise_threshold;
      config.compression = is_compress;
      config.endRecognizeWhenDetectSilence = silence_detect;
      config.silenceDetectDuration = (float)silence_detect_duration / 1000.0;
      config.sliceTime = 40;
      config.shouldSaveAsFile = is_save_audio_file;
      config.saveFilePath = audio_file_path;
      ASRController* controller = [[ASRController alloc] init:config];
      result(@([self addInstance:controller]));
      
  }
  else if([@"ASRController.setObserver" isEqualToString:call.method]){
      int obj_id = [call.arguments[@"id"] integerValue];
      ASRController* ctl = instance_mgr[@(obj_id)];
      int observer_id = [call.arguments[@"observer_id"] integerValue];
      ASRObserver* observer = [[ASRObserver alloc] init:observer_id channel:_channel];
      [ctl setObserver:observer];
      result(nil);
  }
  else if([@"ASRController.setDataSource" isEqualToString:call.method]){
      int obj_id = [call.arguments[@"id"] integerValue];
      ASRController* ctl = instance_mgr[@(obj_id)];
      int datasource_id = [call.arguments[@"datasource_id"] integerValue];
      ASRDataSource* datasource = [[ASRDataSource alloc] init:datasource_id channel:_channel];
      [ctl setDataSource:datasource];
      result(nil);
  }
  else if([@"ASRController.start" isEqualToString:call.method]){
      int obj_id = [call.arguments[@"id"] integerValue];
      ASRController* ctl = instance_mgr[@(obj_id)];
      [ctl start];
      result(nil);
  }
  else if([@"ASRController.stop" isEqualToString:call.method]){
      int obj_id = [call.arguments[@"id"] integerValue];
      ASRController* ctl = instance_mgr[@(obj_id)];
      [ctl stop];
      result(nil);
  }
  else if([@"ASRController.release" isEqualToString:call.method]){
      int obj_id = [call.arguments[@"id"] integerValue];
      [self removeInstance:obj_id];
      result(nil);
  }
  else {
    result(FlutterMethodNotImplemented);
  }
}

@end
