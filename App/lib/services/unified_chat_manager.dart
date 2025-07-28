/// 统一聊天服务管理器
///
/// 提供跨LLM类型的统一聊天接口，核心功能包括：
/// 1. 支持多种类型LLM：
///   - 用户自定义配置API
///   - 提供QwenOmni支持文本/音频混合输入
///   - 自动会话历史维护
///   - 流式/非流式响应处理
/// 2. 会话状态管理：
///   - 聊天记录持久化
///   - 历史记录加载/过滤
///   - 工作状态跟踪
///
/// 主要工作流程：
/// 1. 通过LLMFactory获取当前LLM实例
/// 2. 构建包含上下文的输入内容
/// 3. 处理LLM响应（流式/非流式）
/// 4. 维护聊天会话状态
///
/// 使用示例：
/// ```dart
/// // 初始化管理器
/// final manager = UnifiedChatManager();
/// await manager.init(systemPrompt: "你是一个专业助手");
///
/// // 文本对话
/// final stream = manager.createStreamingRequest(text: "你好");
/// stream.listen((response) => print(response));
///
/// // 音频对话
/// if (manager.supportsAudioInput) {
///   final audioStream = manager.createStreamingRequestWithAudio(
///     audioData: audioBytes,
///   );
/// }
/// ```
///
/// 设计特点：
/// - 与LLMFactory松耦合，可动态切换底层LLM
/// - 自动处理不完整的JSON响应
/// - 内置对话历史管理

import 'dart:async';
import 'dart:convert';
import 'dart:developer' as dev;
import 'dart:typed_data';
import 'package:intl/intl.dart';
import '../models/chat_session.dart';
import '../models/record_entity.dart';
import '../services/objectbox_service.dart';
import 'base_llm.dart';
import 'llm_factory.dart';

/// 统一的聊天管理器 - 使用工厂模式管理所有LLM类型
class UnifiedChatManager {
  final ChatSession chatSession = ChatSession();

  BaseLLM? _currentLLM;
  String? _currentSystemPrompt;

  UnifiedChatManager();

  /// 初始化聊天管理器
  Future<void> init({String? systemPrompt}) async {
    _currentSystemPrompt = systemPrompt;
    _currentLLM = await LLMFactory.instance.getCurrentLLM(
      systemPrompt: systemPrompt,
    );

    // 加载历史记录
    _loadChatHistory();
  }

  /// 重新初始化LLM（配置更改后）
  Future<void> reinitializeLLM() async {
    await LLMFactory.instance.reinitializeLLM();
    _currentLLM = await LLMFactory.instance.getCurrentLLM(
      systemPrompt: _currentSystemPrompt,
    );
  }

  /// 文本流式请求
  Stream<String> createStreamingRequest({required String text}) async* {
    await _ensureLLMInitialized();

    RegExp pattern = RegExp(r'[，。！？；：]|[,.!?;:](?=\s)');

    var lastIndex = 0;
    var content = "";
    var jsonObj = {};

    var messages = [
      {"role": "user", "content": buildInput(text)},
    ];

    final responseStream = _currentLLM!.createStreamingRequest(
      messages: messages,
    );


    await for (var chunk in responseStream) {
      var jsonString = chunk;
      if (chunk.startsWith('{"content":')) {
        try {
          final jsonString = completeJsonIfIncomplete(chunk);
          jsonObj = jsonDecode(jsonString);

          if (jsonObj.containsKey('content')) {
            content = jsonObj['content'];

            Iterable<RegExpMatch> matches = pattern.allMatches(content);
            if (matches.isNotEmpty && matches.last.start + 1 > lastIndex) {
              final match = matches.last;
              final matchedText = match.group(0);

              final delta = content.substring(
                lastIndex,
                matches.last.start + 1,
              );
              lastIndex = matches.last.start + 1;
              yield jsonEncode({
                "content": content,
                "delta": delta,
                "isFinished": false,
                "isEnd": jsonObj['isEnd'] ?? false,
              });
            }
          }
        } catch (e) {
          print(
            "JSON string is incomplete, continue accumulating: $jsonString  ||| $e",
          );
        }
      } else {
        content = chunk;
        Iterable<RegExpMatch> matches = pattern.allMatches(content);
        if (matches.isNotEmpty && matches.last.start + 1 > lastIndex) {
          final match = matches.last;

          final delta = content.substring(lastIndex, matches.last.start + 1);
          lastIndex = matches.last.start + 1;
          yield jsonEncode({
            "content": content,
            "delta": delta,
            "isFinished": false,
            "isEnd": jsonObj['isEnd'] ?? false,
          });
        }
      }
    }

    final remainingText = content.substring(lastIndex);
    yield jsonEncode({
      "content": content,
      "delta": remainingText,
      "isFinished": true,
      "isEnd": jsonObj['isEnd'] ?? false,
    });
  }

  /// 带音频的流式请求（仅QwenOmni支持）
  Stream<String> createStreamingRequestWithAudio({
    Uint8List? audioData,
    String? userMessage,
  }) async* {
    await _ensureLLMInitialized();

    // 检查当前LLM是否支持音频
    if (!LLMFactory.instance.supportsAudioInput) {
      throw UnsupportedError('Current LLM does not support audio input');
    }

    final conversationHistory = getChatSession();
    final responseStream = _currentLLM!.createStreamingRequestWithAudio(
      audioData: audioData,
      conversationHistory: conversationHistory,
      userMessage: userMessage,
    );
    String content = '';
    await for (var chunk in responseStream) {
      dev.log(chunk);
      String p = chunk.substring(content.length);
      content = chunk;
      yield jsonEncode({'delta': p, 'isFinished': false, 'content': content});
    }

    yield jsonEncode({'content': content, 'isFinished': true});
  }

  /// 非流式文本请求
  Future<String> createRequest({required String text}) async {
    await _ensureLLMInitialized();

    final content = buildInput(text);
    return _currentLLM!.createRequest(content: content);
  }

  /// 构建输入内容（包含聊天历史）
  String buildInput(String userInput) {
    final session = loadChatSession();
    DateTime now = DateTime.now();
    var input =
        """
Timestamp: ${now.toIso8601String().split('.').first}\n
Chat Session: \n$session
---\n
User Input: $userInput""";
    return input;
  }

  /// 获取聊天会话
  List<Chat> getChatSession() {
    return chatSession.chatHistory.items;
  }

  /// 加载聊天会话字符串
  String loadChatSession() {
    final chats = getChatSession();
    if (chats.isEmpty) return '';

    StringBuffer buffer = StringBuffer();
    for (var chat in chats) {
      buffer.writeln('${chat.time} - ${chat.role}: ${chat.txt}');
    }
    return buffer.toString();
  }

  /// 添加聊天记录
  void addChatSession(String role, String content, {String? time}) {
    final timestamp =
        time ?? DateFormat('yyyy-MM-dd HH:mm').format(DateTime.now());
    final chat = Chat(role: role, txt: content, time: timestamp);
    chatSession.chatHistory.add(chat);
  }

  /// 更新工作状态
  void updateWorkingState(String state) {
    chatSession.workingState = state;
  }

  /// 清空聊天历史
  void clearChatHistory() {
    chatSession.chatHistory.clear();
  }

  /// 根据时间删除聊天记录
  void removeChatByTime(int time) {
    chatSession.chatHistory.removeWhere((chat) => chat.time == time.toString());
  }

  /// 更新聊天历史（用于搜索等场景）
  void updateChatHistory([List<Chat>? filteredChats]) {
    chatSession.chatHistory.clear();

    if (filteredChats != null && filteredChats.isNotEmpty) {
      for (var chat in filteredChats) {
        chatSession.chatHistory.add(chat);
      }
    } else {
      _loadChatHistory();
    }
  }

  /// 根据角色过滤聊天记录
  void filterChatsByRole(String role) {
    chatSession.chatHistory = LimitedQueue<Chat>(
      chatSession.chatHistory.maxLength,
    )..addAll(chatSession.chatHistory.items.where((chat) => chat.role == role));
  }

  /// 完善不完整的JSON字符串
  String completeJsonIfIncomplete(String jsonString) {
    int braceCount = 0;
    int i = 0;

    // 计算左大括号和右大括号的差值
    for (i = 0; i < jsonString.length; i++) {
      if (jsonString[i] == '{') {
        braceCount++;
      } else if (jsonString[i] == '}') {
        braceCount--;
      }
    }

    // 如果左大括号多于右大括号，补充右大括号
    if (braceCount > 0) {
      for (int j = 0; j < braceCount; j++) {
        jsonString += '}';
      }
    }

    return jsonString;
  }

  /// 获取当前LLM类型
  LLMType? getCurrentLLMType() {
    return LLMFactory.instance.currentType;
  }

  /// 检查是否支持音频输入
  bool get supportsAudioInput => LLMFactory.instance.supportsAudioInput;

  /// 获取可用的LLM类型列表
  Future<List<LLMType>> getAvailableLLMTypes() {
    return LLMFactory.getAvailableLLMTypes();
  }

  /// 切换到指定的LLM类型
  Future<void> switchToLLM(LLMType type) async {
    _currentLLM = await LLMFactory.instance.getLLM(
      type,
      systemPrompt: _currentSystemPrompt,
    );
  }

  /// 确保LLM已初始化
  Future<void> _ensureLLMInitialized() async {
    if (_currentLLM == null) {
      _currentLLM = await LLMFactory.instance.getCurrentLLM(
        systemPrompt: _currentSystemPrompt,
      );
    }
  }

  /// 加载聊天历史
  void _loadChatHistory() {
    List<RecordEntity>? recentRecords = ObjectBoxService().getTermRecords();
    recentRecords?.forEach((RecordEntity recordEntity) {
      String formattedTime = DateFormat(
        'yyyy-MM-dd HH:mm',
      ).format(DateTime.fromMillisecondsSinceEpoch(recordEntity.createdAt!));
      addChatSession(
        recordEntity.role!,
        recordEntity.content!,
        time: formattedTime,
      );
    });
  }

  /// 释放资源
  Future<void> dispose() async {
    // LLM的释放由工厂管理，这里不需要手动释放
  }
}
