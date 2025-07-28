// DO NOT EDIT. This is code generated via package:intl/generate_localized.dart
// This is a library that provides messages for a zh locale. All the
// messages from the main program should be duplicated here with the same
// function name.

// Ignore issues from commonly used lints in this file.
// ignore_for_file:unnecessary_brace_in_string_interps, unnecessary_new
// ignore_for_file:prefer_single_quotes,comment_references, directives_ordering
// ignore_for_file:annotate_overrides,prefer_generic_function_type_aliases
// ignore_for_file:unused_import, file_names, avoid_escaping_inner_quotes
// ignore_for_file:unnecessary_string_interpolations, unnecessary_string_escapes

import 'package:intl/intl.dart';
import 'package:intl/message_lookup_by_library.dart';

final messages = new MessageLookup();

typedef String MessageIfAbsent(String messageStr, List<dynamic> args);

class MessageLookup extends MessageLookupByLibrary {
  String get localeName => 'zh';

  static String m0(e) => "刷新失败: ${e}";

  static String m1(length) => "AI模型状态已刷新，发现${length}个可用模型";

  static String m2(start, end) => "从 ${start} 到 ${end}";

  static String m3(version) => "版本 ${version}";

  static String m4(error) => "错误: ${error}";

  static String m5(count) => "${count} 已选择";

  final messages = _notInlinedMessages(_notInlinedMessages);
  static Map<String, Function> _notInlinedMessages(_) => <String, Function>{
    "ScanningBuddie": MessageLookupByLibrary.simpleMessage("扫描 Buddie 中..."),
    "aiDialogModel": MessageLookupByLibrary.simpleMessage("AI对话模型"),
    "aiDialogModelCustomLLMDescription": MessageLookupByLibrary.simpleMessage(
      "使用您配置的API Key",
    ),
    "aiDialogModelCustomLLMDisplayName": MessageLookupByLibrary.simpleMessage(
      "自定义模型（用户配置）",
    ),
    "aiDialogModelQwenOmniDescription": MessageLookupByLibrary.simpleMessage(
      "支持语音输入输出的多模态AI",
    ),
    "aiDialogModelQwenOmniDisplayName": MessageLookupByLibrary.simpleMessage(
      "通义千问多模态",
    ),
    "aiModelSetTitle": MessageLookupByLibrary.simpleMessage("AI模型设置"),
    "aiModelToastRefreshFailed": m0,
    "aiModelToastRefreshed": MessageLookupByLibrary.simpleMessage("AI模型状态已刷新"),
    "aiModelToastRefreshedAndFound": m1,
    "aiModelToastUnavailable": MessageLookupByLibrary.simpleMessage(
      "当前没有可用的AI模型，请检查配置",
    ),
    "aliyunAPIKeyConfigureDialogSubtitle": MessageLookupByLibrary.simpleMessage(
      "用于启用语音AI对话功能",
    ),
    "aliyunAPIKeyConfigureDialogTextFieldHint":
        MessageLookupByLibrary.simpleMessage("请输入阿里云DashScope API Key"),
    "aliyunAPIKeyConfigureDialogTextFieldLabel":
        MessageLookupByLibrary.simpleMessage("API Key"),
    "aliyunAPIKeyConfigureDialogTip": MessageLookupByLibrary.simpleMessage(
      "获取API Key:",
    ),
    "aliyunAPIKeyConfigureDialogTipContent":
        MessageLookupByLibrary.simpleMessage(
          "1. 访问阿里云DashScope控制台\n2. 创建API Key\n3. 复制API Key并粘贴到上方输入框",
        ),
    "aliyunAPIKeyConfigureDialogTitle": MessageLookupByLibrary.simpleMessage(
      "配置阿里云API Key",
    ),
    "aliyunAPIKeyDeleteDialogContent": MessageLookupByLibrary.simpleMessage(
      "删除后将无法使用语音AI对话功能，确定要删除API Key吗？",
    ),
    "aliyunAPIKeyDeleteDialogTitle": MessageLookupByLibrary.simpleMessage(
      "确认删除",
    ),
    "aliyunAPIKeyManageCurrent": MessageLookupByLibrary.simpleMessage(
      "当前API Key:",
    ),
    "aliyunAPIKeyManageFunctionDescription":
        MessageLookupByLibrary.simpleMessage("功能说明："),
    "aliyunAPIKeyManageFunctionDescriptionContent":
        MessageLookupByLibrary.simpleMessage(
          "• 配置阿里云DashScope API Key后可启用语音AI对话功能\n• 支持语音输入直接输出语音响应\n• 仅在对话模式下可用",
        ),
    "aliyunAPIKeyManageKeyConfigured": MessageLookupByLibrary.simpleMessage(
      "已配置API Key",
    ),
    "aliyunAPIKeyManageKeyUnset": MessageLookupByLibrary.simpleMessage(
      "未配置API Key，无法使用语音AI对话功能",
    ),
    "aliyunAPIKeyManageSubtitle2": MessageLookupByLibrary.simpleMessage(
      "配置语音AI对话功能",
    ),
    "aliyunAPIKeyManageSubtitleConfigured":
        MessageLookupByLibrary.simpleMessage("已配置语音AI对话功能"),
    "aliyunAPIKeyManageTitle": MessageLookupByLibrary.simpleMessage(
      "阿里云API Key管理",
    ),
    "appName": MessageLookupByLibrary.simpleMessage("Buddie"),
    "asrModeCloudOnlineDescription": MessageLookupByLibrary.simpleMessage(
      "短音频识别",
    ),
    "asrModeCloudOnlineTitle": MessageLookupByLibrary.simpleMessage("一句话识别"),
    "asrModeCloudStreamingDescription": MessageLookupByLibrary.simpleMessage(
      "实时流式处理",
    ),
    "asrModeCloudStreamingTitle": MessageLookupByLibrary.simpleMessage(
      "实时语音识别",
    ),
    "asrModeLocalOfflineDescription": MessageLookupByLibrary.simpleMessage(
      "本地离线处理",
    ),
    "asrModeLocalOfflineTitle": MessageLookupByLibrary.simpleMessage("本地模型识别"),
    "asrModeSetSubtitle": MessageLookupByLibrary.simpleMessage(
      "设置不同聊天模式的语音识别方式",
    ),
    "asrModeSetTitle": MessageLookupByLibrary.simpleMessage("ASR模式设置"),
    "audioPlayerTip": MessageLookupByLibrary.simpleMessage("文件不存在"),
    "buttonBack": MessageLookupByLibrary.simpleMessage("返回"),
    "buttonCancel": MessageLookupByLibrary.simpleMessage("取消"),
    "buttonClose": MessageLookupByLibrary.simpleMessage("关闭"),
    "buttonConfigure": MessageLookupByLibrary.simpleMessage("配置"),
    "buttonConfirm": MessageLookupByLibrary.simpleMessage("确认"),
    "buttonDelete": MessageLookupByLibrary.simpleMessage("删除"),
    "buttonHelpMe": MessageLookupByLibrary.simpleMessage("帮帮我 Buddie"),
    "buttonModify": MessageLookupByLibrary.simpleMessage("修改"),
    "buttonNextStep": MessageLookupByLibrary.simpleMessage("下一步"),
    "buttonSave": MessageLookupByLibrary.simpleMessage("保存"),
    "chapterOverview": MessageLookupByLibrary.simpleMessage("章节概述"),
    "chatModeDefault": MessageLookupByLibrary.simpleMessage("转录模式"),
    "chatModeDialog": MessageLookupByLibrary.simpleMessage("对话模式"),
    "chatModeMeeting": MessageLookupByLibrary.simpleMessage("会议模式"),
    "copiedToClipboard": MessageLookupByLibrary.simpleMessage("已复制到剪切板"),
    "copySummary": MessageLookupByLibrary.simpleMessage("复制摘要"),
    "copyTranscriptionText": MessageLookupByLibrary.simpleMessage("复制转录文本"),
    "editTitle": MessageLookupByLibrary.simpleMessage("编辑标题"),
    "exportAudio": MessageLookupByLibrary.simpleMessage("导出音频"),
    "exportDataDialogDateFromTo": m2,
    "exportDataDialogTip": MessageLookupByLibrary.simpleMessage("选择日期范围"),
    "exportDataDialogTipNoDate": MessageLookupByLibrary.simpleMessage(
      "日期范围未选择",
    ),
    "exportDataFileName": MessageLookupByLibrary.simpleMessage("文件名"),
    "exportDataSubtitle": MessageLookupByLibrary.simpleMessage("导出转录结果"),
    "exportDataTitle": MessageLookupByLibrary.simpleMessage("导出数据"),
    "fullTextSummary": MessageLookupByLibrary.simpleMessage("全文摘要"),
    "importAPIKeyDialogApiKeyTextFieldHint":
        MessageLookupByLibrary.simpleMessage("请输入您的 API Key"),
    "importAPIKeyDialogApiKeyTextFieldLabel":
        MessageLookupByLibrary.simpleMessage("API Key"),
    "importAPIKeyDialogApiURLTextFieldHint":
        MessageLookupByLibrary.simpleMessage(
          "https://api.openai.com/v1/chat/completions",
        ),
    "importAPIKeyDialogApiURLTextFieldLabel":
        MessageLookupByLibrary.simpleMessage("API URL"),
    "importAPIKeyDialogModelTextFieldHint":
        MessageLookupByLibrary.simpleMessage("gpt-3.5-turbo"),
    "importAPIKeyDialogModelTextFieldLabel":
        MessageLookupByLibrary.simpleMessage("Model"),
    "importAPIKeyDialogTip": MessageLookupByLibrary.simpleMessage(
      "保存后将使用您自己的 API Key 和指定模型进行对话",
    ),
    "importAPIKeyDialogTitle": MessageLookupByLibrary.simpleMessage(
      "设置自定义 API Key",
    ),
    "importAPIKeySubtitle": MessageLookupByLibrary.simpleMessage(
      "使用您自己的 API key",
    ),
    "importAPIKeyTitle": MessageLookupByLibrary.simpleMessage("输入 API Key"),
    "keyPointsReview": MessageLookupByLibrary.simpleMessage("要点审查"),
    "languageChinese": MessageLookupByLibrary.simpleMessage("中文"),
    "languageEnglish": MessageLookupByLibrary.simpleMessage("英语"),
    "meetingListTile": MessageLookupByLibrary.simpleMessage("会议"),
    "notPairedNoticeText1": MessageLookupByLibrary.simpleMessage(
      "Buddie 耳机未连接",
    ),
    "notPairedNoticeText2": MessageLookupByLibrary.simpleMessage(
      "请打开手机的蓝牙设置\n\n连接耳机以继续",
    ),
    "pageAboutFacebook": MessageLookupByLibrary.simpleMessage("Facebook"),
    "pageAboutOfficialMedias": MessageLookupByLibrary.simpleMessage("官方媒体"),
    "pageAboutOfficialWebsite": MessageLookupByLibrary.simpleMessage("官网"),
    "pageAboutPrivacyPolicy": MessageLookupByLibrary.simpleMessage("隐私政策"),
    "pageAboutTitle": MessageLookupByLibrary.simpleMessage("关于 Buddie"),
    "pageAboutUserAgreement": MessageLookupByLibrary.simpleMessage("用户协议"),
    "pageAboutVersion": m3,
    "pageAboutX": MessageLookupByLibrary.simpleMessage("X"),
    "pageBleConfirmText": MessageLookupByLibrary.simpleMessage("忘记"),
    "pageBleIsYour": MessageLookupByLibrary.simpleMessage("这是你的 Buddie?"),
    "pageBleSaved": MessageLookupByLibrary.simpleMessage("您的耳机已经被保存!"),
    "pageBleToastConnectFailed": MessageLookupByLibrary.simpleMessage(
      "连接失败. 请重试.",
    ),
    "pageBleToastConnectSuccess": MessageLookupByLibrary.simpleMessage("连接成功!"),
    "pageBleToastForgetSuccess": MessageLookupByLibrary.simpleMessage("忘记成功!"),
    "pageBleUnknownDevice": MessageLookupByLibrary.simpleMessage("未知设备"),
    "pageHelpTitle": MessageLookupByLibrary.simpleMessage("帮助说明"),
    "pageHelpUnavailable": MessageLookupByLibrary.simpleMessage("没有帮助说明可用"),
    "pageHomeTextFieldHint": MessageLookupByLibrary.simpleMessage("输入您的消息..."),
    "pageItemDetails": MessageLookupByLibrary.simpleMessage("详情"),
    "pageItemError": m4,
    "pageItemNoRecords": MessageLookupByLibrary.simpleMessage("找不到记录。"),
    "pageMeetingDetailAudioNotFound": MessageLookupByLibrary.simpleMessage(
      "音频文件未找到",
    ),
    "pageMeetingListSearchHint": MessageLookupByLibrary.simpleMessage("搜索会议"),
    "pageMeetingListSelected": m5,
    "pageSettingAIModeSetSubtitle3": MessageLookupByLibrary.simpleMessage(
      "未知模型",
    ),
    "pageSettingAboutSubtitle": MessageLookupByLibrary.simpleMessage(
      "学习更多关于 Buddie",
    ),
    "pageSettingAboutTitle": MessageLookupByLibrary.simpleMessage("关于"),
    "pageSettingDarkMode": MessageLookupByLibrary.simpleMessage("暗黑模式"),
    "pageSettingEarbudsUpgradeSubtitle": MessageLookupByLibrary.simpleMessage(
      "耳机升级",
    ),
    "pageSettingEarbudsUpgradeTitle": MessageLookupByLibrary.simpleMessage(
      "耳机升级",
    ),
    "pageSettingEarbudsUpgradeUnavailable":
        MessageLookupByLibrary.simpleMessage("没有可用于升级的耳机固件"),
    "pageSettingHelpSubtitle": MessageLookupByLibrary.simpleMessage("详细介绍文档"),
    "pageSettingHelpTitle": MessageLookupByLibrary.simpleMessage("帮助文档"),
    "pageSettingLanguage": MessageLookupByLibrary.simpleMessage("语言"),
    "pageSettingTitle": MessageLookupByLibrary.simpleMessage("设置"),
    "pageWelcomeText1": MessageLookupByLibrary.simpleMessage(
      "Thank you \nfor choosing ",
    ),
    "pageWelcomeText2": MessageLookupByLibrary.simpleMessage("Buddie!"),
    "pageWelcomeText3": MessageLookupByLibrary.simpleMessage(
      "Discover recording, \nchatting, and journaling \nfeatures.",
    ),
    "pageWelcomeText4": MessageLookupByLibrary.simpleMessage(
      "Just a few simple steps to \ncomplete your setup.",
    ),
  };
}
