/// 通义千问Omni多模态LLM实现
///
/// 基于阿里云DashScope API实现，提供以下特色功能：
/// 1. 多模态支持：
///   - 音频输入输出（WAV格式）
/// 2. 交互模式：
///   - 混合对话（文本+音频）
///   - 上下文感知的连续对话
///   - 实时流式响应处理
/// 3. 技术特性：
///   - 兼容OpenAI API格式
///   - 自动Base64音频编解码
///   - 音频流实时播放
///   - 自动会话历史管理
///
/// 配置要求：
/// - 阿里云API Key（用户配置或默认配置）
/// - 音频数据格式：16bit WAV PCM

import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';
import 'package:flutter_foreground_task/flutter_foreground_task.dart';
import 'package:http/http.dart' as http;
import '../constants/prompt_constants.dart';
import '../models/chat_session.dart';
import '../utils/sp_util.dart';
import '../config/default_config.dart';
import 'base_llm.dart';

/// QwenOmni多模态LLM实现 - 支持音频输入输出和流式响应
class QwenOmniLLM extends BaseLLM {
  static const String _baseUrl =
      'https://dashscope.aliyuncs.com/compatible-mode/v1';
  static const String _model = 'qwen-omni-turbo';

  String? _apiKey;
  String _systemPrompt;

  QwenOmniLLM({String? systemPrompt})
    : _systemPrompt = systemPrompt ?? systemPromptOfChat;

  @override
  LLMType get type => LLMType.qwenOmni;

  @override
  bool get isAvailable => _apiKey != null && _apiKey!.trim().isNotEmpty;

  @override
  String get systemPrompt => _systemPrompt;

  @override
  void setSystemPrompt(String prompt) {
    _systemPrompt = prompt;
  }

  @override
  Future<void> initialize() async {
    // 加载用户配置的阿里云API Key
    final userApiKey = await SPUtil.getString('alibaba_api_key') ?? '';

    // 使用用户配置，如果没有则使用默认配置
    _apiKey = userApiKey.isNotEmpty
        ? userApiKey
        : DefaultConfig.defaultAlibabaApiKey;
  }

  @override
  Future<void> dispose() async {}

  @override
  Future<String> createRequest({
    required String content,
    LLMConfig? config,
  }) async {
    if (!isAvailable) {
      throw Exception('QwenOmni API Key not configured');
    }

    final url = Uri.parse('$_baseUrl/chat/completions');
    final headers = {
      'Content-Type': 'application/json',
      'Authorization': 'Bearer $_apiKey',
    };

    final body = jsonEncode({
      'model': _model,
      'messages': [
        {"role": "system", "content": _systemPrompt},
        {"role": "user", "content": content},
      ],
    });

    final response = await http.post(url, headers: headers, body: body);
    return _handleResponse(response);
  }

  @override
  Stream<String> createStreamingRequest({
    String? content,
    List<Map<String, String>>? messages,
    LLMConfig? config,
  }) async* {
    if (!isAvailable) {
      throw Exception('QwenOmni API Key not configured');
    }

    final retryCount = config?.retryCount ?? 3;

    for (int i = 0; i < retryCount; i++) {
      try {
        final url = Uri.parse('$_baseUrl/chat/completions');
        final headers = {
          'Content-Type': 'application/json',
          'Authorization': 'Bearer $_apiKey',
        };

        if (messages != null && messages.isNotEmpty) {
          if (messages[0]["role"] != "system") {
            messages.insert(0, {"role": "system", "content": _systemPrompt});
          }
        } else {
          messages = [
            {"role": "system", "content": _systemPrompt},
            {"role": "user", "content": content!},
          ];
        }

        final body = {'model': _model, 'messages': messages, 'stream': true};

        yield* _handleStreamingResponse(url, headers, jsonEncode(body));
        break;
      } catch (e) {
        if (i == retryCount - 1) rethrow;
        await Future.delayed(const Duration(milliseconds: 300));
      }
    }
  }

  @override
  Stream<String> createStreamingRequestWithAudio({
    required Uint8List? audioData,
    required List<Chat> conversationHistory,
    String? userMessage,
    LLMConfig? config,
  }) async* {
    if (!isAvailable) {
      throw Exception('QwenOmni API Key not configured');
    }

    final retryCount = config?.retryCount ?? 3;

    for (int i = 0; i < retryCount; i++) {
      try {
        final url = Uri.parse('$_baseUrl/chat/completions');
        final headers = {
          'Content-Type': 'application/json',
          'Authorization': 'Bearer $_apiKey',
        };

        // 构建包含历史对话的消息列表
        final messages = _buildMessagesWithAudio(
          audioData,
          conversationHistory,
          userMessage,
        );

        final body = {
          'model': _model,
          'messages': messages,
          'stream': true,
          'stream_options': {'include_usage': true},
          'modalities': ['text', 'audio'],
          'audio': {'voice': 'Cherry', 'format': 'wav'},
        };

        yield* _processStreamResponseRealtime(url, headers, jsonEncode(body));
        return;
      } catch (e) {
        if (i == retryCount - 1) rethrow;
        await Future.delayed(const Duration(milliseconds: 300));
      }
    }
  }

  /// 构建包含音频的消息列表
  List<Map<String, dynamic>> _buildMessagesWithAudio(
    Uint8List? audioData,
    List<Chat> conversationHistory,
    String? userMessage,
  ) {
    final messages = <Map<String, dynamic>>[];

    // 添加系统提示
    messages.add({'role': 'system', 'content': _systemPrompt});

    // 添加历史对话
    for (final chat in conversationHistory) {
      messages.add({
        'role': chat.role == 'user' ? 'user' : 'assistant',
        'content': chat.txt,
      });
    }

    // 将音频转换为base64

    // 添加当前音频输入
    final currentContent = <Map<String, dynamic>>[];

    if (audioData != null) {
      final base64Audio = base64Encode(audioData);
      currentContent.add({
        'type': 'input_audio',
        'input_audio': {
          'data': 'data:audio/wav;base64,$base64Audio',
          'format': 'wav',
        },
      });
      currentContent.add({'type': 'text', 'text': '音频里面就是我要说的话，请你保持正常交流'});
    } else {
      currentContent.add({'type': 'text', 'text': userMessage});
    }

    messages.add({'role': 'user', 'content': currentContent});

    return messages;
  }

  /// 处理非流式响应
  String _handleResponse(http.Response response) {
    if (response.statusCode == 200) {
      final data = jsonDecode(
        utf8.decode(response.bodyBytes),
      )['choices'][0]['message'];
      try {
        return data['content'];
      } catch (e) {
        throw Exception('Json decode failed.');
      }
    } else {
      throw Exception(
        'Failed to fetch response from QwenOmni: ${response.statusCode} - ${response.body}',
      );
    }
  }

  /// 处理纯文本流式响应
  Stream<String> _handleStreamingResponse(
    Uri url,
    Map<String, String> headers,
    String body,
  ) async* {
    final request = http.Request('POST', url);
    request.headers.addAll(headers);
    request.body = body;

    final response = await request.send();
    if (response.statusCode != 200) {
      throw Exception(
        'Failed to fetch streaming response from QwenOmni: ${response.statusCode}',
      );
    }

    final responseStream = response.stream.transform(utf8.decoder);
    StringBuffer buffer = StringBuffer();

    await for (var chunk in responseStream) {
      try {
        List<String> jsonParts = chunk.toString().split('\n');

        for (String part in jsonParts) {
          if (part.startsWith('data:')) {
            part = part.replaceFirst('data:', '');
            part = part.trimLeft();
          }

          if (part.trim() == '[DONE]') {
            break;
          }

          if (part.isNotEmpty) {
            try {
              final json = jsonDecode(part);
              var content = json["choices"]?[0]?["delta"]?["content"];
              if (content != null) {
                buffer.write(content);
                yield buffer.toString();
              }
            } catch (e) {
              // JSON字符串不完整，继续累积
              continue;
            }
          }
        }
      } catch (e) {
        print('QwenOmniLLM streaming error: $e');
      }
    }
  }

  /// 处理包含音频的实时流式响应
  Stream<String> _processStreamResponseRealtime(
    Uri url,
    Map<String, String> headers,
    String body,
  ) async* {
    final request = http.Request('POST', url);
    request.headers.addAll(headers);
    request.body = body;

    final response = await request.send();
    if (response.statusCode != 200) {
      throw Exception('QwenOmni request failed: ${response.statusCode}');
    }

    final responseStream = response.stream.transform(utf8.decoder);
    String combinedText = '';
    String combinedAudioBase64 = '';
    StringBuffer incompleteData = StringBuffer();

    await for (var chunk in responseStream) {
      incompleteData.write(chunk);
      String dataBuffer = incompleteData.toString();

      // 按行分割数据
      List<String> lines = dataBuffer.split('\n');

      // 保留最后一行（可能不完整）
      incompleteData.clear();
      if (lines.isNotEmpty && !dataBuffer.endsWith('\n')) {
        incompleteData.write(lines.removeLast());
      }

      for (String line in lines) {
        if (line.startsWith('data:')) {
          final jsonStr = line.substring(5).trim();
          if (jsonStr == '[DONE]') {
            // 处理完成，播放累积的音频
            if (combinedAudioBase64.isNotEmpty) {
              try {
                final audioData = base64Decode(combinedAudioBase64);
                _playAudio(audioData);
              } catch (e) {
                print('Error decoding/playing audio: $e');
              }
            }
            return;
          }

          try {
            final data = jsonDecode(jsonStr);
            if (data['choices'] != null && data['choices'].isNotEmpty) {
              final delta = data['choices'][0]['delta'];

              // 提取文本内容
              if (delta['content'] != null) {
                combinedText += delta['content'];
                yield combinedText;
              }

              // 提取音频转录内容
              if (delta['audio'] != null &&
                  delta['audio']['transcript'] != null) {
                combinedText += delta['audio']['transcript'];
                yield combinedText;
              }

              // 累积音频数据（不返回给调用者）
              if (delta['audio'] != null && delta['audio']['data'] != null) {
                combinedAudioBase64 += delta['audio']['data'];
              }
            }
          } catch (e) {
            print('解析QwenOmni流块出错: $e');
            continue;
          }
        }
      }
    }
  }

  /// 播放音频数据
  Future<void> _playAudio(Uint8List audioData) async {
    FlutterForegroundTask.sendDataToMain({
      'action': 'playAudio',
      'data': audioData,
    });
  }

  /// 停止音频播放
  Future<void> stopAudio() async {
    FlutterForegroundTask.sendDataToMain({'action': 'stopAudio'});
  }
}
