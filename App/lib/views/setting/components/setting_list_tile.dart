import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:provider/provider.dart';

import '../../../controllers/style_controller.dart';
import '../../ui/bud_ui.dart';

class SettingListTile extends StatelessWidget {
  final String leading;
  final String title;
  final String? subtitle;
  final Widget? trailing;
  final GestureTapCallback? onTap;

  const SettingListTile({
    super.key,
    required this.leading,
    required this.title,
    this.subtitle,
    this.trailing,
    this.onTap,
  });

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    return InkWell(
      onTap: onTap,
      child: Container(
        height: 70.sp,
        padding: EdgeInsets.symmetric(vertical: 14.sp),
        child: Row(
          children: [
            BudCard(
              color: isLightMode ? const Color(0xFFEEEEEE) : const Color(0x1AEEEEEE),
              radius: 5.sp,
              padding: EdgeInsets.all(5.sp),
              child: BudIcon(icon: leading, size: 14.sp),
            ),
            SizedBox(width: 16.sp),
            Expanded(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    title,
                    maxLines: 1,
                    overflow: TextOverflow.ellipsis,
                    style: TextStyle(
                      fontWeight: FontWeight.bold,
                      fontSize: 16,
                      color: isLightMode ? Colors.black : Colors.white,
                    ),
                  ),
                  if (subtitle != null)
                    Text(
                      subtitle!,
                      maxLines: 1,
                      overflow: TextOverflow.ellipsis,
                      style: TextStyle(
                        fontWeight: FontWeight.bold,
                        fontSize: 12,
                        color: isLightMode ? const Color(0xFF999999) : const Color(0x99FFFFFF),
                      ),
                    ),
                ],
              ),
            ),
            if (trailing != null)
              Padding(
                padding: EdgeInsets.only(left: 16.sp),
                child: trailing!,
              ),
          ],
        ),
      ),
    );
  }
}
