import 'dart:developer' as dev;
import 'package:flutter/material.dart';
import 'package:app/models/llm_config.dart';
import 'package:flutter_foreground_task/flutter_foreground_task.dart';

import '../constants/record_constants.dart';
import '../services/objectbox_service.dart';


class SettingScreenController {
  State? _state;

  bool isSwitchEnabled = true;
  final ObjectBoxService _objectBoxService = ObjectBoxService();

  @mustCallSuper
  void detach() {
    _state = null;
    FlutterForegroundTask.removeTaskDataCallback(_onReceiveTaskAction);
  }

  @mustCallSuper
  void dispose() {
    detach();
  }

  Future<void> insertApiKey(String apikey) async {
    final existingConfig = _objectBoxService.getConfigsByProvider("OpenAI");

    if (existingConfig != null) {
      existingConfig.apiKey = apikey;
      _objectBoxService.updateConfigByProvider("OpenAI", apiKey: apikey);
    } else {
      _objectBoxService.insertConfig(
        LlmConfigEntity(
          provider: "OpenAI",
          model: "gpt-4o",
          apiKey: apikey,
          baseUrl: "https://api.openai.com",
        ),
      );
    }
  }

  void resetDevice() async {
    FlutterForegroundTask.removeData(key: 'deviceRemoteId');
    FlutterForegroundTask.sendDataToMain({'action': 'deviceReset'});
  }

  void _onReceiveTaskAction(dynamic data) {
    dev.log('Received task data: $data');
    if (data == Constants.actionDone) {
      _state?.setState(() {
        isSwitchEnabled = true;
      });
    }
  }
}
