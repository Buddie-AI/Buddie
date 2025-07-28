import 'package:app/controllers/style_controller.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:provider/provider.dart';

class BudButton extends StatelessWidget {
  final double? height;
  final Color? backgroundColor;
  final Color? color;
  final String text;

  final GestureTapCallback? onTap;

  const BudButton({
    super.key,
    this.height,
    this.backgroundColor,
    this.color,
    required this.text,
    this.onTap,
  });

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    Color defaultBgColor = isLightMode ? const Color(0xFFF3F4F8) : const Color(0x33F3F4F8);
    Color defaultColor = isLightMode ? const Color(0xFF1E1B33) : Colors.white;
    return InkWell(
      splashColor: Colors.transparent,
      highlightColor: Colors.transparent,
      onTap: onTap,
      child: Container(
        height: height ?? 48.sp,
        constraints: const BoxConstraints(
          minWidth: double.infinity,
        ),
        decoration: BoxDecoration(
          color: backgroundColor ?? defaultBgColor,
          borderRadius: BorderRadius.circular(8.sp),
        ),
        child: Center(
          child: Text(
            text,
            style: TextStyle(
              fontWeight: FontWeight.w400,
              fontSize: 16,
              color: color ?? defaultColor,
            ),
          ),
        ),
      ),
    );
  }
}
