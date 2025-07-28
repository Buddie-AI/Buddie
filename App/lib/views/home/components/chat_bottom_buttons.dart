import 'package:app/utils/assets_util.dart';
import 'package:app/views/home/components/animated_scale_button.dart';
import 'package:app/views/ui/button/bud_shadow_button.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';

import '../../../generated/l10n.dart';

class ChatBottomButtons extends StatelessWidget {
  final GestureTapCallback? onTapLeft;
  final GestureTapCallback? onTapHelp;
  final GestureTapCallback? onTapRight;
  final bool isRecording;
  final ValueNotifier<bool> isSpeakValueNotifier;

  const ChatBottomButtons({
    super.key,
    this.onTapLeft,
    this.onTapHelp,
    this.onTapRight,
    required this.isRecording,
    required this.isSpeakValueNotifier,
  });

  static double height = 52.sp;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      height: height,
      child: Row(
        mainAxisSize: MainAxisSize.min,
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          ValueListenableBuilder(
            valueListenable: isSpeakValueNotifier,
            builder: (context, bool isSpeaking, child) {
              return BreathingAnimationWidget(
                isAnimating: isSpeaking && isRecording,
                child: child!,
              );
            },
            child: BudShadowButton(
              onTap: onTapLeft,
              icon: isRecording
                  ? AssetsUtil.icon_btn_recording_mic
                  : AssetsUtil.icon_btn_stop_recording_mic,
            ),
          ),
          SizedBox(width: 22.sp),
          Expanded(
            child: BudShadowButton(
              onTap: onTapHelp,
              icon: AssetsUtil.icon_btn_logo,
              text: S.of(context).buttonHelpMe,
            ),
          ),
          SizedBox(width: 22.sp),
          BudShadowButton(onTap: onTapRight, icon: AssetsUtil.icon_btn_journal),
        ],
      ),
    );
  }
}
