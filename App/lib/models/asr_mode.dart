import 'package:flutter/cupertino.dart';

import '../generated/l10n.dart';

enum AsrMode {
  /// Local offline ASR - for pure transcription scenarios
  localOffline,

  /// Cloud ASR - for AI dialogue scenarios
  cloudOnline,

  /// Cloud streaming ASR - for meeting transcription scenarios
  cloudStreaming,
}

extension AsrModeExtension on AsrMode {
  /// Get ASR mode name
  String get name {
    switch (this) {
      case AsrMode.localOffline:
        return '本地离线ASR';
      case AsrMode.cloudOnline:
        return '云端ASR';
      case AsrMode.cloudStreaming:
        return '云端流式ASR';
    }
  }

  /// Get ASR mode description
  String get description {
    switch (this) {
      case AsrMode.localOffline:
        return '适用于纯转录，隐私性好，无需网络';
      case AsrMode.cloudOnline:
        return '适用于AI对话，识别准确率高';
      case AsrMode.cloudStreaming:
        return '适用于会议转录，实时流式处理';
    }
  }

  String getTitle(BuildContext context) {
    switch (this) {
      case AsrMode.cloudStreaming:
        return S.of(context).asrModeCloudStreamingTitle;
      case AsrMode.cloudOnline:
        return S.of(context).asrModeCloudOnlineTitle;
      case AsrMode.localOffline:
        return S.of(context).asrModeLocalOfflineTitle;
    }
  }
  String getDescription(BuildContext context) {
    switch (this) {
      case AsrMode.cloudStreaming:
        return S.of(context).asrModeCloudStreamingDescription;
      case AsrMode.cloudOnline:
        return S.of(context).asrModeCloudOnlineDescription;
      case AsrMode.localOffline:
        return S.of(context).asrModeLocalOfflineDescription;
    }
  }

  /// Whether to use cloud services
  bool get isCloudBased {
    switch (this) {
      case AsrMode.localOffline:
        return false;
      case AsrMode.cloudOnline:
      case AsrMode.cloudStreaming:
        return true;
    }
  }

  /// Whether streaming processing is supported
  bool get isStreaming {
    return this == AsrMode.cloudStreaming;
  }

  /// Storage key for user preferences
  String get storageKey {
    switch (this) {
      case AsrMode.localOffline:
        return 'local_offline';
      case AsrMode.cloudOnline:
        return 'cloud_online';
      case AsrMode.cloudStreaming:
        return 'cloud_streaming';
    }
  }
}

/// Utility class for ASR mode operations
class AsrModeUtils {
  /// Convert storage key to AsrMode
  static AsrMode? fromStorageKey(String? key) {
    if (key == null) return null;
    switch (key) {
      case 'local_offline':
        return AsrMode.localOffline;
      case 'cloud_online':
        return AsrMode.cloudOnline;
      case 'cloud_streaming':
        return AsrMode.cloudStreaming;
      default:
        return null;
    }
  }
}
