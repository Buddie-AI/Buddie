import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';

import '../../generated/l10n.dart';

class DeviceCard extends StatelessWidget {
  final String deviceName;
  final String text;
  final VoidCallback onConfirm;
  final VoidCallback onCancel;
  final String? confirmText;
  final String? cancelText;

  const DeviceCard({
    super.key,
    required this.deviceName,
    required this.text,
    required this.onConfirm,
    required this.onCancel,
    this.confirmText,
    this.cancelText,
  });

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          Card(
            margin: EdgeInsets.all(16.sp),
            child: Padding(
              padding: EdgeInsets.all(16.sp),
              child: Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  Text(
                    text,
                    style: TextStyle(
                      fontSize: 18.sp,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  SizedBox(height: 8.sp),
                  ListTile(
                    title: Text(deviceName),
                    trailing: const Icon(Icons.earbuds),
                  ),
                ],
              ),
            ),
          ),
          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              SizedBox(width: 16.sp),
              ElevatedButton(
                onPressed: onCancel,
                child: Text(cancelText ?? S.of(context).buttonCancel),
              ),
              const Spacer(),
              ElevatedButton(
                onPressed: onConfirm,
                child: Text(confirmText ?? S.of(context).buttonConfirm),
              ),
              SizedBox(width: 16.sp),
            ],
          ),
        ],
      ),
    );
  }
}
