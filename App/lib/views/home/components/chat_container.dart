import 'package:app/controllers/style_controller.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:provider/provider.dart';

class ChatContainer extends StatelessWidget {
  final String role;
  final double radius;
  final EdgeInsetsGeometry? margin;
  final EdgeInsetsGeometry? padding;
  final Widget? child;

  const ChatContainer({
    super.key,
    required this.role,
    this.radius = 12,
    this.margin,
    this.padding,
    this.child,
  });

  final BoxShadow darkBoxShadow = const BoxShadow(
    color: Color(0x1AA2EDF3),
    blurRadius: 20,
  );

  BoxDecoration getUserDecoration(bool isLightMode) {
    return BoxDecoration(
      gradient: isLightMode
          ? const LinearGradient(
              begin: Alignment.topCenter,
              end: Alignment.bottomCenter,
              colors: [
                Color(0xFFB1F4F9),
                Color(0xFF61D2DB),
              ],
            )
          : null,
      color: isLightMode ? null : const Color(0xFF29BBC6),
      boxShadow: [
        isLightMode
            ? const BoxShadow(
                color: Color(0x3D10939D),
                blurRadius: 8,
              )
            : darkBoxShadow
      ],
    );
  }

  BoxDecoration getAssistantLightDecoration(bool isLightMode) {
    return BoxDecoration(
      gradient: isLightMode
          ? const LinearGradient(
              begin: Alignment.topCenter,
              end: Alignment.bottomCenter,
              colors: [
                Color(0xFFEDFEFF),
                Colors.white,
              ],
            )
          : null,
      color: isLightMode ? null : const Color(0xFF333333),
      boxShadow: [
        isLightMode
            ? const BoxShadow(
                color: Color(0x1A29BBC6),
                blurRadius: 8,
              )
            : darkBoxShadow
      ],
    );
  }

  BoxDecoration getOtherDecoration(bool isLightMode) {
    return BoxDecoration(
      gradient: isLightMode
          ? const LinearGradient(
              colors: [
                Color(0xFFEDF3FF),
                Colors.white,
              ],
            )
          : null,
      color: isLightMode ? null : const Color(0xFF102733),
      boxShadow: [
        isLightMode
            ? const BoxShadow(
                color: Color(0x1A29BBC6),
                blurRadius: 8,
              )
            : darkBoxShadow
      ],
    );
  }

  BoxDecoration getBoxDecoration({
    required String role,
    required bool isLightMode,
  }) {
    BoxDecoration decoration;
    bool isUser = role == 'user';

    if (isUser) {
      decoration = getUserDecoration(isLightMode);
    } else {
      bool isAssistant = role == 'assistant';
      if (isAssistant) {
        decoration = getAssistantLightDecoration(isLightMode);
      } else {
        decoration = getOtherDecoration(isLightMode);
      }
    }
    return decoration;
  }

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    BoxDecoration decoration = getBoxDecoration(role: role, isLightMode: isLightMode);
    return Container(
      decoration: decoration.copyWith(
        borderRadius: BorderRadius.circular(radius),
        border: Border.all(
          width: 0.5,
          color: isLightMode ? Colors.white : Colors.black,
        ),
      ),
      margin: margin,
      padding: padding ?? EdgeInsets.symmetric(horizontal: 18.sp, vertical: 12.sp),
      child: child,
    );
  }
}
