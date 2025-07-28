/// 对话摘要生成模块
///
/// 本模块提供对话摘要的生成能力，支持会议场景和普通对话场景。核心功能包括：
/// 1. 自动摘要生成：基于聊天记录生成结构化摘要
/// 2. 智能分块处理：支持长对话的分块并行处理
/// 3. 重试机制：关键操作内置重试逻辑增强鲁棒性
/// 4. 会议校验：自动检测无效短会议并跳过处理
///
/// 主要工作流程：
/// 1. 校验输入参数（会议需检查时长）
/// 2. 从数据库获取指定时间范围的聊天记录
/// 3. 对长对话进行分块并行处理
/// 4. 合并分块结果生成最终摘要
/// 5. 持久化存储结果并发送通知
///
/// 特殊处理场景：
/// - 会议时长不足1分钟时自动生成提示信息
/// - 支持重试失败的摘要生成操作（默认3次）
/// - 自动处理JSON格式的模型响应
///
/// 依赖服务：
/// - ObjectBoxService: 本地数据库存取
/// - UnifiedChatManager: 大模型交互服务
///
/// 使用示例：
/// ```
/// // 启动会议摘要生成
/// DialogueSummary.start(
///   isMeeting: true,
///   startMeetingTime: startTimestamp,
///   endMeetingTime: endTimestamp,
///   audioPath: 'path/to/recording.mp3'
/// );
/// ```
///
/// 注意事项：
/// 1. 会议摘要需提供起止时间戳，普通对话只需开始时间
/// 2. 摘要生成采用分块策略，单块长度5000字符，重叠区1000字符
/// 3. 并行处理池大小固定为5（需根据设备性能调整）

import 'dart:convert';
import 'package:app/constants/prompt_constants.dart';
import 'package:app/models/record_entity.dart';
import 'package:app/models/summary_entity.dart';
import 'package:app/services/objectbox_service.dart';
import 'package:app/services/unified_chat_manager.dart';
import 'package:flutter/cupertino.dart';
import 'package:intl/intl.dart';
import 'package:pool/pool.dart';

import 'notification.dart';

/// 对异步操作进行重试，直到返回非 null 或达到最大次数
Future<T?> retryAsync<T>(
    Future<T?> Function() action, {
      int maxAttempts = 3,
      Duration delay = const Duration(milliseconds: 500),
    }) async {
  for (var attempt = 1; attempt <= maxAttempts; attempt++) {
    try {
      final result = await action();
      if (result != null) return result;
    } catch (e) {
      debugPrint('Retry #$attempt failed: $e');
    }
    await Future.delayed(delay);
  }
  return null;
}

class DialogueSummary {
  // MainProcess, start the summarization process
  static Future<void> start({
    bool isMeeting = true,
    int? startMeetingTime,
    int? endMeetingTime,
    String? audioPath,
  }) async {
    // 校验并处理会议时长不足场景
    if (isMeeting) {
      final tooShort = _validateMeetingDuration(
        startMeetingTime: startMeetingTime,
        endMeetingTime: endMeetingTime,
        audioPath: audioPath,
      );
      if (tooShort) return;
    }

    // 生成摘要，增加重试机制
    final summary = await retryAsync(
          () => _summarize(
        startTime: startMeetingTime ?? 0,
        isMeeting: isMeeting,
      ),
    );

    if (summary == null) {
      _persistAndNotify(
        SummaryEntity(
          subject: 'Meeting',
          content: '生成摘要失败',
          startTime: startMeetingTime ?? 0,
          endTime: endMeetingTime ?? 0,
          createdAt: DateTime.now().millisecondsSinceEpoch,
          isMeeting: isMeeting,
          audioPath: audioPath,
        ),
        success: false,
      );
      throw Exception('Failed to generate summary');
    }

    // 解析并保存结果
    _handleJsonResult(
      summaryJson: summary,
      isMeeting: isMeeting,
      startTime: startMeetingTime,
      endTime: endMeetingTime,
      audioPath: audioPath,
    );
  }

  /// 校验会议时长，若不足一分钟则直接保存提示
  static bool _validateMeetingDuration({
    required int? startMeetingTime,
    required int? endMeetingTime,
    required String? audioPath,
  }) {
    if (startMeetingTime == null || endMeetingTime == null) return true;
    final duration = endMeetingTime - startMeetingTime;
    if (duration < Duration(minutes: 1).inMilliseconds) {
      final summaryEntity = SummaryEntity(
        subject: 'Meeting',
        content: meetingTooShortHint,
        startTime: startMeetingTime,
        endTime: endMeetingTime,
        createdAt: DateTime.now().millisecondsSinceEpoch,
        isMeeting: true,
        audioPath: audioPath,
      );
      _persistAndNotify(summaryEntity, success: true);
      return true;
    }
    return false;
  }

  /// 构建聊天记录字符串
  static String _buildChatHistory(List<RecordEntity> records) {
    final buffer = StringBuffer();
    final formatter = DateFormat('yyyy-MM-dd HH:mm');
    for (final r in records) {
      final time = formatter.format(
        DateTime.fromMillisecondsSinceEpoch(r.createdAt!),
      );
      buffer.writeln('($time) ${r.role}: ${r.content}');
    }
    return buffer.toString();
  }

  /// 内部摘要生成逻辑
  static Future<String?> _summarize({
    required int startTime,
    bool isMeeting = true
  }) async {
    // 获取记录
    List<RecordEntity>? listRecords = isMeeting
        ? ObjectBoxService().getMeetingRecordsByTimeRange(startTime, DateTime.now().millisecondsSinceEpoch)
        : ObjectBoxService().getRecordsByTimeRange(startTime, DateTime.now().millisecondsSinceEpoch);
    if (listRecords == null) return null;

    // 过滤内容长度
    final chatHistory = _buildChatHistory(listRecords);
    if (chatHistory.length < 1000 && (!isMeeting || chatHistory.length < 10)) {
      return null;
    }

    // 会议摘要流程：分块、并行、合并
    final chunks = _splitChatHistory(chatHistory, 5000, 1000);
    final llm = UnifiedChatManager();
    await llm.init(systemPrompt: systemPromptOfMeetingSummary);
    llm.clearChatHistory();

    final pool = Pool(5);
    final summaries = <String>[];
    for (final chunk in chunks) {
      summaries.add(
        await pool.withResource(() => llm.createRequest(text: chunk)),
      );
    }
    await pool.close();

    final llm2 = UnifiedChatManager();
    await llm2.init(systemPrompt: systemPromptOfMeetingMerge);
    llm2.clearChatHistory();
    final merged = await llm2.createRequest(text: jsonEncode(summaries));

    return jsonEncode({
      'output': [
        {
          'subject': 'Meeting',
          'abstract': _extractJsonContent(merged),
        }
      ]
    });
  }

  /// 分割聊天内容为多个块
  static List<String> _splitChatHistory(
    String chatHistory,
    int chunkSize,
    int overlap,
  ) {
    final chunks = <String>[];
    var start = 0;
    while (start < chatHistory.length) {
      var end = start + chunkSize;
      if (end > chatHistory.length) end = chatHistory.length;
      chunks.add(chatHistory.substring(start, end));
      if (end == chatHistory.length) break;
      start = end - overlap;
    }
    return chunks;
  }

  /// 解析 LLM 返回的 JSON，并持久化
  static void _handleJsonResult({
    required String summaryJson,
    required bool isMeeting,
    int? startTime,
    int? endTime,
    String? audioPath,
  }) {
    final data = jsonDecode(summaryJson)['output'][0];
    final entity = SummaryEntity(
      subject: data['subject'],
      content: data['abstract'],
      startTime: startTime ?? 0,
      endTime: endTime ?? 0,
      createdAt: DateTime.now().millisecondsSinceEpoch,
      isMeeting: isMeeting,
      audioPath: audioPath,
    );
    _persistAndNotify(entity, success: true);
  }

  /// 保存实体并发送通知
  static Future<void> _persistAndNotify(
      SummaryEntity entity, {
        required bool success,
      }) async {
    await ObjectBoxService().insertSummary(entity);
    if (success) {
      showNotificationOfSummaryFinished();
    } else {
      showNotificationOfSummaryFailed();
    }
  }

  /// 提取 JSON Code Block 中的内容
  static String _extractJsonContent(String input) {
    final regex = RegExp(r'```json([\s\S]*?)```');
    final match = regex.firstMatch(input);

    if (match != null) {
      return match.group(1)!.trim();
    } else {
      return input.trim();
    }
  }
}
