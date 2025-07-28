import 'package:app/controllers/style_controller.dart';
import 'package:app/utils/assets_util.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

class AppBackground extends StatelessWidget {
  final Widget? child;

  const AppBackground({
    super.key,
    this.child,
  });

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    String image_path = AssetsUtil.getImagePath(mode: themeNotifier.mode, image: AssetsUtil.appBackground);
    return Stack(
      children: [
        Container(
          width: MediaQuery.of(context).size.width,
          height: MediaQuery.of(context).size.height,
          decoration: BoxDecoration(
            image: DecorationImage(
              image: AssetImage(image_path),
              fit: BoxFit.cover,
            ),
          ),
        ),
        if (child != null) child!,
      ],
    );
  }
}
