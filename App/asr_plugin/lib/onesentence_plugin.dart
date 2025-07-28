import 'dart:collection';
import 'dart:convert';
import 'dart:typed_data';
import 'package:crypto/crypto.dart';
import 'package:http/http.dart' as http;
import 'package:intl/intl.dart';

class OneSentenceASRParams {
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

  static const int WORD_INFO_MODE_0 = 0;
  static const int WORD_INFO_MODE_1 = 1;
  static const int WORD_INFO_MODE_2 = 2;

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

  static const int REINFORCE_HOTWORD_MODE_0 = 0;
  static const int REINFORCE_HOTWORD_MODE_1 = 1;

  String secretID =
      ""; // Tencent Cloud secretID, related information can be queried at https://console.cloud.tencent.com/cam/capi
  String secretKey =
      ""; // Tencent Cloud secretKey, related information can be queried at https://console.cloud.tencent.com/cam/capi
  String?
      token; // Tencent Cloud Token, related information can be queried at https://console.cloud.tencent.com/cam/capi

  // Engine model type
  String get eng_serice_type => _value["EngSerViceType"]! as String;
  set eng_serice_type(String val) => _value["EngSerViceType"] = val;

  // Recognition audio format
  String get voice_format => _value["VoiceFormat"]! as String;
  set voice_format(String val) => _value["VoiceFormat"] = val;

  // Audio URL address, must ensure downloadable
  String? get url => _value["Url"] as String;
  set url(String? val) {
    if (val != null) {
      _value["Url"] = val;
      _value["SourceType"] = 0;
      _value.remove("Data");
      _value.remove("DataLen");
    } else {
      _value.remove("Url");
    }
  }

  // base64 encoded audio data
  String? get data => _value["Data"] as String;
  set data(String? val) {
    if (val != null) {
      _value["Data"] = val;
      _value["DataLen"] = val.length;
      _value["SourceType"] = 1;
      _value.remove("Url");
    } else {
      _value.remove("Data");
    }
  }

  // Original audio data
  set binary_data(Uint8List val) {
    data = base64Encode(val);
  }

  // Whether to display word-level timestamps
  int? get word_info => _value["WordInfo"] as int?;
  set word_info(int? val) => _set_value("WordInfo", val);

  // Whether to filter profanity
  int? get filter_dirty => _value["FilterDirty"] as int?;
  set filter_dirty(int? val) => _set_value("FilterDirty", val);

  // Whether to filter modal particles
  int? get filter_modal => _value["FilterModal"] as int?;
  set filter_modal(int? val) => _set_value("FilterModal", val);

  // Whether to filter punctuation
  int? get filter_punc => _value["FilterPunc"] as int?;
  set filter_punc(int? val) => _set_value("FilterPunc", val);

  // Whether to perform Arabic numeral intelligent conversion
  int? get convert_num_mode => _value["ConvertNumMode"] as int?;
  set convert_num_mode(int? val) => _set_value("ConvertNumMode", val);

  // Hotword id
  String? get hotword_id => _value["HotwordId"] as String?;
  set hotword_id(String? val) => _set_value("HotwordId", val);

  // Custom learning model id
  String? get customization_id => _value["CustomizationId"] as String?;
  set customization_id(String? val) => _set_value("CustomizationId", val);

  // Hotword enhancement feature
  int? get reinforce_hotword => _value["ReinforceHotword"] as int?;
  set reinforce_hotword(int? val) => _set_value("ReinforceHotword", val);

  // Temporary hotword table
  String? get hotword_list => _value["HotwordList"] as String?;
  set hotword_list(String? val) => _set_value("HotwordList", val);

  // Support 8k audio in pcm format to be upsampled to 16k for recognition when sampling rate doesn't match the engine, effectively improving recognition accuracy
  int? get input_sample_rate => _value["InputSampleRate"] as int?;
  set input_sample_rate(int? val) => _set_value("InputSampleRate", val);

  final Map<String, Object> _value = HashMap.of({
    "EngSerViceType": ENGINE_16K_ZH,
    "VoiceFormat": FORMAT_WAV,
    "SourceType": 1,
    "Data": ""
  });

  void _set_value<T>(String key, T? val) {
    if (val != null) {
      _value[key] = val;
    } else {
      _value.remove(key);
    }
  }
}

class SentenceWords {
  String? word; // Word result
  int? offset_start_ms; // Word start time in audio
  int? offset_end_ms; // Word end time in audio

  factory SentenceWords.fromJson(Map<String, dynamic> json) => SentenceWords(
      json["word"], json["offset_start_ms"], json["offset_end_ms"]);

  SentenceWords(this.word, this.offset_start_ms, this.offset_end_ms);
}

class Error {
  String code; // Error code
  String message; // Error message

  factory Error.fromJson(Map<String, dynamic> json) =>
      Error(json["Code"], json["Message"]);

  Error(this.code, this.message);
}

class OneSentenceASRResult {
  String response_body = ""; // Original response from server
  String? request_id; // Unique request ID
  String? result; // Recognition result
  int? duration; // Audio duration of request in ms
  int? word_size; // Length of word timestamp list
  List<SentenceWords>? word_list; // Word timestamp list
  Error? error; // Error information

  factory OneSentenceASRResult.fromJson(Map<String, dynamic> json) =>
      OneSentenceASRResult(
          json["Response"]["RequestId"],
          json["Response"]["Result"],
          json["Response"]["AudioDuration"],
          json["Response"]["WordSize"],
          (json["Response"]["WordList"] as List?)
              ?.map((e) => SentenceWords.fromJson(e))
              .toList(),
          ((e) =>
              e == null ? null : Error.fromJson(e))(json["Response"]["Error"]));
  OneSentenceASRResult(this.request_id, this.result, this.duration,
      this.word_size, this.word_list, this.error);
}

class OneSentenceASRController {
  http.Client _client = http.Client();
  static const String HOST = "asr.tencentcloudapi.com";

  Future<OneSentenceASRResult> recognize(OneSentenceASRParams params) async {
    var url = Uri.https(HOST);
    var body = params._value;
    var request = http.Request("POST", url);
    request.headers["Content-Type"] = "application/json; charset=utf-8";
    request.headers["Host"] = HOST;
    request.headers["X-TC-Action"] = "SentenceRecognition";
    request.headers["X-TC-Version"] = "2019-06-14";
    request.body = jsonEncode(body);
    request.headers["Authorization"] =
        _signature(request, params.secretID, params.secretKey, "asr");
    if (params.token != null) {
      request.headers["X-TC-Token"] = params.token!;
    }
    var in_stream = await _client.send(request);
    var response = await http.Response.fromStream(in_stream);
    var resp_body = utf8.decode(response.bodyBytes);
    var result = OneSentenceASRResult.fromJson(json.decode(resp_body));
    result.response_body = resp_body;
    return result;
  }

  String _signature(http.Request request, String secret_id, String secret_key,
      String service) {
    var date = DateTime.now().toUtc();
    var timestamp = date.millisecondsSinceEpoch ~/ 1000;
    var utc = DateFormat("yyyy-MM-dd").format(date);
    request.headers["X-TC-Timestamp"] = timestamp.toString();
    var hashed_request_payload = sha256.convert(utf8.encode(request.body));
    var canonical_request =
        "POST\n/\n\ncontent-type:${request.headers["Content-Type"]}\nhost:${request.url.host}\n\ncontent-type;host\n$hashed_request_payload";
    var hashed_canonical_request =
        sha256.convert(utf8.encode(canonical_request));
    var string_to_sign =
        "TC3-HMAC-SHA256\n$timestamp\n$utc/$service/tc3_request\n$hashed_canonical_request";
    var hmac = Hmac(sha256, utf8.encode("TC3${secret_key}"));
    var secret_date = hmac.convert(utf8.encode(utc));
    hmac = Hmac(sha256, secret_date.bytes);
    var secret_service = hmac.convert(utf8.encode(service));
    hmac = Hmac(sha256, secret_service.bytes);
    var secret_signing = hmac.convert(utf8.encode("tc3_request"));
    hmac = Hmac(sha256, secret_signing.bytes);
    var signature = hmac.convert(utf8.encode(string_to_sign));
    return "TC3-HMAC-SHA256 Credential=${secret_id}/${utc}/${service}/tc3_request, SignedHeaders=content-type;host, Signature=${signature}";
  }
}
