# Flutter SDK(录音文件识别极速版)
SDK以插件的方式封装了录音文件识别极速版功能,提供flutter版本的录音文件识别极速版,本文介绍SDK的安装方法及示例

## 开发环境
- Dart >= 2.18.4
- Flutter >= 3.3.8
- Android API Level >= 16
- iOS >= 9.0

## 获取安装
[下载SDK]() SDK内asr_plugin目录即为flutter插件,插件内example目录下为demo示例

## 接入指引

**此插件仅支持Android和iOS两个平台且包含了平台相关的库,使用时请确保开发环境包含Android Studio及XCode否则可能会出现集成时的编译问题**

1. 将项目中asr_plugin目录复制到自己的Flutter工程下

2. 在自己项目的配置文件pubspec.yaml下添加依赖

   ```yaml
     asr_plugin:
       # 该路径根据asr_plugin存放路径改变
       path: ../asr_plugin
   ```

3. 在需要使用到的页面,导入asr_plugin的依赖

   ```dart
   import 'package:asr_plugin/asr_plugin.dart';
   ```

   

## 接口说明

接口示例代码为demo部分代码,完整代码请参考位于example里的demo示例
### FlashFileASRParams
录音文件识别极速版请求的相关参数,可参考[语音识别 录音文件识别极速版-API 文档-文档中心-腾讯云 (tencent.com)](https://cloud.tencent.com/document/product/1093/52097#3.2-.E8.AF.B7.E6.B1.82.E5.8F.82.E6.95.B0.E8.AF.B4.E6.98.8E)的描述

**示例**

```dart
var _params = FlashFileASRParams();
_params.data = Uint8List.view((await rootBundle.load("assets/30s.wav")).buffer);
_params.voice_format = OneSentenceASRParams.FORMAT_WAV;
```
### FlashFileASRController
控制录音文件识别极速版的流程及获取录音文件识别极速版的结果

**方法**
```dart
Future<FlashFileASRResult> recognize(FlashFileASRParams params) async;
```
**示例**
```dart
var ret = (await _controller.recognize(_params));
```
### FlashFileASRResult
返回录音文件识别极速版的结果,数据类型与API文档描述对应,可参考[语音识别 录音文件识别极速版-API 文档-文档中心-腾讯云 (tencent.com)](https://cloud.tencent.com/document/product/1093/52097#3.4-.E5.93.8D.E5.BA.94.E7.BB.93.E6.9E.9C.E8.AF.B4.E6.98.8E)的描述

**参数**

```dart
 String response_body = ""; // 服务端返回原始信息
 late int code; // 0：正常，其他，发生错误
 late String message; // code 非0时，message 中会有错误消息
 late String request_id; // 请求唯一标识，请您记录该值，以便排查错误
 late int audio_duration; // 音频时长，单位为毫秒
 List<Result>? flash_result; // 声道识别结果列表
```
### Sentence
Sentence数据类型,可参考[语音识别 录音文件识别极速版-API 文档-文档中心-腾讯云 (tencent.com)](https://cloud.tencent.com/document/product/1093/52097#3.4-.E5.93.8D.E5.BA.94.E7.BB.93.E6.9E.9C.E8.AF.B4.E6.98.8E)的描述

**参数**


```dart
 String text; // 句子/段落级别文本
 int start_time; // 开始时间
 int end_time; // 结束时间
 int speaker_id; // 说话人 Id（请求中如果设置了 speaker_diarization，可以按照 speaker_id 来区分说话人）
 List<Word>? word_list;// 词级别的识别结果列表
```

### Result
Result数据类型,可参考[语音识别 录音文件识别极速版-API 文档-文档中心-腾讯云 (tencent.com)](https://cloud.tencent.com/document/product/1093/52097#3.4-.E5.93.8D.E5.BA.94.E7.BB.93.E6.9E.9C.E8.AF.B4.E6.98.8E)的描述

**参数**
```dart
 int channel_id = 0; // 声道标识，从0开始，对应音频声道数
 String text = ""; // 声道音频完整识别结果
 List<Sentence>? sentence_list; // 句子/段落级别的识别结果列表
```

