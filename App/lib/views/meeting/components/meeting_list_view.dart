import 'package:flutter/cupertino.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';

import '../model/meeting_model.dart';
import 'draggable_meeting_tile.dart';

class MeetingListView extends StatelessWidget {
  final bool shrinkWrap;
  final List<MeetingModel> list;
  final Function(int index, bool isSelected) onSelect;
  final bool isMultiSelectMode;
  final Set<int> selectedIndexes;
  final Function onRefresh;

  const MeetingListView({
    super.key,
    this.shrinkWrap = false,
    required this.list,
    required this.onSelect,
    required this.isMultiSelectMode,
    required this.selectedIndexes,
    required this.onRefresh,
  });

  @override
  Widget build(BuildContext context) {
    return ListView.separated(
      shrinkWrap: shrinkWrap,
      padding: EdgeInsets.all(16.sp),
      itemCount: list.length,
      separatorBuilder: (context, index) => SizedBox(height: 12.sp),
      itemBuilder: (context, index) {
        MeetingModel model = list[index];
        return DraggableMeetingTile(
          model: model,
          isMultiSelectMode: isMultiSelectMode,
          isSelected: selectedIndexes.contains(index),
          onSelect: (isSelected) => onSelect(index, isSelected ?? false),
          onRefresh: onRefresh,
        );
      },
    );
  }
}