/// 聊天界面核心控制器
///
/// 负责管理聊天界面的完整生命周期和交互逻辑：
/// 1. 消息管理：
///   - 历史消息加载（分页机制）
///   - 实时消息处理（流式响应）
///   - 消息状态跟踪（已读/未读）
/// 2. 语音交互：
///   - 语音活动检测（VAD）
///   - ASR流式消息处理（实时语音转文字）
/// 3. LLM集成：
///   - 统一LLM管理器接入
///   - 动态LLM切换
/// 4. 状态管理：
///   - 滚动位置智能维护
///   - 跨实例状态同步
///
/// 核心工作流程：
/// 1. 初始化：加载历史消息、配置LLM、准备音频系统
/// 2. 消息处理：
///   - 用户输入 → 发送到LLM → 处理响应 → 更新UI
///   - ASR流 → 实时显示 → 最终确认 → 保存记录
///
/// 使用示例：
/// ```dart
/// // 创建控制器
/// final controller = ChatController(
///   onNewMessage: () => _scrollToBottom(),
/// );
///
/// // 发送文本消息
/// controller.sendMessage(text: '你好');
///
/// // 处理音频响应
/// FlutterForegroundTask.sendDataToTask({
///   'action': 'playAudio',
///   'data': audioBytes,
/// });
/// ```

import 'dart:async';
import 'dart:developer' as dev;
import 'dart:io';

import 'package:app/constants/prompt_constants.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_foreground_task/flutter_foreground_task.dart';
import 'package:flutter_sound/public/flutter_sound_player.dart' as player;
import 'package:flutter_sound_platform_interface/flutter_sound_platform_interface.dart'
    as code;
import 'package:uuid/uuid.dart';

import '../models/record_entity.dart';
import '../models/chat_mode.dart';
import '../services/unified_chat_manager.dart';
import '../services/base_llm.dart';
import '../services/objectbox_service.dart';

class ChatController extends ChangeNotifier {
  // 使用统一LLM架构替代传统ChatManager
  late final UnifiedChatManager unifiedChatManager;
  late final UnifiedChatManager unifiedChatHelp;
  final String helpMessage =
      'Based on historical information, I think the following may help you:\n\n';
  final ObjectBoxService _objectBoxService = ObjectBoxService();
  final List<Map<String, dynamic>> historyMessages = [];
  final List<Map<String, dynamic>> newMessages = [];
  final TextEditingController textController = TextEditingController();
  final Function onNewMessage;
  final ScrollController scrollController = ScrollController();
  Map<String, String?> userToResponseMap = {};

  final ValueNotifier<Set<String>> unReadMessageId = ValueNotifier({});

  static const int _pageSize = 25;
  bool isLoading = false;
  bool hasMoreMessages = true;
  bool bleConnection = false;

  // 添加ASR流消息处理的字段
  Map<String, String> asrStreamingMessages = {}; // messageId -> current text
  Map<String, int> asrMessageIndices =
      {}; // messageId -> message index in newMessages

  static final Set<ChatController> _instances = <ChatController>{};

  final player.FlutterSoundPlayer _audioPlayer = player.FlutterSoundPlayer();

  ChatController({required this.onNewMessage}) {
    _instances.add(this);
    _initialize();
  }

  @override
  void dispose() {
    _instances.remove(this);
    super.dispose();
    textController.dispose();
    scrollController.dispose();
    // 清理ASR流状态
    asrStreamingMessages.clear();
    asrMessageIndices.clear();
    _audioPlayer.closePlayer();
    FlutterForegroundTask.removeTaskDataCallback(_onReceiveTaskData);
  }

  static Future<void> reinitializeAllLLM() async {
    for (final controller in _instances) {
      await controller.reinitializeLLM();
    }
  }

  /// Determine current chat mode based on data
  ChatMode _getChatMode(Map<String, dynamic> data) {
    final inDialogMode = data['inDialogMode'] as bool?;
    final isMeeting = data['isMeeting'] as bool?;

    ChatMode mode;
    if (isMeeting == true) {
      mode = ChatMode.meetingMode;
    } else if (inDialogMode == true) {
      mode = ChatMode.dialogMode;
    } else {
      mode = ChatMode.defaultMode;
    }

    return mode;
  }

  /// Unified handling of regular messages (non-ASR stream messages)
  void _handleRegularMessage(
    String text,
    String? speaker,
    bool isEndpoint,
    ChatMode chatMode,
    bool hasAsrMessageId,
  ) {
    if (!isEndpoint || !chatMode.shouldInsertMessage(hasAsrMessageId)) {
      return;
    }
    isSpeakValueNotifier.value = false;

    final messageData = {
      'id': const Uuid().v4(),
      'text': text,
      'isUser': speaker ?? 'unknown',
    };

    insertNewMessage(messageData);

    // In dialog mode, need to set AI response mapping
    if (chatMode == ChatMode.dialogMode && speaker == 'user') {
      userToResponseMap[messageData['id'] as String] = null;
    }
  }

  Future<void> _initialize() async {
    // 初始化统一ChatManager
    unifiedChatManager = UnifiedChatManager();
    unifiedChatHelp = UnifiedChatManager();
    await unifiedChatManager.init(
      systemPrompt: '$systemPromptOfChat\n\n${systemPromptOfScenario['text']}',
    );
    await unifiedChatHelp.init(systemPrompt: systemPromptOfHelp);

    await loadMoreMessages(reset: true);
    // 初始化音频播放器
    if (Platform.isAndroid) {
      await _audioPlayer.openPlayer(isBGService: true);
    } else {
      await _audioPlayer.openPlayer();
    }
    FlutterForegroundTask.addTaskDataCallback(_onReceiveTaskData);
  }

  Future<void> reinitializeLLM() async {
    await unifiedChatManager.reinitializeLLM();
    await unifiedChatHelp.reinitializeLLM();
  }

  Future<void> loadMoreMessages({bool reset = false}) async {
    if (isLoading) return;

    isLoading = true;

    if (reset) {
      historyMessages.clear();
      newMessages.clear();
      hasMoreMessages = true;
      // Clean up ASR stream state
      _cleanupAsrStreams();
      unifiedChatManager.updateChatHistory();
      unifiedChatHelp.updateChatHistory();
    }

    List<RecordEntity>? records = _objectBoxService.getChatRecords(
      offset: historyMessages.length + newMessages.length,
      limit: _pageSize,
    );

    if (records != null && records.isNotEmpty) {
      List<Map<String, dynamic>> loadMessages = records.map((record) {
        return {
          'id': Uuid().v4(),
          'text': record.content,
          'isUser': record.role,
        };
      }).toList();
      if (newMessages.isEmpty) {
        newMessages.insertAll(0, loadMessages.toList());
        tryNotifyListeners();
        firstScrollToBottom();
      } else {
        historyMessages.insertAll(0, loadMessages.reversed.toList());
      }
      tryNotifyListeners();
    } else {
      hasMoreMessages = false;
    }

    isLoading = false;
  }

  ValueNotifier<bool> isSpeakValueNotifier = ValueNotifier(false);

  void _onReceiveTaskData(Object data) {
    if (data == 'refresh') {
      loadMoreMessages(reset: true);
      return;
    }

    if (data is Map<String, dynamic>) {
      if (data['action'] == 'playAudio') {
        _playAudio(data['data']);
        return;
      }

      if (data['action'] == 'stopAudio') {
        _stopAudio();
        return;
      }

      // Handle ASR stream data (typewriter effect)
      final asrMessageId = data['asrMessageId'] as String?;
      final asrText = data['asrText'] as String?;
      final asrIsStreaming = data['asrIsStreaming'] as bool?;
      final asrIsEndpoint = data['asrIsEndpoint'] as bool?;

      if (asrMessageId != null && asrText != null) {
        _handleAsrStreamData(
          asrMessageId,
          asrText,
          asrIsStreaming ?? false,
          asrIsEndpoint ?? false,
          data,
        );
        return;
      }

      // Handle other types of data
      final text = data['text'] as String?;
      final currentText = data['currentText'] as String?;
      final speaker = data['speaker'] as String?;
      final isEndpoint = data['isEndpoint'] as bool?;
      final isFinished = data['isFinished'] as bool?;
      final delta = data['content'] as String?;
      final isSpeaking = data['isVadDetected'] as bool?;

      if (isSpeaking != null && isSpeaking) {
        isSpeakValueNotifier.value = true;
        if (!bleConnection) {
          FlutterForegroundTask.sendDataToMain({'connectionState': true});
          FlutterForegroundTask.sendDataToTask("InitTTS");
          bleConnection = true;
        }
      } else if (isSpeaking != null && !isSpeaking) {
        isSpeakValueNotifier.value = false;
        if (!bleConnection) {
          FlutterForegroundTask.sendDataToMain({'connectionState': true});
          FlutterForegroundTask.sendDataToTask("InitTTS");
          bleConnection = true;
        }
      }
      final chatMode = _getChatMode(data);
      // Unified handling of regular messages (non-ASR and non-chat assistant streams)
      if (isEndpoint != null && text != null) {
        final hasAsrMessageId = data['asrMessageId'] != null;

        _handleRegularMessage(
          text,
          speaker,
          isEndpoint,
          chatMode,
          hasAsrMessageId,
        );
      }

      if (isFinished != null && delta != null) {
        int userIndex = newMessages.indexWhere(
          (msg) => msg['text'] == currentText && msg['isUser'] == 'user',
        );

        if (userIndex != -1) {
          String? responseId = userToResponseMap[newMessages[userIndex]['id']];
          bool isInBottom = checkInBottom();

          if (responseId == null) {
            responseId = const Uuid().v4();
            userToResponseMap[newMessages[userIndex]['id']] = responseId;
            newMessages.insert(0, {
              'id': responseId,
              'text': '',
              'isUser': 'assistant',
            });
          }

          int botIndex = newMessages.indexWhere(
            (msg) => msg['id'] == responseId,
          );
          if (botIndex != -1) {
            newMessages[botIndex]['text'] += delta;
            tryNotifyListeners();

            if (isInBottom) {
              firstScrollToBottom();
            }
            if (isFinished) {
              newMessages[botIndex]['text'] = newMessages[botIndex]['text']
                  .trim();
              userToResponseMap.remove(newMessages[userIndex]['id']);
            }
          }
        }
      }
    }
  }

  // 添加处理ASR流数据的方法
  void _handleAsrStreamData(
    String asrMessageId,
    String asrText,
    bool isStreaming,
    bool isEndpoint,
    Map<String, dynamic> data,
  ) {
    final speaker = data['speaker'] as String?;

    bool isInBottom = checkInBottom();

    if (isStreaming && !isEndpoint) {
      // 处理流式数据（打字机效果）
      if (!asrMessageIndices.containsKey(asrMessageId)) {
        // 创建新的ASR流消息
        final messageData = {
          'id': asrMessageId,
          'text': asrText,
          'isUser': speaker,
          'isAsrStreaming': true,
        };

        newMessages.insert(0, messageData);
        asrMessageIndices[asrMessageId] = 0;
        asrStreamingMessages[asrMessageId] = asrText;

        if (!isInBottom) {
          unReadMessageId.value.add(asrMessageId);
          unReadMessageId.notifyListeners();
        }

        tryNotifyListeners();
        if (isInBottom) {
          firstScrollToBottom();
        }
      } else {
        // 更新现有的ASR流消息
        final messageIndex = asrMessageIndices[asrMessageId]!;
        if (messageIndex < newMessages.length &&
            newMessages[messageIndex]['id'] == asrMessageId) {
          newMessages[messageIndex]['text'] = asrText;
          asrStreamingMessages[asrMessageId] = asrText;

          tryNotifyListeners();
          if (isInBottom) {
            firstScrollToBottom();
          }
        }
      }
    } else if (isEndpoint) {
      // 处理最终结果
      if (asrMessageIndices.containsKey(asrMessageId)) {
        final messageIndex = asrMessageIndices[asrMessageId]!;
        if (messageIndex < newMessages.length &&
            newMessages[messageIndex]['id'] == asrMessageId) {
          newMessages[messageIndex]['text'] = asrText;
          newMessages[messageIndex]['isAsrStreaming'] = false;

          // 清理ASR流状态
          asrStreamingMessages.remove(asrMessageId);
          asrMessageIndices.remove(asrMessageId);

          tryNotifyListeners();
          if (isInBottom) {
            firstScrollToBottom();
          }
        }
      } else {
        // 直接插入最终消息（如果没有流式过程）
        insertNewMessage({
          'id': asrMessageId,
          'text': asrText,
          'isUser': speaker,
        });
      }
    }
  }

  // 添加清理ASR流状态的方法
  void _cleanupAsrStreams() {
    asrStreamingMessages.clear();
    asrMessageIndices.clear();
  }

  tryNotifyListeners() {
    onNewMessage();
    if (hasListeners) {
      notifyListeners();
    }
  }

  Future<void> sendMessage({String? initialText}) async {
    String text = initialText ?? textController.text;

    if (text.isNotEmpty) {
      textController.clear();

      insertNewMessage({
        'id': const Uuid().v4(),
        'text': text,
        'isUser': 'user',
      });
      _objectBoxService.insertDialogueRecord(
        RecordEntity(role: 'user', content: text),
      );
      firstScrollToBottom();

      unifiedChatManager.addChatSession('user', text);
      await _getBotResponse(text);
    }
  }

  Future<void> askHelp() async {
    String text = "Please help me";

    unifiedChatHelp.updateChatHistory();

    if (text.isNotEmpty) {
      textController.clear();

      insertNewMessage({
        'id': const Uuid().v4(),
        'text': 'Help me Buddie',
        'isUser': 'user',
      });
      _objectBoxService.insertDialogueRecord(
        RecordEntity(role: 'user', content: 'Help me Buddie'),
      );
      firstScrollToBottom();

      unifiedChatManager.addChatSession('user', 'Help me Buddie');
      await _getBotResponse(text, isHelp: true);
    }
  }

  Future<void> _getBotResponse(String userInput, {bool isHelp = false}) async {
    try {
      tryNotifyListeners();

      String? responseId;
      final chatResponse = isHelp ? unifiedChatHelp : unifiedChatManager;
      final response = await chatResponse.createRequest(text: userInput);
      bool isInBottom = checkInBottom();

      if (responseId == null) {
        responseId = const Uuid().v4();
        if (isHelp) {
          newMessages.insert(0, {
            'id': responseId,
            'text': helpMessage,
            'isUser': 'assistant',
          });
        } else {
          newMessages.insert(0, {
            'id': responseId,
            'text': '',
            'isUser': 'assistant',
          });
        }
      }

      // 更新消息文本（统一LLM返回的是完整响应）
      updateMessageText(responseId, response, isFinal: true, isHelp: isHelp);

      if (isInBottom) {
        firstScrollToBottom();
      }
      // 保存到数据库
      if (responseId != null) {
        final finalText =
            newMessages.firstWhere((msg) => msg['id'] == responseId)['text']
                as String;
        final cleanText = isHelp
            ? finalText.replaceFirst(helpMessage, '')
            : finalText;

        _objectBoxService.insertDialogueRecord(
          RecordEntity(role: 'assistant', content: cleanText),
        );
        chatResponse.addChatSession('assistant', cleanText);
      }
    } catch (e) {
      String errorInfo = e.toString();

      newMessages.insert(0, {
        'id': Uuid().v4(),
        'text': 'Error: $errorInfo',
        'isUser': 'assistant',
      });

      tryNotifyListeners();
    }
  }

  void updateMessageText(
    String messageId,
    String text, {
    bool isFinal = false,
    bool isHelp = false,
  }) {
    int index = newMessages.indexWhere((msg) => msg['id'] == messageId);
    if (index != -1) {
      if (!isFinal) {
        newMessages[index]['text'] += text;
      } else {
        newMessages[index]['text'] = isHelp ? helpMessage + text : text;
      }
      tryNotifyListeners();
    }
  }

  void insertNewMessage(Map<String, dynamic> data) {
    bool isInBottom = checkInBottom();
    if (!isInBottom) {
      unReadMessageId.value.add(data['id']);
    }
    newMessages.insert(0, data);
    tryNotifyListeners();
    if (isInBottom) {
      firstScrollToBottom();
    }
  }

  void copyToClipboard(BuildContext context, String text) {
    Clipboard.setData(ClipboardData(text: text));
    ScaffoldMessenger.of(context).showSnackBar(
       SnackBar(
        content: Text('Copied to clipboard!'),
        duration: Duration(milliseconds: 500),
      ),
    );
  }

  bool isInAnimation = false;

  bool checkInBottom() {
    if (!scrollController.hasClients) return true;
    double maxScroll = scrollController.position.maxScrollExtent;
    double currentScroll = scrollController.offset;
    return currentScroll >= maxScroll - 20;
  }

  firstScrollToBottom({bool isAnimated = true}) {
    WidgetsBinding.instance.addPostFrameCallback((_) async {
      if (!scrollController.hasClients) return;
      if (isInAnimation) return;
      isInAnimation = true;
      double maxScroll = scrollController.position.maxScrollExtent;
      double currentScroll = scrollController.offset;
      while (currentScroll < maxScroll) {
        if (isAnimated) {
          // Perform the animated scroll only on the first call
          await scrollController.animateTo(
            maxScroll,
            duration: const Duration(milliseconds: 100),
            curve: Curves.linear,
          );
          await Future.delayed(const Duration(milliseconds: 10));
        } else {
          // Perform an immediate jump to the bottom on subsequent recursive calls
          scrollController.jumpTo(maxScroll);
        }
        maxScroll = scrollController.position.maxScrollExtent;
        currentScroll = scrollController.offset;
      }
      isInAnimation = false;
    });
  }

  /// Get current LLM type
  LLMType? getCurrentLLMType() {
    return unifiedChatManager.getCurrentLLMType();
  }

  /// Check if audio input is supported
  bool get supportsAudioInput => unifiedChatManager.supportsAudioInput;

  /// Get list of available LLM types
  Future<List<LLMType>> getAvailableLLMTypes() {
    return unifiedChatManager.getAvailableLLMTypes();
  }

  /// Switch to specified LLM type
  Future<void> switchToLLM(LLMType type) async {
    await unifiedChatManager.switchToLLM(type);
    await unifiedChatHelp.switchToLLM(type);
    notifyListeners();
  }

  /// Play audio data
  Future<void> _playAudio(Uint8List audioData) async {
    try {
      await _audioPlayer.stopPlayer();

      // 尝试直接播放PCM数据 (QwenOmni返回原始PCM数据)
      await _audioPlayer.startPlayer(
        codec: code.Codec.pcm16,
        fromDataBuffer: audioData,
        sampleRate: 24000, // 24kHz采样率
        numChannels: 1, // 单声道
      );
    } catch (e) {
      // 备选方案1：尝试WAV格式
      try {
        await _audioPlayer.stopPlayer();
        await _audioPlayer.startPlayer(
          codec: code.Codec.pcm16WAV,
          fromDataBuffer: audioData,
        );
      } catch (e2) {
        // 备选方案2：尝试AAC格式
        try {
          await _audioPlayer.stopPlayer();
          await _audioPlayer.startPlayer(
            codec: code.Codec.aacMP4,
            fromDataBuffer: audioData,
          );
        } catch (e3) {
          // 备选方案3：尝试默认编码
          try {
            await _audioPlayer.stopPlayer();
            await _audioPlayer.startPlayer(fromDataBuffer: audioData);
          } catch (e4) {
            dev.log('QwenOmni音频播放失败: $e');
            throw Exception('无法播放音频');
          }
        }
      }
    }
  }

  /// Stop audio playback
  Future<void> _stopAudio() async {
    await _audioPlayer.stopPlayer();
  }
}
