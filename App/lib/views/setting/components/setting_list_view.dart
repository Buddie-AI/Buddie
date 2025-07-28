
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:provider/provider.dart';

import '../../../controllers/style_controller.dart';
import '../../ui/bud_ui.dart';

class SectionListView extends StatelessWidget {
  final List<Widget> children;

  const SectionListView({super.key, required this.children});

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    return BudCard(
      color: isLightMode ? const Color(0xFFFAFAFA) : const Color(0x33FFFFFF),
      radius: 8.sp,
      child: ListView.separated(
        shrinkWrap: true,
        physics: const NeverScrollableScrollPhysics(),
        padding: EdgeInsets.symmetric(horizontal: 32.sp),
        itemCount: children.length,
        separatorBuilder: (_, index) =>
        const Divider(height: 1, color: Color.fromRGBO(0, 0, 0, 0.1)),
        itemBuilder: (_, index) => children[index],
      ),
    );
  }
}