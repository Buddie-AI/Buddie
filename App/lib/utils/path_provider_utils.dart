import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:path_provider/path_provider.dart';

class PathProviderUtil {
  /// get App Save directory
  static Future<String> getAppSaveDirectory() async {
    String? path;
    if (Platform.isAndroid) {
      path = await getExternalStoragePath();
    } else if (Platform.isIOS) {
      path = await getApplicationDocumentsPath();
    }
    return path!;
  }

  /// * Android
  ///   "/data/user/0/inc.buddie.app/cache"
  static Future<String> getTemporaryPath() async {
    final directory = await getTemporaryDirectory();
    String path = directory.path;
    debugPrint('getTemporaryDirectory:$path');
    return path;
  }

  /// * Android
  ///   "/data/user/0/inc.buddie.app/files"
  static Future<String> getApplicationSupportPath() async {
    final directory = await getApplicationSupportDirectory();
    String path = directory.path;
    debugPrint('getApplicationSupportDirectory:$path');
    return path;
  }

  /// * Android
  ///   "/data/user/0/inc.buddie.app/app_flutter"
  /// * iOS
  ///   "/var/mobile/Containers/Data/Application/A7CA544C-8C07-41CA-B2DD-C79CC221965F/Documents"
  static Future<String> getApplicationDocumentsPath() async {
    final directory = await getApplicationDocumentsDirectory();
    String path = directory.path;
    debugPrint('getApplicationDocumentsDirectory:$path');
    return path;
  }

  /// * Android
  ///   "/data/user/0/inc.buddie.app/cache"
  static Future<String> getApplicationCachePath() async {
    final directory = await getApplicationCacheDirectory();
    String path = directory.path;
    debugPrint('getApplicationCacheDirectory:$path');
    return path;
  }

  /// * Android
  ///   "/storage/emulated/0/Android/data/inc.buddie.app/files/downloads"
  static Future<String?> getDownloadsPath() async {
    final directory = await getDownloadsDirectory();
    String? path = directory?.path;
    debugPrint('getDownloadsDirectory:$path');
    return path;
  }

  /// * Android Only
  ///  "/storage/emulated/0/Android/data/inc.buddie.app/files"
  static Future<String?> getExternalStoragePath() async {
    final directory = await getExternalStorageDirectory();
    String? path = directory?.path;
    debugPrint('getExternalStorageDirectory:$path');
    return path;
  }
}
