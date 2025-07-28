import 'dart:collection';
import 'dart:io';
import 'dart:math';
import 'dart:typed_data';
import 'package:app/utils/path_provider_utils.dart';
import 'package:app/utils/wav/wav.util.dart';
import 'audio_save_util.dart';

class PCMModel {
  final int memoryCheckLength;
  final int numChannels;
  final int sampleRate;
  final int bitDepth;

  PCMModel({
    this.memoryCheckLength = 100,
    required this.numChannels,
    required this.sampleRate,
    this.bitDepth = 16,
  });

  /// use timestamp for file name
  final int timestamp = DateTime.now().millisecondsSinceEpoch;

  /// file save directory
  String? _directory;

  Future<String> getDirectory() async {
    _directory ??= await PathProviderUtil.getAppSaveDirectory();
    return _directory!;
  }

  String? _pcmFilePath;
  String? _wavFilePath;

  /// pcm queue
  final Queue<Uint8List> _pcmDataQueue = Queue<Uint8List>();

  void clearPCMData() {
    _pcmDataQueue.clear();
  }

  void addPCMData(Uint8List data) {
    _pcmDataQueue.add(data);
    // to avoid out of memory
    if (_pcmDataQueue.length > memoryCheckLength) {
      _addPCMToFile();
    }
  }

  Future<String?> _addPCMToFile() async {
    if (_pcmDataQueue.isEmpty) return _pcmFilePath;
    String directory = await getDirectory();
    String path = '$directory/$timestamp.pcm';

    /// get Uint8List from Queue
    List<Uint8List> dataList = [];
    int length = min(memoryCheckLength, _pcmDataQueue.length);
    for (int i = 0; i < length; i++) {
      Uint8List pcmData = _pcmDataQueue.removeFirst();
      dataList.add(pcmData);
    }

    bool success = await WavUtil.saveFile(
      path: path,
      dataList: dataList,
    );
    if (success) {
      _pcmFilePath = path;
    }
    return _pcmFilePath;
  }

  Future<Uint8List?> _getFullPCMData() async {
    BytesBuilder bytesBuilder = BytesBuilder();

    if (_pcmFilePath != null) {
      File pcmFile = File(_pcmFilePath!);
      if (await pcmFile.exists()) {
        Uint8List fileData = await pcmFile.readAsBytes();
        bytesBuilder.add(fileData);
      }
    }
    while (_pcmDataQueue.isNotEmpty) {
      Uint8List queueData = _pcmDataQueue.removeFirst();
      bytesBuilder.add(queueData);
    }
    if (bytesBuilder.isNotEmpty) {
      Uint8List pcmData = bytesBuilder.toBytes();
      return pcmData;
    }
    return null;
  }

  Future<String?> pcm2wav() async {
    if (_wavFilePath != null) return _wavFilePath;

    Uint8List? pcmData = await _getFullPCMData();
    if (pcmData != null) {
      String directory = await getDirectory();
      String path = '$directory/$timestamp.wav';
      Uint8List wavHead = WavUtil.createWavHead(
        dataLength: pcmData.length,
        numChannels: numChannels,
        sampleRate: sampleRate,
        bitDepth: bitDepth,
      );
      List<Uint8List> wavData = [wavHead, pcmData];
      bool success = await WavUtil.saveFile(path: path, dataList: wavData);
      if (success) {
        _wavFilePath = path;
        deletePCMFile();
      }
    }
    return _wavFilePath;
  }

  void deletePCMFile() async {
    if (_pcmFilePath != null) {
      File pcmFile = File(_pcmFilePath!);
      if (await pcmFile.exists()) {
        await pcmFile.delete();
      }
    }
  }
}