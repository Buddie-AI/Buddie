import 'package:app/controllers/style_controller.dart';
import 'package:app/utils/assets_util.dart';
import 'package:app/utils/route_utils.dart';
import 'package:app/views/ui/app_background.dart';
import 'package:dots_indicator/dots_indicator.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:go_router/go_router.dart';
import 'package:provider/provider.dart';

import '../../controllers/record_controller.dart';
import '../../generated/l10n.dart';

class WelcomeScreen extends StatefulWidget {
  const WelcomeScreen({super.key, this.controller});

  final RecordScreenController? controller;

  @override
  State<WelcomeScreen> createState() => _WelcomeScreenState();
}

class _WelcomeScreenState extends State<WelcomeScreen> {
  bool _isLogoVisible = true;
  double _position = 0;

  @override
  void initState() {
    super.initState();
  }

  void _clickNextStep() async {
    if (mounted) {
      context.pushReplacementNamed(RouteName.home_chat);
    }
  }

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;

    TextStyle? textStyle = Theme.of(context).textTheme.displayMedium?.copyWith(
      color: isLightMode ? const Color(0xCC082E45) : Colors.white,
    );
    List<Widget> pages = [
      WelcomeText1(style: textStyle),
      WelcomeText2(style: textStyle),
      WelcomeText3(style: textStyle),
    ];
    return Scaffold(
      body: AppBackground(
        child: PageView(
          onPageChanged: (int index) {
            setState(() {
              _position = index.toDouble();
            });
          },
          children: [
            for (Widget text in pages)
              Column(
                children: [
                  SizedBox(height: 113.h),
                  SizedBox(
                    width: 208.r,
                    height: 208.r,
                    child: Center(
                      child: AnimatedOpacity(
                        opacity: _isLogoVisible ? 1.0 : 0.0,
                        duration: const Duration(seconds: 1),
                        child: Image.asset(
                          AssetsUtil.logo_hd,
                          width: 116.sp,
                          height: 116.sp,
                        ),
                      ),
                    ),
                  ),
                  SizedBox(height: 14.h),
                  Expanded(child: text),
                  if (_position < pages.length - 1)
                    DotsIndicator(
                      dotsCount: pages.length,
                      position: _position,
                      decorator: DotsDecorator(
                        color: isLightMode ? const Color(0x33000000) : const Color(0x33FFFFFF),
                        activeColor: isLightMode ? Colors.black : Colors.white,
                        size: Size.square(6.sp),
                        activeSize: Size(14.sp, 6.sp),
                        activeShape: RoundedRectangleBorder(
                          borderRadius: BorderRadius.circular(6.sp),
                        ),
                      ),
                    )
                  else
                    NextStepButton(onPressed: _clickNextStep),
                  SizedBox(height: 101.h),
                ],
              )
          ],
        ),
      ),
    );
  }
}

class WelcomeText1 extends StatelessWidget {
  final TextStyle? style;

  const WelcomeText1({super.key, this.style});

  @override
  Widget build(BuildContext context) {
    final adjustedStyle = style?.copyWith(
      height: 1.8,
      fontSize: 20.sp,
    ) ??
        TextStyle(
          fontSize: 20.sp,
          height: 1.8,
          color: Colors.black,
        );

    return RichText(
      textAlign: TextAlign.center,
      text: TextSpan(
        text: S.of(context).pageWelcomeText1,
        style: adjustedStyle,
        children: [
          TextSpan(
            text: S.of(context).pageWelcomeText2,
            style: style?.copyWith(color: const Color(0xFF29BBC6)),
          ),
        ],
      ),
    );
  }
}

class WelcomeText2 extends StatelessWidget {
  final TextStyle? style;

  const WelcomeText2({
    super.key,
    this.style,
  });

  @override
  Widget build(BuildContext context) {
    final adjustedStyle = style?.copyWith(
      height: 1.8,
      fontSize: 20.sp,
    ) ??
        TextStyle(
          fontSize: 20.sp,
          height: 1.8,
          color: Colors.black,
        );

    return RichText(
      textAlign: TextAlign.center,
      text: TextSpan(
        text: S.of(context).pageWelcomeText3,
        style: adjustedStyle,
      ),
    );
  }
}

class WelcomeText3 extends StatelessWidget {
  final TextStyle? style;

  const WelcomeText3({
    super.key,
    this.style,
  });

  @override
  Widget build(BuildContext context) {
    final adjustedStyle = style?.copyWith(
      height: 1.8,
      fontSize: 20.sp,
    ) ??
        TextStyle(
          fontSize: 20.sp,
          height: 1.8,
          color: Colors.black,
        );

    return RichText(
      textAlign: TextAlign.center,
      text: TextSpan(
        text: S.of(context).pageWelcomeText4,
        style: adjustedStyle,
      ),
    );
  }
}

class NextStepButton extends StatelessWidget {
  final VoidCallback onPressed;

  const NextStepButton({super.key, required this.onPressed});

  @override
  Widget build(BuildContext context) {
    return ElevatedButton.icon(
      onPressed: onPressed,
      style: ButtonStyle(
        backgroundColor: const WidgetStatePropertyAll(Color(0xFF29BBC6)),
        padding: WidgetStatePropertyAll(
          EdgeInsets.symmetric(horizontal: 28.w, vertical: 10.h),
        ),
      ),
      label: Text(
        S.of(context).buttonNextStep,
        style: TextStyle(
          fontSize: 20.sp,
          color: Colors.white,
        ),
      ),
      iconAlignment: IconAlignment.end,
    );
  }
}
