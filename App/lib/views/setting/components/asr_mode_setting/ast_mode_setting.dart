import 'package:app/views/setting/components/asr_mode_setting/chat_mode_list_tile.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';

import '../../../../generated/l10n.dart';
import '../../../../models/chat_mode.dart';

class AstModeSetting extends StatefulWidget {
  const AstModeSetting({super.key});

  @override
  State<AstModeSetting> createState() => _AstModeSettingState();
}

class _AstModeSettingState extends State<AstModeSetting> {
  @override
  Widget build(BuildContext context) {
    return Container(
      height: MediaQuery.of(context).size.height * 0.7,
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
              S.of(context).asrModeSetTitle,
              style: TextStyle(fontSize: 18.sp, fontWeight: FontWeight.bold),
            ),
          ),
          Expanded(
            child: SingleChildScrollView(
              child: Padding(
                padding: EdgeInsets.symmetric(horizontal: 16.sp),
                child: const Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    ChatModeListTile(chatMode: ChatMode.defaultMode),
                    ChatModeListTile(chatMode: ChatMode.dialogMode),
                    ChatModeListTile(chatMode: ChatMode.meetingMode),
                  ],
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }
}
