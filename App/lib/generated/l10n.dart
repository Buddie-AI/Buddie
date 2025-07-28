// GENERATED CODE - DO NOT MODIFY BY HAND
import 'package:flutter/material.dart';
import 'package:intl/intl.dart';
import 'intl/messages_all.dart';

// **************************************************************************
// Generator: Flutter Intl IDE plugin
// Made by Localizely
// **************************************************************************

// ignore_for_file: non_constant_identifier_names, lines_longer_than_80_chars
// ignore_for_file: join_return_with_assignment, prefer_final_in_for_each
// ignore_for_file: avoid_redundant_argument_values, avoid_escaping_inner_quotes

class S {
  S();

  static S? _current;

  static S get current {
    assert(
      _current != null,
      'No instance of S was loaded. Try to initialize the S delegate before accessing S.current.',
    );
    return _current!;
  }

  static const AppLocalizationDelegate delegate = AppLocalizationDelegate();

  static Future<S> load(Locale locale) {
    final name = (locale.countryCode?.isEmpty ?? false)
        ? locale.languageCode
        : locale.toString();
    final localeName = Intl.canonicalizedLocale(name);
    return initializeMessages(localeName).then((_) {
      Intl.defaultLocale = localeName;
      final instance = S();
      S._current = instance;

      return instance;
    });
  }

  static S of(BuildContext context) {
    final instance = S.maybeOf(context);
    assert(
      instance != null,
      'No instance of S present in the widget tree. Did you add S.delegate in localizationsDelegates?',
    );
    return instance!;
  }

  static S? maybeOf(BuildContext context) {
    return Localizations.of<S>(context, S);
  }

  /// `Buddie`
  String get appName {
    return Intl.message('Buddie', name: 'appName', desc: '', args: []);
  }

  /// `Next step`
  String get buttonNextStep {
    return Intl.message(
      'Next step',
      name: 'buttonNextStep',
      desc: '',
      args: [],
    );
  }

  /// `Cancel`
  String get buttonCancel {
    return Intl.message('Cancel', name: 'buttonCancel', desc: '', args: []);
  }

  /// `Confirm`
  String get buttonConfirm {
    return Intl.message('Confirm', name: 'buttonConfirm', desc: '', args: []);
  }

  /// `Save`
  String get buttonSave {
    return Intl.message('Save', name: 'buttonSave', desc: '', args: []);
  }

  /// `Delete`
  String get buttonDelete {
    return Intl.message('Delete', name: 'buttonDelete', desc: '', args: []);
  }

  /// `Close`
  String get buttonClose {
    return Intl.message('Close', name: 'buttonClose', desc: '', args: []);
  }

  /// `Back`
  String get buttonBack {
    return Intl.message('Back', name: 'buttonBack', desc: '', args: []);
  }

  /// `Modify`
  String get buttonModify {
    return Intl.message('Modify', name: 'buttonModify', desc: '', args: []);
  }

  /// `Configure`
  String get buttonConfigure {
    return Intl.message(
      'Configure',
      name: 'buttonConfigure',
      desc: '',
      args: [],
    );
  }

  /// `Help me Buddie`
  String get buttonHelpMe {
    return Intl.message(
      'Help me Buddie',
      name: 'buttonHelpMe',
      desc: '',
      args: [],
    );
  }

  /// `Thank you \nfor choosing `
  String get pageWelcomeText1 {
    return Intl.message(
      'Thank you \nfor choosing ',
      name: 'pageWelcomeText1',
      desc: '',
      args: [],
    );
  }

  /// `Buddie!`
  String get pageWelcomeText2 {
    return Intl.message(
      'Buddie!',
      name: 'pageWelcomeText2',
      desc: '',
      args: [],
    );
  }

  /// `Discover recording, \nchatting, and journaling \nfeatures.`
  String get pageWelcomeText3 {
    return Intl.message(
      'Discover recording, \nchatting, and journaling \nfeatures.',
      name: 'pageWelcomeText3',
      desc: '',
      args: [],
    );
  }

  /// `Just a few simple steps to \ncomplete your setup.`
  String get pageWelcomeText4 {
    return Intl.message(
      'Just a few simple steps to \ncomplete your setup.',
      name: 'pageWelcomeText4',
      desc: '',
      args: [],
    );
  }

  /// `Enter your message...`
  String get pageHomeTextFieldHint {
    return Intl.message(
      'Enter your message...',
      name: 'pageHomeTextFieldHint',
      desc: '',
      args: [],
    );
  }

  /// `Scanning Buddie...`
  String get ScanningBuddie {
    return Intl.message(
      'Scanning Buddie...',
      name: 'ScanningBuddie',
      desc: '',
      args: [],
    );
  }

  /// `Buddie Earbuds Not Connected`
  String get notPairedNoticeText1 {
    return Intl.message(
      'Buddie Earbuds Not Connected',
      name: 'notPairedNoticeText1',
      desc: '',
      args: [],
    );
  }

  /// `Please go to your phone's Bluetooth settings\n\nConnect them before continuing`
  String get notPairedNoticeText2 {
    return Intl.message(
      'Please go to your phone\'s Bluetooth settings\n\nConnect them before continuing',
      name: 'notPairedNoticeText2',
      desc: '',
      args: [],
    );
  }

  /// `File does not exist`
  String get audioPlayerTip {
    return Intl.message(
      'File does not exist',
      name: 'audioPlayerTip',
      desc: '',
      args: [],
    );
  }

  /// `Settings`
  String get pageSettingTitle {
    return Intl.message(
      'Settings',
      name: 'pageSettingTitle',
      desc: '',
      args: [],
    );
  }

  /// `Dark Mode`
  String get pageSettingDarkMode {
    return Intl.message(
      'Dark Mode',
      name: 'pageSettingDarkMode',
      desc: '',
      args: [],
    );
  }

  /// `Language`
  String get pageSettingLanguage {
    return Intl.message(
      'Language',
      name: 'pageSettingLanguage',
      desc: '',
      args: [],
    );
  }

  /// `English`
  String get languageEnglish {
    return Intl.message('English', name: 'languageEnglish', desc: '', args: []);
  }

  /// `Chinese`
  String get languageChinese {
    return Intl.message('Chinese', name: 'languageChinese', desc: '', args: []);
  }

  /// `ASR Mode Setting`
  String get asrModeSetTitle {
    return Intl.message(
      'ASR Mode Setting',
      name: 'asrModeSetTitle',
      desc: '',
      args: [],
    );
  }

  /// `Set different speech recognition methods for each chat mode`
  String get asrModeSetSubtitle {
    return Intl.message(
      'Set different speech recognition methods for each chat mode',
      name: 'asrModeSetSubtitle',
      desc: '',
      args: [],
    );
  }

  /// `Transcription Mode`
  String get chatModeDefault {
    return Intl.message(
      'Transcription Mode',
      name: 'chatModeDefault',
      desc: '',
      args: [],
    );
  }

  /// `Dialog Mode`
  String get chatModeDialog {
    return Intl.message(
      'Dialog Mode',
      name: 'chatModeDialog',
      desc: '',
      args: [],
    );
  }

  /// `Meeting Mode`
  String get chatModeMeeting {
    return Intl.message(
      'Meeting Mode',
      name: 'chatModeMeeting',
      desc: '',
      args: [],
    );
  }

  /// `Real-time speech recognition`
  String get asrModeCloudStreamingTitle {
    return Intl.message(
      'Real-time speech recognition',
      name: 'asrModeCloudStreamingTitle',
      desc: '',
      args: [],
    );
  }

  /// `Real-time streaming processing`
  String get asrModeCloudStreamingDescription {
    return Intl.message(
      'Real-time streaming processing',
      name: 'asrModeCloudStreamingDescription',
      desc: '',
      args: [],
    );
  }

  /// `Sentence recognition`
  String get asrModeCloudOnlineTitle {
    return Intl.message(
      'Sentence recognition',
      name: 'asrModeCloudOnlineTitle',
      desc: '',
      args: [],
    );
  }

  /// `Short audio recognition`
  String get asrModeCloudOnlineDescription {
    return Intl.message(
      'Short audio recognition',
      name: 'asrModeCloudOnlineDescription',
      desc: '',
      args: [],
    );
  }

  /// `Local model recognition`
  String get asrModeLocalOfflineTitle {
    return Intl.message(
      'Local model recognition',
      name: 'asrModeLocalOfflineTitle',
      desc: '',
      args: [],
    );
  }

  /// `Local offline processing`
  String get asrModeLocalOfflineDescription {
    return Intl.message(
      'Local offline processing',
      name: 'asrModeLocalOfflineDescription',
      desc: '',
      args: [],
    );
  }

  /// `AI Model Settings`
  String get aiModelSetTitle {
    return Intl.message(
      'AI Model Settings',
      name: 'aiModelSetTitle',
      desc: '',
      args: [],
    );
  }

  /// `AI model status has been refreshed, Discover {length} available models`
  String aiModelToastRefreshedAndFound(Object length) {
    return Intl.message(
      'AI model status has been refreshed, Discover $length available models',
      name: 'aiModelToastRefreshedAndFound',
      desc: '',
      args: [length],
    );
  }

  /// `The status of the AI model has been refreshed`
  String get aiModelToastRefreshed {
    return Intl.message(
      'The status of the AI model has been refreshed',
      name: 'aiModelToastRefreshed',
      desc: '',
      args: [],
    );
  }

  /// `No AI model is available. Please check your API Key or network settings`
  String get aiModelToastUnavailable {
    return Intl.message(
      'No AI model is available. Please check your API Key or network settings',
      name: 'aiModelToastUnavailable',
      desc: '',
      args: [],
    );
  }

  /// `Refresh failed: {e}`
  String aiModelToastRefreshFailed(Object e) {
    return Intl.message(
      'Refresh failed: $e',
      name: 'aiModelToastRefreshFailed',
      desc: '',
      args: [e],
    );
  }

  /// `AI dialogue model`
  String get aiDialogModel {
    return Intl.message(
      'AI dialogue model',
      name: 'aiDialogModel',
      desc: '',
      args: [],
    );
  }

  /// `Custom model (user configuration)`
  String get aiDialogModelCustomLLMDisplayName {
    return Intl.message(
      'Custom model (user configuration)',
      name: 'aiDialogModelCustomLLMDisplayName',
      desc: '',
      args: [],
    );
  }

  /// `Use the API Key you configured`
  String get aiDialogModelCustomLLMDescription {
    return Intl.message(
      'Use the API Key you configured',
      name: 'aiDialogModelCustomLLMDescription',
      desc: '',
      args: [],
    );
  }

  /// `Qwen Omni Multimodal`
  String get aiDialogModelQwenOmniDisplayName {
    return Intl.message(
      'Qwen Omni Multimodal',
      name: 'aiDialogModelQwenOmniDisplayName',
      desc: '',
      args: [],
    );
  }

  /// `Multimodal AI that supports voice input and output`
  String get aiDialogModelQwenOmniDescription {
    return Intl.message(
      'Multimodal AI that supports voice input and output',
      name: 'aiDialogModelQwenOmniDescription',
      desc: '',
      args: [],
    );
  }

  /// `Unknown model`
  String get pageSettingAIModeSetSubtitle3 {
    return Intl.message(
      'Unknown model',
      name: 'pageSettingAIModeSetSubtitle3',
      desc: '',
      args: [],
    );
  }

  /// `Import API Key`
  String get importAPIKeyTitle {
    return Intl.message(
      'Import API Key',
      name: 'importAPIKeyTitle',
      desc: '',
      args: [],
    );
  }

  /// `Use your own API key`
  String get importAPIKeySubtitle {
    return Intl.message(
      'Use your own API key',
      name: 'importAPIKeySubtitle',
      desc: '',
      args: [],
    );
  }

  /// `Setting up custom API Key`
  String get importAPIKeyDialogTitle {
    return Intl.message(
      'Setting up custom API Key',
      name: 'importAPIKeyDialogTitle',
      desc: '',
      args: [],
    );
  }

  /// `API Key`
  String get importAPIKeyDialogApiKeyTextFieldLabel {
    return Intl.message(
      'API Key',
      name: 'importAPIKeyDialogApiKeyTextFieldLabel',
      desc: '',
      args: [],
    );
  }

  /// `Please input your API Key`
  String get importAPIKeyDialogApiKeyTextFieldHint {
    return Intl.message(
      'Please input your API Key',
      name: 'importAPIKeyDialogApiKeyTextFieldHint',
      desc: '',
      args: [],
    );
  }

  /// `API URL`
  String get importAPIKeyDialogApiURLTextFieldLabel {
    return Intl.message(
      'API URL',
      name: 'importAPIKeyDialogApiURLTextFieldLabel',
      desc: '',
      args: [],
    );
  }

  /// `https://api.openai.com/v1/chat/completions`
  String get importAPIKeyDialogApiURLTextFieldHint {
    return Intl.message(
      'https://api.openai.com/v1/chat/completions',
      name: 'importAPIKeyDialogApiURLTextFieldHint',
      desc: '',
      args: [],
    );
  }

  /// `Model`
  String get importAPIKeyDialogModelTextFieldLabel {
    return Intl.message(
      'Model',
      name: 'importAPIKeyDialogModelTextFieldLabel',
      desc: '',
      args: [],
    );
  }

  /// `gpt-3.5-turbo`
  String get importAPIKeyDialogModelTextFieldHint {
    return Intl.message(
      'gpt-3.5-turbo',
      name: 'importAPIKeyDialogModelTextFieldHint',
      desc: '',
      args: [],
    );
  }

  /// `After saving, you will use your own API Key and the specified model to talk.`
  String get importAPIKeyDialogTip {
    return Intl.message(
      'After saving, you will use your own API Key and the specified model to talk.',
      name: 'importAPIKeyDialogTip',
      desc: '',
      args: [],
    );
  }

  /// `Aliyun API Key Management`
  String get aliyunAPIKeyManageTitle {
    return Intl.message(
      'Aliyun API Key Management',
      name: 'aliyunAPIKeyManageTitle',
      desc: '',
      args: [],
    );
  }

  /// `Configured voice AI dialogue function`
  String get aliyunAPIKeyManageSubtitleConfigured {
    return Intl.message(
      'Configured voice AI dialogue function',
      name: 'aliyunAPIKeyManageSubtitleConfigured',
      desc: '',
      args: [],
    );
  }

  /// `Configure voice AI dialogue function`
  String get aliyunAPIKeyManageSubtitle2 {
    return Intl.message(
      'Configure voice AI dialogue function',
      name: 'aliyunAPIKeyManageSubtitle2',
      desc: '',
      args: [],
    );
  }

  /// `API Key Configured`
  String get aliyunAPIKeyManageKeyConfigured {
    return Intl.message(
      'API Key Configured',
      name: 'aliyunAPIKeyManageKeyConfigured',
      desc: '',
      args: [],
    );
  }

  /// `Current API Key:`
  String get aliyunAPIKeyManageCurrent {
    return Intl.message(
      'Current API Key:',
      name: 'aliyunAPIKeyManageCurrent',
      desc: '',
      args: [],
    );
  }

  /// `API Key is not configured, voice AI dialogue function cannot be used`
  String get aliyunAPIKeyManageKeyUnset {
    return Intl.message(
      'API Key is not configured, voice AI dialogue function cannot be used',
      name: 'aliyunAPIKeyManageKeyUnset',
      desc: '',
      args: [],
    );
  }

  /// `Function description:`
  String get aliyunAPIKeyManageFunctionDescription {
    return Intl.message(
      'Function description:',
      name: 'aliyunAPIKeyManageFunctionDescription',
      desc: '',
      args: [],
    );
  }

  /// `• Enable voice AI chat by configuring your DashScope API Key\n• Speak and hear responses in real time\n• Only available in dialog mode`
  String get aliyunAPIKeyManageFunctionDescriptionContent {
    return Intl.message(
      '• Enable voice AI chat by configuring your DashScope API Key\n• Speak and hear responses in real time\n• Only available in dialog mode',
      name: 'aliyunAPIKeyManageFunctionDescriptionContent',
      desc: '',
      args: [],
    );
  }

  /// `Configure Aliyun API Key`
  String get aliyunAPIKeyConfigureDialogTitle {
    return Intl.message(
      'Configure Aliyun API Key',
      name: 'aliyunAPIKeyConfigureDialogTitle',
      desc: '',
      args: [],
    );
  }

  /// `Used to enable voice AI conversation`
  String get aliyunAPIKeyConfigureDialogSubtitle {
    return Intl.message(
      'Used to enable voice AI conversation',
      name: 'aliyunAPIKeyConfigureDialogSubtitle',
      desc: '',
      args: [],
    );
  }

  /// `API Key`
  String get aliyunAPIKeyConfigureDialogTextFieldLabel {
    return Intl.message(
      'API Key',
      name: 'aliyunAPIKeyConfigureDialogTextFieldLabel',
      desc: '',
      args: [],
    );
  }

  /// `Please enter Aliyun DashScope API Key`
  String get aliyunAPIKeyConfigureDialogTextFieldHint {
    return Intl.message(
      'Please enter Aliyun DashScope API Key',
      name: 'aliyunAPIKeyConfigureDialogTextFieldHint',
      desc: '',
      args: [],
    );
  }

  /// `Get API Key:`
  String get aliyunAPIKeyConfigureDialogTip {
    return Intl.message(
      'Get API Key:',
      name: 'aliyunAPIKeyConfigureDialogTip',
      desc: '',
      args: [],
    );
  }

  /// `1. Visit the Alibaba Cloud DashScope Console\n2. Create an API Key\n3. Copy the API Key and paste it into the input field above`
  String get aliyunAPIKeyConfigureDialogTipContent {
    return Intl.message(
      '1. Visit the Alibaba Cloud DashScope Console\n2. Create an API Key\n3. Copy the API Key and paste it into the input field above',
      name: 'aliyunAPIKeyConfigureDialogTipContent',
      desc: '',
      args: [],
    );
  }

  /// `Confirm`
  String get aliyunAPIKeyDeleteDialogTitle {
    return Intl.message(
      'Confirm',
      name: 'aliyunAPIKeyDeleteDialogTitle',
      desc: '',
      args: [],
    );
  }

  /// `Deleting the API Key will turn off voice AI chat. Are you sure you want to continue?`
  String get aliyunAPIKeyDeleteDialogContent {
    return Intl.message(
      'Deleting the API Key will turn off voice AI chat. Are you sure you want to continue?',
      name: 'aliyunAPIKeyDeleteDialogContent',
      desc: '',
      args: [],
    );
  }

  /// `Export Data`
  String get exportDataTitle {
    return Intl.message(
      'Export Data',
      name: 'exportDataTitle',
      desc: '',
      args: [],
    );
  }

  /// `Export transcription results`
  String get exportDataSubtitle {
    return Intl.message(
      'Export transcription results',
      name: 'exportDataSubtitle',
      desc: '',
      args: [],
    );
  }

  /// `Select Date Range`
  String get exportDataDialogTip {
    return Intl.message(
      'Select Date Range',
      name: 'exportDataDialogTip',
      desc: '',
      args: [],
    );
  }

  /// `No date range selected`
  String get exportDataDialogTipNoDate {
    return Intl.message(
      'No date range selected',
      name: 'exportDataDialogTipNoDate',
      desc: '',
      args: [],
    );
  }

  /// `From {start} to {end}`
  String exportDataDialogDateFromTo(Object start, Object end) {
    return Intl.message(
      'From $start to $end',
      name: 'exportDataDialogDateFromTo',
      desc: '',
      args: [start, end],
    );
  }

  /// `File Name`
  String get exportDataFileName {
    return Intl.message(
      'File Name',
      name: 'exportDataFileName',
      desc: '',
      args: [],
    );
  }

  /// `Earbuds upgrade`
  String get pageSettingEarbudsUpgradeTitle {
    return Intl.message(
      'Earbuds upgrade',
      name: 'pageSettingEarbudsUpgradeTitle',
      desc: '',
      args: [],
    );
  }

  /// `Earbuds upgrade`
  String get pageSettingEarbudsUpgradeSubtitle {
    return Intl.message(
      'Earbuds upgrade',
      name: 'pageSettingEarbudsUpgradeSubtitle',
      desc: '',
      args: [],
    );
  }

  /// `No earbud firmware available for upgrade`
  String get pageSettingEarbudsUpgradeUnavailable {
    return Intl.message(
      'No earbud firmware available for upgrade',
      name: 'pageSettingEarbudsUpgradeUnavailable',
      desc: '',
      args: [],
    );
  }

  /// `About`
  String get pageSettingAboutTitle {
    return Intl.message(
      'About',
      name: 'pageSettingAboutTitle',
      desc: '',
      args: [],
    );
  }

  /// `Learn more about Buddie`
  String get pageSettingAboutSubtitle {
    return Intl.message(
      'Learn more about Buddie',
      name: 'pageSettingAboutSubtitle',
      desc: '',
      args: [],
    );
  }

  /// `Help document`
  String get pageSettingHelpTitle {
    return Intl.message(
      'Help document',
      name: 'pageSettingHelpTitle',
      desc: '',
      args: [],
    );
  }

  /// `Detailed introduction document`
  String get pageSettingHelpSubtitle {
    return Intl.message(
      'Detailed introduction document',
      name: 'pageSettingHelpSubtitle',
      desc: '',
      args: [],
    );
  }

  /// `About Buddie`
  String get pageAboutTitle {
    return Intl.message(
      'About Buddie',
      name: 'pageAboutTitle',
      desc: '',
      args: [],
    );
  }

  /// `Official Medias`
  String get pageAboutOfficialMedias {
    return Intl.message(
      'Official Medias',
      name: 'pageAboutOfficialMedias',
      desc: '',
      args: [],
    );
  }

  /// `Official website`
  String get pageAboutOfficialWebsite {
    return Intl.message(
      'Official website',
      name: 'pageAboutOfficialWebsite',
      desc: '',
      args: [],
    );
  }

  /// `Facebook`
  String get pageAboutFacebook {
    return Intl.message(
      'Facebook',
      name: 'pageAboutFacebook',
      desc: '',
      args: [],
    );
  }

  /// `X`
  String get pageAboutX {
    return Intl.message('X', name: 'pageAboutX', desc: '', args: []);
  }

  /// `Version {version}`
  String pageAboutVersion(Object version) {
    return Intl.message(
      'Version $version',
      name: 'pageAboutVersion',
      desc: '',
      args: [version],
    );
  }

  /// `User Agreement`
  String get pageAboutUserAgreement {
    return Intl.message(
      'User Agreement',
      name: 'pageAboutUserAgreement',
      desc: '',
      args: [],
    );
  }

  /// `Privacy Policy`
  String get pageAboutPrivacyPolicy {
    return Intl.message(
      'Privacy Policy',
      name: 'pageAboutPrivacyPolicy',
      desc: '',
      args: [],
    );
  }

  /// `Help instructions`
  String get pageHelpTitle {
    return Intl.message(
      'Help instructions',
      name: 'pageHelpTitle',
      desc: '',
      args: [],
    );
  }

  /// `Help content is not available at the moment`
  String get pageHelpUnavailable {
    return Intl.message(
      'Help content is not available at the moment',
      name: 'pageHelpUnavailable',
      desc: '',
      args: [],
    );
  }

  /// `Search meetings`
  String get pageMeetingListSearchHint {
    return Intl.message(
      'Search meetings',
      name: 'pageMeetingListSearchHint',
      desc: '',
      args: [],
    );
  }

  /// `{count} selected`
  String pageMeetingListSelected(Object count) {
    return Intl.message(
      '$count selected',
      name: 'pageMeetingListSelected',
      desc: '',
      args: [count],
    );
  }

  /// `Meeting`
  String get meetingListTile {
    return Intl.message('Meeting', name: 'meetingListTile', desc: '', args: []);
  }

  /// `Cannot find audio file.`
  String get pageMeetingDetailAudioNotFound {
    return Intl.message(
      'Cannot find audio file.',
      name: 'pageMeetingDetailAudioNotFound',
      desc: '',
      args: [],
    );
  }

  /// `Copy summary`
  String get copySummary {
    return Intl.message(
      'Copy summary',
      name: 'copySummary',
      desc: '',
      args: [],
    );
  }

  /// `Copy Transcription Text`
  String get copyTranscriptionText {
    return Intl.message(
      'Copy Transcription Text',
      name: 'copyTranscriptionText',
      desc: '',
      args: [],
    );
  }

  /// `Export Audio`
  String get exportAudio {
    return Intl.message(
      'Export Audio',
      name: 'exportAudio',
      desc: '',
      args: [],
    );
  }

  /// `Copied to clipboard`
  String get copiedToClipboard {
    return Intl.message(
      'Copied to clipboard',
      name: 'copiedToClipboard',
      desc: '',
      args: [],
    );
  }

  /// `Edit title`
  String get editTitle {
    return Intl.message('Edit title', name: 'editTitle', desc: '', args: []);
  }

  /// `Full text summary`
  String get fullTextSummary {
    return Intl.message(
      'Full text summary',
      name: 'fullTextSummary',
      desc: '',
      args: [],
    );
  }

  /// `Chapter Overview`
  String get chapterOverview {
    return Intl.message(
      'Chapter Overview',
      name: 'chapterOverview',
      desc: '',
      args: [],
    );
  }

  /// `Key points review`
  String get keyPointsReview {
    return Intl.message(
      'Key points review',
      name: 'keyPointsReview',
      desc: '',
      args: [],
    );
  }

  /// `Error: {error}`
  String pageItemError(Object error) {
    return Intl.message(
      'Error: $error',
      name: 'pageItemError',
      desc: '',
      args: [error],
    );
  }

  /// `Details`
  String get pageItemDetails {
    return Intl.message('Details', name: 'pageItemDetails', desc: '', args: []);
  }

  /// `No records found.`
  String get pageItemNoRecords {
    return Intl.message(
      'No records found.',
      name: 'pageItemNoRecords',
      desc: '',
      args: [],
    );
  }

  /// `Is this your Buddie?`
  String get pageBleIsYour {
    return Intl.message(
      'Is this your Buddie?',
      name: 'pageBleIsYour',
      desc: '',
      args: [],
    );
  }

  /// `Connected successfully!`
  String get pageBleToastConnectSuccess {
    return Intl.message(
      'Connected successfully!',
      name: 'pageBleToastConnectSuccess',
      desc: '',
      args: [],
    );
  }

  /// `Connection failed. Please try again.`
  String get pageBleToastConnectFailed {
    return Intl.message(
      'Connection failed. Please try again.',
      name: 'pageBleToastConnectFailed',
      desc: '',
      args: [],
    );
  }

  /// `Device forgotten successfully!`
  String get pageBleToastForgetSuccess {
    return Intl.message(
      'Device forgotten successfully!',
      name: 'pageBleToastForgetSuccess',
      desc: '',
      args: [],
    );
  }

  /// `Unknown device`
  String get pageBleUnknownDevice {
    return Intl.message(
      'Unknown device',
      name: 'pageBleUnknownDevice',
      desc: '',
      args: [],
    );
  }

  /// `Your earbuds have already been saved!`
  String get pageBleSaved {
    return Intl.message(
      'Your earbuds have already been saved!',
      name: 'pageBleSaved',
      desc: '',
      args: [],
    );
  }

  /// `Forget Device`
  String get pageBleConfirmText {
    return Intl.message(
      'Forget Device',
      name: 'pageBleConfirmText',
      desc: '',
      args: [],
    );
  }
}

class AppLocalizationDelegate extends LocalizationsDelegate<S> {
  const AppLocalizationDelegate();

  List<Locale> get supportedLocales {
    return const <Locale>[
      Locale.fromSubtags(languageCode: 'en'),
      Locale.fromSubtags(languageCode: 'zh'),
    ];
  }

  @override
  bool isSupported(Locale locale) => _isSupported(locale);
  @override
  Future<S> load(Locale locale) => S.load(locale);
  @override
  bool shouldReload(AppLocalizationDelegate old) => false;

  bool _isSupported(Locale locale) {
    for (var supportedLocale in supportedLocales) {
      if (supportedLocale.languageCode == locale.languageCode) {
        return true;
      }
    }
    return false;
  }
}
