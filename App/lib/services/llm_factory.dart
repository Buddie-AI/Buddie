/// 大语言模型(LLM)工厂与管理中心
///
/// 提供统一的LLM实例生命周期管理，主要功能包括：
/// 1. 实例管理：
///   - 自动创建/销毁LLM实例（单例模式）
///   - 配置变更自动重新初始化
///   - 资源释放管理
/// 2. 智能切换：
///   - 根据配置自动选择LLM类型
///   - 支持手动切换
///   - 配置持久化存储（SharedPreferences）
/// 3. 状态查询：
///   - 获取可用LLM类型列表
///   - 检查功能支持（如音频输入）
///   - 获取模型元信息（名称/描述）
///
/// 配置持久化：
/// - 用户选择的LLM类型自动保存到SharedPreferences
/// - 重新启动应用后保持上次选择

import 'dart:async';
import 'dart:developer' as dev;
import '../utils/sp_util.dart';
import '../config/default_config.dart';
import 'base_llm.dart';
import 'custom_llm.dart';
import 'qwen_omni_llm.dart';

/// LLM工厂类 - 统一管理所有LLM实例
class LLMFactory {
  static LLMFactory? _instance;

  BaseLLM? _currentLLM;
  LLMType? _currentType;
  String? _currentSystemPrompt;

  LLMFactory._();

  static LLMFactory get instance {
    _instance ??= LLMFactory._();
    return _instance!;
  }

  /// 获取当前LLM实例，如果未初始化则自动创建
  Future<BaseLLM> getCurrentLLM({String? systemPrompt}) async {
    // 检查是否需要重新创建LLM（配置变更或系统提示变更）
    final targetType = await _determineBestLLMType();
    final needRecreate =
        _currentLLM == null ||
        _currentType != targetType ||
        (systemPrompt != null && _currentSystemPrompt != systemPrompt);

    if (needRecreate) {
      await _currentLLM?.dispose();
      _currentLLM = await _createLLM(targetType, systemPrompt);
      _currentType = targetType;
      _currentSystemPrompt = systemPrompt;
    }

    return _currentLLM!;
  }

  /// 获取指定类型的LLM实例
  Future<BaseLLM> getLLM(LLMType type, {String? systemPrompt}) async {
    final llm = await _createLLM(type, systemPrompt);
    return llm;
  }

  /// 重新初始化当前LLM（配置更改后调用）
  Future<void> reinitializeLLM() async {
    if (_currentLLM != null) {
      await _currentLLM!.reinitialize();
    }
  }

  /// 直接切换到指定的LLM类型（类似ASR切换方式）
  Future<void> switchToLLMType(LLMType type, {String? systemPrompt}) async {
    try {
      dev.log('LLMFactory: 开始切换到 ${type.name}');

      // 保存用户选择的LLM类型到SharedPreferences
      await SPUtil.setString('current_llm_type', type.name);

      // 销毁当前LLM实例
      await _currentLLM?.dispose();

      // 创建新的LLM实例
      _currentLLM = await _createLLM(
        type,
        systemPrompt ?? _currentSystemPrompt,
      );
      _currentType = type;
      if (systemPrompt != null) {
        _currentSystemPrompt = systemPrompt;
      }

      dev.log('LLMFactory: 成功切换到 ${type.name}');
    } catch (e) {
      dev.log('LLMFactory: 切换LLM失败: $e');
      throw e;
    }
  }

  /// 重新加载LLM配置（从SharedPreferences读取用户设置）
  Future<void> reloadLLMConfig({String? systemPrompt}) async {
    try {
      dev.log('LLMFactory: 开始重新加载LLM配置...');

      // 获取用户选择的LLM类型
      final userSelectedType = await _getUserSelectedLLMType();
      final targetType = userSelectedType ?? await _determineBestLLMType();

      dev.log('LLMFactory: 用户选择的LLM类型: $userSelectedType');
      dev.log('LLMFactory: 目标LLM类型: $targetType');

      // 检查是否需要重新创建LLM
      final needRecreate =
          _currentLLM == null ||
          _currentType != targetType ||
          (systemPrompt != null && _currentSystemPrompt != systemPrompt);

      if (needRecreate) {
        await _currentLLM?.dispose();
        _currentLLM = await _createLLM(
          targetType,
          systemPrompt ?? _currentSystemPrompt,
        );
        _currentType = targetType;
        if (systemPrompt != null) {
          _currentSystemPrompt = systemPrompt;
        }
        dev.log('LLMFactory: LLM配置重新加载完成，当前类型: ${_currentType?.name}');
      } else {
        // 如果不需要重新创建，只需重新初始化
        await _currentLLM?.reinitialize();
        dev.log('LLMFactory: LLM重新初始化完成');
      }
    } catch (e) {
      dev.log('LLMFactory: 重新加载LLM配置失败: $e');
      throw e;
    }
  }

  /// 获取用户在设置中选择的LLM类型
  Future<LLMType?> _getUserSelectedLLMType() async {
    final typeString = await SPUtil.getString('current_llm_type');
    if (typeString == null) return null;

    switch (typeString) {
      case 'customLLM':
        return LLMType.customLLM;
      case 'qwenOmni':
        return LLMType.qwenOmni;
      default:
        return null;
    }
  }

  /// 释放所有资源
  Future<void> dispose() async {
    await _currentLLM?.dispose();
    _currentLLM = null;
    _currentType = null;
    _currentSystemPrompt = null;
  }

  /// 根据配置确定最佳的LLM类型
  Future<LLMType> _determineBestLLMType() async {
    // 首先检查用户是否有明确选择
    final userSelected = await _getUserSelectedLLMType();
    if (userSelected != null && await isLLMAvailable(userSelected)) {
      return userSelected;
    }

    // 如果用户没有选择或选择的不可用，按优先级选择
    // 优先级：用户自定义LLM > QwenOmni > 默认LLM

    // 检查用户自定义LLM是否可用
    if (await CustomLLM.isConfigured()) {
      return LLMType.customLLM;
    }

    // 检查QwenOmni是否可用
    if (await isLLMAvailable(LLMType.qwenOmni)) {
      return LLMType.qwenOmni;
    }

    // 如果没有可用的配置，抛出异常
    throw Exception(
      'No available LLM configuration found. Please configure CustomLLM or QwenOmni.',
    );
  }

  /// 创建指定类型的LLM实例
  Future<BaseLLM> _createLLM(LLMType type, String? systemPrompt) async {
    BaseLLM llm;

    switch (type) {
      case LLMType.customLLM:
        llm = CustomLLM(systemPrompt: systemPrompt);
        break;
      case LLMType.qwenOmni:
        llm = QwenOmniLLM(systemPrompt: systemPrompt);
        break;
    }

    await llm.initialize();

    if (!llm.isAvailable) {
      throw Exception(
        'LLM ${type.name} is not available or not properly configured',
      );
    }

    return llm;
  }

  /// 检查指定LLM类型是否可用
  static Future<bool> isLLMAvailable(LLMType type) async {
    switch (type) {
      case LLMType.customLLM:
        return await CustomLLM.isConfigured();
      case LLMType.qwenOmni:
        // 检查用户配置的API Key
        final userApiKey = await SPUtil.getString('alibaba_api_key') ?? '';
        if (userApiKey.trim().isNotEmpty) {
          return true;
        }
        // 如果用户没有配置，检查是否有默认配置
        return DefaultConfig.defaultAlibabaApiKey.trim().isNotEmpty;
    }
  }

  /// 获取所有可用的LLM类型
  static Future<List<LLMType>> getAvailableLLMTypes() async {
    final available = <LLMType>[];

    for (final type in LLMType.values) {
      if (await isLLMAvailable(type)) {
        available.add(type);
      }
    }

    return available;
  }

  /// 获取LLM类型的显示名称
  static String getLLMDisplayName(LLMType type) {
    switch (type) {
      case LLMType.customLLM:
        return '用户自定义LLM';
      case LLMType.qwenOmni:
        return 'QwenOmni多模态LLM';
    }
  }

  /// 获取LLM类型的描述
  static String getLLMDescription(LLMType type) {
    switch (type) {
      case LLMType.customLLM:
        return '使用用户配置的API Key和模型';
      case LLMType.qwenOmni:
        return '支持音频输入输出的多模态对话';
    }
  }

  /// 获取当前LLM类型
  LLMType? get currentType => _currentType;

  /// 检查当前LLM是否支持音频输入
  bool get supportsAudioInput => _currentType == LLMType.qwenOmni;
}
