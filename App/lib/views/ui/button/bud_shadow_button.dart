import 'package:app/controllers/style_controller.dart';
import 'package:app/views/ui/bud_icon.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:provider/provider.dart';

class BudShadowButton extends StatelessWidget {
  final String icon;
  final String? text;
  final GestureTapCallback? onTap;

  const BudShadowButton({
    super.key,
    required this.icon,
    this.text,
    this.onTap,
  });

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    return InkWell(
      splashColor: Colors.transparent,
      highlightColor: Colors.transparent,
      onTap: onTap,
      child: Container(
        decoration: BoxDecoration(
          shape: text == null ? BoxShape.circle : BoxShape.rectangle,
          color: isLightMode ? const Color(0x99FFFFFF) : const Color(0xFF333333),
          borderRadius: text != null ? BorderRadius.circular(46) : null,
          boxShadow: isLightMode
              ? const [
                  BoxShadow(
                    color: Color.fromRGBO(198, 229, 255, 0.4),
                    offset: Offset(0, 17),
                    blurRadius: 29,
                  ),
                  // inset
                  BoxShadow(
                    color: Color.fromRGBO(255, 255, 255, 0.55),
                    offset: Offset(-2, 2),
                    blurRadius: 4,
                  ),
                ]
              : null,
        ),
        padding: EdgeInsets.symmetric(horizontal: 14.sp, vertical: 12.sp),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.center,
          mainAxisSize: MainAxisSize.min,
          children: [
            BudIcon(
              icon: icon,
              size: 22.sp,
            ),
            if (text != null)
              Padding(
                padding: EdgeInsets.only(left: 6.sp),
                child: Text(
                  text!,
                  maxLines: 1,
                  overflow: TextOverflow.ellipsis,
                  style: TextStyle(
                    fontWeight: FontWeight.bold,
                    fontSize: 16.sp,
                    color: isLightMode ? const Color(0xFF333333) : Colors.white,
                  ),
                ),
              ),
          ],
        ),
      ),
    );
  }
}
