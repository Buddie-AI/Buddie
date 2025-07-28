import 'dart:io';
import 'package:flutter/material.dart';
import 'package:share_plus/share_plus.dart';

class SharePlusUtil {
  static Future<bool> shareFile({
    required String path,
    String? subject,
    String? text,
    List<String>? fileNameOverrides,
  }) async {
    File file = File(path);
    bool exists = await file.exists();
    if (exists) {
      XFile? xFile = XFile(
        path,
        // mimeType: 'audio/x-wav',
      );

      try {
        ShareResult shareResult = await Share.shareXFiles(
          <XFile>[xFile],
          subject: subject,
          text: text,
          fileNameOverrides: fileNameOverrides,
        );
        return shareResult.status == ShareResultStatus.success;
      } catch (e) {
        debugPrint(e.toString());
        return false;
      }
    }

    return false;
  }

  /// share file
  /// iOS support png/mp3/wav
  static Future<bool> shareXFiles(
      List<XFile> files, {
        String? subject,
        String? text,
        List<String>? fileNameOverrides,
        BuildContext? context,
      }) async {
    ShareResult shareResult = await Share.shareXFiles(
      files,
      subject: subject,
      text: text,
      fileNameOverrides: fileNameOverrides,
      sharePositionOrigin: _iPadSharePositionOrigin(context: context),
    );
    return shareResult.status == ShareResultStatus.success;
  }

  static Rect? _iPadSharePositionOrigin({
    BuildContext? context,
  }) {
    if (context == null) return null;
    final RenderBox? box = context.findRenderObject() as RenderBox?;
    final Rect rect = box!.localToGlobal(Offset.zero) & box.size;
    return rect;
  }
}
