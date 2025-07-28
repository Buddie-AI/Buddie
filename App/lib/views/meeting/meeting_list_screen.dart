/// 会议列表界面组件
///
/// 提供会议记录的浏览和管理功能，主要特性包括：
/// 1. 会议记录展示：
///   - 按时间倒序排列
///   - 支持摘要和完整内容查看
///   - 关联音频路径显示
/// 2. 搜索功能：
///   - 关键词全文检索
///   - 动态结果过滤
///   - 搜索状态切换
/// 3. 批量操作：
///   - 多选模式管理
///   - 批量删除确认
///   - 选择状态持久化
///
/// 状态管理：
/// - 使用本地状态管理：
///   - 原始列表数据
///   - 搜索结果数据
///   - 多选模式状态
///   - 选中索引集合
///
/// 使用示例：
/// ```dart
/// // 简单初始化
/// const MeetingListScreen();
///
/// // 通过路由跳转
/// context.pushNamed(RouteName.meeting_list);
/// ```

import 'dart:convert';

import 'package:app/models/summary_entity.dart';
import 'package:app/services/objectbox_service.dart';
import 'package:app/utils/assets_util.dart';
import 'package:app/views/meeting/components/meeting_list_view.dart';
import 'package:app/views/ui/bud_ui.dart';
import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:go_router/go_router.dart';
import 'package:keyboard_dismisser/keyboard_dismisser.dart';

import '../../generated/l10n.dart';
import 'model/meeting_model.dart';

class MeetingListScreen extends StatefulWidget {
  const MeetingListScreen({super.key});

  @override
  State<MeetingListScreen> createState() => _MeetingListScreenState();
}

class _MeetingListScreenState extends State<MeetingListScreen> {
  List<MeetingModel> _list = [];
  bool _isMultiSelectMode = false;
  final Set<int> _selectedIndexes = {};

  /// search
  final TextEditingController _searchController = TextEditingController();
  bool _onSearch = false;
  List<MeetingModel> _searchResultList = [];

  @override
  void dispose() {
    _searchController.dispose();
    super.dispose();
  }

  @override
  void initState() {
    super.initState();
    _initList();
  }

  void _selectItem(int index, bool isSelected) {
    setState(() {
      if (isSelected) {
        _selectedIndexes.add(index);
      } else {
        _selectedIndexes.remove(index);
      }
      _isMultiSelectMode = _selectedIndexes.isNotEmpty;
    });
  }

  void _deleteSelectedItems() {
    List<int> selectedIds = [];
    setState(() {
      _selectedIndexes.toList()
        ..sort((a, b) => b.compareTo(a))
        ..forEach((index) {
          if (_onSearch) {
            selectedIds.add(_searchResultList[index].id);
            _searchResultList.removeAt(index);
          } else {
            selectedIds.add(_list[index].id);
            _list.removeAt(index);
          }
        });

      _selectedIndexes.clear();
      _isMultiSelectMode = false;
    });

    ObjectBoxService().deleteSummaries(selectedIds);
  }

  void _cancelSelection() {
    setState(() {
      _selectedIndexes.clear();
      _isMultiSelectMode = false;
    });
  }

  void _initList() {
    final results = ObjectBoxService().getMeetingSummaries();
    if (results != null) {
      setState(() {
        _list = results.reversed.map<MeetingModel>((SummaryEntity record) {
          MeetingModel model = MeetingModel(
            id: record.id,
            content: jsonDecode(record.content!)['abstract'],
            startTime: record.startTime,
            endTime: record.endTime,
            createdAt: record.createdAt,
            fullContent: record.content!,
            title: record.title,
            audioPath: record.audioPath,
          );
          return model;
        }).toList();
      });
    }
  }

  void _onSearchSubmitted(String query) {
    setState(() {
      if (query.isNotEmpty) {
        _onSearch = true;
        final results = ObjectBoxService().getSummariesByKeyword(
          query,
          isMeeting: true,
        );
        _searchResultList =
            results?.reversed
                .map(
                  (record) => MeetingModel(
                    id: record.id,
                    content: jsonDecode(record.content!)['abstract'],
                    startTime: record.startTime,
                    endTime: record.endTime,
                    createdAt: record.createdAt,
                    fullContent: record.content!,
                    title: record.title,
                  ),
                )
                .toList() ??
            [];
      } else {
        _onSearch = false;
        _searchResultList.clear();
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    return KeyboardDismisser(
      child: Scaffold(
        body: AppBackground(
          child: SafeArea(
            child: Column(
              children: [
                Padding(
                  padding: EdgeInsets.all(16.sp),
                  child: BudSearchBar(
                    controller: _searchController,
                    onTapLeading: () => {
                      _onSearch
                          ? setState(() {
                              _onSearch = !_onSearch;
                              _searchController.clear();
                            })
                          : context.pop(),
                    },
                    leadingIcon: AssetsUtil.icon_arrow_back,
                    trailingIcon: AssetsUtil.icon_search,
                    hintText: S.of(context).pageMeetingListSearchHint,
                    onSubmitted: _onSearchSubmitted,
                  ),
                ),
                Expanded(
                  child: MeetingListView(
                    list: _onSearch ? _searchResultList : _list,
                    onSelect: _selectItem,
                    isMultiSelectMode: _isMultiSelectMode,
                    selectedIndexes: _selectedIndexes,
                    onRefresh: () {
                      setState(() {
                        _initList();
                      });
                    },
                  ),
                ),
              ],
            ),
          ),
        ),
        bottomNavigationBar: _isMultiSelectMode
            ? BottomAppBar(
                color: Colors.white,
                child: SizedBox(
                  height: 20.sp,
                  child: Row(
                    children: [
                      Text(
                        S
                            .of(context)
                            .pageMeetingListSelected(
                              '${_selectedIndexes.length}',
                            ),
                        style: TextStyle(
                          fontSize: 16.sp,
                          fontWeight: FontWeight.bold,
                          color: Colors.black,
                        ),
                      ),
                      const Spacer(),
                      TextButton(
                        onPressed: _cancelSelection,
                        child: Text(
                          S.of(context).buttonCancel,
                          style: const TextStyle(color: Colors.blue),
                        ),
                      ),
                      TextButton(
                        onPressed: _deleteSelectedItems,
                        child: Text(
                          S.of(context).buttonDelete,
                          style: const TextStyle(color: Colors.red),
                        ),
                      ),
                    ],
                  ),
                ),
              )
            : null,
      ),
    );
  }
}
