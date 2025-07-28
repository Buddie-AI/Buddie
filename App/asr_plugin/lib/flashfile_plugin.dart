import 'dart:collection';
import 'dart:convert';
import 'dart:typed_data';
import 'package:crypto/crypto.dart';
import 'package:http/http.dart' as http;

class FlashFileASRParams {
  static const String ENGINE_8K_ZH = "8k_zh";
  static const String ENGINE_8K_EN = "8k_en";
  static const String ENGINE_16K_ZH = "16k_zh";
  static const String ENGINE_16K_ZH_PY = "16k_zh-PY";
  static const String ENGINE_16K_ZH_MEDICAL = "16k_zh_medical";
  static const String ENGINE_16K_EN = "16k_en";
  static const String ENGINE_16K_YUE = "16k_yue";
  static const String ENGINE_16K_JA = "16k_ja";
  static const String ENGINE_16K_KO = "16k_ko";
  static const String ENGINE_16K_VI = "16k_vi";
  static const String ENGINE_16K_MS = "16k_ms";
  static const String ENGINE_16K_ID = "16k_id";
  static const String ENGINE_16K_FIL = "16k_fil";
  static const String ENGINE_16K_TH = "16k_th";
  static const String ENGINE_16K_PT = "16k_pt";
  static const String ENGINE_16K_TR = "16k_tr";
  static const String ENGINE_16K_AR = "16k_ar";
  static const String ENGINE_16K_ES = "16k_es";
  static const String ENGINE_16K_HI = "16k_hi";
  static const String ENGINE_16K_ZH_DIALECT = "16k_zh_dialect";

  static const String FORMAT_WAV = "wav";
  static const String FORMAT_PCM = "pcm";
  static const String FORMAT_OGG_OPUS = "ogg-opus";
  static const String FORMAT_SPEEX = "speex";
  static const String FORMAT_SILK = "silk";
  static const String FORMAT_MP3 = "mp3";
  static const String FORMAT_M4A = "m4a";
  static const String FORMAT_AAC = "aac";
  static const String FORMAT_AMR = "amr";

  static const SPEAKER_DIARIZATION_MODE_0 = 0;
  static const SPEAKER_DIARIZATION_MODE_1 = 1;

  static const int FILTER_DIRTY_MODE_0 = 0;
  static const int FILTER_DIRTY_MODE_1 = 1;
  static const int FILTER_DIRTY_MODE_2 = 2;

  static const int FILTER_MODAL_MODE_0 = 0;
  static const int FILTER_MODAL_MODE_1 = 1;
  static const int FILTER_MODAL_MODE_2 = 2;

  static const int FILTER_PUNC_MODE_0 = 0;
  static const int FILTER_PUNC_MODE_1 = 1;
  static const int FILTER_PUNC_MODE_2 = 2;

  static const int CONVERT_NUM_NODE_0 = 0;
  static const int CONVERT_NUM_NODE_1 = 1;

  static const int WORD_INFO_MODE_0 = 0;
  static const int WORD_INFO_MODE_1 = 1;
  static const int WORD_INFO_MODE_2 = 2;

  static const FIRST_CHANNEL_ONLY_MODE_0 = 0;
  static const FIRST_CHANNEL_ONLY_MODE_1 = 1;

  static const int REINFORCE_HOTWORD_MODE_0 = 0;
  static const int REINFORCE_HOTWORD_MODE_1 = 1;

  List<int> data = Uint8List(0);
  int appid =
      0; // 腾讯云的AppId, 相关信息可在https://console.cloud.tencent.com/cam/capi查询
  String secretkey =
      ""; // 腾讯云的secretKey, 相关信息可在https://console.cloud.tencent.com/cam/capi查询

  String? token = null; //腾讯云临时鉴权token

  // 腾讯云的secretID, 相关信息可在https://console.cloud.tencent.com/cam/capi查询
  String get secretid => _value["secretid"]! as String;
  set secretid(String val) => _value["secretid"] = val;

  // 引擎模型类型
  String get engine_type => _value["engine_type"]! as String;
  set engine_type(String val) => _value["engine_type"] = val;

  // 音频格式
  String get voice_format => _value["voice_format"]! as String;
  set voice_format(String val) => _value["voice_format"] = val;

  // 是否开启说话人分离
  int? get speaker_diarization => _value["speaker_diarization"] as int?;
  set speaker_diarization(int? val) => _set_value("speaker_diarization", val);

  // 热词表 id
  String? get hotword_id => _value["hotword_id"] as String?;
  set hotword_id(String? val) => _set_value("hotword_id", val);

  // 热词增强功能
  int? get reinforce_hotword => _value["reinforce_hotword"] as int?;
  set reinforce_hotword(int? val) => _set_value("reinforce_hotword", val);

  // 自学习模型 id
  int? get customization_id => _value["customization_id"] as int?;
  set customization_id(int? val) => _set_value("customization_id", val);

  // 是否过滤脏词
  int? get filter_dirty => _value["filter_dirty"] as int?;
  set filter_dirty(int? val) => _set_value("filter_dirty", val);

  // 是否过滤语气词
  int? get filter_modal => _value["filter_modal"] as int?;
  set filter_modal(int? val) => _set_value("filter_modal", val);

  // 否过滤标点符号
  int? get filter_punc => _value["filter_punc"] as int?;
  set filter_punc(int? val) => _set_value("filter_punc", val);

  // 是否进行阿拉伯数字智能转换
  int? get convert_num_mode => _value["convert_num_mode"] as int?;
  set convert_num_mode(int? val) => _set_value("convert_num_mode", val);

  // 是否显示词级别时间戳
  int? get word_info => _value["word_info"] as int?;
  set word_info(int? val) => _set_value("word_info", val);

  // 是否只识别首个声道
  int? get first_channel_only => _value["first_channel_only"] as int?;
  set first_channel_only(int? val) => _set_value("first_channel_only", val);

  // 单标点最多字数
  int? get sentence_max_length => _value["sentence_max_length"] as int?;
  set sentence_max_length(int? val) => _set_value("sentence_max_length", val);

  // 临时热词表
  String? get hotword_list => _value["hotword_list"] as String?;
  set hotword_list(String? val) => _set_value("hotword_list", val);

  // 支持pcm格式的8k音频在与引擎采样率不匹配的情况下升采样到16k后识别，能有效提升识别准确率
  int? get input_sample_rate => _value["input_sample_rate"] as int?;
  set input_sample_rate(int? val) => _set_value("input_sample_rate", val);

  void _set_value<T>(String key, T? val) {
    if (val != null) {
      _value[key] = val;
    } else {
      _value.remove(key);
    }
  }

  final Map<String, Object> _value = SplayTreeMap.of({
    "secretid": "",
    "engine_type": ENGINE_16K_ZH,
    "voice_format": FORMAT_WAV,
    "timestamp": 0,
  });
}

class Word {
  String word; // 词级别文本
  int start_time; // 开始时间
  int end_time; // 结束时间

  Word(this.word, this.start_time, this.end_time);

  factory Word.fromJson(Map<String, dynamic> json) =>
      Word(json['word'], json['start_time'], json['end_time']);
}

class Sentence {
  String text; // 句子/段落级别文本
  int start_time; // 开始时间
  int end_time; // 结束时间
  int speaker_id; // 说话人 Id（请求中如果设置了 speaker_diarization，可以按照 speaker_id 来区分说话人）
  List<Word>? word_list; // 词级别的识别结果列表

  Sentence(this.text, this.start_time, this.end_time, this.speaker_id,
      this.word_list);

  factory Sentence.fromJson(Map<String, dynamic> json) => Sentence(
      json["text"],
      json["start_time"],
      json["end_time"],
      json["speaker_id"],
      (json["word_list"] as List?)?.map((e) => Word.fromJson(e)).toList());
}

class Result {
  int channel_id = 0; // 声道标识，从0开始，对应音频声道数
  String text = ""; // 声道音频完整识别结果
  List<Sentence>? sentence_list; // 句子/段落级别的识别结果列表

  Result(this.channel_id, this.text, this.sentence_list);

  factory Result.fromJson(Map<String, dynamic> json) => Result(
      json["channel_id"],
      json["text"],
      (json["sentence_list"] as List?)
          ?.map((e) => Sentence.fromJson(e))
          .toList());
}

class FlashFileASRResult {
  String response_body = ""; // 服务端返回原始信息
  late int code; // 0：正常，其他，发生错误
  late String message; // code 非0时，message 中会有错误消息
  late String request_id; // 请求唯一标识，请您记录该值，以便排查错误
  late int audio_duration; // 音频时长，单位为毫秒
  List<Result>? flash_result; // 声道识别结果列表

  factory FlashFileASRResult.fromJson(Map<String, dynamic> json) =>
      FlashFileASRResult(
          json["code"],
          json["message"],
          json["request_id"],
          json["audio_duration"],
          (json["flash_result"] as List?)
              ?.map((e) => Result.fromJson(e))
              .toList());

  FlashFileASRResult(this.code, this.message, this.request_id,
      this.audio_duration, this.flash_result);
}

class FlashFileASRController {
  final http.Client _client = http.Client();

  static const HOST = "asr.cloud.tencent.com";

  Future<FlashFileASRResult> recognize(FlashFileASRParams params) async {
    params._value["timestamp"] =
        DateTime.now().toUtc().millisecondsSinceEpoch ~/ 1000;
    var url = Uri.https(HOST, "asr/flash/v1/${params.appid}",
        params._value.map((key, value) => MapEntry(key, value.toString())));
    var request = http.Request("POST", url);
    request.headers["Host"] = HOST;
    request.headers["Authorization"] = _signature(request, params.secretkey);
    if (params.token != null) {
      request.headers["X-TC-Token"] = params.token!;
    }
    request.headers["Content-Type"] = "application/octet-stream";
    request.headers["Content-Length"] = params.data.length.toString();
    request.bodyBytes = params.data;
    var in_stream = await _client.send(request);
    var response = await http.Response.fromStream(in_stream);
    var body = utf8.decode(response.bodyBytes);
    var result = FlashFileASRResult.fromJson(json.decode(body));
    result.response_body = body;
    return result;
  }

  String _signature(http.Request request, String secretkey) {
    String str = "POST$HOST${request.url.path}?${request.url.query}";
    var hmac = Hmac(sha1, utf8.encode(secretkey));
    return base64Encode(hmac.convert(utf8.encode(str)).bytes);
  }
}
