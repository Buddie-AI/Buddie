import 'package:app/controllers/style_controller.dart';
import 'package:app/utils/assets_util.dart';
import 'package:app/views/home/components/chat_container.dart';
import 'package:app/views/ui/bud_icon.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:provider/provider.dart';

class ChatListTile extends StatelessWidget {
  final String role;
  final String text;
  final TextStyle? style;
  final EdgeInsetsGeometry? padding;
  final GestureLongPressCallback? onLongPress;

  const ChatListTile({
    super.key,
    required this.role,
    required this.text,
    this.style,
    this.padding,
    this.onLongPress,
  });

  static final double _iconSize = 24.sp;
  static final double _iconRight = 8.sp;
  static final double _containMarginHorizontal = 16.sp;
  static final double textWidthSpace =
      _iconSize + _iconRight + _containMarginHorizontal;

  static EdgeInsets _getChatContainerMargin(String role) {
    bool isUser = role == 'user';
    EdgeInsets margin = EdgeInsets.only(
      left: isUser ? (_iconSize + _iconRight + _containMarginHorizontal) : 0,
      right: isUser ? 0 : _containMarginHorizontal,
    );
    return margin;
  }

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    bool isUser = role == 'user';
    bool isAssistant = role == 'assistant';
    TextStyle textStyle =
        style ?? const TextStyle(fontWeight: FontWeight.bold, fontSize: 14);
    return Row(
      mainAxisAlignment: isUser
          ? MainAxisAlignment.end
          : MainAxisAlignment.start,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        if (!isUser)
          Padding(
            padding: EdgeInsets.only(right: _iconRight),
            child: BudIcon(
              icon: isAssistant
                  ? AssetsUtil.icon_chat_logo
                  : AssetsUtil.icon_chat_meeting,
              size: _iconSize,
            ),
          ),
        Flexible(
          child: GestureDetector(
            onLongPress: onLongPress,
            child: ChatContainer(
              role: role,
              margin: _getChatContainerMargin(role),
              padding:
                  padding ??
                  EdgeInsets.symmetric(horizontal: 18.sp, vertical: 12.sp),
              child: Text(
                text,
                style: textStyle.copyWith(
                  color: isLightMode
                      ? const Color(0xFF383838)
                      : isUser
                      ? const Color(0xE6FFFFFF)
                      : const Color(0xB3FFFFFF),
                ),
              ),
            ),
          ),
        ),
      ],
    );
  }
}
