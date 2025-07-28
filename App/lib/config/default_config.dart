import 'package:flutter_dotenv/flutter_dotenv.dart';
import 'package:flutter/foundation.dart';

/// 默认配置管理器
///
/// 使用flutter_dotenv从env文件中读取默认配置值
/// 这些值只作为默认值，用户可以在应用设置中覆盖
class DefaultConfig {
  static bool _isInitialized = false;

  /// 初始化配置
  static Future<void> initialize() async {
    if (_isInitialized) return;

    try {
      await dotenv.load(fileName: "env");
      _isInitialized = true;
      if (kDebugMode) {
        print('DefaultConfig: env文件加载成功');
      }
    } catch (e) {
      if (kDebugMode) {
        print('DefaultConfig: env文件加载失败: $e');
        print('DefaultConfig: 将使用硬编码的默认值');
      }
      _isInitialized = true;
    }
  }

  /// 获取环境变量值，如果不存在则返回fallback值
  static String _getEnvValue(String key, String fallback) {
    if (!_isInitialized) {
      if (kDebugMode) {
        print('DefaultConfig: 配置未初始化，返回fallback值');
      }
      return fallback;
    }

    try {
      return dotenv.env[key] ?? fallback;
    } catch (e) {
      if (kDebugMode) {
        print('DefaultConfig: 获取环境变量 $key 失败: $e');
      }
      return fallback;
    }
  }

  // ===== 自定义LLM默认配置 =====

  /// 默认LLM Token (OpenAI API Key)
  static String get defaultLlmToken => _getEnvValue(
    'DEFAULT_LLM_TOKEN',
    '', // 空值表示没有默认值，用户必须配置
  );

  /// 默认LLM URL
  static String get defaultLlmUrl => _getEnvValue(
    'DEFAULT_LLM_URL',
    'https://api.openai.com/v1/chat/completions',
  );

  /// 默认LLM模型
  static String get defaultLlmModel =>
      _getEnvValue('DEFAULT_LLM_MODEL', 'gpt-4o');

  // ===== 阿里云API配置 =====

  /// 默认阿里云DashScope API Key
  static String get defaultAlibabaApiKey => _getEnvValue(
    'DEFAULT_ALIBABA_API_KEY',
    '', // 空值表示没有默认值，用户必须配置
  );

  // ===== 腾讯云ASR配置 =====

  /// 默认腾讯云Secret ID
  static String get defaultTencentSecretId =>
      _getEnvValue('DEFAULT_TENCENT_SECRET_ID', '');

  /// 默认腾讯云Secret Key
  static String get defaultTencentSecretKey =>
      _getEnvValue('DEFAULT_TENCENT_SECRET_KEY', '');

  /// 默认腾讯云Token
  static String get defaultTencentToken =>
      _getEnvValue('DEFAULT_TENCENT_TOKEN', '');

  // ===== 工具方法 =====

  /// 检查是否有有效的默认配置
  static bool get hasValidDefaultLlmConfig {
    return defaultLlmToken.isNotEmpty &&
        defaultLlmUrl.isNotEmpty &&
        defaultLlmModel.isNotEmpty;
  }

  /// 检查是否有有效的阿里云配置
  static bool get hasValidDefaultAlibabaConfig {
    return defaultAlibabaApiKey.isNotEmpty;
  }

  /// 检查是否有有效的腾讯云配置
  static bool get hasValidDefaultTencentConfig {
    return defaultTencentSecretId.isNotEmpty &&
        defaultTencentSecretKey.isNotEmpty &&
        defaultTencentToken.isNotEmpty;
  }

  /// 打印所有配置状态（仅在debug模式）
  static void printConfigStatus() {
    if (!kDebugMode) return;

    print('=== DefaultConfig 状态 ===');
    print('初始化状态: $_isInitialized');
    print('默认LLM配置: ${hasValidDefaultLlmConfig ? "✓" : "✗"}');
    print('默认阿里云配置: ${hasValidDefaultAlibabaConfig ? "✓" : "✗"}');
    print('默认腾讯云配置: ${hasValidDefaultTencentConfig ? "✓" : "✗"}');
    print('LLM URL: ${defaultLlmUrl}');
    print('LLM Model: ${defaultLlmModel}');
    print('========================');
  }
}
