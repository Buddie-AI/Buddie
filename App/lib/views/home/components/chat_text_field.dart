import 'package:app/controllers/style_controller.dart';
import 'package:app/utils/assets_util.dart';
import 'package:app/views/ui/bud_icon.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:provider/provider.dart';

import '../../../generated/l10n.dart';

class ChatTextField extends StatelessWidget {
  final FocusNode? focusNode;
  final TextEditingController? controller;

  final ValueChanged<String>? onSubmitted;
  final GestureTapCallback? onTapKeyboard;
  final GestureTapCallback? onTapSend;

  const ChatTextField({
    super.key,
    this.focusNode,
    this.controller,
    this.onSubmitted,
    this.onTapKeyboard,
    this.onTapSend,
  });

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    return Container(
      decoration: BoxDecoration(
        borderRadius: BorderRadius.circular(6),
        color: isLightMode ? const Color(0x99FFFFFF) : const Color(0xFF333333),
      ),
      padding: EdgeInsets.only(left: 8.sp, right: 12.sp),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.end,
        children: [
          Padding(
            padding: const EdgeInsets.only(bottom: 13),
            child: InkWell(
              splashColor: Colors.transparent,
              highlightColor: Colors.transparent,
              onTap: onTapKeyboard,
              child: const BudIcon(icon: AssetsUtil.icon_keyboard, size: 22),
            ),
          ),
          SizedBox(width: 12.sp),
          Expanded(
            child: TextField(
              focusNode: focusNode,
              controller: controller,
              onSubmitted: onSubmitted,
              minLines: 1,
              maxLines: 9,
              textInputAction: TextInputAction.send,
              style: TextStyle(
                color: isLightMode ? Colors.black : Colors.white,
              ),
              decoration: InputDecoration(
                border: InputBorder.none,
                hintText: S.of(context).pageHomeTextFieldHint,
                hintStyle: TextStyle(
                  fontWeight: FontWeight.bold,
                  fontSize: 14,
                  color: isLightMode
                      ? const Color(0xFF999999)
                      : const Color(0x99FFFFFF),
                ),
              ),
            ),
          ),
          SizedBox(width: 12.sp),
          Padding(
            padding: const EdgeInsets.only(bottom: 8),
            child: InkWell(
              splashColor: Colors.transparent,
              highlightColor: Colors.transparent,
              onTap: onTapSend,
              child: const BudIcon(
                icon: AssetsUtil.icon_send_message,
                size: 32,
              ),
            ),
          ),
        ],
      ),
    );
  }
}
