import 'package:flutter/material.dart';

import '../../generated/l10n.dart';

class ConnectionBottomBar extends StatelessWidget {
  final VoidCallback onConfirm;
  final VoidCallback onCancel;
  final String? confirmText;
  final String? cancelText;

  const ConnectionBottomBar({
    super.key,
    required this.onConfirm,
    required this.onCancel,
    this.confirmText,
    this.cancelText,
  });

  @override
  Widget build(BuildContext context) {
    return BottomAppBar(
      child: Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          ElevatedButton(
            onPressed: onCancel,
            child: Text(cancelText ?? S.of(context).buttonCancel),
          ),
          const Spacer(),
          ElevatedButton(
            onPressed: onConfirm,
            child: Text(confirmText ?? S.of(context).buttonConfirm),
          ),
        ],
      ),
    );
  }
}
