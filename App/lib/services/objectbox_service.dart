/// ObjectBox 数据库服务
///
/// 提供本地数据库的存储和查询能力，管理以下数据类型：
/// - RecordEntity: 对话记录实体
/// - SummaryEntity: 会议摘要实体
/// - LlmConfigEntity: 大模型配置实体
///
/// 核心功能：
/// 1. 数据库初始化与清理
/// 2. 实体增删改查操作：
///   - 支持分类插入（普通对话、会议记录）
///   - 按时间范围查询记录
///   - 向量相似度搜索
/// 3. 摘要管理：
///   - 按类型获取（会议/日常）
///   - 关键词搜索
///   - 标题更新
/// 4. 配置管理：
///   - API密钥更新
///   - 按模型/提供商查询
///
/// 使用示例：
/// ```dart
/// // 初始化数据库
/// await ObjectBoxService.initialize();
///
/// // 插入新记录
/// ObjectBoxService().insertMeetingRecord(record);
///
/// // 查询会议摘要
/// final summaries = ObjectBoxService().getMeetingSummaries();
/// ```
///
/// 注意事项：
/// - 采用单例模式确保全局唯一实例
/// - 向量搜索支持最近邻算法（nearestNeighborsF32）

import 'dart:async';
import 'package:app/models/llm_config.dart';
import 'package:path/path.dart' as p;
import 'package:path_provider/path_provider.dart';
import '../models/record_entity.dart';
import '../models/summary_entity.dart';
import '../models/objectbox.g.dart';

class ObjectBoxService {
  static late final Store store;
  static late final Box<RecordEntity> recordBox;
  static late final Box<SummaryEntity> summaryBox;
  static late final Box<LlmConfigEntity> configBox;

  // Singleton pattern to ensure only one instance of ObjectBoxService
  static final ObjectBoxService _instance = ObjectBoxService._internal();

  factory ObjectBoxService() => _instance;

  ObjectBoxService._internal();

  static Future<void> initialize() async {
    final docsDir = await getApplicationDocumentsDirectory();
    final dbDir = p.join(docsDir.path, 'person-db');

    if (Store.isOpen(dbDir)) {
      // applicable when store is from other isolate
      store = Store.attach(getObjectBoxModel(), dbDir);
    } else {
      try {
        store = await openStore(directory: dbDir);
      } catch (error) {
        // If the store cannot be opened, it might already be open.
        // Try to attach to it instead.
        store = Store.attach(getObjectBoxModel(), dbDir);
      }
    }

    recordBox = Box<RecordEntity>(store);
    summaryBox = Box<SummaryEntity>(store);
    configBox = Box<LlmConfigEntity>(store);
  }

  static void clearAllData() {
    recordBox.removeAll();
    summaryBox.removeAll();
    configBox.removeAll();
  }

  void insertRecord(RecordEntity record, String category) {
    record.category = category;
    recordBox.put(record);
  }

  void insertDefaultRecord(RecordEntity record) {
    insertRecord(record, RecordEntity.categoryDefault);
  }

  void insertDialogueRecord(RecordEntity record) {
    insertRecord(record, RecordEntity.categoryDialogue);
  }

  void insertMeetingRecord(RecordEntity record) {
    insertRecord(record, RecordEntity.categoryMeeting);
  }

  List<RecordEntity>? getMeetingRecordsByTimeRange(startTime, endTime) {
    final queryBuilder = recordBox
        .query(
          RecordEntity_.createdAt
              .between(startTime, endTime)
              .and(RecordEntity_.category.equals(RecordEntity.categoryMeeting)),
        )
        .order(RecordEntity_.createdAt);
    final query = queryBuilder.build();
    query.limit = 1000;
    return query.find();
  }

  Future<void> insertRecords(List<RecordEntity> vectors) async {
    recordBox.putMany(vectors);
  }

  RecordEntity? getLastRecord() {
    return recordBox.isEmpty() ? null : recordBox.getAll().last;
  }

  List<RecordEntity>? getRecords() {
    return recordBox.getAll();
  }

  List<RecordEntity>? getChatRecords({int offset = 0, int limit = 50}) {
    final queryBuilder = recordBox.query().order(
      RecordEntity_.createdAt,
      flags: Order.descending,
    );

    final query = queryBuilder.build();
    query.offset = offset;
    query.limit = limit;
    return query.find();
  }

  List<RecordEntity>? getTermRecords() {
    final queryBuilder = recordBox.query().order(
      RecordEntity_.createdAt,
      flags: Order.descending,
    );
    final query = queryBuilder.build();
    query.limit = 16;
    return query.find().reversed.toList();
  }

  List<RecordEntity>? getChatRecordsByTimeRange(startTime, endTime) {
    final queryBuilder = recordBox
        .query(RecordEntity_.createdAt.between(startTime, endTime))
        .order(RecordEntity_.createdAt, flags: Order.descending);
    final query = queryBuilder.build();
    query.limit = 1000;
    return query.find();
  }

  List<RecordEntity> getRecordsBySubject(String subject) {
    final summaryQuery = summaryBox
        .query(SummaryEntity_.subject.equals(subject, caseSensitive: false))
        .build();
    final summaryResults = summaryQuery.find();

    // If no summaries are found, return an empty list
    if (summaryResults.isEmpty) return [];

    final List<RecordEntity> finalRecords = [];

    for (final summary in summaryResults) {
      final recordQuery = recordBox
          .query(
            RecordEntity_.createdAt.between(summary.startTime, summary.endTime),
          )
          .build();

      final recordResults = recordQuery.find();

      finalRecords.addAll(recordResults);
    }

    return finalRecords;
  }

  List<RecordEntity> getRecordsByTimeRange(int startTime, int endTime) {
    return recordBox
        .query(RecordEntity_.createdAt.between(startTime, endTime))
        .build()
        .find();
  }

  List<Map<RecordEntity, double>> getSimilarRecordsByContents(
    List<double> queryVector,
    int topK,
  ) {
    final results = recordBox
        .query(RecordEntity_.vector.nearestNeighborsF32(queryVector, topK))
        .build()
        .findWithScores();

    return results.map((result) => {result.object: result.score}).toList();
  }

  List<Map<RecordEntity, double>> getSimilarRecordsBySummaries(
    List<double> queryVector,
    int topK,
  ) {
    final summaryQuery = summaryBox
        .query(SummaryEntity_.vector.nearestNeighborsF32(queryVector, topK))
        .build();

    final summaryResults = summaryQuery.findWithScores();

    if (summaryResults.isEmpty) return [];

    // Prepare a list to store the results from the RecordEntity search
    final List<Map<RecordEntity, double>> finalResults = [];

    for (final summaryResult in summaryResults) {
      final summary = summaryResult.object;

      final recordQuery = recordBox
          .query(
            RecordEntity_.createdAt.between(summary.startTime, summary.endTime),
          )
          .build();

      final recordResults = recordQuery.findWithScores();

      // Combine record results with their scores from the summary
      finalResults.addAll(
        recordResults.map((result) => {result.object: result.score}),
      );
    }

    return finalResults;
  }

  Future<void> deleteAllRecords() async {
    recordBox.removeAll();
  }

  Future<void> deleteAllSummaries() async {
    summaryBox.removeAll();
  }

  Future<void> deleteSummary(int id) async {
    summaryBox.remove(id);
  }

  Future<void> deleteSummaries(List<int> ids) async {
    summaryBox.removeMany(ids);
  }

  Future<void> insertConfig(LlmConfigEntity llmConfig) async {
    configBox.put(llmConfig);
  }

  Future<void> updateConfigByProvider(String provider, {String? apiKey}) async {
    final query = configBox
        .query(LlmConfigEntity_.provider.equals(provider))
        .build();
    final existingConfig = query.findFirst();
    if (existingConfig != null) {
      if (apiKey != null) existingConfig.apiKey = apiKey;
      configBox.put(existingConfig);
    }
  }

  Future<void> insertConfigs(List<LlmConfigEntity> vectors) async {
    configBox.putMany(vectors);
  }

  LlmConfigEntity? getLastConfig() {
    return configBox.isEmpty() ? null : configBox.getAll().last;
  }

  List<LlmConfigEntity>? getConfigs() {
    return configBox.getAll();
  }

  LlmConfigEntity? getConfigsByModel(String model) {
    final configQuery = configBox
        .query(LlmConfigEntity_.model.equals(model))
        .build();

    return configQuery.findFirst();
  }

  LlmConfigEntity? getConfigsByProvider(String provider) {
    final configQuery = configBox
        .query(LlmConfigEntity_.provider.equals(provider))
        .build();

    return configQuery.findFirst();
  }

  Future<void> deleteAllConfigs() async {
    configBox.removeAll();
  }

  Future<void> insertSummary(SummaryEntity record) async {
    summaryBox.put(record);
  }

  Future<void> insertSummaries(List<SummaryEntity> vectors) async {
    summaryBox.putMany(vectors);
  }

  List<SummaryEntity>? getSummaries() {
    return summaryBox.getAll();
  }

  List<SummaryEntity>? getMeetingSummaries() {
    List<SummaryEntity>? list = summaryBox.isEmpty()
        ? null
        : summaryBox
              .query(SummaryEntity_.isMeeting.equals(true))
              .build()
              .find();
    return list;
  }

  List<SummaryEntity>? getDailySummaries() {
    return summaryBox.isEmpty()
        ? null
        : summaryBox
              .query(SummaryEntity_.isMeeting.equals(false))
              .build()
              .find();
  }

  SummaryEntity? getLastSummary({bool isMeeting = false}) {
    if (isMeeting) {
      final results = summaryBox
          .query(SummaryEntity_.isMeeting.equals(true))
          .order(SummaryEntity_.createdAt, flags: Order.descending)
          .build()
          .find();
      return results.isEmpty ? null : results.last;
    }
    return summaryBox.isEmpty() ? null : summaryBox.getAll().last;
  }

  List<SummaryEntity>? getSummariesBySubject(String subject) {
    return summaryBox.isEmpty()
        ? null
        : summaryBox
              .query(SummaryEntity_.subject.equals(subject))
              .build()
              .find();
  }

  List<SummaryEntity>? getSummariesByKeyword(
    String keyword, {
    required bool isMeeting,
  }) {
    return summaryBox.isEmpty()
        ? null
        : summaryBox
              .query(
                SummaryEntity_.isMeeting
                    .equals(isMeeting)
                    .and(
                      SummaryEntity_.content
                          .contains(keyword)
                          .or(SummaryEntity_.subject.contains(keyword)),
                    ),
              )
              .build()
              .find();
  }

  List<Map<SummaryEntity, double>> getSimilarSummariesByContents(
    List<double> queryVector,
    int topK,
  ) {
    final results = summaryBox
        .query(SummaryEntity_.vector.nearestNeighborsF32(queryVector, topK))
        .build()
        .findWithScores();

    return results.map((result) => {result.object: result.score}).toList();
  }

  Future<void> updateSummaryTitle(int id, String title) async {
    SummaryEntity? summary = summaryBox.get(id);
    if (summary != null) {
      summary.title = title;
      summaryBox.put(summary);
    }
  }
}
