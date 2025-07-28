import 'package:flutter/material.dart';

class BreathingAnimationWidget extends StatefulWidget {
  final Widget child;
  final bool isAnimating;
  final Duration duration;
  final double minScale;
  final double maxScale;
  final Curve curve;

  const BreathingAnimationWidget({
    Key? key,
    required this.child,
    this.isAnimating = false,
    this.duration = const Duration(milliseconds: 800),
    this.minScale = 0.9,
    this.maxScale = 1.1,
    this.curve = Curves.easeInOut,
  }) : super(key: key);

  @override
  State<BreathingAnimationWidget> createState() => _BreathingAnimationWidgetState();
}

class _BreathingAnimationWidgetState extends State<BreathingAnimationWidget>
    with SingleTickerProviderStateMixin {
  late AnimationController _controller;
  late Animation<double> _animation;

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      vsync: this,
      duration: widget.duration,
    );
    _setupAnimation();
    _updateAnimationState();
  }

  @override
  void didUpdateWidget(BreathingAnimationWidget oldWidget) {
    super.didUpdateWidget(oldWidget);

    if (widget.duration != oldWidget.duration) {
      _controller.duration = widget.duration;
    }

    if (widget.minScale != oldWidget.minScale ||
        widget.maxScale != oldWidget.maxScale ||
        widget.curve != oldWidget.curve) {
      _setupAnimation();
    }

    if (widget.isAnimating != oldWidget.isAnimating) {
      _updateAnimationState();
    }
  }

  void _setupAnimation() {
    _animation = TweenSequence<double>([
      TweenSequenceItem(
        tween: Tween<double>(
          begin: widget.minScale,
          end: widget.maxScale,
        ).chain(CurveTween(curve: widget.curve)),
        weight: 50,
      ),
      TweenSequenceItem(
        tween: Tween<double>(
          begin: widget.maxScale,
          end: widget.minScale,
        ).chain(CurveTween(curve: widget.curve)),
        weight: 50,
      ),
    ]).animate(_controller);
  }

  void _updateAnimationState() {
    if (widget.isAnimating) {
      _controller.repeat();
    } else {
      _controller.stop();
      _controller.animateTo(0, duration: const Duration(milliseconds: 300));
    }
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedBuilder(
      animation: _animation,
      builder: (context, child) {
        return Transform.scale(
          scale: _animation.value,
          child: widget.child,
        );
      },
    );
  }
}