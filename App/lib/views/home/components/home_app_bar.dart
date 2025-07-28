import 'package:app/controllers/style_controller.dart';
import 'package:app/utils/assets_util.dart';
import 'package:app/utils/route_utils.dart';
import 'package:app/views/ui/bud_icon.dart';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:go_router/go_router.dart';
import 'package:provider/provider.dart';

import '../../../generated/l10n.dart';

class HomeAppBar extends StatelessWidget {
  final GestureTapCallback? onTapLogo;
  final BluetoothConnectionState bluetoothConnected;
  final GestureTapCallback? onTapBluetooth;

  const HomeAppBar({
    super.key,
    this.onTapLogo,
    this.bluetoothConnected = BluetoothConnectionState.disconnected,
    this.onTapBluetooth,
  });

  void _onClickSetting(BuildContext context) {
    context.pushNamed(RouteName.setting);
  }

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;

    return Row(
      children: [
        InkWell(
          splashColor: Colors.transparent,
          highlightColor: Colors.transparent,
          onTap: onTapLogo,
          child: BudIcon(icon: AssetsUtil.icon_chat_logo, size: 26.sp),
        ),
        SizedBox(width: 4.sp),
        Expanded(
          child: Text(
            S.of(context).appName,
            style: TextStyle(
              fontWeight: FontWeight.bold,
              fontSize: 18,
              color: isLightMode ? Colors.black : Colors.white,
            ),
          ),
        ),
        InkWell(
          splashColor: Colors.transparent,
          highlightColor: Colors.transparent,
          onTap: onTapBluetooth,
          child: BudIcon(
            icon: bluetoothConnected == BluetoothConnectionState.connected
                ? AssetsUtil.icon_bluetooth_connected
                : AssetsUtil.icon_bluetooth_disconnected,
            size: 26.sp,
          ),
        ),

        SizedBox(width: 12.sp),
        InkWell(
          splashColor: Colors.transparent,
          highlightColor: Colors.transparent,
          onTap: () => _onClickSetting(context),
          child: BudIcon(icon: AssetsUtil.icon_btn_setting, size: 26.sp),
        ),
      ],
    );
  }
}
