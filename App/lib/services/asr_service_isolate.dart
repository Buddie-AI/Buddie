/// 本地离线语音转录模块
///
/// 本文件定义了 `AsrServiceIsolate` 类，用于在独立的 Dart Isolate 中运行本地离线 ASR（自动语音识别）逻辑，
// 以避免阻塞主线程。主要功能包括：
// 1. 初始化 ASR 模型及标点模型（Paraformer、Whisper 等），并在后台 Isolate 内创建识别器。
// 2. 提供 `sendData` 接口，将音频数据打包发送给后台 Isolate，进行识别和标点处理。
// 3. 管理识别器生命周期：支持初始化（`init`）、发送数据（`sendData`）和停止记录（`stopRecord`）等操作。
// 4. 内部使用 `Task` 类封装消息和回调端口，确保主 Isolate 与后台 Isolate 的双向通信。
// 5. 利用 `BackgroundIsolateBinaryMessenger` 确保与 Flutter 引擎的消息通道在后台 Isolate 中也可用。
///
/// 使用示例：
/// ```dart
/// final asrService = AsrServiceIsolate();
/// await asrService.init();                      // 启动后台 Isolate 并加载模型
/// String text = await asrService.sendData(data); // 发送音频数据获取识别结果
/// await asrService.stopRecord();                // 释放识别器资源
/// ```
///
/// 注意：
/// - 依赖 `sherpa_onnx` 包提供的离线识别和标点功能；
/// - 在调用 `init` 前无需手动调用 `initBindings`，内部会自动处理；
/// - Isolate 内部通过 `ReceivePort`/`SendPort` 进行消息传递，不要在主线程直接操作识别器实例。


import 'dart:isolate';
import 'dart:typed_data';
import 'package:flutter/services.dart';
import 'package:sherpa_onnx/sherpa_onnx.dart';

import '../utils/asr_utils.dart';

class AsrServiceIsolate {
  late SendPort sendPort;
  bool isInitialized = false;
  RootIsolateToken rootIsolateToken = RootIsolateToken.instance!;

  late OfflineRecognizer offlineRecognizer;
  late OfflineModelConfig offlineModelConfig;

  late OfflinePunctuation offlinePunctuation;
  late OfflinePunctuationModelConfig offlinePunctuationModelConfig;
  late OnlinePunctuation onlinePunctuation;
  late OnlinePunctuationModelConfig onlinePunctuationModelConfig;

  Future init() async {
    var receivePort = ReceivePort();
    // offlineModelConfig = await getWhisperModelConfig();
    offlineModelConfig = await getParaformerModelConfig();
    offlinePunctuationModelConfig = await getOfflinePunctuationModelConfig();
    await Isolate.spawn(_handle, receivePort.sendPort);
    sendPort = await receivePort.first;
    receivePort.close();
    final task = Task("init", "");
    sendPort.send(task.toList());
    await task.response.first;
    isInitialized = true;
  }

  _handle(SendPort sendPort) async {
    var receivePort = ReceivePort();
    sendPort.send(receivePort.sendPort);
    await for (var msg in receivePort) {
      if (msg is List<Object>) {
        final action = msg[0] as String;
        final sendPort = msg[2] as SendPort;
        if (action == 'init') {
          initBindings();
          BackgroundIsolateBinaryMessenger.ensureInitialized(rootIsolateToken);
          offlineRecognizer = await createOfflineRecognizer(offlineModelConfig);
          offlinePunctuation = await getOfflinePunctuation(
              offlinePunctuationModelConfig);
          sendPort.send("initialized");
        } else if (action == 'stopRecord') {
          offlineRecognizer.free();
          sendPort.send("stopped");
        } else if (action == 'sendData') {
          var samples = msg[1] as Float32List;
          final offlineStream = offlineRecognizer.createStream();
          offlineStream.acceptWaveform(samples: samples, sampleRate: 16000);
          offlineRecognizer.decode(offlineStream);
          var result = offlineRecognizer.getResult(offlineStream).text;
          result = offlinePunctuation.addPunct(result);
          result = result
              .replaceAll('，', ', ').replaceAll('。', '. ')
              .replaceAll('！', '! ').replaceAll('？', '? ')
              .replaceAll(''', " '").replaceAll(''', "' ")
              .replaceAll('"', ' "').replaceAll('"', '" ');
          sendPort.send(result);
          offlineStream.free();
        }
      }
    }
  }

  Future<String> sendData(Float32List data) async {
    final task = Task("sendData", data);
    sendPort.send(task.toList());
    final result = await task.response.first;
    return result;
  }

  Future stopRecord() async {
    final task = Task("stopRecord", "");
    sendPort.send(task.toList());
    await task.response.first;
  }
}

class Task {
  final String action;
  final dynamic data;
  final ReceivePort response = ReceivePort();
  Task(this.action, this.data);

  List<Object> toList() => [action, data, response.sendPort];
}

Future<OfflineRecognizer> createOfflineRecognizer(
    OfflineModelConfig modelConfig) async {
  return OfflineRecognizer(OfflineRecognizerConfig(
      model: modelConfig
  ));
}

Future<OfflineModelConfig> getWhisperModelConfig() async {
  const modelDir = 'assets/sherpa-onnx-whisper-tiny.en';
  return OfflineModelConfig(
      whisper: OfflineWhisperModelConfig(
          encoder: await copyAssetFile('$modelDir/tiny.en-encoder.int8.onnx'),
          decoder: await copyAssetFile('$modelDir/tiny.en-decoder.int8.onnx'),
          language: 'en',
          task: 'transcribe',
          tailPaddings: 1000
      ),
      tokens: await copyAssetFile('$modelDir/tiny.en-tokens.txt'),
      modelType: 'whisper'
  );
}

Future<OfflineModelConfig> getParaformerModelConfig() async {
  const modelDir = 'assets/sherpa-onnx-paraformer-zh-2024-03-09';
  return OfflineModelConfig(
      paraformer: OfflineParaformerModelConfig(
          model: await copyAssetFile('$modelDir/model.int8.onnx')
      ),
      tokens: await copyAssetFile('$modelDir/tokens.txt'),
      modelType: 'paraformer'
  );
}

Future<OfflinePunctuation> getOfflinePunctuation(
    OfflinePunctuationModelConfig offlinePunctuationModelConfig) async {
  return OfflinePunctuation(config: OfflinePunctuationConfig(
      model: offlinePunctuationModelConfig
  ));
}

Future<OfflinePunctuationModelConfig> getOfflinePunctuationModelConfig() async {
  const modelDir = 'assets/sherpa-onnx-punct-ct-transformer-zh-en-vocab272727-2024-04-12';
  return OfflinePunctuationModelConfig(
      ctTransformer: await copyAssetFile('$modelDir/model.onnx'),
      debug: false
  );
}
