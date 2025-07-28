import 'package:flutter/material.dart';

class BudCard extends StatelessWidget {
  final double radius;
  final Color? color;
  final EdgeInsetsGeometry? margin;
  final EdgeInsetsGeometry? padding;
  final Widget? child;

  const BudCard({
    super.key,
    this.radius = 0,
    this.color,
    this.margin,
    this.padding,
    this.child,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: BoxDecoration(
        color: color,
        borderRadius: BorderRadius.circular(radius),
      ),
      margin: margin,
      padding: padding,
      child: child,
    );
  }
}
