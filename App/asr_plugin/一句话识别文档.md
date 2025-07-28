# Flutter SDK(一句话识别)
SDK以插件的方式封装了一句话识别功能,提供flutter版本的一句话识别,本文介绍SDK的安装方法及示例

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
### OneSentenceASRParams
一句话识别请求的相关参数,可参考[语音识别 一句话识别-API 文档-文档中心-腾讯云 (tencent.com)](https://cloud.tencent.com/document/product/1093/35646#2.-.E8.BE.93.E5.85.A5.E5.8F.82.E6.95.B0)的描述

**示例**

```dart
var _params = OneSentenceASRParams();
_params.binary_data = Uint8List.view((await rootBundle.load("assets/30s.wav")).buffer);
_params.voice_format = OneSentenceASRParams.FORMAT_WAV;
```
### OneSentenceASRController
控制一句话识别的流程及获取一句话识别的结果

**方法**
```dart
Future<OneSentenceASRResult> recognize(OneSentenceASRParams params) async;
```
**示例**
```dart
_result = (await _controller.recognize(_params)).response_body;
```
### OneSentenceASRResult
返回一句话识别的结果,数据类型与API文档描述对应,可参考[语音识别 一句话识别-API 文档-文档中心-腾讯云 (tencent.com)](https://cloud.tencent.com/document/product/1093/35646#3.-.E8.BE.93.E5.87.BA.E5.8F.82.E6.95.B0)的描述

**参数**

```dart
 String response_body = ""; // 服务端返回原始信息
 String? request_id; // 唯一请求 ID
 String? result; // 识别结果
 int? duration; // 请求的音频时长，单位为ms
 int? word_size; // 词时间戳列表的长度
 List<SentenceWords>? word_list; //词时间戳列表
 Error? error; // 错误信息
```
### SentenceWords
SentenceWords数据类型,可参考[语音识别 数据结构-API 中心-腾讯云 (tencent.com)](https://cloud.tencent.com/document/api/1093/37824#SentenceWord)的描述

**参数**


```dart
String? word; // 词结果
int? offset_start_ms; // 词在音频中的开始时间
int? offset_end_ms; // 词在音频中的结束时间
```

### Error
服务端返回的错误信息

**参数**
```dart
String code; // 错误码
String message; // 错误信息
```

