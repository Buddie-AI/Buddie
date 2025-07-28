import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';

class BudSwitch extends StatelessWidget {
  final bool value;
  final ValueChanged<bool> onChanged;

  const BudSwitch({
    super.key,
    required this.value,
    required this.onChanged,
  });

  @override
  Widget build(BuildContext context) {
    Color activeTrackColor = const Color(0xFFE0FCFE);
    Color inactiveTrackColor = const Color(0x29787880);
    return Switch(
      value: value,
      onChanged: onChanged,
      activeColor: const Color(0xFF29BBC6),
      activeTrackColor: activeTrackColor,
      inactiveThumbColor: Colors.white,
      inactiveTrackColor: inactiveTrackColor,
      trackOutlineColor: WidgetStateProperty.resolveWith((Set<WidgetState> states) {
        return Colors.transparent;
      }),
      padding: EdgeInsets.all(2.sp),
    );
  }
}
