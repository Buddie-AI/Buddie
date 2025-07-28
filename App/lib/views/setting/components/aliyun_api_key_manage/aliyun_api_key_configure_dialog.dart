import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';

import '../../../../generated/l10n.dart';

class AliyunApiKeyConfigureDialog extends StatefulWidget {
  final String? apiKey;
  final Future<void> Function(String) onSaveAlibabaApiKey;

  const AliyunApiKeyConfigureDialog({
    super.key,
    this.apiKey,
    required this.onSaveAlibabaApiKey,
  });

  @override
  State<AliyunApiKeyConfigureDialog> createState() =>
      _AliyunApiKeyConfigureDialogState();
}

class _AliyunApiKeyConfigureDialogState
    extends State<AliyunApiKeyConfigureDialog> {
  final TextEditingController apiKeyController = TextEditingController();

  @override
  void dispose() {
    apiKeyController.dispose();
    super.dispose();
  }

  @override
  void initState() {
    super.initState();
    if (widget.apiKey != null) {
      apiKeyController.text = widget.apiKey!;
    }
  }

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      contentPadding: EdgeInsets.fromLTRB(24.w, 20.h, 24.w, 0),
      actionsPadding: EdgeInsets.fromLTRB(16.w, 0, 16.w, 16.h),
      title: Text(
        S.of(context).aliyunAPIKeyConfigureDialogTitle,
        style: TextStyle(fontSize: 16.sp, fontWeight: FontWeight.w600),
      ),
      content: Container(
        width: double.maxFinite,
        constraints: BoxConstraints(
          maxHeight: MediaQuery.of(context).size.height * 0.5,
        ),
        child: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              SizedBox(height: 8.h),
              Text(
                S.of(context).aliyunAPIKeyConfigureDialogSubtitle,
                style: TextStyle(fontSize: 14.sp, color: Colors.grey[600]),
              ),
              SizedBox(height: 16.h),
              TextField(
                controller: apiKeyController,
                style: TextStyle(fontSize: 14.sp),
                decoration: InputDecoration(
                  labelText: S
                      .of(context)
                      .aliyunAPIKeyConfigureDialogTextFieldLabel,
                  labelStyle: TextStyle(fontSize: 12.sp),
                  hintText: S
                      .of(context)
                      .aliyunAPIKeyConfigureDialogTextFieldHint,
                  hintStyle: TextStyle(
                    fontSize: 12.sp,
                    color: Colors.grey[600],
                  ),
                  border: OutlineInputBorder(
                    borderRadius: BorderRadius.circular(8.r),
                  ),
                  contentPadding: EdgeInsets.symmetric(
                    horizontal: 12.w,
                    vertical: 12.h,
                  ),
                ),
                obscureText: true,
              ),
              SizedBox(height: 16.h),
              Container(
                padding: EdgeInsets.all(12.w),
                decoration: BoxDecoration(
                  color: Colors.blue.withValues(alpha: 0.1),
                  borderRadius: BorderRadius.circular(8.r),
                ),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      S.of(context).aliyunAPIKeyConfigureDialogTip,
                      style: TextStyle(
                        fontSize: 12.sp,
                        fontWeight: FontWeight.w500,
                      ),
                    ),
                    SizedBox(height: 4.h),
                    Text(
                      S.of(context).aliyunAPIKeyConfigureDialogTipContent,
                      style: TextStyle(
                        fontSize: 11.sp,
                        color: Colors.grey[700],
                      ),
                    ),
                  ],
                ),
              ),
            ],
          ),
        ),
      ),
      actions: [
        TextButton(
          onPressed: () => Navigator.of(context).pop(),
          child: Text(
            S.of(context).buttonCancel,
            style: TextStyle(fontSize: 14.sp, color: Colors.grey[600]),
          ),
        ),
        SizedBox(width: 8.w),
        ElevatedButton(
          onPressed: () async {
            await widget.onSaveAlibabaApiKey(apiKeyController.text);
            if (context.mounted) {
              Navigator.of(context).pop();
            }
          },
          style: ElevatedButton.styleFrom(
            backgroundColor: Colors.blue,
            foregroundColor: Colors.white,
            padding: EdgeInsets.symmetric(horizontal: 20.w, vertical: 8.h),
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(6.r),
            ),
          ),
          child: Text(
            S.of(context).buttonSave,
            style: TextStyle(fontSize: 14.sp),
          ),
        ),
      ],
    );
  }
}
