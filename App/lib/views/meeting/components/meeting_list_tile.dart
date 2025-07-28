import 'package:app/controllers/style_controller.dart';
import 'package:app/extension/datetime_extension.dart';
import 'package:app/extension/duration_extension.dart';
import 'package:app/utils/assets_util.dart';
import 'package:app/views/ui/bud_ui.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:provider/provider.dart';

import '../../../generated/l10n.dart';
import '../model/meeting_model.dart';

class MeetingListTile extends StatelessWidget {
  final MeetingModel model;
  final GestureTapCallback? onTap;
  final bool isMultiSelectMode;
  final bool isSelected;

  const MeetingListTile({
    super.key,
    required this.model,
    this.onTap,
    required this.isMultiSelectMode,
    required this.isSelected,
  });

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    return InkWell(
      splashColor: Colors.transparent,
      highlightColor: Colors.transparent,
      onTap: onTap,
      child: Container(
        decoration: BoxDecoration(
          borderRadius: BorderRadius.circular(8),
          color: isLightMode ? null : const Color(0x33FFFFFF),
          gradient: isLightMode
              ? const LinearGradient(
                  begin: Alignment.topCenter,
                  end: Alignment.bottomCenter,
                  colors: [Color(0xFFEDFEFF), Color(0xFFFFFFFF)],
                )
              : null,
          boxShadow: [
            isLightMode
                ? const BoxShadow(
                    color: Color(0x172A9ACA),
                    offset: Offset(0, 4),
                    blurRadius: 9,
                  )
                : const BoxShadow(color: Color(0x1AA2EDF3), blurRadius: 20),
          ],
        ),
        child: ListTile(
          trailing: isMultiSelectMode
              ? Icon(
                  isSelected
                      ? Icons.check_circle
                      : Icons.radio_button_unchecked,
                  color: isSelected ? Colors.blue : Colors.grey,
                )
              : null,
          title: Text(
            model.title ?? '',
            style: TextStyle(
              fontWeight: FontWeight.bold,
              fontSize: 16.sp,
              color: isLightMode ? Colors.black : Colors.white,
            ),
          ),
          subtitle: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Text(
                model.content,
                style: TextStyle(
                  fontSize: 14.sp,
                  color: isLightMode
                      ? const Color(0xFF666666)
                      : const Color(0x99FFFFFF),
                ),
              ),
              SizedBox(height: 12.sp),
              DefaultTextStyle(
                style: TextStyle(
                  fontSize: 12.sp,
                  color: isLightMode
                      ? const Color(0xFF999999)
                      : const Color(0x99FFFFFF),
                ),
                child: Row(
                  children: [
                    BudIcon(icon: AssetsUtil.icon_meeting, size: 14.sp),
                    Text(' ${S.of(context).meetingListTile}'),
                    SizedBox(width: 12.sp),
                    BudIcon(icon: AssetsUtil.icon_clock_1, size: 14.sp),
                    Text(' ${model.duration.toTimeFormatString()}'),
                    const Spacer(),
                    Text(
                      model.datetime.toDateFormatString(
                        showTime: false,
                        dateSplit: '/',
                      ),
                    ),
                  ],
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
