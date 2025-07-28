/// 云端语音识别(ASR)服务
///
/// 提供实时流式识别和文件式识别两种模式，支持腾讯云语音识别服务：
/// 1. 流式识别模式：
///   - 实时音频数据分块推送（1280字节/40ms）
///   - 防溢出缓冲区管理（自动清理旧数据）
///   - 实时结果回调机制
/// 2. 文件识别模式：
///   - 音频数据转WAV格式存储
///   - 调用API进行整句识别
/// 3. 资源管理：
///   - 自动初始化/释放识别控制器
///   - 临时文件自动清理
///   - 异常处理与日志记录
///
/// 使用示例：
/// ```dart
/// // 初始化服务
/// final asr = CloudAsr();
/// await asr.init();
///
/// // 流式识别
/// asr.onASRResult = (text, isFinal) {
///   print('识别结果: $text ${isFinal ? '(最终)' : ''}');
/// };
/// asr.pushStreamData(audioChunk);
///
/// // 文件识别
/// final result = await asr.recognize(audioData);
/// ```
///
/// 注意事项：
/// - 依赖腾讯云语音识别服务（需配置secretID/secretKey）
/// - 流式识别采用固定分块大小(1280字节)和间隔(40ms)
/// - 缓冲区自动清理机制防止内存溢出
/// - 文件识别会产生临时WAV文件，识别后自动删除

import 'dart:typed_data';
import 'package:app/config/default_config.dart';
import 'package:asr_plugin/asr_plugin.dart';
import 'package:asr_plugin/flashfile_plugin.dart';
import 'package:asr_plugin/onesentence_plugin.dart';
import 'dart:developer' as dev;
import 'dart:async';
import 'dart:io';
import 'package:path_provider/path_provider.dart';
import 'package:wav/wav.dart';
import 'package:flutter/foundation.dart';

typedef OnASRResult = void Function(String data, bool isFinish);

class CloudAsr {
  static const int CHUNK_SIZE =
      1280; // Data size per packet required by Tencent SDK (bytes)
  static const int PUSH_INTERVAL_MS =
      40; // Push interval required by Tencent SDK (milliseconds)
  static const maxBufferSize = 16000 * 2 * 5; // 5 seconds of 16000Hz 16bit data
  static get secretID => DefaultConfig.defaultTencentSecretId;
  static get secretKey => DefaultConfig.defaultTencentSecretKey;
  static get token => DefaultConfig.defaultTencentToken;

  // Audio data buffer related - Used for packet push according to Tencent SDK requirements
  final List<int> _audioBuffer = [];
  Timer? _streamPushTimer;

  StreamController<Uint8List> source = StreamController();
  StreamSubscription<ASRData>? _asrStreamSubscription;

  bool get isAvailable {
    return secretID.isNotEmpty && secretKey.isNotEmpty && token.isNotEmpty;
  }

  bool get canUseStream {
    return asrController != null && isAvailable;
  }

  OnASRResult? onASRResult;

  ASRController? asrController;

  Future<void> init() async {
    try {
      var _config = ASRControllerConfig();
      _config.filter_dirty = 1;
      _config.filter_modal = 0;
      _config.filter_punc = 0;
      _config.appID = 1360165317;
      _config.secretID = secretID;
      _config.secretKey = secretKey;
      _config.token = token;
      asrController = await _config.build();
    } catch (e) {
      dev.log('CloudAsr init error: $e');
    }
  }

  void stopStream() {
    try {
      _asrStreamSubscription?.cancel();
      _asrStreamSubscription = null;
      asrController?.stop();

      // Stop timed push
      _streamPushTimer?.cancel();
      _streamPushTimer = null;

      // Clear audio buffer
      _audioBuffer.clear();
    } catch (e) {
      dev.log('CloudAsr stopStream error: $e');
    }
  }

  void startStream() {
    if (_asrStreamSubscription != null) {
      return;
    }
    source = StreamController();
    // Start timed push mechanism, push 1280 bytes data every 40ms
    _startStreamPushTimer();

    try {
      _asrStreamSubscription = asrController
          ?.recognizeWithDataSource(source.stream)
          .listen(
            (ASRData data) {
              if (data.res != null) {
                if (data.type == ASRDataType.SEGMENT) {
                  dev.log('ASR Data Type: ${data.type.name}, Result: ${data}');
                }
                onASRResult?.call(data.res!, data.type == ASRDataType.SEGMENT);
              }
            },
            onError: (error) {
              dev.log('ASR Stream异常: $error');
              _asrStreamSubscription?.cancel();
              _asrStreamSubscription = null;
              asrController?.stop();

              // Stop timed push
              _streamPushTimer?.cancel();
              _streamPushTimer = null;
              _audioBuffer.clear();
            },
            onDone: () {
              dev.log('ASR stream completed');
              _asrStreamSubscription = null;

              // Stop timed push
              _streamPushTimer?.cancel();
              _streamPushTimer = null;
              _audioBuffer.clear();
            },
          );
    } catch (e) {
      dev.log('CloudAsr startStream error: $e');
      // Stop timed push
      _streamPushTimer?.cancel();
      _streamPushTimer = null;
      _audioBuffer.clear();
    }
  }

  // ==================== 音频处理和一句话识别方法 ====================

  Future<File> _writeAudioToFile(Float32List audioData) async {
    final tempDir = await getTemporaryDirectory();
    final timestamp = DateTime.now().millisecondsSinceEpoch;
    final file = File('${tempDir.path}/recorded_audio_$timestamp.wav');

    final wav = Wav([Float64List.fromList(audioData)], 16000);
    await file.writeAsBytes(wav.write());
    return file;
  }

  final FlashFileASRController _flashFileASRController =
      FlashFileASRController();

  Future<String> _sendToWhisperApi(File audioFile) async {
    var params = FlashFileASRParams();
    params.appid = 1360165317;
    params.secretid = secretID;
    params.secretkey = secretKey;
    params.token = token;
    params.data = await audioFile.readAsBytes();
    params.voice_format = OneSentenceASRParams.FORMAT_WAV;
    var ret = (await _flashFileASRController.recognize(params));
    await audioFile.delete();
    return ret.flash_result?.map((e) => e.text).toList().join('\n') ?? '';
  }

  void pushStreamData(Uint8List data) {
    startStream();
    // Add data to buffer instead of direct push
    _audioBuffer.addAll(data);
  }

  /// Start timed push mechanism, push 1280 bytes data every 40ms
  void _startStreamPushTimer() {
    _streamPushTimer?.cancel(); // Ensure no duplicate start
    _streamPushTimer = Timer.periodic(
      const Duration(milliseconds: PUSH_INTERVAL_MS),
      (timer) => _pushAudioChunk(),
    );
  }

  /// Push fixed-size audio data chunks from buffer
  void _pushAudioChunk() {
    if (_audioBuffer.length >= CHUNK_SIZE) {
      // Extract 1280 bytes data
      final chunk = Uint8List.fromList(_audioBuffer.take(CHUNK_SIZE).toList());
      // Remove extracted data from buffer
      _audioBuffer.removeRange(0, CHUNK_SIZE);

      // Push to Tencent SDK
      source.add(chunk);
    }

    // Prevent buffer from becoming too large (clear part if data exceeds 5 seconds)
    if (_audioBuffer.length > maxBufferSize) {
      final removeCount = _audioBuffer.length - maxBufferSize;
      _audioBuffer.removeRange(0, removeCount);
      dev.log('音频缓冲区过大，清理了$removeCount字节旧数据');
    }
  }

  Future<String> recognize(Float32List audioData) async {
    final audioFile = await _writeAudioToFile(audioData);
    try {
      final transcription = await _sendToWhisperApi(audioFile);
      dev.log("Whisper API result: $transcription");
      return transcription;
    } catch (error) {
      dev.log("Recognition error: $error");
      throw error;
    }
  }

  void dispose() {
    try {
      // Stop timed push
      _streamPushTimer?.cancel();
      _streamPushTimer = null;

      // Clear audio buffer
      _audioBuffer.clear();

      // Cancel subscription
      _asrStreamSubscription?.cancel();
      _asrStreamSubscription = null;

      // Release ASR controller
      asrController?.release();
      asrController = null;

      // Close data stream
      source.close();

      dev.log('CloudAsr资源已清理完成');
    } catch (e) {
      dev.log('CloudAsr dispose时发生错误: $e');
    }
  }
}
