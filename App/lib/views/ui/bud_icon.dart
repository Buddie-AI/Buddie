import 'package:app/controllers/style_controller.dart';
import 'package:app/utils/assets_util.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

class BudIcon extends StatelessWidget {
  final String icon;
  final double? size;

  const BudIcon({
    super.key,
    required this.icon,
    this.size,
  });

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    String path = AssetsUtil.getIconPath(
      mode: themeNotifier.mode,
      icon: icon,
    );
    return Image.asset(
      path,
      width: size,
      height: size,
    );
  }
}
