// DO NOT EDIT. This is code generated via package:intl/generate_localized.dart
// This is a library that provides messages for a en locale. All the
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
  String get localeName => 'en';

  static String m0(e) => "Refresh failed: ${e}";

  static String m1(length) =>
      "AI model status has been refreshed, Discover ${length} available models";

  static String m2(start, end) => "From ${start} to ${end}";

  static String m3(version) => "Version ${version}";

  static String m4(error) => "Error: ${error}";

  static String m5(count) => "${count} selected";

  final messages = _notInlinedMessages(_notInlinedMessages);
  static Map<String, Function> _notInlinedMessages(_) => <String, Function>{
    "ScanningBuddie": MessageLookupByLibrary.simpleMessage(
      "Scanning Buddie...",
    ),
    "aiDialogModel": MessageLookupByLibrary.simpleMessage("AI dialogue model"),
    "aiDialogModelCustomLLMDescription": MessageLookupByLibrary.simpleMessage(
      "Use the API Key you configured",
    ),
    "aiDialogModelCustomLLMDisplayName": MessageLookupByLibrary.simpleMessage(
      "Custom model (user configuration)",
    ),
    "aiDialogModelQwenOmniDescription": MessageLookupByLibrary.simpleMessage(
      "Multimodal AI that supports voice input and output",
    ),
    "aiDialogModelQwenOmniDisplayName": MessageLookupByLibrary.simpleMessage(
      "Qwen Omni Multimodal",
    ),
    "aiModelSetTitle": MessageLookupByLibrary.simpleMessage(
      "AI Model Settings",
    ),
    "aiModelToastRefreshFailed": m0,
    "aiModelToastRefreshed": MessageLookupByLibrary.simpleMessage(
      "The status of the AI model has been refreshed",
    ),
    "aiModelToastRefreshedAndFound": m1,
    "aiModelToastUnavailable": MessageLookupByLibrary.simpleMessage(
      "No AI model is available. Please check your API Key or network settings",
    ),
    "aliyunAPIKeyConfigureDialogSubtitle": MessageLookupByLibrary.simpleMessage(
      "Used to enable voice AI conversation",
    ),
    "aliyunAPIKeyConfigureDialogTextFieldHint":
        MessageLookupByLibrary.simpleMessage(
          "Please enter Aliyun DashScope API Key",
        ),
    "aliyunAPIKeyConfigureDialogTextFieldLabel":
        MessageLookupByLibrary.simpleMessage("API Key"),
    "aliyunAPIKeyConfigureDialogTip": MessageLookupByLibrary.simpleMessage(
      "Get API Key:",
    ),
    "aliyunAPIKeyConfigureDialogTipContent": MessageLookupByLibrary.simpleMessage(
      "1. Visit the Alibaba Cloud DashScope Console\n2. Create an API Key\n3. Copy the API Key and paste it into the input field above",
    ),
    "aliyunAPIKeyConfigureDialogTitle": MessageLookupByLibrary.simpleMessage(
      "Configure Aliyun API Key",
    ),
    "aliyunAPIKeyDeleteDialogContent": MessageLookupByLibrary.simpleMessage(
      "Deleting the API Key will turn off voice AI chat. Are you sure you want to continue?",
    ),
    "aliyunAPIKeyDeleteDialogTitle": MessageLookupByLibrary.simpleMessage(
      "Confirm",
    ),
    "aliyunAPIKeyManageCurrent": MessageLookupByLibrary.simpleMessage(
      "Current API Key:",
    ),
    "aliyunAPIKeyManageFunctionDescription":
        MessageLookupByLibrary.simpleMessage("Function description:"),
    "aliyunAPIKeyManageFunctionDescriptionContent":
        MessageLookupByLibrary.simpleMessage(
          "• Enable voice AI chat by configuring your DashScope API Key\n• Speak and hear responses in real time\n• Only available in dialog mode",
        ),
    "aliyunAPIKeyManageKeyConfigured": MessageLookupByLibrary.simpleMessage(
      "API Key Configured",
    ),
    "aliyunAPIKeyManageKeyUnset": MessageLookupByLibrary.simpleMessage(
      "API Key is not configured, voice AI dialogue function cannot be used",
    ),
    "aliyunAPIKeyManageSubtitle2": MessageLookupByLibrary.simpleMessage(
      "Configure voice AI dialogue function",
    ),
    "aliyunAPIKeyManageSubtitleConfigured":
        MessageLookupByLibrary.simpleMessage(
          "Configured voice AI dialogue function",
        ),
    "aliyunAPIKeyManageTitle": MessageLookupByLibrary.simpleMessage(
      "Aliyun API Key Management",
    ),
    "appName": MessageLookupByLibrary.simpleMessage("Buddie"),
    "asrModeCloudOnlineDescription": MessageLookupByLibrary.simpleMessage(
      "Short audio recognition",
    ),
    "asrModeCloudOnlineTitle": MessageLookupByLibrary.simpleMessage(
      "Sentence recognition",
    ),
    "asrModeCloudStreamingDescription": MessageLookupByLibrary.simpleMessage(
      "Real-time streaming processing",
    ),
    "asrModeCloudStreamingTitle": MessageLookupByLibrary.simpleMessage(
      "Real-time speech recognition",
    ),
    "asrModeLocalOfflineDescription": MessageLookupByLibrary.simpleMessage(
      "Local offline processing",
    ),
    "asrModeLocalOfflineTitle": MessageLookupByLibrary.simpleMessage(
      "Local model recognition",
    ),
    "asrModeSetSubtitle": MessageLookupByLibrary.simpleMessage(
      "Set different speech recognition methods for each chat mode",
    ),
    "asrModeSetTitle": MessageLookupByLibrary.simpleMessage("ASR Mode Setting"),
    "audioPlayerTip": MessageLookupByLibrary.simpleMessage(
      "File does not exist",
    ),
    "buttonBack": MessageLookupByLibrary.simpleMessage("Back"),
    "buttonCancel": MessageLookupByLibrary.simpleMessage("Cancel"),
    "buttonClose": MessageLookupByLibrary.simpleMessage("Close"),
    "buttonConfigure": MessageLookupByLibrary.simpleMessage("Configure"),
    "buttonConfirm": MessageLookupByLibrary.simpleMessage("Confirm"),
    "buttonDelete": MessageLookupByLibrary.simpleMessage("Delete"),
    "buttonHelpMe": MessageLookupByLibrary.simpleMessage("Help me Buddie"),
    "buttonModify": MessageLookupByLibrary.simpleMessage("Modify"),
    "buttonNextStep": MessageLookupByLibrary.simpleMessage("Next step"),
    "buttonSave": MessageLookupByLibrary.simpleMessage("Save"),
    "chapterOverview": MessageLookupByLibrary.simpleMessage("Chapter Overview"),
    "chatModeDefault": MessageLookupByLibrary.simpleMessage(
      "Transcription Mode",
    ),
    "chatModeDialog": MessageLookupByLibrary.simpleMessage("Dialog Mode"),
    "chatModeMeeting": MessageLookupByLibrary.simpleMessage("Meeting Mode"),
    "copiedToClipboard": MessageLookupByLibrary.simpleMessage(
      "Copied to clipboard",
    ),
    "copySummary": MessageLookupByLibrary.simpleMessage("Copy summary"),
    "copyTranscriptionText": MessageLookupByLibrary.simpleMessage(
      "Copy Transcription Text",
    ),
    "editTitle": MessageLookupByLibrary.simpleMessage("Edit title"),
    "exportAudio": MessageLookupByLibrary.simpleMessage("Export Audio"),
    "exportDataDialogDateFromTo": m2,
    "exportDataDialogTip": MessageLookupByLibrary.simpleMessage(
      "Select Date Range",
    ),
    "exportDataDialogTipNoDate": MessageLookupByLibrary.simpleMessage(
      "No date range selected",
    ),
    "exportDataFileName": MessageLookupByLibrary.simpleMessage("File Name"),
    "exportDataSubtitle": MessageLookupByLibrary.simpleMessage(
      "Export transcription results",
    ),
    "exportDataTitle": MessageLookupByLibrary.simpleMessage("Export Data"),
    "fullTextSummary": MessageLookupByLibrary.simpleMessage(
      "Full text summary",
    ),
    "importAPIKeyDialogApiKeyTextFieldHint":
        MessageLookupByLibrary.simpleMessage("Please input your API Key"),
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
      "After saving, you will use your own API Key and the specified model to talk.",
    ),
    "importAPIKeyDialogTitle": MessageLookupByLibrary.simpleMessage(
      "Setting up custom API Key",
    ),
    "importAPIKeySubtitle": MessageLookupByLibrary.simpleMessage(
      "Use your own API key",
    ),
    "importAPIKeyTitle": MessageLookupByLibrary.simpleMessage("Import API Key"),
    "keyPointsReview": MessageLookupByLibrary.simpleMessage(
      "Key points review",
    ),
    "languageChinese": MessageLookupByLibrary.simpleMessage("Chinese"),
    "languageEnglish": MessageLookupByLibrary.simpleMessage("English"),
    "meetingListTile": MessageLookupByLibrary.simpleMessage("Meeting"),
    "notPairedNoticeText1": MessageLookupByLibrary.simpleMessage(
      "Buddie Earbuds Not Connected",
    ),
    "notPairedNoticeText2": MessageLookupByLibrary.simpleMessage(
      "Please go to your phone\'s Bluetooth settings\n\nConnect them before continuing",
    ),
    "pageAboutFacebook": MessageLookupByLibrary.simpleMessage("Facebook"),
    "pageAboutOfficialMedias": MessageLookupByLibrary.simpleMessage(
      "Official Medias",
    ),
    "pageAboutOfficialWebsite": MessageLookupByLibrary.simpleMessage(
      "Official website",
    ),
    "pageAboutPrivacyPolicy": MessageLookupByLibrary.simpleMessage(
      "Privacy Policy",
    ),
    "pageAboutTitle": MessageLookupByLibrary.simpleMessage("About Buddie"),
    "pageAboutUserAgreement": MessageLookupByLibrary.simpleMessage(
      "User Agreement",
    ),
    "pageAboutVersion": m3,
    "pageAboutX": MessageLookupByLibrary.simpleMessage("X"),
    "pageBleConfirmText": MessageLookupByLibrary.simpleMessage("Forget Device"),
    "pageBleIsYour": MessageLookupByLibrary.simpleMessage(
      "Is this your Buddie?",
    ),
    "pageBleSaved": MessageLookupByLibrary.simpleMessage(
      "Your earbuds have already been saved!",
    ),
    "pageBleToastConnectFailed": MessageLookupByLibrary.simpleMessage(
      "Connection failed. Please try again.",
    ),
    "pageBleToastConnectSuccess": MessageLookupByLibrary.simpleMessage(
      "Connected successfully!",
    ),
    "pageBleToastForgetSuccess": MessageLookupByLibrary.simpleMessage(
      "Device forgotten successfully!",
    ),
    "pageBleUnknownDevice": MessageLookupByLibrary.simpleMessage(
      "Unknown device",
    ),
    "pageHelpTitle": MessageLookupByLibrary.simpleMessage("Help instructions"),
    "pageHelpUnavailable": MessageLookupByLibrary.simpleMessage(
      "Help content is not available at the moment",
    ),
    "pageHomeTextFieldHint": MessageLookupByLibrary.simpleMessage(
      "Enter your message...",
    ),
    "pageItemDetails": MessageLookupByLibrary.simpleMessage("Details"),
    "pageItemError": m4,
    "pageItemNoRecords": MessageLookupByLibrary.simpleMessage(
      "No records found.",
    ),
    "pageMeetingDetailAudioNotFound": MessageLookupByLibrary.simpleMessage(
      "Cannot find audio file.",
    ),
    "pageMeetingListSearchHint": MessageLookupByLibrary.simpleMessage(
      "Search meetings",
    ),
    "pageMeetingListSelected": m5,
    "pageSettingAIModeSetSubtitle3": MessageLookupByLibrary.simpleMessage(
      "Unknown model",
    ),
    "pageSettingAboutSubtitle": MessageLookupByLibrary.simpleMessage(
      "Learn more about Buddie",
    ),
    "pageSettingAboutTitle": MessageLookupByLibrary.simpleMessage("About"),
    "pageSettingDarkMode": MessageLookupByLibrary.simpleMessage("Dark Mode"),
    "pageSettingEarbudsUpgradeSubtitle": MessageLookupByLibrary.simpleMessage(
      "Earbuds upgrade",
    ),
    "pageSettingEarbudsUpgradeTitle": MessageLookupByLibrary.simpleMessage(
      "Earbuds upgrade",
    ),
    "pageSettingEarbudsUpgradeUnavailable":
        MessageLookupByLibrary.simpleMessage(
          "No earbud firmware available for upgrade",
        ),
    "pageSettingHelpSubtitle": MessageLookupByLibrary.simpleMessage(
      "Detailed introduction document",
    ),
    "pageSettingHelpTitle": MessageLookupByLibrary.simpleMessage(
      "Help document",
    ),
    "pageSettingLanguage": MessageLookupByLibrary.simpleMessage("Language"),
    "pageSettingTitle": MessageLookupByLibrary.simpleMessage("Settings"),
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
