// Just a simple toast wdiget with a simple fade animation, nothing more (valhalla)
import 'package:flutter/material.dart';

class ToastWidget extends StatefulWidget {
  const ToastWidget({
    super.key,
    required this.text,
    this.duration = const Duration(seconds: 3),
    this.transitionDuration = const Duration(milliseconds: 250),
  });
  final String text;
  final Duration duration;
  final Duration transitionDuration;

  @override
  State<ToastWidget> createState() => _ToastWidgetState();
}

class _ToastWidgetState extends State<ToastWidget>
    with SingleTickerProviderStateMixin {
  late final AnimationController opacity;

  @override
  void initState() {
    super.initState();
    opacity = AnimationController(
      vsync: this,
      duration: widget.transitionDuration,
    )..forward();

    final startFadeOutAt = widget.duration - widget.transitionDuration;

    Future.delayed(startFadeOutAt, opacity.reverse);
  }

  @override
  void dispose() {
    opacity.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return FadeTransition(
      opacity: opacity,
      child: Align(
        alignment: Alignment.bottomCenter,
        child: Container(
          decoration: BoxDecoration(
            color: Colors.black.withOpacity(.65),
            borderRadius: const BorderRadius.all(Radius.circular(32)),
          ),
          margin: EdgeInsets.only(
            left: 16,
            right: 16,
            bottom: MediaQuery.of(context).size.height * .125,
          ),
          padding: const EdgeInsets.symmetric(
            horizontal: 24,
            vertical: 14,
          ),
          child: Text(
            widget.text,
            style: const TextStyle(
              fontSize: 14,
              color: Colors.white,
              decoration: TextDecoration.none,
              fontWeight: FontWeight.normal,
            ),
          ),
        ),
      ),
    );
  }
}

extension ToastExtension on BuildContext {
  void showToast(
      String text, {
        Duration duration = const Duration(seconds: 3),
        Duration transitionDuration = const Duration(milliseconds: 250),
      }) {
    // Get the OverlayState
    final overlayState = Overlay.of(this);
    // Create an OverlayEntry with your custom widget
    final toast = OverlayEntry(
      builder: (_) => ToastWidget(
        text: text,
        transitionDuration: transitionDuration,
        duration: duration,
      ),
    );
    // then insert it to the overlay
    // this will show the toast widget on the screen
    overlayState!.insert(toast);
    // 3 secs later remove the toast from the stack
    // and this one will remove the toast from the screen
    Future.delayed(duration, toast.remove);
  }
}