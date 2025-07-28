import 'dart:math';
import 'package:flutter/foundation.dart';
import 'package:linalg/linalg.dart';


const int nPcaByte = 47;
const int nSignByte = 33;
const int nDct = 257;

class AudioProcessingUtil {
  static List<double> processSinglePackage(
    Uint8List packageData,
    Matrix iPcaMatrix,
    Matrix iDctMatrix
  ) {
    final pcaData = ByteData.sublistView(
        packageData.sublist(0, nPcaByte)
    );
    final signData = packageData.sublist(nPcaByte, nPcaByte + nSignByte);

    Vector compressedSpectrum = Vector.row(List.generate(nPcaByte, (i) {
      return pcaData.getInt8(i).toDouble() / 128;
    }));
    Vector spectrum = (
        compressedSpectrum * iPcaMatrix
    ).toVector(false).map((value) => max(0, value));
    int k = 0;
    for (int i = 0; i < nSignByte; i++) {
      int num = signData[i].toInt();
      for (int j = 0; j < 8; j++) {
        if ((num & (1 << j)) > 0) {
          spectrum[k] = -spectrum[k];
        }
        k++;
        if (k == nDct) {
          break;
        }
      }
      if (k == nDct) {
        break;
      }
    }

    Vector audioClip = (spectrum * iDctMatrix).toVector(false);

    return audioClip.toList(false).sublist(1);
  }
}
