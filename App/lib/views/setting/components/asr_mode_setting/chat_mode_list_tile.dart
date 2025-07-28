import 'package:app/views/setting/components/asr_mode_setting/asr_mode_list_tile.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';

import '../../../../models/asr_mode.dart';
import '../../../../models/chat_mode.dart';

class ChatModeListTile extends StatelessWidget {
  final ChatMode chatMode;

  const ChatModeListTile({super.key, required this.chatMode});

  @override
  Widget build(BuildContext context) {
    String title = chatMode.getTitle(context);
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          title,
          style: TextStyle(fontSize: 16.sp, fontWeight: FontWeight.w600, color: Colors.grey[700]),
        ),
        SizedBox(height: 8.h),
        AsrModeListTile(chatMode: chatMode, asrMode: AsrMode.cloudStreaming),
        AsrModeListTile(chatMode: chatMode, asrMode: AsrMode.cloudOnline),
        AsrModeListTile(chatMode: chatMode, asrMode: AsrMode.localOffline),
        SizedBox(height: 16.h),
      ],
    );
  }
}
