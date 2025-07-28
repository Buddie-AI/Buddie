import 'package:app/controllers/style_controller.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

class BudShadowContainer extends StatelessWidget {
  final double radius;
  final Color? color;
  final EdgeInsetsGeometry? padding;

  final Widget? child;

  const BudShadowContainer({
    super.key,
    this.radius = 12,
    this.color,
    this.padding,
    this.child,
  });

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    final Color defaultBgColor = isLightMode ? Colors.white : const Color(0x33FFFFFF);
    return Container(
      decoration: BoxDecoration(
        borderRadius: BorderRadius.circular(radius),
        color: color ?? defaultBgColor,
        boxShadow: [
          if (isLightMode)
            const BoxShadow(
              color: Color(0x331ABAC6),
              blurRadius: 50,
            )
        ],
      ),
      padding: padding,
      child: child,
    );
  }
}
