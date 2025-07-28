import 'package:app/controllers/style_controller.dart';
import 'package:app/utils/assets_util.dart';
import 'package:app/views/ui/app_background.dart';
import 'package:app/views/ui/bud_icon.dart';
import 'package:extra_hittest_area/extra_hittest_area.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:go_router/go_router.dart';
import 'package:provider/provider.dart';

class BudScaffold extends StatelessWidget {
  final bool automaticallyImplyLeading;
  final Widget? leading;
  final String? title;
  final List<Widget>? actions;
  final Widget? body;
  final Widget? floatingActionButton;
  final Widget? bottomNavigationBar;
  final Widget? bottomSheet;

  const BudScaffold({
    super.key,
    this.automaticallyImplyLeading = true,
    this.leading,
    this.title,
    this.actions,
    this.body,
    this.floatingActionButton,
    this.bottomNavigationBar,
    this.bottomSheet,
  });

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    return Scaffold(
      resizeToAvoidBottomInset: false,
      extendBodyBehindAppBar: true,
      appBar: AppBar(
        automaticallyImplyLeading: false,
        elevation: 0,
        backgroundColor: Colors.transparent,
        iconTheme: IconThemeData(
          color: isLightMode ? const Color(0xFF111111) : Colors.white,
        ),
        title: RowHitTestWithoutSizeLimit(
          children: [
            if (automaticallyImplyLeading)
              GestureDetectorHitTestWithoutSizeLimit(
                // debugHitTestAreaColor: Colors.red.withOpacity(0.5),
                extraHitTestArea: EdgeInsets.all(20.sp),
                onTap: () => context.pop(),
                child: Padding(
                  padding: EdgeInsets.only(right: 4.sp),
                  child: BudIcon(
                    icon: AssetsUtil.icon_arrow_back,
                    size: 20.sp,
                  ),
                ),
              ),
            if (leading != null)
              Padding(
                padding: EdgeInsets.only(right: 4.sp),
                child: leading!,
              ),
            if (title != null)
              Text(
                title!,
                style: TextStyle(
                  fontWeight: FontWeight.bold,
                  fontSize: 18,
                  color: isLightMode ? Colors.black : Colors.white,
                ),
              )
          ],
        ),
        actions: actions,
      ),
      body: AppBackground(
        child: Padding(
          padding: EdgeInsets.only(
            top: MediaQuery.of(context).padding.top + kToolbarHeight,
          ),
          child: body,
        ),
      ),
      floatingActionButtonLocation: FloatingActionButtonLocation.centerDocked,
      floatingActionButton: floatingActionButton,
      bottomNavigationBar: bottomNavigationBar,
      bottomSheet: bottomSheet,
    );
  }
}
