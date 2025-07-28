class MeetingModel {
  final int id;

  final String content;
  final String fullContent;

  /// milliseconds
  final int startTime;
  final int endTime;

  Duration get duration {
    int s = startTime;
    Duration duration = Duration(milliseconds: (endTime - s));
    return duration;
  }

  final int? createdAt;

  DateTime get datetime {
    int millisecondsSinceEpoch = startTime;
    DateTime dateTime = DateTime.fromMillisecondsSinceEpoch(millisecondsSinceEpoch);
    return dateTime;
  }

  String? get formatRecordString {
    return '${datetime.year}-${datetime.month}-${datetime.day} ${datetime.hour}:${datetime.minute}';
  }

  final String? audioPath;
  late final String? title;

  MeetingModel({
    required this.id,
    required this.content,
    required this.startTime,
    required this.endTime,
    required this.createdAt,
    required this.fullContent,
    required this.title,
    this.audioPath,
  });
}