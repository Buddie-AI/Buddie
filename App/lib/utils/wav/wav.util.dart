import 'dart:io';
import 'dart:typed_data';

class WavUtil {
  /// create wav head
  static Uint8List createWavHead({
    required int dataLength,
    required int numChannels,
    required int sampleRate,
    int bitDepth = 16,
  }) {
    Endian endian = Endian.little;

    // WAVE head costs 44 bytes
    ByteData waveHead = ByteData(44);
    // 0x00 - "RIFF" Tag
    waveHead.setUint32(0, 0x46464952, endian);
    // 0x04 - file length
    waveHead.setUint32(4, dataLength + 36, endian);
    // 0x08 - "WAVE" Tag
    waveHead.setUint32(8, 0x45564157, endian);
    // 0x0C - "fmt " Tag
    waveHead.setUint32(12, 0x20746d66, endian);
    // 0x10 - fmt sub block size(usually is 16 bytes)
    waveHead.setUint32(16, 16, endian);
    // 0x14 - format(0x10 is PCM)
    waveHead.setUint16(20, 1, endian);
    // 0x16 - num of Channels(1-mono,2-stereo)
    waveHead.setUint16(22, numChannels, endian);
    // 0x18 - sample rate
    waveHead.setUint32(24, sampleRate, endian);
    int x = (bitDepth / 8).toInt(); // bitDepth / 8
    // 0x1C - transfer rate(numChannels * sampleRate * bitDepth / 8)
    waveHead.setUint32(28, numChannels * sampleRate * x, endian);
    // 0x20 - sample frame size(numChannels * bitDepth / 8)
    waveHead.setUint16(32, numChannels * x, endian);
    // 0x22 - bit depth(8bit or 16bit)
    waveHead.setUint16(34, bitDepth, endian);
    // 0x24 - "data" Tag
    waveHead.setUint32(36, 0x61746164, endian);
    // 0x28 - data length
    waveHead.setUint32(40, dataLength, endian);
    Uint8List result = waveHead.buffer.asUint8List();
    return result;
  }


  /// common save file method
  static Future<bool> saveFile({
    required String path,
    required List<Uint8List> dataList,
  }) async {
    try {
      File file = File(path);
      FileMode mode = await file.exists() ? FileMode.append : FileMode.write;
      IOSink sink = file.openWrite(mode: mode);
      for (Uint8List data in dataList) {
        sink.add(data);
      }
      await sink.flush();
      await sink.close();
      return true;
    } catch (e) {
      return false;
    }
  }
}
