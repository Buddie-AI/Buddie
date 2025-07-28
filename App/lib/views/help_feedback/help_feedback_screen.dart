import 'dart:convert';

import 'package:app/extension/context_extension.dart';
import 'package:app/utils/assets_util.dart';
import 'package:app/views/ui/app_background.dart';
import 'package:extra_hittest_area/extra_hittest_area.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:go_router/go_router.dart';
import '../../generated/l10n.dart';
import '../ui/bud_icon.dart';

class HelpFeedbackScreen extends StatefulWidget {
  final String locale;

  const HelpFeedbackScreen({super.key, this.locale = 'en'});

  @override
  State<HelpFeedbackScreen> createState() => _HelpFeedbackScreenState();
}

class _HelpFeedbackScreenState extends State<HelpFeedbackScreen> {
  List<_InfoData> _quickHelp = [];
  bool _isLoading = true;

  @override
  void initState() {
    super.initState();
    _loadQAData();
  }

  Future<void> _loadQAData() async {
    try {
      String path = 'assets/qa_${widget.locale}.json';
      final String qaJson = await rootBundle.loadString(path);
      final Map<String, dynamic> data = json.decode(qaJson);

      final List<_InfoData> qaList = [];

      data.forEach((key, value) {
        final String question = value['question'];
        final dynamic answer = value['answer'];

        if (answer is String) {
          qaList.add(_InfoData(question, answer));
        } else if (answer is List) {
          qaList.add(_InfoData(question, "", isList: true, listContent: List<String>.from(answer)));
        } else if (answer is Map) {
          qaList.add(_InfoData(question, "", isStructured: true, structuredContent: Map<String, dynamic>.from(answer)));
        }
      });

      setState(() {
        _quickHelp = qaList;
        _isLoading = false;
      });
    } catch (e) {
      setState(() {
        _isLoading = false;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: AppBackground(
        child: SafeArea(
          child: DefaultTextStyle(
            style: TextStyle(color: context.isLightMode ? Colors.black : Colors.white),
            child: Column(
              children: [
                _buildAppbar(context),
                SizedBox(height: 20.h),
                Expanded(
                  child: _isLoading
                      ? Center(
                          child: CircularProgressIndicator(color: context.isLightMode ? Colors.black : Colors.white),
                        )
                      : _quickHelp.isEmpty
                      ? Center(
                          child: Text(
                            S.of(context).pageHelpUnavailable,
                            style: TextStyle(fontSize: 14.sp, color: context.isLightMode ? Colors.black : Colors.white),
                          ),
                        )
                      : Column(
                          children: [
                            Flexible(
                              child: Padding(
                                padding: EdgeInsets.all(16.r),
                                child: _buildInfoGroup(context, _quickHelp, true),
                              ),
                            ),
                          ],
                        ),
                ),
                SizedBox(height: 17.h),
              ],
            ),
          ),
        ),
      ),
    );
  }

  Widget _buildAppbar(BuildContext context) {
    return SizedBox(
      height: 40.r,
      child: Stack(
        children: [
          Align(
            alignment: AlignmentDirectional.centerStart,
            child: GestureDetectorHitTestWithoutSizeLimit(
              // debugHitTestAreaColor: Colors.red.withOpacity(0.5),
              extraHitTestArea: EdgeInsets.all(20.sp),
              onTap: () => context.pop(),
              child: Padding(
                padding: EdgeInsets.only(right: 18.sp),
                child: BudIcon(icon: AssetsUtil.icon_arrow_back, size: 20.sp),
              ),
            ),
          ),
          Center(
            child: Text(
              S.of(context).pageHelpTitle,
              style: TextStyle(
                color: context.isLightMode ? Colors.black : Colors.white,
                fontSize: 16.sp,
                fontWeight: FontWeight.bold,
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildInfoGroup(BuildContext context, List<_InfoData> infoList, bool scrollable) {
    return Container(
      decoration: BoxDecoration(
        color: context.isLightMode ? Colors.white : const Color(0xff394044),
        borderRadius: BorderRadius.circular(8).r,
      ),
      child: Theme(
        data: Theme.of(context).copyWith(
          splashColor: Colors.transparent,
          highlightColor: Colors.transparent,
          splashFactory: NoSplash.splashFactory,
        ),
        child: ListView.separated(
          shrinkWrap: true,
          physics: scrollable ? const ClampingScrollPhysics() : const NeverScrollableScrollPhysics(),
          itemBuilder: (context, index) {
            final info = infoList[index];
            return ExpansionTile(
              title: Text(
                info.title,
                style: TextStyle(color: context.isLightMode ? Colors.black : Colors.white, fontSize: 14.sp),
              ),
              tilePadding: EdgeInsets.symmetric(horizontal: 7.w),
              collapsedIconColor: context.isLightMode ? Colors.black : Colors.white,
              collapsedBackgroundColor: Colors.transparent,
              backgroundColor: Colors.transparent,
              iconColor: context.isLightMode ? Colors.black : Colors.white,
              shape: const Border(),
              collapsedShape: const Border(),
              children: [
                Padding(
                  padding: EdgeInsets.symmetric(horizontal: 10.w),
                  child: _buildContent(context, info),
                ),
                SizedBox(height: 15.h),
              ],
            );
          },
          separatorBuilder: (context, index) {
            return Divider(
              height: 2,
              indent: 7.w,
              endIndent: 7.w,
              color: context.isLightMode ? Colors.black.withAlpha(10) : Colors.white.withAlpha(10),
            );
          },
          itemCount: infoList.length,
        ),
      ),
    );
  }

  Widget _buildContent(BuildContext context, _InfoData info) {
    final textStyle = TextStyle(
      color: context.isLightMode ? const Color(0xff999999) : Colors.white.withAlpha(80),
      fontSize: 12.sp,
    );

    if (info.isStructured) {
      return Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: info.structuredContent.entries.map((entry) {
          if (entry.key.startsWith("steps")) {
            return Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: (entry.value as List).map((step) {
                return Padding(
                  padding: EdgeInsets.only(left: 5.w, top: 5.h),
                  child: Row(
                    children: [
                      Text("• ", style: textStyle),
                      Expanded(child: Text(step, style: textStyle)),
                    ],
                  ),
                );
              }).toList(),
            );
          } else {
            return Padding(
              padding: EdgeInsets.only(top: 5.h),
              child: Text(entry.value.toString(), style: textStyle),
            );
          }
        }).toList(),
      );
    } else if (info.isList) {
      return Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: info.listContent.map((item) {
          return Padding(
            padding: EdgeInsets.only(left: 5.w, top: 5.h),
            child: Row(
              children: [
                Text("• ", style: textStyle),
                Expanded(child: Text(item, style: textStyle)),
              ],
            ),
          );
        }).toList(),
      );
    } else {
      return Text(info.content, style: textStyle);
    }
  }
}

class _InfoData {
  final String title;
  final String content;
  final bool isStructured;
  final bool isList;
  final Map<String, dynamic> structuredContent;
  final List<String> listContent;

  _InfoData(
    this.title,
    this.content, {
    this.isStructured = false,
    this.isList = false,
    this.structuredContent = const {},
    this.listContent = const [],
  });
}
