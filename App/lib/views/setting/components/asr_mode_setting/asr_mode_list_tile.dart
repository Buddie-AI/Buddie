import 'package:app/models/asr_mode.dart';
import 'package:app/models/chat_mode.dart';
import 'package:flutter/material.dart';
import 'package:flutter_foreground_task/flutter_foreground_task.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';

import '../../../../utils/sp_util.dart';

class AsrModeListTile extends StatelessWidget {
  final ChatMode chatMode;
  final AsrMode asrMode;

  const AsrModeListTile({super.key, required this.chatMode, required this.asrMode});

  void _onClickItem(BuildContext context, String title) async {
    // 使用枚举值而不是中文名称
    await SPUtil.setString('asr_mode_${chatMode.name}', asrMode.storageKey);

    // 通知后台服务重新加载ASR配置
    FlutterForegroundTask.sendDataToTask({'action': 'reloadAsrConfig'});

    if (context.mounted) {
      Navigator.pop(context);
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('已设置${chatMode.name}的ASR模式为：$title'), duration: const Duration(seconds: 2)),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    String title = asrMode.getTitle(context);
    String description = asrMode.getDescription(context);
    return FutureBuilder<String?>(
      future: SPUtil.getString('asr_mode_${chatMode.name}'),
      builder: (context, snapshot) {
        // If not set, use default mode
        final defaultAsrModeKey = chatMode.defaultAsrMode.storageKey;
        final currentModeKey = asrMode.storageKey;

        final currentAsrMode = snapshot.data ?? defaultAsrModeKey;
        final isSelected = currentAsrMode == currentModeKey;

        return GestureDetector(
          onTap: () {
            _onClickItem(context, title);
          },
          child: Container(
            margin: EdgeInsets.only(bottom: 8.h),
            padding: EdgeInsets.symmetric(horizontal: 12.w, vertical: 8.h),
            decoration: BoxDecoration(
              color: isSelected ? Colors.blue.withValues(alpha: 0.1) : Colors.grey[50],
              borderRadius: BorderRadius.circular(8.r),
              border: Border.all(color: isSelected ? Colors.blue : Colors.grey[300]!, width: 1),
            ),
            child: Row(
              children: [
                Icon(Icons.info_outline, color: isSelected ? Colors.blue : Colors.grey[600], size: 20.sp),
                SizedBox(width: 8.w),
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        title,
                        style: TextStyle(
                          fontSize: 14.sp,
                          fontWeight: FontWeight.w500,
                          color: isSelected ? Colors.blue : Colors.grey[800],
                        ),
                      ),
                      SizedBox(height: 2.h),
                      Text(
                        description,
                        style: TextStyle(fontSize: 12.sp, color: Colors.grey[600]),
                      ),
                    ],
                  ),
                ),
                if (isSelected) Icon(Icons.check_circle, color: Colors.blue, size: 20.sp),
              ],
            ),
          ),
        );
      },
    );
  }
}
