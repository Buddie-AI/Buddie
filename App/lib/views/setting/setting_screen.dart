import 'dart:async';
import 'dart:developer' as dev;

import 'package:app/utils/assets_util.dart';
import 'package:app/utils/route_utils.dart';
import 'package:app/utils/sp_util.dart';
import 'package:app/views/setting/components/aliyun_api_key_manage/aliyun_api_key_configure_dialog.dart';
import 'package:app/views/setting/components/aliyun_api_key_manage/aliyun_api_key_delete_dialog.dart';
import 'package:app/views/setting/components/import_api_key/import_api_key_dialog.dart';
import 'package:app/views/ui/bud_switch.dart';
import 'package:app/views/ui/layout/bud_scaffold.dart';
import 'package:flutter/material.dart';
import 'package:flutter_foreground_task/flutter_foreground_task.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:flutter_smart_dialog/flutter_smart_dialog.dart';
import 'package:get/get.dart';
import 'package:go_router/go_router.dart';
import 'package:provider/provider.dart';
import 'package:shared_preferences/shared_preferences.dart';

import '../../controllers/chat_controller.dart';
import '../../controllers/locale_controller.dart';
import '../../controllers/setting_controller.dart';
import '../../controllers/style_controller.dart';
import '../../generated/l10n.dart';
import '../../services/base_llm.dart';
import '../../services/llm_factory.dart';
import 'components/ai_model_setting/ai_model_list_tile.dart';
import 'components/aliyun_api_key_manage/aliyun_api_key_manage_dialog.dart';
import 'components/asr_mode_setting/ast_mode_setting.dart';
import 'components/export_data/export_data_dialog.dart';
import 'components/setting_list_tile.dart';
import 'components/setting_list_view.dart';

class SettingScreen extends StatefulWidget {
  const SettingScreen({super.key});

  @override
  State<SettingScreen> createState() => _SettingScreenState();
}

class _SettingScreenState extends State<SettingScreen> {
  final SettingScreenController _controller = SettingScreenController();

  LLMType? _currentLLMType;
  List<LLMType> _availableLLMTypes = [];

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  void initState() {
    super.initState();
    updateUserInfo();
    _loadCachedAlibabaApiKey();
    _loadLLMStatus();
    // 进入设置页面时自动刷新LLM状态
    WidgetsBinding.instance.addPostFrameCallback((_) {
      _forceReloadLLMStatus();
    });
  }

  Future<void> _loadLLMStatus() async {
    try {
      dev.log('开始加载LLM状态...');

      // 直接通过LLMFactory获取状态，不依赖ChatController
      final availableTypes = await LLMFactory.getAvailableLLMTypes();

      // 获取用户设置的LLM类型
      final userSelectedTypeString = await SPUtil.getString('current_llm_type');
      LLMType? currentType;

      if (userSelectedTypeString != null) {
        switch (userSelectedTypeString) {
          case 'customLLM':
            currentType = LLMType.customLLM;
            break;
          case 'qwenOmni':
            currentType = LLMType.qwenOmni;
            break;
        }
      }

      // 如果没有用户设置或设置的类型不可用，使用第一个可用的类型
      if (currentType == null || !availableTypes.contains(currentType)) {
        currentType = availableTypes.isNotEmpty ? availableTypes.first : null;
      }

      dev.log('获取到的可用LLM类型: $availableTypes');
      dev.log('当前LLM类型: $currentType');

      if (mounted) {
        setState(() {
          _availableLLMTypes = availableTypes;
          _currentLLMType = currentType;
        });
        dev.log('LLM状态更新完成，可用类型数量: ${_availableLLMTypes.length}');
      }

      // 如果没有可用的LLM类型，通知后台服务重新加载配置
      if (availableTypes.isEmpty) {
        dev.log('没有可用的LLM类型，通知后台服务重新加载配置...');
        FlutterForegroundTask.sendDataToTask({'action': 'reloadLLMConfig'});

        // 等待一会儿后重试
        await Future.delayed(const Duration(seconds: 1));
        final retryAvailableTypes = await LLMFactory.getAvailableLLMTypes();

        if (mounted) {
          setState(() {
            _availableLLMTypes = retryAvailableTypes;
            _currentLLMType = retryAvailableTypes.isNotEmpty ? retryAvailableTypes.first : null;
          });
          dev.log('重试后可用类型数量: ${_availableLLMTypes.length}');
        }
      }
    } catch (e) {
      dev.log('加载LLM状态失败: $e');
    }
  }

  updateUserInfo() {
    // No longer needed without authentication
  }

  void _onClickExportData() {
    showDialog(
      context: context,
      builder: (context) {
        return const ExportDataDialog();
      },
    );
  }

  void _onClickAbout() {
    context.pushNamed(RouteName.about);
  }

  void _onClickHeadphoneUpgrade() {
    SmartDialog.showLoading(clickMaskDismiss: false, backDismiss: false);
    Future.delayed(const Duration(seconds: 3), () {
      SmartDialog.dismiss();
      SmartDialog.showToast(S.current.pageSettingEarbudsUpgradeUnavailable);
    });
  }

  void _onClickImportApikey() {
    showDialog(
      context: context,
      builder: (BuildContext context) {
        return ImportApiKeyDialog(
          onRecheckLLMAvailabilityAndAutoSwitch: _recheckLLMAvailabilityAndAutoSwitch,
          onForceReloadLLMStatus: _forceReloadLLMStatus,
        );
      },
    );
  }

  void _onClickManageAlibabaApiKey() {
    _showAlibabaApiKeyManagementDialog();
  }

  String _getLLMDisplayName(BuildContext context, LLMType? type) {
    if (type != null) return type.getLLMDisplayName(context);
    return S.of(context).pageSettingAIModeSetSubtitle3;
  }

  /// Show dialog to configure Alibaba API Key for S2S functionality
  void _showAlibabaApiKeyDialog() {
    String? apiKey;
    // Load existing configuration from cached value
    if (_cachedAlibabaApiKey?.isNotEmpty ?? false) {
      apiKey = _cachedAlibabaApiKey!;
    }

    showDialog(
      context: context,
      builder: (BuildContext context) {
        return AliyunApiKeyConfigureDialog(apiKey: apiKey, onSaveAlibabaApiKey: _saveAlibabaApiKey);
      },
    );
  }

  /// Show management dialog for Alibaba API Key
  void _showAlibabaApiKeyManagementDialog() {
    final bool hasApiKey = _isAlibabaApiKeyAvailable();

    showDialog(
      context: context,
      builder: (BuildContext context) {
        return AliyunApiKeyManageDialog(
          hasApiKey: hasApiKey,
          apiKey: _maskApiKey(_cachedAlibabaApiKey ?? ""),
          onPressedDeleteAlibabaApiKey: () {
            _deleteAlibabaApiKey(context);
          },
          onPressedShowAlibabaApiKeyDialog: _showAlibabaApiKeyDialog,
        );
      },
    );
  }

  /// Mask API Key for display (show first 8 and last 4 characters)
  String _maskApiKey(String apiKey) {
    if (apiKey.length <= 12) {
      return '*' * apiKey.length;
    }
    return '${apiKey.substring(0, 8)}${'*' * (apiKey.length - 12)}${apiKey.substring(apiKey.length - 4)}';
  }

  /// Delete Alibaba API Key
  Future<void> _deleteAlibabaApiKey(BuildContext context) async {
    final confirmed = await showDialog<bool>(
      context: context,
      builder: (BuildContext context) {
        return AliyunApiKeyDeleteDialog();
      },
    );

    if (confirmed == true) {
      try {
        await SPUtil.remove('alibaba_api_key');
        _cachedAlibabaApiKey = null;

        SmartDialog.showToast('API Key 已删除');

        // 重新检查LLM可用性并自动切换
        await _recheckLLMAvailabilityAndAutoSwitch();

        // 自动刷新LLM状态
        await _forceReloadLLMStatus();

        // Notify background service to reload configuration
        FlutterForegroundTask.sendDataToTask({'action': 'reloadAsrConfig'});

        // Refresh the UI
        if (mounted) {
          setState(() {});
          Navigator.of(context).pop(); // Close the management dialog
        }
      } catch (e) {
        print('删除API Key失败: $e');
        SmartDialog.showToast('删除失败，请重试');
      }
    }
  }

  void _onClickHelpAndFeedback(String locale) {
    context.pushNamed(RouteName.help_feedback, extra: locale);
  }

  /// 调试方法：检查阿里云API Key保存状态
  Future<void> _debugCheckAlibabaApiKey() async {
    final savedKey = await SPUtil.getString('alibaba_api_key');
    dev.log('=== 调试阿里云API Key ===');
    dev.log('保存的API Key: ${savedKey ?? "null"}');
    dev.log('API Key是否为空: ${savedKey?.trim().isEmpty ?? true}');
    dev.log('缓存的API Key: ${_cachedAlibabaApiKey ?? "null"}');
    dev.log('_isAlibabaApiKeyAvailable(): ${_isAlibabaApiKeyAvailable()}');

    // 检查LLMFactory的判断
    final isQwenAvailable = await LLMFactory.isLLMAvailable(LLMType.qwenOmni);
    dev.log('LLMFactory.isLLMAvailable(qwenOmni): $isQwenAvailable');

    // 检查所有可用的LLM类型
    final availableTypes = await LLMFactory.getAvailableLLMTypes();
    dev.log('LLMFactory.getAvailableLLMTypes(): $availableTypes');
    dev.log('======================');
  }

  /// Save Alibaba API Key configuration
  Future<void> _saveAlibabaApiKey(String apiKey) async {
    if (apiKey.trim().isEmpty) {
      SmartDialog.showToast('请输入阿里云API Key');
      return;
    }

    try {
      dev.log('开始保存阿里云API Key: ${apiKey.trim()}');

      // Save to SharedPreferences
      await SPUtil.setString('alibaba_api_key', apiKey.trim());

      // Update cached value
      _cachedAlibabaApiKey = apiKey.trim();

      dev.log('阿里云API Key已保存到SharedPreferences');

      // 调试检查保存状态
      await _debugCheckAlibabaApiKey();

      SmartDialog.showToast('阿里云API Key 配置已保存');

      // 重新检查LLM可用性并自动切换
      await _recheckLLMAvailabilityAndAutoSwitch();

      // 自动刷新LLM状态
      await _forceReloadLLMStatus();

      // Notify background service to reload configuration
      FlutterForegroundTask.sendDataToTask({'action': 'reloadAsrConfig'});

      // Refresh the UI to show the S2S option
      if (mounted) {
        setState(() {});
      }
    } catch (e) {
      print('保存阿里云API Key失败: $e');
      SmartDialog.showToast('保存失败，请重试');
    }
  }

  /// Check if Alibaba API Key is available for S2S functionality
  bool _isAlibabaApiKeyAvailable() {
    // Use a cached value or default to false for synchronous check
    return _cachedAlibabaApiKey?.trim().isNotEmpty ?? false;
  }

  String? _cachedAlibabaApiKey;

  /// Load cached Alibaba API Key for UI state checks
  Future<void> _loadCachedAlibabaApiKey() async {
    _cachedAlibabaApiKey = await SPUtil.getString('alibaba_api_key');
  }

  void _onClickAsrMode() {
    showModalBottomSheet(
      context: context,
      backgroundColor: Colors.transparent,
      isScrollControlled: true,
      builder: (BuildContext context) {
        return const AstModeSetting();
      },
    );
  }

  // 添加LLM模式切换方法
  void _onClickLlmMode() {
    // 添加调试信息
    dev.log('显示LLM模式底部对话框，可用类型数量: ${_availableLLMTypes.length}');
    dev.log('当前LLM类型: $_currentLLMType');

    // 如果没有可用的LLM类型，显示提示并重新加载
    if (_availableLLMTypes.isEmpty) {
      SmartDialog.showToast('正在加载AI模型配置...');
      _loadLLMStatus().then((_) {
        if (_availableLLMTypes.isNotEmpty) {
          _onClickLlmMode(); // 重新调用显示对话框
        } else {
          SmartDialog.showToast('暂无可用的AI模型');
        }
      });
      return;
    }

    showModalBottomSheet(
      context: context,
      backgroundColor: Colors.transparent,
      isScrollControlled: true,
      builder: (BuildContext context) {
        return Container(
          height: MediaQuery.of(context).size.height * 0.6,
          decoration: BoxDecoration(
            color: Colors.white,
            borderRadius: BorderRadius.only(topLeft: Radius.circular(20.r), topRight: Radius.circular(20.r)),
          ),
          child: Column(
            children: [
              Container(
                margin: EdgeInsets.only(top: 8.h),
                height: 4.h,
                width: 40.w,
                decoration: BoxDecoration(color: Colors.grey[300], borderRadius: BorderRadius.circular(2.r)),
              ),
              Padding(
                padding: EdgeInsets.all(16.sp),
                child: Text(
                  S.of(context).aiModelSetTitle,
                  style: TextStyle(fontSize: 18.sp, fontWeight: FontWeight.bold),
                ),
              ),
              Expanded(
                child: SingleChildScrollView(
                  child: Padding(
                    padding: EdgeInsets.symmetric(horizontal: 16.sp),
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text(
                          S.of(context).aiDialogModel,
                          style: TextStyle(fontSize: 16.sp, fontWeight: FontWeight.w600, color: Colors.grey[700]),
                        ),
                        SizedBox(height: 8.h),
                        // 显示当前状态
                        if (_availableLLMTypes.isEmpty)
                          Container(
                            padding: EdgeInsets.all(16.w),
                            decoration: BoxDecoration(
                              color: Colors.orange.withValues(alpha: 0.1),
                              borderRadius: BorderRadius.circular(8.r),
                            ),
                            child: Text(
                              '暂无可用的AI模型，请检查网络连接或API配置',
                              style: TextStyle(fontSize: 14.sp, color: Colors.orange[700]),
                            ),
                          )
                        else
                          // 遍历所有可用的LLM类型
                          for (final llmType in _availableLLMTypes) _buildLlmModeSelector(llmType),
                        SizedBox(height: 16.h),
                      ],
                    ),
                  ),
                ),
              ),
            ],
          ),
        );
      },
    );
  }

  // 添加LLM模式选择器
  Widget _buildLlmModeSelector(LLMType llmType) {
    final isSelected = _currentLLMType == llmType;
    return AIModelListTile(
      onTap: () async {
        if (!isSelected) {
          try {
            SmartDialog.showLoading(clickMaskDismiss: false, backDismiss: false);

            dev.log('开始通过后台服务切换LLM: $llmType');

            // 使用后台服务切换LLM（类似ASR切换方式）
            // 先保存用户选择到SharedPreferences
            await SPUtil.setString('current_llm_type', llmType.name);

            // 通知后台服务切换LLM类型
            FlutterForegroundTask.sendDataToTask({'action': 'switchLLMType', 'llmType': llmType.name});

            // 等待后台服务响应（设置超时时间）
            await _waitForLLMSwitchResult();

            if (mounted) {
              setState(() {
                _currentLLMType = llmType;
              });
            }

            SmartDialog.dismiss();
            SmartDialog.showToast('已切换到${_getLLMDisplayName(context, llmType)}');
            Navigator.of(context).pop();
          } catch (e) {
            SmartDialog.dismiss();
            SmartDialog.showToast('切换失败: $e');
            dev.log('切换LLM失败: $e');
          }
        }
      },
      llmType: llmType,
      isSelected: isSelected,
    );
  }

  /// 等待LLM切换结果
  Future<void> _waitForLLMSwitchResult() async {
    // 简化实现：给后台服务一点时间来处理，然后验证结果
    await Future.delayed(const Duration(seconds: 2));

    // 验证当前LLM类型是否已切换
    final savedType = await SPUtil.getString('current_llm_type');
    dev.log('切换后保存的LLM类型: $savedType');
  }

  /// 重新检查LLM可用性并自动切换（API Key配置更改后调用）
  Future<void> _recheckLLMAvailabilityAndAutoSwitch() async {
    try {
      dev.log('=== 开始重新检查LLM可用性 ===');

      // 先直接检查LLMFactory的状态
      await _debugCheckAlibabaApiKey();

      // 1. 通知后台服务重新加载LLM配置
      dev.log('步骤1: 通知后台服务重新加载LLM配置...');
      FlutterForegroundTask.sendDataToTask({'action': 'reloadLLMConfig'});

      // 等待后台服务处理
      await Future.delayed(const Duration(seconds: 2));

      // 2. 获取新的可用LLM类型列表
      dev.log('步骤2: 获取可用LLM类型列表...');
      final newAvailableTypes = await LLMFactory.getAvailableLLMTypes();
      final oldCurrentType = _currentLLMType;

      dev.log('重新检查后的可用LLM类型: $newAvailableTypes');
      dev.log('当前选中的LLM类型: $oldCurrentType');

      // 3. 检查当前选中的LLM是否仍然可用
      bool needAutoSwitch = false;
      LLMType? newCurrentType = oldCurrentType;

      if (oldCurrentType == null || !newAvailableTypes.contains(oldCurrentType)) {
        needAutoSwitch = true;
        dev.log('步骤3: 需要自动切换 - 当前LLM不可用');

        // 4. 自动切换到优先级最高的可用LLM（与LLMFactory._determineBestLLMType一致）
        if (newAvailableTypes.isNotEmpty) {
          // 优先级：customLLM > qwenOmni
          if (newAvailableTypes.contains(LLMType.customLLM)) {
            newCurrentType = LLMType.customLLM;
          } else if (newAvailableTypes.contains(LLMType.qwenOmni)) {
            newCurrentType = LLMType.qwenOmni;
          } else {
            newCurrentType = newAvailableTypes.first;
          }

          dev.log('步骤4: 自动切换到LLM类型: $newCurrentType');

          // 通过后台服务执行切换
          await SPUtil.setString('current_llm_type', newCurrentType.name);
          FlutterForegroundTask.sendDataToTask({'action': 'switchLLMType', 'llmType': newCurrentType.name});

          // 等待切换完成
          await Future.delayed(const Duration(seconds: 1));

          SmartDialog.showToast('已自动切换到${_getLLMDisplayName(context, newCurrentType)}');
        } else {
          dev.log('警告：没有可用的LLM类型');
          SmartDialog.showToast('警告：暂无可用的AI模型');
          newCurrentType = null;
        }
      } else {
        dev.log('当前LLM仍然可用，无需切换');
      }

      // 5. 更新UI状态
      if (mounted) {
        setState(() {
          _availableLLMTypes = newAvailableTypes;
          _currentLLMType = newCurrentType;
        });
      }

      dev.log('LLM可用性检查完成，当前类型: $newCurrentType');
      dev.log('UI状态更新完成，_availableLLMTypes: $_availableLLMTypes');
      dev.log('=== LLM可用性检查结束 ===');
    } catch (e) {
      dev.log('重新检查LLM可用性失败: $e');
      SmartDialog.showToast('AI模型配置更新失败: $e');
    }
  }

  /// 强制重新加载配置并更新LLM状态
  Future<void> _forceReloadLLMStatus() async {
    try {
      dev.log('=== 强制重新加载LLM状态 ===');

      // 1. 强制重新加载SharedPreferences
      final prefs = await SharedPreferences.getInstance();
      await prefs.reload();

      // 2. 重新检查阿里云API Key
      final reloadedKey = await SPUtil.getString('alibaba_api_key');
      dev.log('重新加载的阿里云API Key: ${reloadedKey ?? "null"}');
      _cachedAlibabaApiKey = reloadedKey;

      // 3. 重新检查自定义LLM配置
      final customToken = await SPUtil.getString('llmToken') ?? '';
      final customUrl = await SPUtil.getString('llmUrl') ?? '';
      final customModel = await SPUtil.getString('llmModel') ?? '';
      dev.log('自定义LLM配置 - Token: ${customToken.isNotEmpty ? "已配置" : "未配置"}');
      dev.log('自定义LLM配置 - URL: $customUrl');
      dev.log('自定义LLM配置 - Model: $customModel');

      // 4. 直接调用LLMFactory重新检查所有类型
      final isCustomAvailable = await LLMFactory.isLLMAvailable(LLMType.customLLM);
      final isQwenAvailable = await LLMFactory.isLLMAvailable(LLMType.qwenOmni);

      dev.log('LLM可用性检查结果:');
      dev.log('- customLLM: $isCustomAvailable');
      dev.log('- qwenOmni: $isQwenAvailable');

      // 5. 获取最新的可用LLM列表
      final newAvailableTypes = await LLMFactory.getAvailableLLMTypes();
      dev.log('重新获取的可用LLM类型: $newAvailableTypes');

      // 6. 尝试获取当前LLM类型（如果ChatController可用）
      LLMType? currentType = _currentLLMType;
      try {
        final chatController = Get.find<ChatController>();
        currentType = chatController.getCurrentLLMType();
        dev.log('从ChatController获取的当前LLM类型: $currentType');
      } catch (e) {
        dev.log('ChatController不可用，使用本地状态: $currentType');
      }

      // 7. 如果当前类型不可用，选择最优的可用类型
      if (currentType == null || !newAvailableTypes.contains(currentType)) {
        if (newAvailableTypes.isNotEmpty) {
          // 优先级：customLLM > qwenOmni
          if (newAvailableTypes.contains(LLMType.customLLM)) {
            currentType = LLMType.customLLM;
          } else if (newAvailableTypes.contains(LLMType.qwenOmni)) {
            currentType = LLMType.qwenOmni;
          } else {
            currentType = newAvailableTypes.first;
          }
          dev.log('自动选择LLM类型: $currentType');
        }
      }

      // 8. 更新UI状态
      if (mounted) {
        setState(() {
          _availableLLMTypes = newAvailableTypes;
          _currentLLMType = currentType;
        });
      }

      dev.log('强制重新加载完成，UI已更新');
      dev.log('最终状态 - 可用类型: $_availableLLMTypes, 当前类型: $_currentLLMType');

      // 显示用户友好的提示
      if (newAvailableTypes.length > 1) {
        SmartDialog.showToast(S.current.aiModelToastRefreshedAndFound('${newAvailableTypes.length}'));
      } else if (newAvailableTypes.isNotEmpty) {
        SmartDialog.showToast(S.current.aiModelToastRefreshed);
      } else {
        SmartDialog.showToast(S.current.aiModelToastUnavailable);
      }
    } catch (e) {
      dev.log('强制重新加载失败: $e');
      SmartDialog.showToast(S.current.aiModelToastRefreshFailed('$e'));
    }
  }

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    final localeNotifier = Provider.of<LocaleNotifier>(context);
    return BudScaffold(
      title: S.of(context).pageSettingTitle,
      body: ListView(
        padding: EdgeInsets.all(16.sp),
        children: [
          SectionListView(
            children: [
              SettingListTile(
                leading: AssetsUtil.icon_dark_mode,
                title: S.of(context).pageSettingDarkMode,
                trailing: BudSwitch(
                  value: themeNotifier.mode == Mode.dark,
                  onChanged: (value) {
                    themeNotifier.toggleTheme();
                  },
                ),
              ),
              SettingListTile(
                onTap: () {
                  localeNotifier.toggleLocale();
                },
                leading: AssetsUtil.icon_set_up,
                title: S.of(context).pageSettingLanguage,
                subtitle: localeNotifier.locale == 'en' ? S.of(context).languageEnglish : S.of(context).languageChinese,
              ),
              SettingListTile(
                onTap: _onClickAsrMode,
                leading: AssetsUtil.icon_apikey,
                title: S.of(context).asrModeSetTitle,
                subtitle: S.of(context).asrModeSetSubtitle,
              ),
              SettingListTile(
                onTap: _onClickLlmMode,
                leading: AssetsUtil.icon_apikey,
                title: S.of(context).aiModelSetTitle,
                subtitle: _getLLMDisplayName(context, _currentLLMType),
                trailing: Row(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    // 临时调试按钮
                    GestureDetector(
                      onTap: _forceReloadLLMStatus,
                      child: Icon(Icons.refresh, color: Colors.orange, size: 16.sp),
                    ),
                    SizedBox(width: 8.w),
                    if (_currentLLMType != null)
                      Icon(_currentLLMType!.icon, color: _currentLLMType!.color, size: 20.sp),
                  ],
                ),
              ),
              SettingListTile(
                onTap: _onClickImportApikey,
                leading: AssetsUtil.icon_apikey,
                title: S.of(context).importAPIKeyTitle,
                subtitle: S.of(context).importAPIKeySubtitle,
                // 移除启用/禁用开关
              ),
              SettingListTile(
                onTap: _onClickManageAlibabaApiKey,
                leading: AssetsUtil.icon_apikey,
                title: S.of(context).aliyunAPIKeyManageTitle,
                subtitle: _isAlibabaApiKeyAvailable()
                    ? S.of(context).aliyunAPIKeyManageSubtitleConfigured
                    : S.of(context).aliyunAPIKeyManageSubtitle2,
                trailing: _isAlibabaApiKeyAvailable()
                    ? Icon(Icons.check_circle, color: Colors.green, size: 20.sp)
                    : Icon(Icons.key_off, color: Colors.grey, size: 20.sp),
              ),
              SettingListTile(
                leading: AssetsUtil.icon_export_data,
                title: S.of(context).exportDataTitle,
                subtitle: S.of(context).exportDataSubtitle,
                onTap: _onClickExportData,
              ),
              SettingListTile(
                leading: AssetsUtil.icon_set_up,
                title: S.of(context).pageSettingEarbudsUpgradeTitle,
                subtitle: S.of(context).pageSettingEarbudsUpgradeSubtitle,
                onTap: _onClickHeadphoneUpgrade,
              ),
            ],
          ),
          SizedBox(height: 12.sp),
          SectionListView(
            children: [
              SettingListTile(
                leading: AssetsUtil.icon_about,
                title: S.of(context).pageSettingAboutTitle,
                subtitle: S.of(context).pageSettingAboutSubtitle,
                onTap: _onClickAbout,
              ),
              SettingListTile(
                leading: AssetsUtil.icon_feedback,
                title: S.of(context).pageSettingHelpTitle,
                subtitle: S.of(context).pageSettingHelpSubtitle,
                onTap: () {
                  _onClickHelpAndFeedback(localeNotifier.locale);
                },
              ),
            ],
          ),
        ],
      ),
    );
  }
}
