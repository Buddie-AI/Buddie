import 'package:app/controllers/style_controller.dart';
import 'package:app/extension/media_query_data_extension.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:provider/provider.dart';

class BudBottomSheet extends StatelessWidget {
  final Widget? child;
  final Color? color;

  const BudBottomSheet({
    super.key,
    this.child,
    this.color,
  });

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    return Container(
      padding: EdgeInsets.only(
        top: 16.sp,
        left: 16.sp,
        right: 16.sp,
        bottom: MediaQuery.of(context).fixedBottom,
      ),
      decoration: BoxDecoration(
        color: color ?? (isLightMode ? Colors.white : const Color(0xFF333333)),
        borderRadius: BorderRadius.vertical(
          top: Radius.circular(8.sp),
        ),
      ),
      child: child,
    );
  }
}
