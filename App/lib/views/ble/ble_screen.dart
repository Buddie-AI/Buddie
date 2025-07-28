import 'dart:async';
import 'dart:io';

import 'package:app/views/ui/toast_widget.dart';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_foreground_task/flutter_foreground_task.dart';

import '../../generated/l10n.dart';
import 'device_card.dart';
import 'not_paired_notice.dart';
import 'scanning_card.dart';

class BLEScreen extends StatefulWidget {
  final bool paired;
  final String? remoteId;
  final String? deviceName;

  const BLEScreen({
    super.key,
    required this.paired,
    this.remoteId,
    this.deviceName,
  });

  @override
  _BLEScreenState createState() => _BLEScreenState();
}

class _BLEScreenState extends State<BLEScreen> {
  List<BluetoothDevice> pairedDevices = [];
  List<ScanResult> _devices = [];
  bool _isScanning = false;
  late bool _paired;
  String _statusMessage = '';
  StreamSubscription<ScanResult>? _subscription;
  ScanResult? _selectedDevice;
  String? _remoteId;
  String? _deviceName;

  @override
  void initState() {
    super.initState();
    _paired = widget.paired;
    _remoteId = widget.remoteId;
    _deviceName = widget.deviceName;
    if (!_paired) {
      _isScanning = false;
    } else {
      getPairedDevices();
      // Start to scan
      startScanning();
    }
  }

  @override
  void dispose() {
    _subscription?.cancel();
    super.dispose();
  }

  void getPairedDevices() async {
    if (Platform.isIOS) {
      // List<Guid> withServices = [
      //   // Guid("00001108-0000-1000-8000-00805f9b34fb"), // UUID
      // ];
      // pairedDevices = await FlutterBluePlus.systemDevices(withServices);
      // pairedDevices = await FlutterBluePlus.systemDevices([]);
      // if (pairedDevices.isEmpty) {
      //   pairedDevices = await FlutterBluePlus.bondedDevices;
      // }
    } else if (Platform.isAndroid) {
      pairedDevices = await FlutterBluePlus.bondedDevices;
    }
  }

  // identify whether match the blue device
  bool _isFirstThreeMatch(String id1, String id2) {
    return id1.substring(0, 9) == id2.substring(0, 9);
  }

  void startScanning() async {
    setState(() {
      _isScanning = true;
      _statusMessage = 'Scanning...';
      _devices.clear();
    });

    final stream = scanDevices();

    _subscription = stream.listen(
      (result) {
        setState(() {
          if (Platform.isAndroid) {
            if (result.device.platformName.startsWith("Buddie")) {
              bool isPaired = pairedDevices.any(
                (device) => _isFirstThreeMatch(
                  device.remoteId.str,
                  result.device.remoteId.str,
                ),
              );
              if (isPaired) {
                _devices.add(result);
              }
            }
          } else if (Platform.isIOS) {
            if (result.device.platformName.startsWith("Buddie")) {
              if (pairedDevices.isNotEmpty) {
                bool isPaired = pairedDevices.any(
                  (device) => _isFirstThreeMatch(
                    device.remoteId.str,
                    result.device.remoteId.str,
                  ),
                );
                if (isPaired) {
                  _devices.add(result);
                }
              } else {
                _devices.add(result);
              }
            }
          }
          if (_devices.isNotEmpty) {
            _selectedDevice = _devices[0];
          }
        });
      },
      onDone: () {
        setState(() {
          _isScanning = false;
          _statusMessage = _devices.isEmpty
              ? 'No devices found'
              : 'Scan completed!';
        });
      },
      onError: (error) {
        setState(() {
          _isScanning = false;
          _statusMessage = 'Scan failed: $error';
        });
      },
    );
  }

  Future<bool> startConnecting(ScanResult result) async {
    await FlutterBluePlus.stopScan();
    await _subscription?.cancel();

    final success = await connectToDevice(result);

    setState(() {
      _statusMessage = success
          ? 'Connected to ${result.advertisementData.advName}!'
          : 'Failed to connect to ${result.advertisementData.advName}';
      _isScanning = false;
    });

    return success;
  }

  Widget _buildPairedView() {
    if (_remoteId == null) {
      return Stack(
        children: [
          if (_devices.isNotEmpty)
            DeviceCard(
              deviceName: getDeviceName(_devices[0]),
              text: S.of(context).pageBleIsYour,
              onConfirm: () async {
                final success = await startConnecting(_selectedDevice!);
                Navigator.pop(context);
                if (success) {
                  context.showToast(S.of(context).pageBleToastConnectSuccess);
                } else {
                  context.showToast(S.of(context).pageBleToastConnectFailed);
                }
              },
              onCancel: () => Navigator.pop(context),
            ),
          if (_isScanning && _devices.isEmpty) const ScanningCard(),
        ],
      );
    } else {
      return Stack(
        children: [
          DeviceCard(
            deviceName: _deviceName ?? S.of(context).pageBleUnknownDevice,
            text: S.of(context).pageBleSaved,
            onConfirm: () async {
              FlutterForegroundTask.sendDataToTask('forget');
              Navigator.pop(context);
              context.showToast(S.of(context).pageBleToastForgetSuccess);
            },
            onCancel: () => Navigator.pop(context),
            confirmText: 'Forget',
          ),
        ],
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: SafeArea(
        child: _paired ? _buildPairedView() : const NotPairedNotice(),
      ),
    );
  }
}

Stream<ScanResult> scanDevices({
  Duration timeout = const Duration(seconds: 10),
}) async* {
  await FlutterBluePlus.adapterState
      .where((val) => val == BluetoothAdapterState.on)
      .first;

  final Set<String> foundDeviceIds = {};

  final StreamController<ScanResult> controller =
      StreamController<ScanResult>();

  final subscription = FlutterBluePlus.scanResults.listen((results) {
    for (final result in results) {
      final deviceId = result.device.remoteId.toString();

      if (!foundDeviceIds.contains(deviceId)) {
        foundDeviceIds.add(deviceId);
        controller.add(result);
      }
    }
  });

  try {
    await FlutterBluePlus.startScan();
    yield* controller.stream;
  } finally {
    await FlutterBluePlus.stopScan();
    await subscription.cancel();
    await controller.close();
  }
}

Future<bool> connectToDevice(ScanResult selectedResult) async {
  try {
    await FlutterBluePlus.adapterState
        .where((val) => val == BluetoothAdapterState.on)
        .first;

    await FlutterForegroundTask.saveData(
      key: 'deviceRemoteId',
      value: selectedResult.device.remoteId.toString(),
    );
    await FlutterForegroundTask.saveData(
      key: 'deviceName',
      value: getDeviceName(selectedResult).toString(),
    );
    FlutterForegroundTask.sendDataToTask('device');
    return true;
  } catch (e) {
    return false;
  }
}

String getDeviceName(ScanResult result) {
  if (result.device.platformName.isNotEmpty) {
    return result.device.platformName;
  }
  if (result.advertisementData.advName.isNotEmpty) {
    return result.advertisementData.advName;
  }
  return "Unknown device";
}
