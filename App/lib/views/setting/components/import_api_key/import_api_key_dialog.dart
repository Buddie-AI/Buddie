import 'package:flutter/material.dart';
import 'package:flutter_foreground_task/flutter_foreground_task.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:flutter_smart_dialog/flutter_smart_dialog.dart';

import '../../../../config/default_config.dart';
import '../../../../generated/l10n.dart';
import '../../../../utils/sp_util.dart';

class ImportApiKeyDialog extends StatefulWidget {
  /// 重新检查LLM可用性并自动切换
  final Future<void> Function() onRecheckLLMAvailabilityAndAutoSwitch;

  /// 自动刷新LLM状态
  final Future<void> Function() onForceReloadLLMStatus;

  const ImportApiKeyDialog({
    super.key,
    required this.onRecheckLLMAvailabilityAndAutoSwitch,
    required this.onForceReloadLLMStatus,
  });

  @override
  State<ImportApiKeyDialog> createState() => _ImportApiKeyDialogState();
}

class _ImportApiKeyDialogState extends State<ImportApiKeyDialog> {
  final TextEditingController apiKeyController = TextEditingController();
  final TextEditingController urlController = TextEditingController();
  final TextEditingController modelController = TextEditingController();

  @override
  void dispose() {
    apiKeyController.dispose();
    urlController.dispose();
    modelController.dispose();
    super.dispose();
  }

  @override
  void initState() {
    super.initState();
    _loadExistingConfig();
  }

  Future<void> _loadExistingConfig() async {
    // 加载用户配置，如果没有则显示默认值
    final userApiKey = await SPUtil.getString('llmToken') ?? '';
    final userUrl = await SPUtil.getString('llmUrl') ?? '';
    final userModel = await SPUtil.getString('llmModel') ?? '';

    apiKeyController.text = userApiKey.isNotEmpty ? userApiKey : DefaultConfig.defaultLlmToken;
    urlController.text = userUrl.isNotEmpty ? userUrl : DefaultConfig.defaultLlmUrl;
    modelController.text = userModel.isNotEmpty ? userModel : DefaultConfig.defaultLlmModel;
  }

  Future<void> _saveApiKeyConfig(String apiKey, String url, String model) async {
    // 如果全部为空，则删除配置
    if (apiKey.trim().isEmpty && url.trim().isEmpty && model.trim().isEmpty) {
      await SPUtil.remove('llmToken');
      await SPUtil.remove('llmUrl');
      await SPUtil.remove('llmModel');

      SmartDialog.showToast('自定义API Key配置已删除');

      // 重新检查LLM可用性并自动切换
      await widget.onRecheckLLMAvailabilityAndAutoSwitch();

      FlutterForegroundTask.sendDataToTask('reinitializeLLM');
      return;
    }

    // 如果部分为空，要求完整输入
    if (apiKey.trim().isEmpty || url.trim().isEmpty || model.trim().isEmpty) {
      SmartDialog.showToast('请输入完整的 API Key、URL 和 Model，或全部清空以删除配置');
      return;
    }

    if (!url.trim().startsWith('http')) {
      SmartDialog.showToast('请输入有效的 URL（需要以 http 或 https 开头）');
      return;
    }

    await SPUtil.setString('llmToken', apiKey.trim());
    await SPUtil.setString('llmUrl', url.trim());
    await SPUtil.setString('llmModel', model.trim());

    SmartDialog.showToast('API Key 配置已保存');

    // 重新检查LLM可用性并自动切换
    await widget.onRecheckLLMAvailabilityAndAutoSwitch();

    // 自动刷新LLM状态
    await widget.onForceReloadLLMStatus();

    FlutterForegroundTask.sendDataToTask('reinitializeLLM');
  }

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      contentPadding: EdgeInsets.fromLTRB(24.w, 20.h, 24.w, 0),
      actionsPadding: EdgeInsets.fromLTRB(16.w, 0, 16.w, 16.h),
      title: Text(
        S.of(context).importAPIKeyDialogTitle,
        style: TextStyle(fontSize: 16.sp, fontWeight: FontWeight.w600),
      ),
      content: Container(
        width: double.maxFinite,
        constraints: BoxConstraints(maxHeight: MediaQuery.of(context).size.height * 0.6),
        child: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              SizedBox(height: 8.h),
              TextField(
                controller: apiKeyController,
                style: TextStyle(fontSize: 14.sp),
                decoration: InputDecoration(
                  labelText: S.of(context).importAPIKeyDialogApiKeyTextFieldLabel,
                  labelStyle: TextStyle(fontSize: 12.sp),
                  hintText: S.of(context).importAPIKeyDialogApiKeyTextFieldHint,
                  hintStyle: TextStyle(fontSize: 9.sp, color: Colors.grey[600]),
                  border: OutlineInputBorder(borderRadius: BorderRadius.circular(8.r)),
                  contentPadding: EdgeInsets.symmetric(horizontal: 12.w, vertical: 12.h),
                ),
                obscureText: true,
              ),
              SizedBox(height: 20.h),
              TextField(
                controller: urlController,
                style: TextStyle(fontSize: 14.sp),
                decoration: InputDecoration(
                  labelText: S.of(context).importAPIKeyDialogApiURLTextFieldLabel,
                  labelStyle: TextStyle(fontSize: 12.sp),
                  hintText: S.of(context).importAPIKeyDialogApiURLTextFieldHint,
                  hintStyle: TextStyle(fontSize: 9.sp, color: Colors.grey[600]),
                  border: OutlineInputBorder(borderRadius: BorderRadius.circular(8.r)),
                  contentPadding: EdgeInsets.symmetric(horizontal: 12.w, vertical: 12.h),
                ),
              ),
              SizedBox(height: 20.h),
              TextField(
                controller: modelController,
                style: TextStyle(fontSize: 14.sp),
                decoration: InputDecoration(
                  labelText: S.of(context).importAPIKeyDialogModelTextFieldLabel,
                  labelStyle: TextStyle(fontSize: 12.sp),
                  hintText: S.of(context).importAPIKeyDialogModelTextFieldHint,
                  hintStyle: TextStyle(fontSize: 9.sp, color: Colors.grey[600]),
                  border: OutlineInputBorder(borderRadius: BorderRadius.circular(8.r)),
                  contentPadding: EdgeInsets.symmetric(horizontal: 12.w, vertical: 12.h),
                ),
              ),
              SizedBox(height: 16.h),
              Container(
                padding: EdgeInsets.all(12.w),
                decoration: BoxDecoration(
                  color: Colors.blue[50],
                  borderRadius: BorderRadius.circular(8.r),
                  border: Border.all(color: Colors.blue[100]!),
                ),
                child: Row(
                  children: [
                    Icon(Icons.info_outline, size: 16.sp, color: Colors.blue[600]),
                    SizedBox(width: 8.w),
                    Expanded(
                      child: Text(
                        S.of(context).importAPIKeyDialogTip,
                        style: TextStyle(fontSize: 11.sp, color: Colors.blue[700]),
                      ),
                    ),
                  ],
                ),
              ),
              SizedBox(height: 8.h),
            ],
          ),
        ),
      ),
      actions: [
        TextButton(
          onPressed: () {
            Navigator.of(context).pop();
          },
          child: Text(
            S.of(context).buttonCancel,
            style: TextStyle(fontSize: 14.sp, color: Colors.grey[600]),
          ),
        ),
        SizedBox(width: 8.w),
        ElevatedButton(
          onPressed: () async {
            await _saveApiKeyConfig(apiKeyController.text, urlController.text, modelController.text);
            if (context.mounted) {
              Navigator.of(context).pop();
            }
          },
          style: ElevatedButton.styleFrom(
            backgroundColor: Colors.blue,
            foregroundColor: Colors.white,
            padding: EdgeInsets.symmetric(horizontal: 20.w, vertical: 8.h),
            shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(6.r)),
          ),
          child: Text(S.of(context).buttonSave, style: TextStyle(fontSize: 14.sp)),
        ),
      ],
    );
  }
}
