import 'package:app/controllers/style_controller.dart';
import 'package:flutter/widgets.dart';
import 'package:provider/provider.dart';

extension ContextExtension on BuildContext {
  bool get isLightMode {
    final themeNotifier = Provider.of<ThemeNotifier>(this);
    return themeNotifier.mode == Mode.light;
  }
}