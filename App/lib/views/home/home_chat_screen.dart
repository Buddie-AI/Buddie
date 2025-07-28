/// 主聊天界面组件
///
/// 实现应用核心聊天功能的关键界面，主要特性包括：
/// 1. 聊天交互系统：
///   - 支持文本/语音输入
///   - 消息历史分页加载
///   - 实时消息流处理
/// 2. 设备集成：
///   - 蓝牙设备连接状态显示
///   - 录音控制集成
///   - 前台服务管理
///
/// 核心交互流程：
/// 1. 初始化阶段：
///   - 加载控制器（ChatController/RecordScreenController）
///   - 检查首次启动显示欢迎消息
///   - 启动录音服务
/// 2. 运行阶段：
///   - 处理消息发送/接收
///   - 同步蓝牙/录音状态
/// 3. 导航功能：
///   - 跳转会议列表
///   - 显示蓝牙配对面板
///   - 帮助功能
///
/// 使用示例：
/// ```dart
/// // 简单初始化
/// const HomeChatScreen();
///
/// // 带自定义录音控制器
/// HomeChatScreen(controller: customRecordController);
/// ```

import 'dart:io';

import 'package:app/constants/welcome_constants.dart';
import 'package:app/extension/media_query_data_extension.dart';
import 'package:app/utils/route_utils.dart';
import 'package:app/views/home/components/chat_list_tile.dart';
import 'package:app/views/home/components/home_app_bar.dart';
import 'package:app/views/home/components/home_bottom_bar.dart';
import 'package:app/views/ui/app_background.dart';
import 'package:easy_refresh/easy_refresh.dart';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_foreground_task/flutter_foreground_task.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:go_router/go_router.dart';
import 'package:keyboard_dismisser/keyboard_dismisser.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'package:visibility_detector/visibility_detector.dart';

import '../../controllers/chat_controller.dart';
import '../../controllers/record_controller.dart';
import '../../utils/assets_util.dart';
import '../ble/ble_screen.dart';

VoidCallback? onTokenExpire;

class HomeChatScreen extends StatefulWidget {
  final RecordScreenController? controller;

  const HomeChatScreen({super.key, this.controller});

  @override
  State<HomeChatScreen> createState() => _HomeChatScreenState();
}

class _HomeChatScreenState extends State<HomeChatScreen>
    with WidgetsBindingObserver {
  late ChatController _chatController;
  final FocusNode _focusNode = FocusNode();
  late RecordScreenController _audioController;

  final _listenable = IndicatorStateListenable();
  bool _shrinkWrap = false;
  double? _viewportDimension;

  final TextStyle textTextStyle = const TextStyle(
    fontWeight: FontWeight.bold,
    fontSize: 14,
  );

  final EdgeInsets chatPadding = EdgeInsets.symmetric(
    horizontal: 18.sp,
    vertical: 12.sp,
  );

  final double lineSpace = 16.sp;

  List<BluetoothDevice> pairedDevices = [];
  bool _paired = false;
  bool _isBottomSheetShown = false;

  @override
  void initState() {
    super.initState();
    onTokenExpire = () async {
      // No longer needed without authentication
    };
    _init();
    // Register this class as an observer to listen for keyboard changes
    WidgetsBinding.instance.addObserver(this);
    WidgetsBinding.instance.addPostFrameCallback((_) {
      currentBottomInset = MediaQuery.of(context).viewInsets.bottom;
    });
  }

  @override
  void dispose() {
    WidgetsBinding.instance.removeObserver(this);
    _focusNode.dispose();
    _chatController.dispose();
    _listenable.removeListener(_onHeaderChange);
    onTokenExpire = null;
    super.dispose();
  }

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    super.didChangeAppLifecycleState(state);
    if (state == AppLifecycleState.resumed) {
      // No longer needed without authentication
    }
  }

  double currentBottomInset = 0;

  @override
  void didChangeMetrics() {
    // This is called when the metrics change (including keyboard visibility)
    final bottomInset = MediaQuery.of(context).viewInsets.bottom;
    final diff = bottomInset - currentBottomInset;
    currentBottomInset = bottomInset;
    double jumpOffset = _chatController.scrollController.offset + diff;

    if (jumpOffset >= 0 &&
        jumpOffset <=
            _chatController.scrollController.position.maxScrollExtent) {
      _chatController.scrollController.jumpTo(jumpOffset);
    }
  }

  Future<void> _getPairedDevices() async {
    if (Platform.isIOS) {
      _paired = true;
    } else if (Platform.isAndroid) {
      bool found = false;
      try {
        pairedDevices = await FlutterBluePlus.bondedDevices;
      } catch (e) {
        debugPrint('Cannot find bounded devices: $e');
        _paired = true;
        return;
      }

      for (final pairedDevice in pairedDevices) {
        if (pairedDevice.platformName.startsWith("Buddie")) {
          found = true;
          break;
        }
      }

      _paired = found;
    }
  }

  void _init() {
    if (widget.controller == null) {
      _audioController = RecordScreenController();
      _audioController.load();
    } else {
      _audioController = widget.controller!;
    }
    _audioController.attach(this);
    _listenable.addListener(_onHeaderChange);
    _chatController = ChatController(onNewMessage: onNewMessage);
    WidgetsBinding.instance.addPostFrameCallback((_) async {
      // No longer need to get user information without authentication
      final prefs = await SharedPreferences.getInstance();
      final isFirstLaunch = prefs.getBool("isFirstLaunch") ?? true;
      if (isFirstLaunch) {
        await prefs.setBool("isFirstLaunch", false);
        debugPrint('init recording');
        _showWelcomeMessages();
      } else {
        debugPrint('start recording');
      }
      FlutterForegroundTask.sendDataToTask('startRecording');
    });
  }

  Future<void> _showWelcomeMessages() async {
    await Future.delayed(const Duration(milliseconds: 500));
    setState(() {
      _chatController.newMessages.insert(
        0,
        welcome_constants.welcomeSentences[0],
      );
    });

    await Future.delayed(const Duration(milliseconds: 1000));
    setState(() {
      _chatController.newMessages.insert(
        0,
        welcome_constants.welcomeSentences[1],
      );
    });

    await Future.delayed(const Duration(milliseconds: 1000));
    setState(() {
      _chatController.newMessages.insert(
        0,
        welcome_constants.welcomeSentences[2],
      );
    });

    await Future.delayed(const Duration(milliseconds: 1000));
    setState(() {
      _chatController.newMessages.insert(
        0,
        welcome_constants.welcomeSentences[3],
      );
    });

    await Future.delayed(const Duration(milliseconds: 1000));
    setState(() {
      _chatController.newMessages.insert(
        0,
        welcome_constants.welcomeSentences[4],
      );
    });

    await Future.delayed(const Duration(milliseconds: 1000));
    setState(() {
      _chatController.newMessages.insert(
        0,
        welcome_constants.welcomeSentences[5],
      );
    });
  }

  void _onHeaderChange() {
    final state = _listenable.value;
    if (state != null) {
      final position = state.notifier.position;
      _viewportDimension ??= position.viewportDimension;
      final shrinkWrap = state.notifier.position.maxScrollExtent == 0;
      if (_shrinkWrap != shrinkWrap &&
          _viewportDimension == position.viewportDimension) {
        setState(() {
          _shrinkWrap = shrinkWrap;
        });
      }
    }
  }

  void onNewMessage() {
    if (mounted) {
      setState(() {});
    }
  }

  Widget _buildMsg(Map<String, dynamic> message) {
    final role = message['isUser'];
    final text = message['text'];
    final id = message['id'];
    Widget body = Padding(
      padding: EdgeInsets.only(bottom: lineSpace),
      child: ChatListTile(
        onLongPress: () => _chatController.copyToClipboard(context, text),
        role: role,
        text: text,
        style: textTextStyle,
        padding: chatPadding,
      ),
    );

    if (_chatController.unReadMessageId.value.contains(id)) {
      body = VisibilityDetector(
        key: UniqueKey(),
        onVisibilityChanged: (info) {
          if (info.visibleFraction == 1) {
            _chatController.unReadMessageId.value.remove(id);
            _chatController.unReadMessageId.notifyListeners();
          }
        },
        child: body,
      );
    }

    return body;
  }

  void _onClickKeyboard() {
    if (_focusNode.hasFocus) {
      _focusNode.unfocus();
    } else {
      _focusNode.requestFocus();
    }
  }

  void _onClickBluetooth() async {
    await _getPairedDevices();
    final remoteId = await FlutterForegroundTask.getData(key: 'deviceRemoteId');
    final deviceName = await FlutterForegroundTask.getData(key: 'deviceName');
    setState(() {
      if (_isBottomSheetShown) return;
      _isBottomSheetShown = true;
      showModalBottomSheet(
        context: context,
        isScrollControlled: true,
        builder: (context) {
          return SafeArea(
            child: ConstrainedBox(
              constraints: BoxConstraints(
                maxHeight:
                    (MediaQuery.of(context).size.height - kToolbarHeight) *
                    2 /
                    5,
              ),
              child: BLEScreen(
                paired: _paired,
                remoteId: remoteId,
                deviceName: deviceName,
              ),
            ),
          );
        },
      ).whenComplete(() {
        _isBottomSheetShown = false;
      });
    });
  }

  void _onClickRecord() {
    setState(() {
      _audioController.toggleRecording();
    });
  }

  void _onClickSendMessage() {
    _chatController.sendMessage();
  }

  void _onClickHelp() {
    _chatController.askHelp();
  }

  void _onClickBottomRight() {
    _focusNode.unfocus();
    context.pushNamed(RouteName.meeting_list);
  }

  var centerKey = GlobalKey();

  @override
  Widget build(BuildContext context) {
    final slivers = <Widget>[];
    final history = _chatController.historyMessages.reversed.toList();
    slivers.add(
      SliverList(
        delegate: SliverChildBuilderDelegate((BuildContext context, int i) {
          if (i > history.length) {
            return const SizedBox();
          }
          return _buildMsg(history[i]);
        }, childCount: history.length),
      ),
    );

    slivers.add(SliverPadding(padding: EdgeInsets.zero, key: centerKey));
    final newMessage = _chatController.newMessages.reversed.toList();
    slivers.add(
      SliverList(
        delegate: SliverChildBuilderDelegate((BuildContext context, int i) {
          if (i > newMessage.length) {
            return const SizedBox();
          }
          return _buildMsg(newMessage[i]);
        }, childCount: newMessage.length),
      ),
    );
    return KeyboardDismisser(
      child: Scaffold(
        resizeToAvoidBottomInset: true,
        body: AppBackground(
          child: Padding(
            padding: EdgeInsets.only(
              top: MediaQuery.of(context).padding.top,
              left: 10.sp,
              right: 10.sp,
              bottom: MediaQuery.of(context).fixedBottom,
            ),
            child: Column(
              children: [
                Padding(
                  padding: EdgeInsets.symmetric(vertical: 8.sp),
                  child: HomeAppBar(
                    bluetoothConnected: _audioController.connectionState,
                    onTapBluetooth: _onClickBluetooth,
                  ),
                ),
                SizedBox(height: 18.sp),
                Expanded(
                  child: Padding(
                    padding: EdgeInsets.only(bottom: 8.sp),
                    child: Stack(
                      children: [
                        RefreshIndicator(
                          displacement: 10,
                          onRefresh: _chatController.loadMoreMessages,
                          child: ClipRect(
                            child: CustomScrollView(
                              controller: _chatController.scrollController,
                              clipBehavior: Clip.none,
                              center: centerKey,
                              cacheExtent: 3,
                              slivers: slivers,
                            ),
                          ),
                        ),
                        Align(
                          alignment: AlignmentDirectional.bottomEnd,
                          child: ValueListenableBuilder(
                            valueListenable: _chatController.unReadMessageId,
                            builder: (context, ids, _) {
                              if (ids.isEmpty) return const SizedBox();
                              return GestureDetector(
                                onTap: () {
                                  _chatController.unReadMessageId.value = {};
                                  _chatController.firstScrollToBottom();
                                },
                                child: Container(
                                  width: 30,
                                  height: 30,
                                  decoration: BoxDecoration(
                                    borderRadius: BorderRadius.circular(30),
                                    color: Colors.blue,
                                  ),
                                  child: Center(
                                    child: Text(
                                      ids.length.toString(),
                                      style: const TextStyle(
                                        color: Colors.white,
                                      ),
                                    ),
                                  ),
                                ),
                              );
                            },
                          ),
                        ),
                      ],
                    ),
                  ),
                ),
                HomeBottomBar(
                  controller: _chatController.textController,
                  onTapKeyboard: _onClickKeyboard,
                  onSubmitted: (_) {},
                  onTapSend: _onClickSendMessage,
                  onTapLeft: _onClickRecord,
                  onTapHelp: _onClickHelp,
                  onTapRight: _onClickBottomRight,
                  isRecording: _audioController.isRecording,
                  isSpeakValueNotifier: _chatController.isSpeakValueNotifier,
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}

class EarphoneDialog extends StatelessWidget {
  final GestureTapCallback? onClickConnect;

  const EarphoneDialog({super.key, this.onClickConnect});

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        Image.asset(AssetsUtil.logo_hd, width: 116.sp, height: 116.sp),
      ],
    );
  }
}
