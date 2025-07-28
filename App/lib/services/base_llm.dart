/// 统一大语言模型(LLM)抽象接口
///
/// 定义与各种LLM服务交互的统一接口，支持多种模型类型和工作模式：
/// 1. 模型类型支持：
///   - 自定义API配置模型 (customLLM)
///   - 多模态模型 (qwenOmni)
/// 2. 交互模式：
///   - 同步文本对话 (非流式)
///   - 异步文本对话 (流式)
///   - 多模态对话 (音频+文本，仅qwenOmni支持)

import 'dart:async';
import 'dart:typed_data';
import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';

import '../generated/l10n.dart';
import '../models/chat_session.dart';

/// LLM类型枚举
enum LLMType {
  /// 用户自定义LLM - 用户配置的API Key
  customLLM,

  /// QwenOmni多模态LLM - 支持音频输入输出
  qwenOmni,
}

extension LLMTypeExtension on LLMType {
  Color get color {
    switch (this) {
      case LLMType.customLLM:
        return Colors.green;
      case LLMType.qwenOmni:
        return Colors.orange;
    }
  }

  IconData get icon {
    switch (this) {
      case LLMType.customLLM:
        return Icons.person;
      case LLMType.qwenOmni:
        return Icons.multitrack_audio;
    }
  }

  String getLLMDisplayName(BuildContext context) {
    switch (this) {
      case LLMType.customLLM:
        return S.of(context).aiDialogModelCustomLLMDisplayName;
      case LLMType.qwenOmni:
        return S.of(context).aiDialogModelQwenOmniDisplayName;
      // case null:
      //   return S.current.pageSettingAIModeSetSubtitle3;
    }
  }

  String getLLMDescription(BuildContext context) {
    switch (this) {
      case LLMType.customLLM:
        return S.of(context).aiDialogModelCustomLLMDescription;
      case LLMType.qwenOmni:
        return S.of(context).aiDialogModelQwenOmniDescription;
    }
  }
}

/// LLM请求配置
class LLMConfig {
  final String? model;
  final int retryCount;
  final Object? jsonSchema;
  final Map<String, dynamic>? extraParams;

  const LLMConfig({this.model, this.retryCount = 3, this.jsonSchema, this.extraParams});
}

/// 统一的LLM抽象基类
abstract class BaseLLM {
  /// LLM类型
  LLMType get type;

  /// 是否可用（API Key等配置是否正确）
  bool get isAvailable;

  /// 系统提示词
  String get systemPrompt;

  /// 设置系统提示词
  void setSystemPrompt(String prompt);

  /// 初始化LLM
  Future<void> initialize();

  /// 释放资源
  Future<void> dispose();

  /// 文本对话 - 非流式
  Future<String> createRequest({required String content, LLMConfig? config});

  /// 文本对话 - 流式
  Stream<String> createStreamingRequest({String? content, List<Map<String, String>>? messages, LLMConfig? config});

  /// 带音频的对话 - 流式（仅QwenOmni支持，其他LLM抛出异常）
  Stream<String> createStreamingRequestWithAudio({
    required Uint8List? audioData,
    required List<Chat> conversationHistory,
    String? userMessage,
    LLMConfig? config,
  }) {
    throw UnsupportedError('Audio input is not supported by this LLM type');
  }

  /// 重新初始化（用于配置更改后）
  Future<void> reinitialize() async {
    await dispose();
    await initialize();
  }
}
