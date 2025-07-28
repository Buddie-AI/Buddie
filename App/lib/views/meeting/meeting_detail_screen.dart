import 'dart:convert';

import 'package:app/controllers/style_controller.dart';
import 'package:app/extension/datetime_extension.dart';
import 'package:app/services/objectbox_service.dart';
import 'package:app/utils/assets_util.dart';
import 'package:app/utils/share_plus_util.dart';
import 'package:app/views/meeting/components/audio_player.dart';
import 'package:app/views/ui/bud_icon.dart';
import 'package:app/views/ui/bud_tab_bar.dart';
import 'package:app/views/ui/layout/bud_bottom_sheet.dart';
import 'package:app/views/ui/layout/bud_scaffold.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_markdown/flutter_markdown.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:go_router/go_router.dart';
import 'package:provider/provider.dart';

import '../../generated/l10n.dart';
import 'model/meeting_model.dart';

class MeetingDetailScreen extends StatefulWidget {
  final MeetingModel model;

  const MeetingDetailScreen({super.key, required this.model});

  @override
  State<MeetingDetailScreen> createState() => _MeetingDetailScreenState();
}

class _MeetingDetailScreenState extends State<MeetingDetailScreen>
    with SingleTickerProviderStateMixin {
  final List<String> _tabs = ['Transcription', 'Summary'];
  late TabController _tabController;
  bool isEditingTitle = false;

  @override
  void initState() {
    super.initState();
    _tabController = TabController(length: _tabs.length, vsync: this);
  }

  void _onClickShare() {
    showModalBottomSheet(
      context: context,
      barrierColor: const Color.fromRGBO(0, 0, 0, 0.6),
      builder: (context) {
        return ShareBottomSheet(
          audioPath: widget.model.audioPath,
          records: _buildCopiedRecords(),
          summary: _buildCopiedSummary(widget.model.fullContent),
        );
      },
    );
  }

  String _buildCopiedRecords() {
    String ret = '';
    final records = ObjectBoxService().getRecordsByTimeRange(
      widget.model.startTime,
      widget.model.endTime,
    );

    for (final record in records) {
      ret +=
          '${DateTime.fromMillisecondsSinceEpoch(record.createdAt!)} ${record.role}: ${record.content}\n';
    }

    return ret;
  }

  String _buildCopiedSummary(String fullContent) {
    final jsonContent = json.decode(fullContent);
    String ret =
        """## Full text summary:
${jsonContent['abstract']}

## Chapter overview:
${_buildContentForSections(jsonContent['sections'])}

## Key points review:
${_buildContentForKeyPoints(jsonContent['key_points'])}
""";

    return ret;
  }

  void _onClickMore(BuildContext context, TapDownDetails details) {
    showModalBottomSheet(
      context: context,
      barrierColor: const Color.fromRGBO(0, 0, 0, 0.6),
      builder: (context) {
        return EditBottomSheet(
          onEditTitle: _switchToSummaryAndEditTitle,
          audioPath: widget.model.audioPath,
          records: _buildCopiedRecords(),
          summary: _buildCopiedSummary(widget.model.fullContent),
        );
      },
    );
  }

  void _switchToSummaryAndEditTitle() {
    setState(() {
      isEditingTitle = true;
      _tabController.index = 1;
    });
  }

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    return DefaultTabController(
      length: _tabs.length,
      child: BudScaffold(
        title: 'meeting',
        actions: [
          InkWell(
            splashColor: Colors.transparent,
            highlightColor: Colors.transparent,
            onTap: _onClickShare,
            child: BudIcon(icon: AssetsUtil.icon_btn_share, size: 20.sp),
          ),
          SizedBox(width: 16.sp),
          InkWell(
            splashColor: Colors.transparent,
            highlightColor: Colors.transparent,
            onTapDown: (TapDownDetails details) =>
                _onClickMore(context, details),
            child: BudIcon(icon: AssetsUtil.icon_btn_more, size: 20.sp),
          ),
          SizedBox(width: 10.sp),
        ],
        body: Column(
          children: [
            Container(
              decoration: BoxDecoration(
                color: isLightMode ? Colors.white : const Color(0xFF141414),
                borderRadius: const BorderRadius.vertical(
                  bottom: Radius.circular(12),
                ),
              ),
              child: Column(
                children: [
                  Padding(
                    padding: EdgeInsets.only(
                      top: 21.sp,
                      left: 16.sp,
                      right: 16.sp,
                      bottom: 16.sp,
                    ),
                    child: widget.model.audioPath != null
                        ? AudioPlayer(filePath: widget.model.audioPath!)
                        : Text(S.of(context).pageMeetingDetailAudioNotFound),
                  ),
                  BudTabBar(tabs: _tabs, controller: _tabController),
                ],
              ),
            ),
            Expanded(
              child: TabBarView(
                controller: _tabController,
                children: [
                  ConvertVoiceWidget(
                    list: ObjectBoxService()
                        .getRecordsByTimeRange(
                          widget.model.startTime,
                          widget.model.endTime,
                        )
                        .map(
                          (record) => VoiceModel(
                            name: record.role!,
                            avatar: '',
                            timestamp: DateTime.fromMillisecondsSinceEpoch(
                              record.createdAt!,
                            ).toTimeFormatString(),
                            content: record.content!,
                          ),
                        )
                        .toList(),
                  ),
                  SummaryWidget(
                    title: widget.model.title,
                    content: widget.model.fullContent,
                    isEditingTitle: isEditingTitle,
                    onTitleEditComplete: (newTitle) {
                      ObjectBoxService().updateSummaryTitle(
                        widget.model.id,
                        newTitle,
                      );

                      setState(() {
                        isEditingTitle = false;
                        widget.model.title = newTitle;
                      });
                    },
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
}

class ShareBottomSheet extends StatelessWidget {
  final String? audioPath;
  final String? records;
  final String? summary;

  const ShareBottomSheet({
    super.key,
    this.audioPath,
    this.records,
    this.summary,
  });

  Widget buildItem({
    required IconData icon,
    required String title,
    GestureTapCallback? onTap,
  }) {
    return InkWell(
      onTap: onTap,
      child: Container(
        margin: EdgeInsets.only(bottom: 8.sp),
        padding: EdgeInsets.symmetric(horizontal: 16.sp, vertical: 18.sp),
        decoration: BoxDecoration(
          color: Colors.white,
          borderRadius: BorderRadius.circular(12.sp),
        ),
        child: Row(
          children: [
            Icon(icon, size: 24.sp),
            Expanded(
              child: Padding(
                padding: EdgeInsets.symmetric(horizontal: 8.sp),
                child: Text(title),
              ),
            ),
            Icon(
              Icons.arrow_forward_ios,
              size: 16.sp,
              color: const Color(0xFF999999),
            ),
          ],
        ),
      ),
    );
  }

  void _onClickCopySummary(BuildContext context) {
    Clipboard.setData(ClipboardData(text: summary!));
    ScaffoldMessenger.of(
      context,
    ).showSnackBar(SnackBar(content: Text(S.of(context).copiedToClipboard)));
    context.pop();
  }

  void _onClickCopyAndConvert(BuildContext context) {
    Clipboard.setData(ClipboardData(text: records!));
    ScaffoldMessenger.of(
      context,
    ).showSnackBar(SnackBar(content: Text(S.of(context).copiedToClipboard)));
    context.pop();
  }

  void _onClickExportAudio(BuildContext context) async {
    if (audioPath != null) {
      bool result = await SharePlusUtil.shareFile(path: audioPath!);
      debugPrint('share result:$result');
    }
    context.pop();
  }

  @override
  Widget build(BuildContext context) {
    return BudBottomSheet(
      color: const Color(0xFFF0F0F0),
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          buildItem(
            icon: Icons.copy,
            title: S.of(context).copySummary,
            onTap: () {
              _onClickCopySummary(context);
            },
          ),
          buildItem(
            icon: Icons.text_fields,
            title: S.of(context).copyTranscriptionText,
            onTap: () {
              _onClickCopyAndConvert(context);
            },
          ),
          buildItem(
            icon: Icons.exit_to_app,
            title: S.of(context).exportAudio,
            onTap: () {
              _onClickExportAudio(context);
            },
          ),
        ],
      ),
    );
  }
}

class EditBottomSheet extends StatelessWidget {
  final String? audioPath;
  final String? records;
  final String? summary;
  final VoidCallback? onEditTitle;

  const EditBottomSheet({
    super.key,
    this.audioPath,
    this.records,
    this.summary,
    this.onEditTitle,
  });

  Widget buildItem({
    required IconData icon,
    required String title,
    GestureTapCallback? onTap,
  }) {
    return InkWell(
      onTap: onTap,
      child: Container(
        margin: EdgeInsets.only(bottom: 8.sp),
        padding: EdgeInsets.symmetric(horizontal: 16.sp, vertical: 18.sp),
        decoration: BoxDecoration(
          color: Colors.white,
          borderRadius: BorderRadius.circular(12.sp),
        ),
        child: Row(
          children: [
            Icon(icon, size: 24.sp),
            Expanded(
              child: Padding(
                padding: EdgeInsets.symmetric(horizontal: 8.sp),
                child: Text(title),
              ),
            ),
            Icon(
              Icons.arrow_forward_ios,
              size: 16.sp,
              color: const Color(0xFF999999),
            ),
          ],
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return BudBottomSheet(
      color: const Color(0xFFF0F0F0),
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          buildItem(
            icon: Icons.edit,
            title: S.of(context).editTitle,
            onTap: () {
              if (onEditTitle != null) {
                onEditTitle!();
              }
              context.pop();
            },
          ),
        ],
      ),
    );
  }
}

class VoiceModel {
  final String name;

  bool get isMe => name == '发言人1';
  final String avatar;
  final String timestamp;
  final String content;

  VoiceModel({
    required this.name,
    required this.avatar,
    required this.timestamp,
    required this.content,
  });
}

class ConvertVoiceWidget extends StatelessWidget {
  final List<VoiceModel> list;

  const ConvertVoiceWidget({super.key, required this.list});

  @override
  Widget build(BuildContext context) {
    return ListView.separated(
      padding: EdgeInsets.only(top: 16.sp),
      itemCount: list.length,
      separatorBuilder: (_, index) => SizedBox(height: 16.sp),
      itemBuilder: (_, index) {
        VoiceModel model = list[index];
        return ConvertVoiceListTile(model: model);
      },
    );
  }
}

class ConvertVoiceListTile extends StatelessWidget {
  final VoiceModel model;
  final GestureTapCallback? onTapPlay;

  const ConvertVoiceListTile({super.key, required this.model, this.onTapPlay});

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    Widget playButton = InkWell(
      splashColor: Colors.transparent,
      highlightColor: Colors.transparent,
      onTap: onTapPlay,
      child: BudIcon(icon: AssetsUtil.icon_play_section, size: 14.sp),
    );
    Widget avatar = BudIcon(
      icon: model.isMe
          ? AssetsUtil.icon_spokesperson_1
          : AssetsUtil.icon_spokesperson_2,
      size: 16.sp,
    );
    TextStyle textStyle = const TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: 14,
    );
    return Padding(
      padding: EdgeInsets.symmetric(horizontal: 10.sp),
      child: Column(
        crossAxisAlignment: model.isMe
            ? CrossAxisAlignment.end
            : CrossAxisAlignment.start,
        children: [
          Row(
            mainAxisAlignment: model.isMe
                ? MainAxisAlignment.end
                : MainAxisAlignment.start,
            children: [
              if (model.isMe) playButton else avatar,
              Flexible(
                child: Padding(
                  padding: EdgeInsets.symmetric(horizontal: 8.sp),
                  child: Text(
                    '${model.name}  ${model.timestamp}',
                    style: textStyle.copyWith(
                      color: isLightMode
                          ? const Color(0xCC000000)
                          : const Color(0xCCFFFFFF),
                    ),
                  ),
                ),
              ),
              if (model.isMe) avatar else playButton,
            ],
          ),
          SizedBox(height: 12.sp),
          Text(
            model.content,
            textAlign: model.isMe ? TextAlign.right : TextAlign.left,
            style: textStyle.copyWith(
              color: isLightMode ? Colors.black : Colors.white,
            ),
          ),
        ],
      ),
    );
  }
}

class SummaryWidget extends StatefulWidget {
  final String? title;
  final String? content;
  final bool isEditingTitle;
  final Function? onTitleEditComplete;

  const SummaryWidget({
    super.key,
    this.title,
    this.content,
    this.isEditingTitle = false,
    this.onTitleEditComplete,
  });

  @override
  State<SummaryWidget> createState() => _SummaryWidgetState();
}

class _SummaryWidgetState extends State<SummaryWidget> {
  late TextEditingController _titleController;

  @override
  void initState() {
    super.initState();
    _titleController = TextEditingController(text: widget.title);
  }

  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    bool isEditing = widget.isEditingTitle;
    TextStyle titleTextStyle = TextStyle(
      fontWeight: FontWeight.bold,
      fontSize: 16,
      color: isLightMode ? Colors.black : Colors.white,
    );
    return Padding(
      padding: EdgeInsets.only(left: 16.sp, right: 16.sp),
      child: ListView(
        padding: EdgeInsets.zero,
        children: [
          Padding(
            padding: EdgeInsets.symmetric(vertical: 16.sp),
            child: Row(
              children: [
                BudIcon(icon: AssetsUtil.icon_clock_2, size: 14.sp),
                SizedBox(width: 4.sp),
                isEditing
                    ? Expanded(
                        child: TextField(
                          controller: _titleController,
                          style: titleTextStyle,
                          decoration: const InputDecoration(
                            border: UnderlineInputBorder(),
                            isDense: true,
                          ),
                          onSubmitted: (value) {
                            if (widget.onTitleEditComplete != null) {
                              widget.onTitleEditComplete!(value);
                            }
                          },
                        ),
                      )
                    : Text(_titleController.text, style: titleTextStyle),
              ],
            ),
          ),
          ExpansionCard(
            icon: AssetsUtil.icon_summary_1,
            title: S.of(context).fullTextSummary,
            content: jsonDecode(widget.content!)['abstract'],
          ),
          SizedBox(height: 12.sp),
          ExpansionCard(
            icon: AssetsUtil.icon_summary_2,
            title: S.of(context).chapterOverview,
            content: _buildContentForSections(
              jsonDecode(widget.content!)['sections'],
            ),
          ),
          SizedBox(height: 12.sp),
          ExpansionCard(
            icon: AssetsUtil.icon_summary_3,
            title: S.of(context).keyPointsReview,
            content: _buildContentForKeyPoints(
              jsonDecode(widget.content!)['key_points'],
            ),
          ),
        ],
      ),
    );
  }
}

class ExpansionCard extends StatefulWidget {
  final String icon;
  final String title;
  final String content;

  const ExpansionCard({
    super.key,
    required this.icon,
    required this.title,
    required this.content,
  });

  @override
  State<ExpansionCard> createState() => _ExpansionCardState();
}

class _ExpansionCardState extends State<ExpansionCard> {
  @override
  Widget build(BuildContext context) {
    final themeNotifier = Provider.of<ThemeNotifier>(context);
    bool isLightMode = themeNotifier.mode == Mode.light;
    return Container(
      padding: EdgeInsets.only(
        top: 8.sp,
        left: 16.sp,
        right: 16.sp,
        bottom: 12.sp,
      ),
      decoration: BoxDecoration(
        borderRadius: BorderRadius.circular(8.sp),
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
      child: Column(
        children: [
          Row(
            children: [
              BudIcon(icon: widget.icon, size: 12.sp),
              SizedBox(width: 8.sp),
              Text(
                widget.title,
                style: TextStyle(
                  fontWeight: FontWeight.bold,
                  fontSize: 16,
                  color: isLightMode ? Colors.black : Colors.white,
                ),
              ),
            ],
          ),
          SizedBox(height: 8.sp),
          Container(
            padding: EdgeInsets.symmetric(horizontal: 16.sp, vertical: 10.sp),
            decoration: BoxDecoration(
              color: isLightMode ? Colors.white : const Color(0x12FFFFFF),
              borderRadius: BorderRadius.circular(6),
            ),
            child: AnimatedCrossFade(
              duration: Duration(milliseconds: 300),
              firstChild: MarkdownBody(
                data: widget.content,
                styleSheet: MarkdownStyleSheet(
                  p: TextStyle(
                    fontSize: 14,
                    color: isLightMode
                        ? const Color(0xFF666666)
                        : const Color(0x99FFFFFF),
                  ),
                ),
              ),
              secondChild: Container(
                height: 50,
                child: SingleChildScrollView(
                  physics: NeverScrollableScrollPhysics(),
                  child: MarkdownBody(
                    data: widget.content,
                    styleSheet: MarkdownStyleSheet(
                      p: TextStyle(
                        fontSize: 14,
                        color: isLightMode
                            ? const Color(0xFF666666)
                            : const Color(0x99FFFFFF),
                      ),
                    ),
                  ),
                ),
              ),
              crossFadeState: CrossFadeState.showFirst,
            ),
          ),
        ],
      ),
    );
  }
}

String _buildContentForSections(List<dynamic> data) {
  StringBuffer markdownBuffer = StringBuffer();

  for (int i = 0; i < data.length; i++) {
    final section = data[i];
    final title = section["section_title"] ?? "Untitled";
    final description = section["detailed_description"] ?? "";

    markdownBuffer.writeln("#### $title");
    markdownBuffer.writeln();
    markdownBuffer.writeln(description);
    markdownBuffer.writeln();
  }

  return markdownBuffer.toString();
}

String _buildContentForKeyPoints(List<dynamic> data) {
  StringBuffer markdownBuffer = StringBuffer();

  for (int i = 0; i < data.length; i++) {
    final section = data[i];
    final title = section["owner"] ?? "Untitled".toString();
    final description = section["description"] ?? "";

    markdownBuffer.writeln("${i + 1}. $description");
    markdownBuffer.writeln();
  }

  return markdownBuffer.toString();
}
