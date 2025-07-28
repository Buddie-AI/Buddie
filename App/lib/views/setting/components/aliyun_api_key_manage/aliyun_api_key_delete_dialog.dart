import 'package:flutter/material.dart';

import '../../../../generated/l10n.dart';

class AliyunApiKeyDeleteDialog extends StatelessWidget {
  const AliyunApiKeyDeleteDialog({super.key});

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      title: Text(S.of(context).aliyunAPIKeyDeleteDialogTitle),
      content: Text(S.of(context).aliyunAPIKeyDeleteDialogContent),
      actions: [
        TextButton(
          onPressed: () => Navigator.of(context).pop(false),
          child: Text(S.of(context).buttonCancel),
        ),
        TextButton(
          onPressed: () => Navigator.of(context).pop(true),
          child: Text(
            S.of(context).buttonDelete,
            style: const TextStyle(color: Colors.red),
          ),
        ),
      ],
    );
  }
}
