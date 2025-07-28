/// 自动语音转录模块
///
/// 该类作为 Flutter 前台服务的任务处理器，负责：
/// 1. 初始化及管理所有依赖资源：
///    - 配置与加载 ASR（本地/云）、VAD、关键字检测、TTS、BLE 设备
///    - 启动 ObjectBox 数据库、加载LLM服务
/// 2. 处理音频录制与流：
///    - 管理录音、音频包解码、音频处理
///    - 按需保存 WAV 文件
///    - 基于 VAD 检测语音活动，推送音频数据到本地或云端 ASR
/// 3. 支持对话/会议两种模式：
///    - 自动识别并切换对话模式与会议模式
/// 4. 集成 UnifiedChatManager：
///    - 在对话模式下将 ASR 文本推送给 LLM，支持带音频的流式请求
///    - 管理聊天流状态，处理响应并持久化助手/用户对话
/// 5. 与前台任务通信：
///    - 通过 FlutterForegroundTask.sendDataToMain 发送服务状态、ASR/VAD 事件、聊天内容
///    - 响应来自主线程的控制命令（开始/停止录音、切换设备、重载配置等）
///
/// 该处理器单例将在前台服务启动时通过 `startRecordService()` 注册，
/// 生命周期严格与 FlutterForegroundTask 绑定，确保音频处理与任务处理逻辑在后台持续运行。

import 'dart:async';
import 'dart:convert';
import 'dart:developer' as dev;
import 'dart:io';
import 'dart:math';
import 'dart:typed_data';
import 'package:app/config/default_config.dart';
import 'package:app/constants/prompt_constants.dart';
import 'package:app/constants/wakeword_constants.dart';
import 'package:app/models/chat_mode.dart';
import 'package:app/models/asr_mode.dart';

import 'package:app/services/cloud_asr.dart';
import 'package:app/utils/text_process_utils.dart';
import 'package:app/utils/sp_util.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_foreground_task/flutter_foreground_task.dart';
import 'package:opus_dart/opus_dart.dart';
import 'package:opus_flutter/opus_flutter.dart' as opus_flutter;
import 'package:record/record.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'package:sherpa_onnx/sherpa_onnx.dart';
import 'package:sherpa_onnx/sherpa_onnx.dart' as sherpa_onnx;
import 'package:flutter_tts/flutter_tts.dart';
import 'package:audioplayers/audioplayers.dart';
import 'package:linalg/linalg.dart';
import 'package:uuid/uuid.dart';
import '../constants/record_constants.dart';
import '../constants/system_constants.dart';
import '../models/record_entity.dart';
import '../services/objectbox_service.dart';
import '../services/unified_chat_manager.dart';
import '../services/llm_factory.dart';
import '../services/base_llm.dart';
import '../utils/asr_utils.dart';
import '../utils/audio_process_util.dart';
import 'asr_service_isolate.dart';
import '../utils/wav/audio_save_util.dart';

import 'ble_service.dart';

const int nDct = 257;
const int nPca = 47;
const int nPcaPackageByte = 244;
const int nOpusPackageByte = 84;
final Float32List silence = Float32List((16000 * 5).toInt());

@pragma('vm:entry-point')
void startRecordService() {
  FlutterForegroundTask.setTaskHandler(RecordServiceHandler());
}

class RecordServiceHandler extends TaskHandler {
  AudioRecorder _record = AudioRecorder();

  sherpa_onnx.VoiceActivityDetector? _vad;
  late KeywordSpotter _keywordSpotter;
  late OnlineStream _keywordSpotterStream;

  StreamSubscription<RecordState>? _recordSub;

  final ObjectBoxService _objectBoxService = ObjectBoxService();

  bool _inDialogMode = false;
  bool _isMeeting = false;

  /// Get current chat mode
  ChatMode get _currentChatMode {
    if (_isMeeting) {
      return ChatMode.meetingMode;
    } else if (_inDialogMode) {
      return ChatMode.dialogMode;
    } else {
      return ChatMode.defaultMode;
    }
  }

  /// Get current ASR mode
  AsrMode get _currentAsrMode {
    // Use the ASR mode corresponding to current chat mode
    return _getUserConfiguredAsrMode() ?? _currentChatMode.defaultAsrMode;
  }

  /// Get user configured ASR mode from SharedPreferences
  AsrMode? _getUserConfiguredAsrMode() {
    // This method should be async, but we need sync access for the getter
    // We'll use a cached value that gets updated when settings change
    return _cachedUserAsrMode ?? _currentChatMode.defaultAsrMode;
  }

  /// Cached user ASR mode to avoid async calls in getter
  AsrMode? _cachedUserAsrMode;

  /// Load user configured ASR mode from SharedPreferences
  Future<void> _loadUserAsrModeConfig() async {
    try {
      final chatMode = _currentChatMode; // 获取当前聊天模式的快照
      final instance = await SharedPreferences.getInstance();
      await instance.reload();
      final asrModeKey = await SPUtil.getString('asr_mode_${chatMode.name}');

      // 再次检查聊天模式是否发生变化，避免竞态条件
      if (chatMode != _currentChatMode) {
        dev.log('聊天模式在配置加载过程中发生变化，重新加载配置');
        return _loadUserAsrModeConfig(); // 递归重新加载
      }

      _cachedUserAsrMode = AsrModeUtils.fromStorageKey(asrModeKey);
      dev.log(
        '已加载用户ASR配置: ${_cachedUserAsrMode?.name ?? "使用默认"}，聊天模式: ${chatMode.name}',
      );
    } catch (e) {
      dev.log('加载用户ASR配置失败: $e');
      _cachedUserAsrMode = null;
    }
  }

  /// Whether to use cloud services (determined by current ASR mode)
  bool get _isUsingCloudServices {
    return _currentAsrMode.isCloudBased && _cloudAsr.isAvailable;
  }

  /// Whether to use streaming ASR (determined by current ASR mode)
  bool get _shouldUseStreamingAsr {
    return _currentAsrMode.isStreaming && _cloudAsr.canUseStream;
  }

  bool _isInitialized = false;
  RecordState _recordState = RecordState.stop;
  int _lastDataReceivedTimestamp = 0;
  int _boneDataReceivedTimestamp = 0;
  bool _isBoneConductionActive = true;
  bool _onRecording = false;
  bool _budUser = true;
  int? _startMeetingTime;
  bool _kwsBuddie = false;
  bool _kwsJustListen = false;

  late FlutterTts _flutterTts;
  final CloudAsr _cloudAsr = CloudAsr();

  final UnifiedChatManager _unifiedChatManager = UnifiedChatManager();

  int currentStep = 0;
  String currentSpeaker = '';

  List<double> samplesFloat32Buffer = [];

  StreamSubscription<Uint8List>? _bleDataSubscription;
  Timer? _bleTimer;

  Stream<Uint8List>? _recordStream;

  Matrix iDctWeightMatrix = Matrix.fill(nDct, nDct, 0.0);
  Matrix iPcaWeightMatrix = Matrix.fill(nPca, nDct, 0.0);

  List<double> combinedAudio = [];

  List<int> combinedOpusAudio = [];

  final StreamController<Uint8List> _bleAudioStreamController = StreamController<Uint8List>();
  StreamSubscription<Uint8List>? _bleAudioStreamSubscription;

  bool _onMicrophone = false;
  var operationId;

  final AsrServiceIsolate _asrServiceIsolate = AsrServiceIsolate();

  late SimpleOpusDecoder opusDecoder;

  int? _ffStartTime;
  int? _feStartTime;

  // ASR流消息处理的字段
  String? _currentAsrMessageId;
  StreamSubscription? _currentChatSubscription; // 重命名并独立管理聊天流订阅
  bool _isProcessingChat = false; // 添加聊天处理状态标记

  // 新增：用于qwenOmni的音频保存

  /// 检查是否满足音频保存条件
  /// 1. 当前LLM是qwenOmni
  /// 2. 当前ASR模式是本地的ASR模式
  /// 3. 在对话模式下
  bool _shouldSaveAudioForQwenOmni() {
    try {
      // 检查当前LLM类型
      final currentLLMType = LLMFactory.instance.currentType;
      if (currentLLMType != LLMType.qwenOmni) {
        return false;
      }

      // 检查是否在对话模式
      if (!_inDialogMode) {
        return false;
      }

      return true;
    } catch (e) {
      dev.log('检查音频保存条件时出错: $e');
      return false;
    }
  }

  /// 将Float32List转换为Uint8List (16位PCM格式)
  Uint8List _convertFloat32ToUint8List(Float32List samples) {
    final bytes = ByteData(samples.length * 2); // 每个样本2字节

    for (int i = 0; i < samples.length; i++) {
      // 将浮点值 (-1.0 to 1.0) 转换为 16位整数 (-32768 to 32767)
      int intSample = (samples[i] * 32767).round().clamp(-32768, 32767);
      bytes.setInt16(i * 2, intSample, Endian.little);
    }

    return bytes.buffer.asUint8List();
  }

  @override
  Future<void> onStart(DateTime timestamp, TaskStarter starter) async {
    await DefaultConfig.initialize();

    await ObjectBoxService.initialize();

    await _unifiedChatManager.init(
      systemPrompt: '$systemPromptOfChat\n\n${systemPromptOfScenario['voice']}',
    );

    iDctWeightMatrix = await loadRealMatrixFromJson(
      'assets/idct_weight.json',
      nDct,
      nDct,
    );
    iPcaWeightMatrix = await loadRealMatrixFromJson(
      'assets/ipca_weight.json',
      nPca,
      nDct,
    );

    initOpus(await opus_flutter.load());
    opusDecoder = SimpleOpusDecoder(sampleRate: 16000, channels: 1);

    // 初始化用户配置的ASR模式缓存
    await _loadUserAsrModeConfig();

    _initTts();
    _initBle();

    await _cloudAsr.init();
    _cloudAsr.onASRResult = onASRResult;
    await _startRecord();
    // await _cloudTts.init();
  }

  @override
  void onRepeatEvent(DateTime timestamp) {}

  @override
  void onReceiveData(Object data) async {
    if (data is Map) {
      // Handle ASR configuration reload command (when settings change)
      if (data['action'] == 'reloadAsrConfig') {
        await _loadUserAsrModeConfig();
        return;
      }

      // Handle LLM configuration reload command (when settings change)
      if (data['action'] == 'reloadLLMConfig') {
        try {
          dev.log('后台服务: 收到LLM配置重载请求');

          // 重新加载LLMFactory配置
          await LLMFactory.instance.reloadLLMConfig();

          // 重新初始化聊天管理器中的LLM
          await _unifiedChatManager.reinitializeLLM();

          // 获取当前LLM状态并反馈给前端
          final currentLLMType = LLMFactory.instance.currentType;
          final availableTypes = await LLMFactory.getAvailableLLMTypes();
          final supportsAudioInput = LLMFactory.instance.supportsAudioInput;

          dev.log('后台服务: LLM配置重载完成，当前类型: ${currentLLMType?.name}');

          FlutterForegroundTask.sendDataToMain({
            'llmConfigReloaded': true,
            'currentLLMType': currentLLMType?.name,
            'availableLLMTypes': availableTypes.map((t) => t.name).toList(),
            'supportsAudioInput': supportsAudioInput,
          });
        } catch (e) {
          dev.log('后台服务: LLM配置重载失败: $e');
          FlutterForegroundTask.sendDataToMain({
            'llmConfigReloaded': false,
            'error': e.toString(),
          });
        }
        return;
      }

      // Handle direct LLM type switch command
      if (data['action'] == 'switchLLMType') {
        final llmTypeName = data['llmType'] as String?;
        if (llmTypeName != null) {
          try {
            dev.log('后台服务: 收到LLM类型切换请求: $llmTypeName');

            // 解析LLM类型
            LLMType? targetType;
            switch (llmTypeName) {
              case 'customLLM':
                targetType = LLMType.customLLM;
                break;
              case 'qwenOmni':
                targetType = LLMType.qwenOmni;
                break;
            }

            if (targetType != null) {
              // 直接切换LLM类型
              await LLMFactory.instance.switchToLLMType(targetType);

              // 重新初始化聊天管理器
              await _unifiedChatManager.reinitializeLLM();

              dev.log('后台服务: LLM类型切换完成: ${targetType.name}');

              FlutterForegroundTask.sendDataToMain({
                'llmSwitchResult': 'success',
                'currentLLMType': targetType.name,
                'supportsAudioInput': LLMFactory.instance.supportsAudioInput,
              });
            } else {
              throw Exception('无效的LLM类型: $llmTypeName');
            }
          } catch (e) {
            dev.log('后台服务: LLM类型切换失败: $e');
            FlutterForegroundTask.sendDataToMain({
              'llmSwitchResult': 'failed',
              'error': e.toString(),
            });
          }
        }
        return;
      }
    }

    if (data == 'startRecording') {
      _onRecording = true;
    } else if (data == 'stopRecording') {
      _onRecording = false;
    } else if (data == 'device') {
      var remoteId = await FlutterForegroundTask.getData(key: 'deviceRemoteId');
      if (remoteId != null) {
        await BleService().getAndConnect(remoteId);
        BleService().listenToConnectionState();
      }
    } else if (data == 'forget') {
      BleService().forgetDevice();
    } else if (data == Constants.actionStartMicrophone) {
      _isMeeting = false;
      FlutterForegroundTask.sendDataToMain({
        'isMeeting': false,
        'connectionState': false,
      });
      // await _startMicrophone();
      // _budUser = false;
      _inDialogMode = false;
      // 聊天模式变化时重新加载ASR配置
      await _loadUserAsrModeConfig();
      // IsolateTts.interrupt();
      _flutterTts.stop();
    } else if (data == Constants.actionStopMicrophone) {
      await _stopMicrophone();
      _budUser = true;
      FlutterForegroundTask.sendDataToMain({'connectionState': true});
    } else if (data == "InitTTS") {
      try {
        // IsolateTts.init();
      } catch (e, stack) {
        dev.log("TTS has Init $e");
      }
    } else if (data == 'resetCloudAsr') {
      // Reset cloud ASR service, usually called after successful payment to get latest quota
      try {
        dev.log('正在重置云端ASR服务...');
        _cloudAsr.dispose();
        await _cloudAsr.init();
        _cloudAsr.onASRResult = onASRResult;
        dev.log('云端ASR服务重置完成');
        FlutterForegroundTask.sendDataToMain({'asrResetResult': 'success'});
      } catch (e) {
        dev.log('云端ASR服务重置失败: $e');
        FlutterForegroundTask.sendDataToMain({
          'asrResetResult': 'failed',
          'error': e.toString(),
        });
      }
    } else if (data == 'reinitializeLLM') {
      // 重新初始化LLM实例（用于API Key配置更改后）
      try {
        dev.log('正在重新初始化LLM...');
        await _unifiedChatManager.reinitializeLLM();
        dev.log('LLM重新初始化完成');
        FlutterForegroundTask.sendDataToMain({'llmReinitResult': 'success'});
      } catch (e) {
        dev.log('LLM重新初始化失败: $e');
        FlutterForegroundTask.sendDataToMain({
          'llmReinitResult': 'failed',
          'error': e.toString(),
        });
      }
    }
    FlutterForegroundTask.sendDataToMain(Constants.actionDone);
  }

  @override
  Future<void> onDestroy(DateTime timestamp) async {
    await _stopRecord();

    _bleDataSubscription?.cancel();
    _bleAudioStreamSubscription?.cancel();
    _bleTimer?.cancel();
    _cloudAsr.dispose();
    BleService().dispose();
  }

  @override
  void onNotificationButtonPressed(String id) async {
    if (id == Constants.actionStopRecord) {
      await _stopRecord();
      if (await FlutterForegroundTask.isRunningService) {
        FlutterForegroundTask.stopService();
      }
    }
  }

  void _initBle() async {
    await BleService().init();
    _bleDataSubscription?.cancel();
    _bleDataSubscription = BleService().dataStream.listen((value) {
      final currentTime = DateTime.now().millisecondsSinceEpoch;

      if (value.length == nPcaPackageByte) {
        if (value[0] == 0xff || value[0] == 0xfe) {
          _processMeetingStatus(value[0] == 0xfe, currentTime);
          _decodeAndProcessPcaPackage(value);
        } else if (value[0] == 0x00 || value[0] == 0x01) {
          _processBoneConduction(value[0] == 0x01, currentTime);
        }
      } else if (value.length == nOpusPackageByte) {
        _processBoneConduction(value[80] == 0x01, currentTime);
        _processMeetingStatus(value[81] == 0xfe, currentTime);
        _decodeAndProcessOpusPackage(value);
      } else {
        if (kDebugMode) {
          dev.log("Unexpected BLE data length: ${value.length}");
        }
      }
    });

    _bleAudioStreamSubscription?.cancel();
    _bleAudioStreamSubscription = _bleAudioStreamController.stream.listen((bleAudioClip) {
      dev.log('Process start!!');
      _processAudioData(bleAudioClip);
    });
  }

  void _processBoneConduction(bool status, int currentTime) {
    if (status) {
      if (!_isBoneConductionActive) {
        _isBoneConductionActive = true;
      }
      _boneDataReceivedTimestamp = currentTime;
    } else {
      if (_isBoneConductionActive && currentTime - _boneDataReceivedTimestamp > 2000) {
        _isBoneConductionActive = false;
      }
    }
  }

  void _processMeetingStatus(bool status, int currentTime) async {
    if (!_isMeeting) {
      if (status) {
        _feStartTime ??= currentTime;
        if (currentTime - _feStartTime! > 10000) {
          _isMeeting = true;
          _startMeetingTime = currentTime;
          _inDialogMode = false;
          _feStartTime = null;
          // 聊天模式变化时重新加载ASR配置
          await _loadUserAsrModeConfig();
          FlutterForegroundTask.sendDataToMain({'isMeeting': true});
          // IsolateTts.interrupt();
          _flutterTts.stop();
          FlutterForegroundTask.sendDataToMain({
            'text': SystemConstants.meetingStart,
            'isEndpoint': true,
            'inDialogMode': _inDialogMode,
            'isMeeting': _isMeeting,
            'speaker': 'assistant',
          });
          _objectBoxService.insertMeetingRecord(
            RecordEntity(
              role: 'assistant',
              content: SystemConstants.meetingStart,
            ),
          );
        }
      } else {
        _feStartTime = null;
      }
    } else {
      if (!status) {
        _ffStartTime ??= currentTime;
        if (currentTime - _ffStartTime! > 10000) {
          _isMeeting = false;
          _ffStartTime = null;
          // 聊天模式变化时重新加载ASR配置
          await _loadUserAsrModeConfig();
          FlutterForegroundTask.sendDataToMain({'isMeeting': false});
          FlutterForegroundTask.sendDataToMain({
            'text': SystemConstants.meetingEnd,
            'isEndpoint': true,
            'inDialogMode': _inDialogMode,
            'isMeeting': _isMeeting,
            'speaker': 'assistant',
          });
          _objectBoxService.insertMeetingRecord(
            RecordEntity(
              role: 'assistant',
              content: SystemConstants.meetingEnd,
            ),
          );
        }
      } else {
        _ffStartTime = null;
      }
    }
  }

  void _decodeAndProcessPcaPackage(Uint8List value) async {
    for (var i = 0; i < 3; i++) {
      var audioSlice = AudioProcessingUtil.processSinglePackage(
        value.sublist(1 + i * 80, 1 + (i + 1) * 80),
        iPcaWeightMatrix,
        iDctWeightMatrix,
      );
      combinedAudio.addAll(audioSlice);
      if (combinedAudio.length == 512) {
        Uint8List data = Uint8List(512 * 2);
        for (var j = 0; j < 512; j++) {
          data[j * 2] = (combinedAudio[j] * 32767).toInt();
          data[j * 2 + 1] = (combinedAudio[j] * 32767).toInt() >> 8;
        }
        _bleAudioStreamController.add(data);
        combinedAudio.clear();
      }
    }
  }

  void _decodeAndProcessOpusPackage(Uint8List value) async {
    Uint8List micPart = value.sublist(0, 40);
    Uint8List spkPart = value.sublist(40, 80);
    bool micPartNoneZero = micPart.reduce((value, element) => value + element.abs()) > 0;
    bool spkPartNoneZero = spkPart.reduce((value, element) => value + element.abs()) > 0;
    if (!micPartNoneZero && !spkPartNoneZero) {
      return;
    }
    Int16List micClip = micPartNoneZero ? opusDecoder.decode(input: micPart) : Int16List(0);
    Int16List spkClip = spkPartNoneZero ? opusDecoder.decode(input: spkPart) : Int16List(0);
    for (var i = 0; i < max(micClip.length, spkClip.length); i++) {
      combinedOpusAudio.add(
          (i < micClip.length ? micClip[i] : 0) + (i < spkClip.length ? spkClip[i] : 0)
      );
    }
    if (combinedOpusAudio.length > 1000) {
      Uint8List pcmData = Uint8List(combinedOpusAudio.length * 2);
      for (var i = 0; i < combinedOpusAudio.length; i++) {
        pcmData[i * 2] = combinedOpusAudio[i];
        pcmData[i * 2 + 1] = combinedOpusAudio[i] >> 8;
      }
      _bleAudioStreamController.add(pcmData);
      combinedOpusAudio = [];
    }
  }

  Future<void> _initTts() async {
    _flutterTts = FlutterTts();
    await _flutterTts.awaitSpeakCompletion(true);
    if (Platform.isAndroid) {
      await _flutterTts.setQueueMode(1);
    }
  }

  Future<void> _initAsr() async {
    if (!_isInitialized) {
      sherpa_onnx.initBindings();

      _vad = await initVad();
      _keywordSpotter = await initKeywordSpotter();
      _keywordSpotterStream = _keywordSpotter.createStream();

      await _asrServiceIsolate.init();

      _recordSub = _record.onStateChanged().listen((recordState) {
        _recordState = recordState;
      });

      _isInitialized = true;
    }
  }

  Future<void> _startRecord() async {
    await _initAsr();

    if (BleService().connectionState == BluetoothConnectionState.disconnected) {
      _startMicrophone();
    }

    FlutterForegroundTask.saveData(key: 'isRecording', value: true);
    // create stop action button
    FlutterForegroundTask.updateService(
      notificationText: 'Recording...',
      notificationButtons: [
        const NotificationButton(id: Constants.actionStopRecord, text: 'stop'),
      ],
    );
  }

  Future<void> _startMicrophone() async {
    if (_onMicrophone) return;
    _onMicrophone = true;
    if (_recordStream != null) return;
    const config = RecordConfig(
      encoder: AudioEncoder.pcm16bits,
      sampleRate: 16000,
      numChannels: 1,
    );
    _record = AudioRecorder();
    _recordStream = await _record.startStream(config);
    _recordStream?.listen((data) {
      _processAudioData(data);
    });
  }

  void onASRResult(String data, bool isFinish) {
    if (data.isNotEmpty) {
      _flutterTts.stop();
    }
    if (isFinish) {
      final operationId = Uuid().v1();
      final text = data
          .replaceFirst('Buddy', 'Buddie')
          .replaceFirst('buddy', 'buddie')
          .replaceFirst('body', 'buddie');
      _processFinalAsrResult(text, operationId);
    } else {
      final text = data
          .replaceFirst('Buddy', 'Buddie')
          .replaceFirst('buddy', 'buddie')
          .replaceFirst('body', 'buddie');
      _processIntermediateAsrResult(text);
    }
  }

  // 处理ASR中间结果（打字机效果）
  void _processIntermediateAsrResult(String text) {
    if (text.isEmpty) return;

    // 如果没有当前消息ID，创建新的
    if (_currentAsrMessageId == null) {
      _currentAsrMessageId = Uuid().v4();
    }

    // 发送ASR流数据
    FlutterForegroundTask.sendDataToMain({
      'asrMessageId': _currentAsrMessageId,
      'asrText': text,
      'asrIsStreaming': true,
      'asrIsEndpoint': false,
      'inDialogMode': _inDialogMode,
      'isMeeting': _isMeeting,
      'speaker': 'user',
      'isVadDetected': true,
    });
  }

  // 处理ASR最终结果
  void _processFinalAsrResult(String text, String? operationId) {
    if (text.isEmpty) return;

    text = text.trim();
    text = TextProcessUtils.removeBracketsContent(text);
    text = TextProcessUtils.clearIfRepeatedMoreThanFiveTimes(text);
    text = text.trim();

    if (text.isEmpty) {
      return;
    }

    // 发送最终ASR数据
    if (_currentAsrMessageId != null) {
      FlutterForegroundTask.sendDataToMain({
        'asrMessageId': _currentAsrMessageId,
        'asrText': text,
        'asrIsStreaming': false,
        'asrIsEndpoint': true,
        'inDialogMode': _inDialogMode,
        'isMeeting': _isMeeting,
        'speaker': 'user',
        'isVadDetected': false,
      });

      // 重置状态
      _currentAsrMessageId = null;
    }

    // 直接调用后续处理逻辑，不使用_processFinalResult避免重复
    _handlePostAsrProcessing(text, operationId);
  }

  // 处理ASR后续逻辑（从_processFinalResult中提取出来）
  void _handlePostAsrProcessing(String text, String? operationId) {
    if (!_inDialogMode &&
        (wakeword_constants.wakeWordStartDialog.any(
              (keyword) => text.toLowerCase().contains(keyword),
            ) ||
            _kwsBuddie)) {
      _kwsBuddie = false;
      _kwsJustListen = false;
      if (!_isMeeting && _budUser) {
        _inDialogMode = true;
        AudioPlayer().play(AssetSource('audios/interruption.wav'));
      } else {
        if (_isMeeting) {
          FlutterForegroundTask.sendDataToMain({
            'text': SystemConstants.callBuddieInMeeting,
            'isEndpoint': true,
            'inDialogMode': _inDialogMode,
            'isMeeting': _isMeeting,
            'speaker': 'assistant',
          });
          _objectBoxService.insertDialogueRecord(
            RecordEntity(
              role: 'assistant',
              content: SystemConstants.callBuddieInMeeting,
            ),
          );
        } else if (!_budUser) {
          FlutterForegroundTask.sendDataToMain({
            'text': SystemConstants.callBuddieNoBud,
            'isEndpoint': true,
            'inDialogMode': _inDialogMode,
            'isMeeting': _isMeeting,
            'speaker': 'assistant',
          });
          _objectBoxService.insertDialogueRecord(
            RecordEntity(
              role: 'assistant',
              content: SystemConstants.callBuddieNoBud,
            ),
          );
        }
      }
    }

    if (_isMeeting) {
      _objectBoxService.insertMeetingRecord(
        RecordEntity(role: 'user', content: text),
      );
      _unifiedChatManager.addChatSession('user', text);
    } else {
      if (_inDialogMode) {
        _objectBoxService.insertDialogueRecord(
          RecordEntity(role: 'user', content: text),
        );
        _unifiedChatManager.addChatSession('user', text);
        if (wakeword_constants.wakeWordEndDialog.any(
              (keyword) => text.toLowerCase().contains(keyword),
            ) ||
            _kwsJustListen) {
          _inDialogMode = false;
          _kwsJustListen = false;
          _kwsBuddie = false;
          _vad!.clear();
          _flutterTts.stop();
          AudioPlayer().play(AssetSource('audios/beep.wav'));
        }
      } else {
        _objectBoxService.insertDefaultRecord(
          RecordEntity(role: 'user', content: text),
        );
        _unifiedChatManager.addChatSession('user', text);
      }
    }

    if (_inDialogMode) {
      _startChatStreamingRequest(text, null);
    }
  }

  void _processAudioData(
    data, {
    String category = RecordEntity.categoryDefault,
    String? operationId,
  }) async {
    if (_vad == null || !_asrServiceIsolate.isInitialized) {
      return;
    }

    FileService.highSaveWav(
      startMeetingTime: _startMeetingTime,
      onRecording: _isMeeting,
      data: data,
      numChannels: 1,
      sampleRate: 16000,
    );

    if (!_onRecording) return;

    if (_shouldUseStreamingAsr) {
      // 流式ASR模式：直接推送到云端流式处理（会议模式）
      _cloudAsr.pushStreamData(data);
      return;
    }

    final samplesFloat32 = convertBytesToFloat32(Uint8List.fromList(data));
    _vad!.acceptWaveform(samplesFloat32);
    _keywordSpotterStream.acceptWaveform(
      samples: samplesFloat32,
      sampleRate: 16000,
    );
    while (_keywordSpotter.isReady(_keywordSpotterStream)) {
      _keywordSpotter.decode(_keywordSpotterStream);
      final text = _keywordSpotter.getResult(_keywordSpotterStream).keyword;
      if (text.isNotEmpty) {
        if (wakeword_constants.wakeWordStartDialog.any(
          (keyword) => text.toLowerCase().contains(keyword),
        )) {
          _kwsBuddie = true;
        } else if (wakeword_constants.wakeWordEndDialog.any(
          (keyword) => text.toLowerCase().contains(keyword),
        )) {
          _kwsJustListen = true;
        } else {}
      }
    }

    if (_vad!.isDetected() &&
        _isBoneConductionActive &&
        _inDialogMode &&
        !_isProcessingChat) {
      _currentChatSubscription?.cancel();
      _isProcessingChat = false;
      _flutterTts.stop();
    }

    if (_vad!.isDetected()) {
      FlutterForegroundTask.sendDataToMain({'isVadDetected': true});
    } else {
      FlutterForegroundTask.sendDataToMain({'isVadDetected': false});
    }

    var text = '';
    while (!_vad!.isEmpty()) {
      final samples = _vad!.front().samples;
      if (samples.length < _vad!.config.sileroVad.windowSize) {
        break;
      }
      _vad!.pop();

      FlutterForegroundTask.sendDataToMain({'action': 'stopAudio'});

      Float32List paddedSamples = await _addSilencePadding(samples);
      var segment = '';

      // 根据当前ASR模式选择识别方式
      if (_isUsingCloudServices) {
        // 云端ASR模式（对话模式）
        dev.log('使用云端ASR: ${_currentAsrMode.name}');
        segment = await _cloudAsr.recognize(paddedSamples);
      } else {
        // 本地离线ASR模式（默认模式）
        dev.log('使用本地ASR: ${_currentAsrMode.name}');

        // 检查是否需要为qwenOmni保存音频

        segment = await _asrServiceIsolate.sendData(paddedSamples);
      }
      segment = segment
          .replaceFirst('Buddy', 'Buddie')
          .replaceFirst('buddy', 'buddie');

      text += segment;

      _processIntermediateResult(segment);

      if (text.isNotEmpty) {
        _processFinalResult(
          text,
          'user',
          category: category,
          operationId: operationId,
          lastAsrAudioData: _shouldSaveAudioForQwenOmni()
              ? paddedSamples
              : null,
        );
      }
    }
  }

  void _processIntermediateResult(String text) {
    if (text.isEmpty) return;

    // 使用ASR流格式，与onASRResult保持一致
    if (_currentAsrMessageId == null) {
      _currentAsrMessageId = Uuid().v4();
    }

    FlutterForegroundTask.sendDataToMain({
      'asrMessageId': _currentAsrMessageId,
      'asrText': text,
      'asrIsStreaming': true,
      'asrIsEndpoint': false,
      'inDialogMode': _inDialogMode,
      'isMeeting': _isMeeting,
      'speaker': 'user',
      'isVadDetected': true,
    });
  }

  void _processFinalResult(
    String text,
    String speaker, {
    String category = RecordEntity.categoryDefault,
    String? operationId,
    bool isS2s = false,
    Float32List? lastAsrAudioData,
  }) {
    if (text.isEmpty) return;

    text = text.trim();
    text = TextProcessUtils.removeBracketsContent(text);
    text = TextProcessUtils.clearIfRepeatedMoreThanFiveTimes(text);
    text = text.trim();

    if (text.isEmpty) {
      return;
    }

    // 如果有ASR消息ID，发送最终的ASR流数据
    if (_currentAsrMessageId != null && speaker == 'user') {
      FlutterForegroundTask.sendDataToMain({
        'asrMessageId': _currentAsrMessageId,
        'asrText': text,
        'asrIsStreaming': false,
        'asrIsEndpoint': true,
        'inDialogMode': _inDialogMode,
        'isMeeting': _isMeeting,
        'speaker': speaker,
      });

      // 重置ASR状态
      _currentAsrMessageId = null;
    } else {
      // 发送普通消息（非ASR流消息）
      FlutterForegroundTask.sendDataToMain({
        'text': text,
        'isEndpoint': true,
        'inDialogMode': _inDialogMode,
        'isMeeting': _isMeeting,
        'speaker': speaker,
      });
    }

    if (!_inDialogMode &&
        speaker == 'user' &&
        (wakeword_constants.wakeWordStartDialog.any(
              (keyword) => text.toLowerCase().contains(keyword),
            ) ||
            _kwsBuddie)) {
      _kwsBuddie = false;
      _kwsJustListen = false;
      if (!_isMeeting && _budUser) {
        _inDialogMode = true;
        // 聊天模式变化时重新加载ASR配置（异步执行，不阻塞当前流程）
        _loadUserAsrModeConfig();
        AudioPlayer().play(AssetSource('audios/interruption.wav'));
      } else {
        if (_isMeeting) {
          FlutterForegroundTask.sendDataToMain({
            'text': SystemConstants.callBuddieInMeeting,
            'isEndpoint': true,
            'inDialogMode': _inDialogMode,
            'isMeeting': _isMeeting,
            'speaker': 'assistant',
          });
          _objectBoxService.insertDialogueRecord(
            RecordEntity(
              role: 'assistant',
              content: SystemConstants.callBuddieInMeeting,
            ),
          );
        } else if (!_budUser) {
          FlutterForegroundTask.sendDataToMain({
            'text': SystemConstants.callBuddieNoBud,
            'isEndpoint': true,
            'inDialogMode': _inDialogMode,
            'isMeeting': _isMeeting,
            'speaker': 'assistant',
          });
          _objectBoxService.insertDialogueRecord(
            RecordEntity(
              role: 'assistant',
              content: SystemConstants.callBuddieNoBud,
            ),
          );
        }
      }
    }

    if (_isMeeting) {
      _objectBoxService.insertMeetingRecord(
        RecordEntity(role: 'user', content: text),
      );
      _unifiedChatManager.addChatSession('user', text);
    } else {
      if (speaker != 'user') {
        _objectBoxService.insertDefaultRecord(
          RecordEntity(role: isS2s ? 'assistant' : 'others', content: text),
        );
        _unifiedChatManager.addChatSession(
          isS2s ? 'assistant' : 'others',
          text,
        );
      } else {
        if (_inDialogMode) {
          _objectBoxService.insertDialogueRecord(
            RecordEntity(role: 'user', content: text),
          );
          _unifiedChatManager.addChatSession('user', text);
          if (wakeword_constants.wakeWordEndDialog.any(
                (keyword) => text.toLowerCase().contains(keyword),
              ) ||
              _kwsJustListen) {
            _inDialogMode = false;
            _kwsJustListen = false;
            _kwsBuddie = false;
            // 聊天模式变化时重新加载ASR配置（异步执行，不阻塞当前流程）
            _loadUserAsrModeConfig();
            _vad!.clear();
            _flutterTts.stop();
            AudioPlayer().play(AssetSource('audios/beep.wav'));
          } else {
            // 只有在不是结束对话时才启动聊天流
            _startChatStreamingRequest(text, lastAsrAudioData);
          }
        } else {
          _objectBoxService.insertDefaultRecord(
            RecordEntity(role: 'user', content: text),
          );
          _unifiedChatManager.addChatSession('user', text);
        }
      }
    }
  }

  // 新增：独立的聊天流请求处理方法
  void _startChatStreamingRequest(String text, Float32List? lastAsrAudioData) {
    // 如果已经在处理聊天，不要重复启动
    if (_isProcessingChat) {
      return;
    }

    // 取消之前的订阅
    _currentChatSubscription?.cancel();
    _currentChatSubscription = null;

    _isProcessingChat = true;
    try {
      if (_unifiedChatManager.getCurrentLLMType() == LLMType.qwenOmni) {
        // 将 Float32List 转换为 Uint8List (16位PCM格式)
        final audioBytes = lastAsrAudioData != null
            ? _convertFloat32ToUint8List(lastAsrAudioData)
            : null;
        _currentChatSubscription = _unifiedChatManager
            .createStreamingRequestWithAudio(
              audioData: audioBytes,
              userMessage: text,
            )
            .listen(
              (response) {
                final res = jsonDecode(response);
                final content = res['content'] ?? res['delta'];
                final isFinished = res['isFinished'];

                FlutterForegroundTask.sendDataToMain({
                  'currentText': text,
                  'isFinished': false,
                  'content': res['delta'],
                });

                if (isFinished) {
                  _objectBoxService.insertDialogueRecord(
                    RecordEntity(role: 'assistant', content: content),
                  );
                  _unifiedChatManager.addChatSession('assistant', content);
                  _isProcessingChat = false; // 处理完成，重置状态
                }
              },
              onError: (error) {
                dev.log('Chat streaming error: $error');
                _isProcessingChat = false; // 出错时重置状态
              },
              onDone: () {
                _isProcessingChat = false; // 流结束时重置状态
              },
            );
      } else {
        _currentChatSubscription = _unifiedChatManager
            .createStreamingRequest(text: text)
            .listen(
              (response) {
                final res = jsonDecode(response);
                final content = res['content'] ?? res['delta'];
                final isFinished = res['isFinished'];

                FlutterForegroundTask.sendDataToMain({
                  'currentText': text,
                  'isFinished': false,
                  'content': res['delta'],
                });

                _flutterTts.speak(res['delta']);

                if (isFinished) {
                  _objectBoxService.insertDialogueRecord(
                    RecordEntity(role: 'assistant', content: content),
                  );
                  _unifiedChatManager.addChatSession('assistant', content);
                  _isProcessingChat = false; // 处理完成，重置状态
                }
              },
              onError: (error) {
                dev.log('Chat streaming error: $error');
                _isProcessingChat = false; // 出错时重置状态
              },
              onDone: () {
                _isProcessingChat = false; // 流结束时重置状态
              },
            );
      }
    } catch (e) {
      dev.log('Failed to start chat streaming: $e');
      _isProcessingChat = false;
    }
  }

  Future<void> _stopRecord() async {
    if (_recordStream != null) {
      await _record.stop();
      await _record.dispose();
      _recordStream = null;
    }

    _recordSub?.cancel();
    _currentChatSubscription?.cancel(); // 修改：使用新的订阅变量名
    _vad?.free();
    _asrServiceIsolate.stopRecord();
    _cloudAsr.stopStream();

    // 重置状态
    _isProcessingChat = false;
    _currentAsrMessageId = null;

    _isInitialized = false;

    FlutterForegroundTask.saveData(key: 'isRecording', value: false);
    FlutterForegroundTask.updateService(
      notificationText: 'Tap to return to the app',
    );
  }

  Future<void> _stopMicrophone() async {
    if (!_onMicrophone) return;
    if (_recordStream != null) {
      await _record.stop();
      await _record.dispose();
      _recordStream = null;
      _onMicrophone = false;
    }
  }

  Future<sherpa_onnx.VoiceActivityDetector> initVad() async =>
      sherpa_onnx.VoiceActivityDetector(
        config: sherpa_onnx.VadModelConfig(
          sileroVad: sherpa_onnx.SileroVadModelConfig(
            model: await copyAssetFile('assets/silero_vad.onnx'),
            minSilenceDuration: 0.5,
            minSpeechDuration: 0.25,
            windowSize: 512,
            maxSpeechDuration: 5.0,
          ),
          numThreads: 1,
          debug: true,
        ),
        bufferSizeInSeconds: 12.0,
      );

  Future<KeywordSpotter> initKeywordSpotter() async {
    const kwsDir =
        'assets/sherpa-onnx-kws-zipformer-gigaspeech-3.3M-2024-01-01';
    const encoder = 'encoder-epoch-12-avg-2-chunk-16-left-64.onnx';
    const decoder = 'decoder-epoch-12-avg-2-chunk-16-left-64.onnx';
    const joiner = 'joiner-epoch-12-avg-2-chunk-16-left-64.onnx';
    KeywordSpotter kws = KeywordSpotter(
      KeywordSpotterConfig(
        model: OnlineModelConfig(
          transducer: OnlineTransducerModelConfig(
            encoder: await copyAssetFile('$kwsDir/$encoder'),
            decoder: await copyAssetFile('$kwsDir/$decoder'),
            joiner: await copyAssetFile('$kwsDir/$joiner'),
          ),
          tokens: await copyAssetFile('$kwsDir/tokens_kws.txt'),
        ),
        keywordsFile: await copyAssetFile('$kwsDir/keywords.txt'),
      ),
    );
    return kws;
  }

  Future<List<List<double>>> loadMatrixFromJson(
    String assetPath,
    int rows,
    int cols,
  ) async {
    String jsonString = await rootBundle.loadString(assetPath);
    List<dynamic> jsonData = jsonDecode(jsonString);
    // Check if jsonData is empty
    if (jsonData.isEmpty) {
      return [];
    }
    // Check if the length of jsonData matches the expected size
    if (jsonData.length != rows * cols) {
      dev.log(
        'Warning: jsonData length (${jsonData.length}) does not match expected size ($rows * $cols).',
      );
      return [];
    }
    List<List<double>> matrix = List.generate(rows, (i) {
      return List.generate(cols, (j) {
        return jsonData[i * cols + j].toDouble();
      });
    });
    return matrix;
  }

  Future<Matrix> loadRealMatrixFromJson(
    String assetPath,
    int rows,
    int cols,
  ) async {
    String jsonString = await rootBundle.loadString(assetPath);
    List<dynamic> jsonData = jsonDecode(jsonString);
    dev.log('Loaded JSON data type: ${jsonData.runtimeType}');
    dev.log('Number of elements in jsonData: ${jsonData.length}');

    Matrix matrix = Matrix.fill(rows, cols, 0.0);
    if (jsonData.isEmpty) {
      // Check if jsonData is empty
      dev.log('jsonData is empty!');
    } else if (jsonData.length != rows * cols) {
      // Check if the length of jsonData matches the expected size
      dev.log(
        'Warning: jsonData length (${jsonData.length}) does not match expected size ($rows * $cols).',
      );
    } else {
      for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
          matrix[i][j] = jsonData[i * cols + j].toDouble();
        }
      }
    }

    return matrix;
  }

  Future<Float32List> _addSilencePadding(Float32List samples) async {
    int totalLength = silence.length * 2 + samples.length;

    Float32List paddedSamples = Float32List(totalLength);

    paddedSamples.setAll(silence.length, samples);

    return paddedSamples;
  }
}
