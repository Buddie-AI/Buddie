import 'package:app/controllers/style_controller.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:provider/provider.dart';

class BudTabBar extends StatelessWidget {
  final List<String> tabs;
  final TabController? controller;

  const BudTabBar({
    super.key,
    required this.tabs,
    this.controller,
  });

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    double fontSize = 16;
    return TabBar(
      controller: controller,
      labelColor: isLightMode ? Colors.black : Colors.white,
      labelStyle: TextStyle(
        fontSize: fontSize,
        fontWeight: FontWeight.bold,
      ),
      unselectedLabelColor: isLightMode ? const Color(0x99000000) : const Color(0x99FFFFFF),
      unselectedLabelStyle: TextStyle(
        fontSize: fontSize,
        fontWeight: FontWeight.w400,
      ),
      indicator: const BoxDecoration(
        gradient: LinearGradient(
          begin: Alignment.topLeft,
          end: Alignment.bottomRight,
          colors: [
            Color(0xFF76A1F1),
            Color(0xFF52ABF4),
            Color(0xFF29BBC6),
          ],
          stops: [0.0, 0.5, 1.0],
          transform: GradientRotation(149 * (3.14159 / 180)),
        ),
      ),
      indicatorPadding: EdgeInsets.only(
        top: 17.sp,
      ),
      dividerColor: Colors.transparent,
      tabs: [
        for (String text in tabs)
          Text(
            text,
          ),
      ],
    );
  }
}
