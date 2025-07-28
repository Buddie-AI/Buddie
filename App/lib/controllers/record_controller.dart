/// 录音界面核心控制器
///
/// 管理录音界面的完整生命周期和交互逻辑，主要功能包括：
/// 1. 录音服务管理：
///   - 前台服务启动/停止
///   - 录音权限管理
///   - 设备状态监控
/// 2. 蓝牙设备集成：
///   - 蓝牙连接状态跟踪
///   - 设备信息维护
///   - 跨平台权限处理
/// 3. 状态同步：
///   - 与前台服务双向通信
///
/// 核心工作流程：
/// 1. 初始化阶段：
///   - 请求必要权限（录音/蓝牙/通知）
///   - 配置前台服务
/// 2. 运行阶段：
///   - 监听服务通信数据
///   - 同步录音/蓝牙状态
///   - 处理用户交互
/// 3. 销毁阶段：
///   - 释放资源
///   - 注销回调
///
/// 使用示例：
/// ```dart
/// // 在StatefulWidget中初始化
/// final controller = RecordScreenController();
///
/// @override
/// void initState() {
///   super.initState();
///   controller.attach(this);
///   controller.load(); // 启动服务
/// }
/// ```

import 'dart:async';
import 'dart:io';
import 'package:permission_handler/permission_handler.dart';
import 'package:record/record.dart';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_foreground_task/flutter_foreground_task.dart';
import '../services/asr_service.dart';


class RecordScreenController {
  State? _state;

  String transcriptionStatus = '';

  bool isRecording = true;
  String? deviceRemoteId;
  String? deviceName;
  BluetoothConnectionState connectionState = BluetoothConnectionState.disconnected;

  Future<void> load() async {
    if (!await FlutterForegroundTask.isRunningService) {
      await _initService();
      await startService();
      await Future.delayed(Duration(seconds: 1));
    }
  }

  @mustCallSuper
  void attach(State state) {
    _state = state;
    FlutterForegroundTask.addTaskDataCallback(_onReceiveTaskData);
  }

  @mustCallSuper
  void detach() {
    _state = null;
    FlutterForegroundTask.removeTaskDataCallback(_onReceiveTaskData);
  }

  @mustCallSuper
  void dispose() {
    detach();
  }

  void toggleRecording() {
    isRecording = !isRecording;
    if (isRecording) {
      FlutterForegroundTask.sendDataToTask("startRecording");
    } else {
      FlutterForegroundTask.sendDataToTask("stopRecording");
    }
  }

  Future<void> _initService() async {
    await _requestPlatformPermissions();

    FlutterForegroundTask.init(
      androidNotificationOptions: AndroidNotificationOptions(
        channelId: 'buddie_service',
        channelName: 'Buddie Service',
        channelImportance: NotificationChannelImportance.HIGH,
        priority: NotificationPriority.HIGH,
      ),
      iosNotificationOptions: const IOSNotificationOptions(
        showNotification: false,
        playSound: false,
      ),
      foregroundTaskOptions: ForegroundTaskOptions(
        eventAction: ForegroundTaskEventAction.repeat(1 * 60 * 1000),
        autoRunOnBoot: true,
        autoRunOnMyPackageReplaced: true,
        allowWakeLock: true,
        allowWifiLock: true,
      ),
    );
  }

  Future<void> startService() async {
    await _requestRecordPermission();
    await _requestBlePermissions();
    final ServiceRequestResult result = await FlutterForegroundTask.startService(
      serviceId: 300,
      notificationTitle: 'Buddie Service',
      notificationText: 'Tap to return to the app',
      callback: startRecordService,
    );

    if (!result.success) {
      throw result.error ?? Exception('An error occurred and the service could not be started.');
    }
  }

  Future<void> stopService() async {
    if (!await FlutterForegroundTask.isRunningService) {
      return;
    }

    final ServiceRequestResult result = await FlutterForegroundTask.stopService();

    if (!result.success) {
      throw result.error ?? Exception('An error occurred and the service could not be stopped.');
    }
  }

  void _onReceiveTaskData(Object data) {
    if (data is Map<String, dynamic>) {
      final action = data['action'] as String?;
      final connection = data['connectionState'] as bool?;
      final devName = data['deviceName'] as String?;
      final devId = data['deviceId'] as String?;
      final recording = data['isRecording'] as bool?;

      if (recording == true) {
        _state?.setState(() {
          isRecording = true;
        });
      }

      if (action == 'deviceReset') {
        _state?.setState(() {
          deviceRemoteId = null;
          deviceName = null;
          connectionState = BluetoothConnectionState.disconnected;
        });
      }

      if (connection == true) {
        _state?.setState(() {
          connectionState = BluetoothConnectionState.connected;
          deviceName = devName;
          deviceRemoteId = devId;
        });
      } else if(connection == false) {
        _state?.setState(() {
          connectionState = BluetoothConnectionState.disconnected;
          deviceName = devName;
          deviceRemoteId = devId;
        });
      }
    }
  }

  Future<void> _requestPlatformPermissions() async {
    // final NotificationPermission notificationPermission =
    //   await FlutterForegroundTask.checkNotificationPermission();
    // if (notificationPermission != NotificationPermission.granted) {
    //   await FlutterForegroundTask.requestNotificationPermission();
    // }

    if (Platform.isAndroid) {
      if (!await FlutterForegroundTask.isIgnoringBatteryOptimizations) {
        await FlutterForegroundTask.requestIgnoreBatteryOptimization();
      }
    }
  }

  Future<void> _requestBlePermissions() async {
    if (Platform.isAndroid) {
      Map<Permission, PermissionStatus> statuses = await [
        Permission.bluetoothScan,
        Permission.bluetoothConnect,
        Permission.bluetoothAdvertise,
        Permission.notification,
      ].request();
    } else if (Platform.isIOS) {
      if (await Permission.bluetooth.isDenied) {
        await Permission.bluetooth.request();
      }
      if (await Permission.notification.isDenied) {
        await Permission.notification.request();
      }
    }
  }

  Future<void> _requestRecordPermission() async {
    if (!await AudioRecorder().hasPermission()) {
      throw Exception('To start record service, you must grant microphone permission.');
    }
  }
}