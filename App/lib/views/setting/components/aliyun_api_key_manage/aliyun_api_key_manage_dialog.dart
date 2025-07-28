import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';

import '../../../../generated/l10n.dart';

class AliyunApiKeyManageDialog extends StatelessWidget {
  final bool hasApiKey;
  final String? apiKey;
  final VoidCallback onPressedDeleteAlibabaApiKey;
  final VoidCallback onPressedShowAlibabaApiKeyDialog;

  const AliyunApiKeyManageDialog({
    super.key,
    required this.hasApiKey,
    this.apiKey,
    required this.onPressedDeleteAlibabaApiKey,
    required this.onPressedShowAlibabaApiKeyDialog,
  });

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      contentPadding: EdgeInsets.fromLTRB(24.w, 20.h, 24.w, 0),
      actionsPadding: EdgeInsets.fromLTRB(16.w, 0, 16.w, 16.h),
      title: Text(
        S.of(context).aliyunAPIKeyManageTitle,
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
              if (hasApiKey) ...[
                Container(
                  padding: EdgeInsets.all(12.w),
                  decoration: BoxDecoration(
                    color: Colors.green.withValues(alpha: 0.1),
                    borderRadius: BorderRadius.circular(8.r),
                    border: Border.all(
                      color: Colors.green.withValues(alpha: 0.3),
                    ),
                  ),
                  child: Row(
                    children: [
                      Icon(
                        Icons.check_circle,
                        color: Colors.green,
                        size: 20.sp,
                      ),
                      SizedBox(width: 8.w),
                      Expanded(
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            Text(
                              S.of(context).aliyunAPIKeyManageKeyConfigured,
                              style: TextStyle(
                                fontSize: 14.sp,
                                fontWeight: FontWeight.w500,
                                color: Colors.green[700],
                              ),
                            ),
                            SizedBox(height: 4.h),
                            Text(
                              '${S.of(context).aliyunAPIKeyManageCurrent}: ${apiKey ?? ''}',
                              style: TextStyle(
                                fontSize: 12.sp,
                                color: Colors.grey[700],
                              ),
                            ),
                          ],
                        ),
                      ),
                    ],
                  ),
                ),
                SizedBox(height: 16.h),
              ] else ...[
                Container(
                  padding: EdgeInsets.all(12.w),
                  decoration: BoxDecoration(
                    color: Colors.orange.withValues(alpha: 0.1),
                    borderRadius: BorderRadius.circular(8.r),
                    border: Border.all(
                      color: Colors.orange.withValues(alpha: 0.3),
                    ),
                  ),
                  child: Row(
                    children: [
                      Icon(Icons.key_off, color: Colors.orange, size: 20.sp),
                      SizedBox(width: 8.w),
                      Expanded(
                        child: Text(
                          S.of(context).aliyunAPIKeyManageKeyUnset,
                          style: TextStyle(
                            fontSize: 14.sp,
                            color: Colors.orange[700],
                          ),
                        ),
                      ),
                    ],
                  ),
                ),
                SizedBox(height: 16.h),
              ],
              Text(
                S.of(context).aliyunAPIKeyManageFunctionDescription,
                style: TextStyle(fontSize: 14.sp, fontWeight: FontWeight.w500),
              ),
              SizedBox(height: 8.h),
              Text(
                S.of(context).aliyunAPIKeyManageFunctionDescriptionContent,
                style: TextStyle(fontSize: 12.sp, color: Colors.grey[600]),
              ),
            ],
          ),
        ),
      ),
      actions: [
        if (hasApiKey) ...[
          TextButton(
            onPressed: onPressedDeleteAlibabaApiKey,
            child: Text(
              S.of(context).buttonDelete,
              style: TextStyle(fontSize: 14.sp, color: Colors.red),
            ),
          ),
          SizedBox(width: 8.w),
        ],
        TextButton(
          onPressed: () => Navigator.of(context).pop(),
          child: Text(
            hasApiKey ? S.of(context).buttonClose : S.of(context).buttonCancel,
            style: TextStyle(fontSize: 14.sp, color: Colors.grey[600]),
          ),
        ),
        SizedBox(width: 8.w),
        ElevatedButton(
          onPressed: () {
            Navigator.of(context).pop();
            onPressedShowAlibabaApiKeyDialog();
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
            hasApiKey
                ? S.of(context).buttonModify
                : S.of(context).buttonConfigure,
            style: TextStyle(fontSize: 14.sp),
          ),
        ),
      ],
    );
  }
}
