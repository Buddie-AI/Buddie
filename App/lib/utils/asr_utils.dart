import 'package:path/path.dart';
import 'package:path_provider/path_provider.dart';
import 'package:flutter/services.dart' show rootBundle;
import 'dart:typed_data';
import "dart:io";

// Copy the asset file from src to dst
Future<String> copyAssetFile(String src, [String? dst]) async {
  final Directory directory = await getApplicationDocumentsDirectory();
  dst ??= basename(src);
  final target = join(directory.path, dst);

  final File targetFile = File(target);
  if (await targetFile.exists()) {
    return target;
  }

  final data = await rootBundle.load(src);
  final List<int> bytes = data.buffer.asUint8List(
    data.offsetInBytes,
    data.lengthInBytes,
  );
  await targetFile.writeAsBytes(bytes);

  return target;
}

Float32List convertBytesToFloat32(Uint8List bytes, [endian = Endian.little]) {
  final values = Float32List(bytes.length ~/ 2);

  final data = ByteData.view(bytes.buffer);

  for (var i = 0; i < bytes.length; i += 2) {
    int short = data.getInt16(i, endian);
    values[i ~/ 2] = short / 32768.0;
  }

  return values;
}

Uint8List convert8bitTo16bit(Uint8List inputData) {
  final byteData = ByteData(inputData.length * 2);

  for (int i = 0; i < inputData.length; i++) {
    final pcmValue = inputData[i] << 8;
    byteData.setInt16(
      i * 2,
      pcmValue,
      Endian.little,
    ); // Write in little-endian order
  }

  return byteData.buffer.asUint8List();
}

Uint8List resampleTo16kHz(Uint8List pcm16Data) {
  final inputSamples = ByteData.sublistView(pcm16Data).buffer.asInt16List();
  final inputSampleCount = inputSamples.length;

  final outputSampleCount = inputSampleCount * 2;
  final outputData = ByteData(outputSampleCount * 2);

  for (int i = 0; i < inputSampleCount - 1; i++) {
    final sample1 = inputSamples[i];
    final sample2 = inputSamples[i + 1];

    outputData.setInt16(i * 4, sample1, Endian.little);

    final interpolatedSample = ((sample1 + sample2) ~/ 2);
    outputData.setInt16(i * 4 + 2, interpolatedSample, Endian.little);
  }

  outputData.setInt16(
    (outputSampleCount - 1) * 2,
    inputSamples[inputSampleCount - 1],
    Endian.little,
  );

  return outputData.buffer.asUint8List();
}

Uint8List processAudioData(List<int> value) {
  final outputData = ByteData(1600);

  int prevSample = value[2] << 8;
  int currentSample;

  int outputIndex = 0;

  for (int i = 2; i < 404; i++) {
    if (i == 202) {
      i = 204;
    }

    currentSample = value[i] << 8;

    outputData.setInt16(outputIndex, prevSample, Endian.little);
    outputIndex += 2;

    final interpolatedSample = (prevSample + currentSample) >> 1;
    outputData.setInt16(outputIndex, interpolatedSample, Endian.little);
    outputIndex += 2;

    prevSample = currentSample;
  }

  // outputData.setInt16(outputIndex, prevSample, Endian.little);
  return outputData.buffer.asUint8List();
}
