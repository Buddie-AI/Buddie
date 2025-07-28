import 'package:app/controllers/style_controller.dart';
import 'package:app/utils/assets_util.dart';
import 'package:app/views/ui/bud_icon.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:provider/provider.dart';

class BudSearchBar extends StatelessWidget {
  final FocusNode? focusNode;
  final String? leadingIcon;
  final GestureTapCallback? onTapLeading;
  final String? hintText;
  final ValueChanged<String>? onSubmitted;
  final String? trailingIcon;
  final TextEditingController? controller;

  const BudSearchBar({
    super.key,
    this.focusNode,
    this.leadingIcon,
    this.onTapLeading,
    this.hintText,
    this.onSubmitted,
    this.trailingIcon,
    this.controller,
  });

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    Mode mode = themeNotifier.mode;
    bool isLightModel = mode == Mode.light;
    double iconSize = 20.sp;
    return Container(
      height: 46.sp,
      padding: EdgeInsets.all(12.sp),
      decoration: BoxDecoration(
        color: isLightModel ? Colors.white : const Color(0xFF333333),
        borderRadius: BorderRadius.circular(6.sp),
        boxShadow: const [
          BoxShadow(
            color: Color.fromRGBO(42, 154, 202, 0.2),
            blurRadius: 16,
          ),
        ],
      ),
      child: Row(
        children: [
          InkWell(
            splashColor: Colors.transparent,
            highlightColor: Colors.transparent,
            onTap: onTapLeading,
            child: BudIcon(
              icon: leadingIcon ?? AssetsUtil.icon_search,
              size: iconSize,
            ),
          ),
          Padding(
            padding: EdgeInsets.symmetric(horizontal: 12.sp),
            child: Image.asset(
              AssetsUtil.getIconPath(mode: mode, icon: AssetsUtil.icon_search_divider),
              width: 1.sp,
              height: 16.sp,
            ),
          ),
          Expanded(
            child: TextField(
              controller: controller,
              focusNode: focusNode,
              style: TextStyle(color: isLightModel ? Colors.black : Colors.white),
              decoration: InputDecoration(
                border: InputBorder.none,
                isCollapsed: true,
                hintText: hintText,
                hintStyle: TextStyle(
                  fontWeight: FontWeight.normal,
                  color: isLightModel ? const Color(0x66000000) : const Color(0x66FFFFFF),
                ),
              ),
              onSubmitted: (String query) {
                onSubmitted?.call(query);
              },
            ),
          ),
          if (trailingIcon != null)
            BudIcon(
              icon: trailingIcon!,
              size: iconSize,
            )
        ],
      ),
    );
  }
}
