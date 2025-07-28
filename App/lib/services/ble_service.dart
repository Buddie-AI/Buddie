/// 蓝牙低功耗(BLE)服务管理
///
/// 提供蓝牙设备连接、数据传输和状态管理的核心功能：
/// 1. 设备管理：
///   - 自动重连上次配对的设备
///   - 手动连接指定设备
///   - 设备遗忘功能
/// 2. 连接管理：
///   - 状态监听（连接/断开）
///   - 防抖处理避免状态抖动
///   - 平台差异化处理（Android/iOS）
/// 3. 数据传输：
///   - 实时数据流接收
///   - 特征值订阅机制
/// 4. 性能优化：
///   - MTU协商（最大传输单元）
///   - 连接优先级设置
///   - PHY参数配置（物理层）
///
/// 使用示例：
/// ```dart
/// // 初始化服务（自动重连）
/// await BleService().init();
///
/// // 手动连接设备
/// await BleService().getAndConnect(deviceId);
///
/// // 监听数据流
/// BleService().dataStream.listen((data) {
///   // 处理接收到的数据
/// });
/// ```
///
/// 注意事项：
/// - 依赖 flutter_blue_plus 和 flutter_foreground_task 插件
/// - Android平台支持更多高级功能（PHY、连接优先级）
/// - 连接状态变化会触发前台任务状态更新
/// - 使用防抖机制优化状态变化处理

import 'dart:async';
import 'dart:developer' as dev;
import 'dart:io';
import 'dart:typed_data';
import 'package:collection/collection.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_foreground_task/flutter_foreground_task.dart';

import '../constants/record_constants.dart';

class BleService {
  static final BleService _instance = BleService._internal();
  factory BleService() => _instance;
  BleService._internal();

  // 当前蓝牙连接状态，初始为断开
  BluetoothConnectionState _connectionState = BluetoothConnectionState.disconnected;
  // 当前设备对象
  BluetoothDevice? _device;
  // 监听连接状态的订阅
  StreamSubscription<BluetoothConnectionState>? _connectionStateSubscription;
  // 设备名称
  String? deviceName;

  final StreamController<Uint8List> _dataController = StreamController<Uint8List>();
  Stream<Uint8List> get dataStream => _dataController.stream;
  StreamSubscription<List<int>>? _dataSubscription;

  String? get deviceRemoteId => _device?.remoteId.toString();
  BluetoothConnectionState get connectionState => _connectionState;

  Stream<BluetoothConnectionState> get connectionStateStream =>
      _device?.connectionState ?? Stream<BluetoothConnectionState>.empty();

  // 用于连接状态变化的防抖定时器
  Timer? _debounceTimer;

  /// 初始化服务
  ///
  /// 从前台任务获取保存的 deviceRemoteId，若存在则自动重连。
  Future<void> init() async {
    var remoteId = await FlutterForegroundTask.getData(key: 'deviceRemoteId');
    if (remoteId != null) {
      _device = BluetoothDevice.fromId(remoteId);
      await FlutterBluePlus.adapterState
          .where((val) => val == BluetoothAdapterState.on)
          .first;
      await _device!.connect(autoConnect: true, mtu: null);
      listenToConnectionState();
    }
    // Android 平台才支持 PHY 查询
    if (Platform.isAndroid) {
      PhySupport phySupport = await FlutterBluePlus.getPhySupport();
      // 可以根据 phySupport 做后续处理
    }
  }

  /// 主动获取 deviceRemoteId 并连接
  Future<void> getAndConnect(dynamic remoteId) async {
    await FlutterBluePlus.adapterState
        .where((val) => val == BluetoothAdapterState.on)
        .first;

    if (remoteId != null) {
      _device = BluetoothDevice.fromId(remoteId);

      final state = await _device!.connectionState.first;
      if (state != BluetoothConnectionState.connected) {
        await _device?.connect(autoConnect: true, mtu: null);
      }
    }
  }

  /// 监听蓝牙连接状态变化
  void listenToConnectionState() {
    _connectionStateSubscription?.cancel();
    _connectionStateSubscription = _device?.connectionState.listen((
      state,
    ) async {
      _connectionState = state;

      // 防抖：防止短时间内大量回调
      _debounceTimer?.cancel();
      _debounceTimer = Timer(Duration(milliseconds: 500), () async {
        if (state == BluetoothConnectionState.connected) {
          // 连接成功后延迟处理，保证稳定
          await Future.delayed(Duration(milliseconds: 3000));
          // 使用Buddie时，借助BLE传输语音数据，通知前台任务停止麦克风录音
          FlutterForegroundTask.sendDataToTask(Constants.actionStopMicrophone);
          FlutterForegroundTask.sendDataToMain({
            'connectionState': true,
            'deviceName': _device?.advName,
            'deviceId': _device?.remoteId.toString(),
          });
          if (Platform.isAndroid) {
            // Android 平台下请求更大的 MTU 和高优先级连接
            await _device!.requestMtu(247);
            await _device!.requestConnectionPriority(
              connectionPriorityRequest: ConnectionPriority.high,
            );
            // 设置 PHY 参数
            await _device!.setPreferredPhy(
              txPhy: Phy.le2m.mask,
              rxPhy: Phy.le2m.mask,
              option: PhyCoding.noPreferred,
            );
          } else {
            _device!.mtu.listen((int mtu) {
              debugPrint("BLE MTU: $mtu");
            });
          }

          List<BluetoothService> services = await _device!.discoverServices();

          BluetoothService? service = services.firstWhereOrNull(
            (service) => service.uuid.toString() == "ae00",
          );
          if (service == null) return;
          dev.log('Service found: ${service.uuid.toString()}');

          var characteristics = service.characteristics;

          BluetoothCharacteristic? chr = characteristics.firstWhereOrNull(
            (characteristic) => characteristic.uuid.toString() == "ae04",
          );
          if (chr == null) return;

          dev.log('Characteristic found: ${chr.uuid.toString()}');

          await chr.setNotifyValue(true);
          _dataSubscription?.cancel();
          _dataSubscription = chr.onValueReceived.listen((value) {
            _dataController.add(Uint8List.fromList(value));
          });
        } else if (state == BluetoothConnectionState.disconnected) {
          FlutterForegroundTask.sendDataToTask(Constants.actionStartMicrophone);
          FlutterForegroundTask.sendDataToMain({
            'connectionState': false,
            'deviceName': _device?.advName,
            'deviceId': _device?.remoteId.toString(),
          });
          dev.log(
            "${_device!.disconnectReason?.code} ${_device!.disconnectReason?.description}",
          );
        }
      });
    });
  }

  /// 忘记设备：断开并清除保存的设备信息
  void forgetDevice() {
    _device?.disconnect();
    FlutterForegroundTask.removeData(key: 'deviceRemoteId');
    FlutterForegroundTask.removeData(key: 'deviceName');
  }

  /// 释放资源，取消订阅并关闭数据流
  void dispose() {
    _connectionStateSubscription?.cancel();
    _dataController.close();
  }
}
