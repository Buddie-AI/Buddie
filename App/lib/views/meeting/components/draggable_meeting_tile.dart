import 'package:app/utils/route_utils.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:go_router/go_router.dart';
import '../model/meeting_model.dart';
import 'meeting_list_tile.dart';

class DraggableMeetingTile extends StatefulWidget {
  final MeetingModel model;
  final bool isMultiSelectMode;
  final bool isSelected;
  final Function(bool)? onSelect;
  final Function onRefresh;

  const DraggableMeetingTile({
    super.key,
    required this.model,
    required this.isMultiSelectMode,
    required this.isSelected,
    required this.onSelect,
    required this.onRefresh,
  });

  @override
  State<DraggableMeetingTile> createState() => _DraggableMeetingTileState();
}

class _DraggableMeetingTileState extends State<DraggableMeetingTile> {
  double _dragOffset = 0.0;
  final double _maxDragOffset = -20.sp;
  bool _isDragged = false;

  void _onDragUpdate(DragUpdateDetails details) {
    setState(() {
      _dragOffset = (_dragOffset + details.delta.dx).clamp(_maxDragOffset, 0);
      if (_dragOffset <= _maxDragOffset * 0.7) {
        _isDragged = true;
      }
    });
  }

  void _onDragEnd(DragEndDetails details) {
    if (_isDragged) {
      widget.onSelect?.call(!widget.isSelected);
    }
    setState(() {
      _dragOffset = 0;
    });
  }

  void _onClickItem({required BuildContext context, required MeetingModel item}) {
    context.pushNamed(
      RouteName.meeting_detail,
      extra: item,
    ).then((result) {
      widget.onRefresh();
    });
  }

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onHorizontalDragUpdate: _onDragUpdate,
      onHorizontalDragEnd: _onDragEnd,
      child: Transform.translate(
        offset: Offset(_dragOffset, 0),
        child: MeetingListTile(
          model: widget.model,
          onTap: widget.isMultiSelectMode
              ? () => widget.onSelect?.call(!widget.isSelected)
              : () => _onClickItem(context: context, item: widget.model),
          isMultiSelectMode: widget.isMultiSelectMode,
          isSelected: widget.isSelected,
        ),
      ),
    );
  }
}