# Flutter SDK(实时语音识别)
SDK以插件的方式封装了Android和iOS实时语音识别功能,提供flutter版本的实时语音识别,本文介绍SDK的安装方法及示例

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
### ASRControllerConfig
配置相关参数用于生成ASRController

**参数**
```dart
int appID = 0; // 腾讯云 appID
int projectID = 0; //腾讯云 projectID
String secretID = ""; //腾讯云 secretID
String secretKey = ""; // 腾讯云 projectKey

String engine_model_type = "16k_zh"; //设置引擎，不设置默认16k_zh
int filter_dirty = 0; //是否过滤脏词，具体的取值见API文档的filter_dirty参数
int filter_modal = 0; //过滤语气词具体的取值见API文档的filter_modal参数
int filter_punc = 0; //过滤句末的句号具体的取值见API文档的filter_punc参数
int convert_num_mode = 1; //是否进行阿拉伯数字智能转换。具体的取值见API文档的convert_num_mode参数
String hotword_id = ""; //热词id。具体的取值见API文档的hotword_id参数
String customization_id = ""; //自学习模型id,详情见API文档
int vad_silence_time = 0; //语音断句检测阈值,详情见API文档
int needvad = 1; //人声切分,详情见API文档
int word_info = 0; //是否显示词级别时间戳,详情见API文档
int reinforce_hotword = 0; //热词增强功能,详情见API文档
double noise_threshold = 0; //噪音参数阈值,详情见API文档

bool is_compress = true; //是否开启音频压缩,开启后使用opus压缩传输数据
bool silence_detect = false; //静音检测功能,开启后检测到静音会停止识别
int silence_detect_duration = 5000; //静音检测时长,开启静音检测功能后生效
bool is_save_audio_file = false; //是否保存音频,仅对内置录音生效,格式为s16le,16000Hz,mono的pcm,开启后会通过NOTIFY类型的ASRData返回到上层,其中ASRData中info为以下的JSON格式{"type":"onAudioFile, "code": 0, "message": "audio file path"}
String audio_file_path = ""; //is_save_audio_file为true时,会将音频保存在指定位置
```
**方法**

```
Future<ASRController> build() async <--> 创建ASRController
```
**示例**
```
var _config = ASRControllerConfig()
_config.filter_dirty = 1;
_config.filter_modal = 0;
_config.filter_punc = 0;
var _controller = await _config.build();
```
### ASRController
控制语音识别的流程及获取语音识别的结果

**方法**
```dart
Stream<ASRData> recognize() async* <--> 开始识别,通过监听Stream可以获得实时语音识别的相关数据
Stream<ASRData> recognizeWithDataSource(Stream<Uint8List>? source) async* <--> 开始识别,可传入自定义数据源进行识别,有关数据源的要求参考自定义数据源
stop() async <--> 停止识别
release() async <--> 释放资源
```
**示例**
```dart
try {
    if (_controller != null) {
      await _controller?.release();
    }
    _controller = await _config.build();
    setState(() {
      _btn_onclick = stopRecognize;
    });
    await for (final val in _controller!.recognize()) {
      switch (val.type) {
        case ASRDataType.SLICE:
        case ASRDataType.SEGMENT:
          var id = val.id!;
          var res = val.res!;
          if (id >= _sentences.length) {
            for (var i = _sentences.length; i <= id; i++) {
              _sentences.add("");
            }
          }
          _sentences[id] = res;
          setState(() {
            _result = _sentences.map((e) => e).join("");
          });
          break;
        case ASRDataType.SUCCESS:
          setState(() {
            _btn_onclick = startRecognize;
            _result = val.result!;
            _sentences = [];
          });
          break;
      }
    }
  } on ASRError catch (e) {
    setState(() {
      _btn_onclick = startRecognize;
      _result = "错误码：${e.code} \n错误信息: ${e.message} \n详细信息: ${e.resp}";
    });
  } catch (e) {
    log(e.toString());
    setState(() {
      _btn_onclick = startRecognize;
    });
  }
}
```
### ASRData
识别过程中返回的数据

**参数**
```
ASRDataType type; //数据类型
int? id; //句子的id
String? res; //数据类型为SLICE和SEGMENT时返回部分识别结果
String? result; //数据类型SUCCESS时返回所有识别结果
String? info; //数据类型为NOTIFY时携带的信息
```
### ASRDataType
ASRData数据类型


```dart
enum ASRDataType {
  SLICE,
  SEGMENT,
  SUCCESS,
  NOTIFY,
}
```

### ASRError
识别过程中的错误

**参数**
```
int code; //错误码 iOS参考QCloudRealTimeClientErrCode Android参考ClientException
String message; //错误消息
String? resp; //服务端返回的原始数据
```

## 自定义数据源

SDK只负责对输入的语音进行识别,不会进行额外的处理.但调用者可以通过自定义数据源来实现对录音数据的处理(降噪,回声消除等)来满足相应的场景需求

自定义数据源需要数据以```Stream<Uint8List>```的方式传入到SDK且需要满足以下的要求

1. 采样数据格式仅支持单通道16000hz、16bit、小端的pcm数据流
2. 采用数据需要每隔40ms向stream推入1280B的数据且数据格式需要满足条件1
