import 'dart:collection';
import 'dart:typed_data';

import 'package:flutter/foundation.dart';
import '../../services/summary.dart';
import 'pcm_model.dart';

class FileService {
  static List<String> savedWavFiles = [];
  static bool _onSaving = false;
  static final Queue<PCMModel> _queue = Queue<PCMModel>();
  static PCMModel? _currentWavFileModel;
  static bool? _onRecordingPrevious;
  static int? startMeetingTime;

  static void highSaveWav({
    int? startMeetingTime,
    required bool onRecording,
    required Uint8List data,
    required int numChannels,
    required int sampleRate,
  }) async {
    if (onRecording) {
      _addPCMData(data: data, numChannels: numChannels, sampleRate: sampleRate);
    } else {
      if (_onRecordingPrevious == onRecording) return;
      if (startMeetingTime != null &&
          DateTime.now().millisecondsSinceEpoch - startMeetingTime >
              20 * 1000) {
        String? path = await _saveWavHighPerformance();

        if (path != null) {
          debugPrint('Audio saved: $path');
          DialogueSummary.start(
            isMeeting: true,
            startMeetingTime: startMeetingTime,
            endMeetingTime: DateTime.now().millisecondsSinceEpoch,
            audioPath: path,
          );
        } else {
          _currentWavFileModel?.clearPCMData();
          _currentWavFileModel = null;
        }
      }
    }
    _onRecordingPrevious = onRecording;
  }

  static void _addPCMData({
    required Uint8List data,
    required int numChannels,
    required int sampleRate,
  }) {
    _currentWavFileModel ??= PCMModel(
      numChannels: numChannels,
      sampleRate: sampleRate,
    );
    _currentWavFileModel!.addPCMData(data);
  }

  static Future<String?> _saveWavHighPerformance() async {
    if (_currentWavFileModel == null) {
      debugPrint('Please call addPCMData method first');
      return null;
    }
    _queue.add(_currentWavFileModel!);
    _currentWavFileModel = null;

    if (_onSaving) return null;
    _onSaving = true;

    PCMModel pcmModel = _queue.removeFirst();

    /// TODO: put the method in isolate
    String? path = await pcmModel.pcm2wav();
    _onSaving = false;
    if (path != null) {
      debugPrint('save wav success:$path');
      pcmModel.deletePCMFile();
      savedWavFiles.add(path);
    }

    return path;
  }
}
