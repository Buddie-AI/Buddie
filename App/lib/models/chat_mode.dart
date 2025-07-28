import 'package:flutter/material.dart';

import '../generated/l10n.dart';
import 'asr_mode.dart';

enum ChatMode {
  /// Default mode - regular speech recognition and recording
  defaultMode,

  /// Dialog mode - interaction with AI assistant
  dialogMode,

  /// Meeting mode - meeting recording
  meetingMode,
}

extension ChatModeExtension on ChatMode {
  /// Get mode name
  String get name {
    switch (this) {
      case ChatMode.defaultMode:
        return '默认模式';
      case ChatMode.dialogMode:
        return '对话模式';
      case ChatMode.meetingMode:
        return '会议模式';
    }
  }

  /// Get record category
  String get recordCategory {
    switch (this) {
      case ChatMode.defaultMode:
        return 'Default';
      case ChatMode.dialogMode:
        return 'Dialogue';
      case ChatMode.meetingMode:
        return 'Meeting';
    }
  }

  String getTitle(BuildContext context) {
    switch (this) {
      case ChatMode.defaultMode:
        return S.of(context).chatModeDefault;
      case ChatMode.dialogMode:
        return S.of(context).chatModeDialog;
      case ChatMode.meetingMode:
        return S.of(context).chatModeMeeting;
    }
  }

  /// Whether new messages should be inserted into the interface
  bool shouldInsertMessage(bool hasAsrMessageId) {
    switch (this) {
      case ChatMode.defaultMode:
        return !hasAsrMessageId; // Default mode only processes non-ASR messages
      case ChatMode.dialogMode:
        return !hasAsrMessageId; // Dialog mode only processes non-ASR messages (ASR has dedicated processing)
      case ChatMode.meetingMode:
        return !hasAsrMessageId; // Meeting mode only processes non-ASR messages
    }
  }

  /// Whether AI response is needed
  bool get needsAiResponse {
    return this == ChatMode.dialogMode;
  }

  /// Get default ASR mode
  AsrMode get defaultAsrMode {
    switch (this) {
      case ChatMode.defaultMode:
        return AsrMode.localOffline; // Transcription mode defaults to local recognition
      case ChatMode.dialogMode:
        return AsrMode.cloudOnline; // Dialog mode defaults to cloud online ASR
      case ChatMode.meetingMode:
        return AsrMode.cloudStreaming; // Meeting mode defaults to real-time speech recognition
    }
  }
}
