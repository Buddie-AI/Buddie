/// 自定义LLM实现 - 支持用户配置API端点
///
/// 实现 [BaseLLM] 接口，提供以下特性：
/// 1. 配置灵活性：
///   - 支持用户自定义API Key、URL和模型名称
///   - 提供默认配置回退机制
///   - 配置变更自动重新初始化
/// 2. 请求处理：
///   - 支持标准OpenAI兼容API格式
///   - 自动注入系统提示词
///   - 内置JSON Schema支持
/// 3. 流式响应：
///   - 实时分块处理SSE(Server-Sent Events)流
///
/// 配置优先级：
/// 1. 用户通过SPUtil存储的配置（高优先级）
/// 2. DefaultConfig中的默认配置（低优先级）

import 'dart:async';
import 'dart:convert';
import 'package:http/http.dart' as http;
import '../constants/prompt_constants.dart';
import '../utils/sp_util.dart';
import '../config/default_config.dart';
import 'base_llm.dart';

/// User custom LLM implementation - Use user configured API Key, URL and model
class CustomLLM extends BaseLLM {
  late String _apiKey;
  late String _baseUrl;
  late String _model;
  String _systemPrompt;
  bool _isEnabled = false;

  CustomLLM({String? systemPrompt})
    : _systemPrompt = systemPrompt ?? systemPromptOfChat;

  @override
  LLMType get type => LLMType.customLLM;

  @override
  bool get isAvailable =>
      _isEnabled && _apiKey.isNotEmpty && _baseUrl.isNotEmpty;

  @override
  String get systemPrompt => _systemPrompt;

  @override
  void setSystemPrompt(String prompt) {
    _systemPrompt = prompt;
  }

  @override
  Future<void> initialize() async {
    // 加载用户自定义配置
    final userLlmToken = await SPUtil.getString('llmToken') ?? '';
    final userLlmUrl = await SPUtil.getString('llmUrl') ?? '';
    final userLlmModel = await SPUtil.getString('llmModel') ?? '';

    // 使用用户配置，如果没有则使用默认配置
    final llmToken = userLlmToken.isNotEmpty
        ? userLlmToken
        : DefaultConfig.defaultLlmToken;
    final llmUrl = userLlmUrl.isNotEmpty
        ? userLlmUrl
        : DefaultConfig.defaultLlmUrl;
    final llmModel = userLlmModel.isNotEmpty
        ? userLlmModel
        : DefaultConfig.defaultLlmModel;

    // 检查最终配置是否有效
    if (llmToken.isNotEmpty && llmUrl.isNotEmpty && llmModel.isNotEmpty) {
      _isEnabled = true;
      _apiKey = 'Bearer $llmToken';
      _baseUrl = llmUrl;
      _model = llmModel;
    } else {
      _isEnabled = false;
      _apiKey = '';
      _baseUrl = '';
      _model = '';
    }
  }

  @override
  Future<void> dispose() async {
    // 用户自定义LLM无需特殊清理
  }

  @override
  Future<String> createRequest({
    required String content,
    LLMConfig? config,
  }) async {
    if (!isAvailable) {
      throw Exception('Custom LLM is not properly configured');
    }

    final url = Uri.parse(_baseUrl);
    final headers = {
      'Content-Type': 'application/json',
      'Authorization': _apiKey,
    };

    final body = jsonEncode({
      'model': config?.model ?? _model,
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
      throw Exception('Custom LLM is not properly configured');
    }

    final retryCount = config?.retryCount ?? 3;

    for (int i = 0; i < retryCount; i++) {
      try {
        final url = Uri.parse(_baseUrl);
        final headers = {
          'Content-Type': 'application/json',
          'Authorization': _apiKey,
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

        final Map<String, Object> responseFormat = {'type': 'json_object'};
        if (config?.jsonSchema != null) {
          responseFormat['json_schema'] = config!.jsonSchema!;
        }

        final body = {
          'model': config?.model ?? _model,
          'messages': messages,
          'stream': true,
        };

        // 添加用户自定义的额外参数
        if (config?.extraParams != null) {
          config!.extraParams!.forEach((key, value) {
            body[key] = value;
          });
        }

        yield* _handleStreamingResponse(url, headers, jsonEncode(body));
        break;
      } catch (e) {
        if (i == retryCount - 1) rethrow;
        await Future.delayed(const Duration(milliseconds: 300));
      }
    }
  }

  /// Handle non-streaming response
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
        'Failed to fetch response from Custom LLM: ${response.statusCode} - ${response.body}',
      );
    }
  }

  /// Handle streaming response
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
        'Failed to fetch streaming response from Custom LLM: ${response.statusCode}',
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

          // 处理结束标记
          if (part.trim() == '[DONE]') {
            break;
          }

          if (part.startsWith('ERROR:')) {
            buffer.write(
              jsonEncode({'content': part.replaceFirst("ERROR:", "").trim()}),
            );
            yield buffer.toString();
            continue;
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
        print('CustomLLM streaming error: $e');
      }
    }
  }

  /// Check if configuration is complete (user or default)
  static Future<bool> isConfigured() async {
    // 检查用户配置
    final userLlmToken = await SPUtil.getString('llmToken') ?? '';
    final userLlmUrl = await SPUtil.getString('llmUrl') ?? '';
    final userLlmModel = await SPUtil.getString('llmModel') ?? '';

    // 如果用户有完整配置，直接返回true
    if (userLlmToken.isNotEmpty &&
        userLlmUrl.isNotEmpty &&
        userLlmModel.isNotEmpty) {
      return true;
    }

    // 检查是否有默认配置可以填补缺失的部分
    final finalToken = userLlmToken.isNotEmpty
        ? userLlmToken
        : DefaultConfig.defaultLlmToken;
    final finalUrl = userLlmUrl.isNotEmpty
        ? userLlmUrl
        : DefaultConfig.defaultLlmUrl;
    final finalModel = userLlmModel.isNotEmpty
        ? userLlmModel
        : DefaultConfig.defaultLlmModel;

    return finalToken.isNotEmpty &&
        finalUrl.isNotEmpty &&
        finalModel.isNotEmpty;
  }
}
