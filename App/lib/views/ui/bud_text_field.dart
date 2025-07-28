import 'package:app/controllers/style_controller.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:provider/provider.dart';

class BudTextField extends StatelessWidget {
  final TextEditingController? controller;
  final ValueChanged<String>? onSubmitted;
  final double? height;
  final Color? backgroundColor;

  final String? hintText;

  const BudTextField({
    super.key,
    this.controller,
    this.onSubmitted,
    this.height,
    this.backgroundColor,
    this.hintText,
  });

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    Color defaultBgColor = isLightMode ? const Color(0xFFF0F0F0) : const Color(0x1AFFFFFF);
    Color hintColor = isLightMode ? const Color(0xFF999999) : const Color(0x99FFFFFF);
    return Container(
      height: height,
      decoration: BoxDecoration(
        color: backgroundColor ?? defaultBgColor,
        borderRadius: BorderRadius.circular(4.sp),
      ),
      padding: EdgeInsets.symmetric(horizontal: 16.sp, vertical: 12.sp),
      child: TextField(
        controller: controller,
        onSubmitted: onSubmitted,
        decoration: InputDecoration(
          border: InputBorder.none,
          hintText: hintText,
          hintStyle: TextStyle(
            fontWeight: FontWeight.w400,
            fontSize: 14,
            color: hintColor,
          ),
        ),
      ),
    );
  }
}
