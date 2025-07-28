import 'package:uuid/uuid.dart';

class welcome_constants{
  static List<Map<String, dynamic>> welcomeSentences = [
    {
      'id': const Uuid().v4(),
      'text': "Hello, I am Buddie.",
      'isUser': 'assistant'
    },
    {
      'id': const Uuid().v4(),
      'text': "Tap the Bluetooth icon to the top-right to pair with Buddie earbuds. Buddie needs two connections to work: one via your phone's Bluetooth settings and one via the App.",
      'isUser': 'assistant'
    },
    {
      'id': const Uuid().v4(),
      'text': "Say 'Hello Buddie' to have a conversation with me. Say 'Just Listen' if you want me to be quiet but transcribe your comments and conversations to refer to later.",
      'isUser': 'assistant'
    },
    {
      'id': const Uuid().v4(),
      'text': "During online meetings, Buddie will automatically transcribe everything for you. You can click the 'Help me Buddie' button for assistance during the meetings!",
      'isUser': 'assistant'
    },
    {
      'id': const Uuid().v4(),
      'text': "Tap the 'journal' icon to the lower-right to review on-line meeting summaries and transcripts. Summaries appear there a minute after the meetings end.",
      'isUser': 'assistant'
    },
    {
      'id': const Uuid().v4(),
      'text': "More help can be found in 'Settings' in the top-right corner of the App --> 'Help document'.",
      'isUser': 'assistant'
    }
  ];
}