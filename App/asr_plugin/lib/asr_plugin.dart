import 'dart:async';
import 'dart:convert';

import 'package:flutter/services.dart';

class _Inner {
  int _id = 1;
  Map<int, dynamic> _instance_mgr = {};
  MethodChannel methodChannel = const MethodChannel('asr_plugin');
  static _Inner _instance = _Inner();
  static _Inner get instance {
    return _instance;
  }

  _Inner() {
    methodChannel.setMethodCallHandler((call) async {
      int id = call.arguments["id"];
      var obj = _instance_mgr[id];
      if (obj == null) {
        return null;
      }
      if (call.method == "onStartRecord") {
        obj.onStartRecord();
      } else if (call.method == "onStopRecord") {
        obj.onStopRecord();
      } else if (call.method == "onSliceSuccess") {
        int sen_id = call.arguments["sentence_id"];
        String sen_text = call.arguments["sentence_text"];
        obj.onSliceSuccess(sen_id, sen_text);
      } else if (call.method == "onSegmentSuccess") {
        int sen_id = call.arguments["sentence_id"];
        String sen_text = call.arguments["sentence_text"];
        obj.onSegmentSuccess(sen_id, sen_text);
      } else if (call.method == "onSuccess") {
        String text = call.arguments["text"];
        obj.onSuccess(text);
      } else if (call.method == "onFailed") {
        int code = call.arguments["code"];
        String msg = call.arguments["message"];
        String resp = call.arguments["response"];
        obj.onFailed(code, msg, resp);
      } else if (call.method == "read") {
        int size = call.arguments["size"];
        return obj.read(size);
      } else if (call.method == "onAudioFile") {
        int code = call.arguments["code"];
        String msg = call.arguments["message"];
        obj.onAudioFile(code, msg);
      }
    });
  }

  int addInstance(dynamic obj) {
    while (_instance_mgr.containsKey(_id)) {
      _id = (_id + 1).toUnsigned(32);
    }
    _instance_mgr.addEntries({_id: obj}.entries);
    return _id;
  }

  void removeInstance(int id) {
    _instance_mgr.remove(id);
  }
}

class _ASRControllerObserver {
  StreamController<ASRData> _stream_ctl;

  _ASRControllerObserver(this._stream_ctl);

  onFailed(int code, String msg, String resp) {
    _stream_ctl.addError(ASRError(code, msg, resp));
  }

  onSegmentSuccess(int id, String res) {
    var data = ASRData(ASRDataType.SEGMENT);
    data.res = res;
    data.id = id;
    _stream_ctl.add(data);
  }

  onSliceSuccess(int id, String res) {
    var data = ASRData(ASRDataType.SLICE);
    data.res = res;
    data.id = id;
    _stream_ctl.add(data);
  }

  onStartRecord() {}

  onStopRecord() {
    _stream_ctl.close();
  }

  onSuccess(String result) {
    var data = ASRData(ASRDataType.SUCCESS);
    data.result = result;
    _stream_ctl.add(data);
  }

  onAudioFile(int code, String msg) {
    var data = ASRData(ASRDataType.NOTIFY);
    data.info = jsonEncode({
      "type": "onAudioFile",
      "code": code,
      "message": msg,
    });
    _stream_ctl.add(data);
  }
}

class _ASRDataSource {
  Stream<Uint8List> _source;
  List<Uint8List> _data = [];

  _ASRDataSource(this._source) {
    _source.listen((event) {
      _data.add(event);
    });
  }

  Future<Uint8List> read(int size) async {
    if (_data.isNotEmpty) {
      return _data.removeAt(0);
    }
    return Uint8List(0);
  }
}

enum ASRDataType {
  SLICE,
  SEGMENT,
  SUCCESS,
  NOTIFY,
}

class ASRData {
  ASRDataType type; // Data type
  int? id; // Sentence ID
  String? res; // Part of recognition result when data type is SLICE or SEGMENT
  String? result; // Return all recognition results when data type is SUCCESS
  String? info; // Information carried by data type NOTIFY
  ASRData(this.type);
}

class ASRError implements Exception {
  int code; // Error code iOS reference QCloudRealTimeClientErrCode Android reference ClientException
  String message; // Error message
  String? resp; // Original data returned by the server
  ASRError(this.code, this.message, this.resp);
}

class ASRControllerConfig {
  int appID = 0; // Tencent Cloud appID
  int projectID = 0; // Tencent Cloud projectID
  String secretID = ""; // Tencent Cloud secretID
  String secretKey = ""; // Tencent Cloud secretKey
  String? token = null; // Tencent Cloud token

  String engine_model_type = "16k_zh"; // Set engine, default 16k_zh if not set
  int filter_dirty =
      0; // Whether to filter dirty words, specific values see API document filter_dirty parameter
  int filter_modal =
      0; // Specific values for filtering intonation words see API document filter_modal parameter
  int filter_punc =
      0; // Specific values for filtering sentence-ending punctuation marks see API document filter_punc parameter
  int convert_num_mode =
      1; // Whether to perform intelligent conversion of Arabic numerals. Specific values see API document convert_num_mode parameter
  String hotword_id =
      ""; // Hot word ID. Specific values see API document hotword_id parameter
  String customization_id = ""; // Customized model ID, details see API document
  int? vad_silence_time =
      1000; // Speech sentence detection threshold, details see API document
  int needvad = 1; // Voice segmentation, details see API document
  int word_info =
      0; // Whether to display word-level timestamps, details see API document
  int reinforce_hotword =
      0; // Hot word enhancement function, details see API document
  double noise_threshold =
      0; // Noise parameter threshold, details see API document

  bool is_compress =
      true; // Whether to enable audio compression, after enabling, use opus to compress and transmit data
  bool silence_detect =
      false; // Silence detection function, after enabling, if silence is detected, recognition will stop
  int silence_detect_duration =
      5000; // Silence detection duration, effective after enabling silence detection function
  bool is_save_audio_file =
      false; // Whether to save audio, only effective for built-in recording, format s16le, 16000Hz, mono pcm, after enabling, it will be returned to the upper layer through NOTIFY type ASRData, where ASRData's info is the following JSON format{"type":"onAudioFile, "code": 0, "message": "audio file path"}
  String audio_file_path =
      ""; // When is_save_audio_file is true, the audio will be saved in the specified position

  ASRControllerConfig clone() {
    var obj = ASRControllerConfig();
    return obj;
  }

  Future<ASRController> build() async {
    final id =
        await _Inner.instance.methodChannel.invokeMethod("ASRController.new", {
      "appID": appID,
      "projectID": projectID,
      "secretID": secretID,
      "secretKey": secretKey,
      "token": token,
      "engine_model_type": engine_model_type,
      "filter_dirty": filter_dirty,
      "filter_modal": filter_modal,
      "filter_punc": filter_punc,
      "convert_num_mode": convert_num_mode,
      "hotword_id": hotword_id,
      "customization_id": customization_id,
      "vad_silence_time": vad_silence_time,
      "needvad": needvad,
      "word_info": word_info,
      "reinforce_hotword": reinforce_hotword,
      "is_compress": is_compress,
      "silence_detect": silence_detect,
      "silence_detect_duration": silence_detect_duration,
      "noise_threshold": noise_threshold,
      "is_save_audio_file": is_save_audio_file,
      "audio_file_path": audio_file_path,
    });
    if (id == null) {
      throw Exception("");
    }
    return ASRController(id);
  }
}

class ASRController {
  int _id;

  ASRController(this._id);

  Stream<ASRData> recognize() async* {
    yield* recognizeWithDataSource(null);
  }

  Stream<ASRData> recognizeWithDataSource(Stream<Uint8List>? source) async* {
    var stream_ctl = StreamController<ASRData>();
    var observer_id =
        _Inner.instance.addInstance(_ASRControllerObserver(stream_ctl));
    await _Inner.instance.methodChannel
        .invokeMethod("ASRController.setObserver", {
      "id": _id,
      "observer_id": observer_id,
    });
    var datasource_id = 0;
    if (source != null) {
      datasource_id = _Inner.instance.addInstance(_ASRDataSource(source));
      await _Inner.instance.methodChannel
          .invokeMethod("ASRController.setDataSource", {
        "id": _id,
        "datasource_id": datasource_id,
      });
    }
    await _Inner.instance.methodChannel
        .invokeMethod("ASRController.start", {"id": _id});
    await for (final val in stream_ctl.stream) {
      yield val;
    }
    if (source != null) {
      _Inner.instance.removeInstance(datasource_id);
    }
    _Inner.instance.removeInstance(observer_id);
  }

  stop() async {
    await _Inner.instance.methodChannel
        .invokeMethod("ASRController.stop", {"id": _id});
  }

  release() async {
    await _Inner.instance.methodChannel
        .invokeMethod("ASRController.release", {"id": _id});
  }
}
