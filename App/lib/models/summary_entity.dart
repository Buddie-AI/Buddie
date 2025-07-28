import 'package:app/extension/datetime_extension.dart';
import 'package:objectbox/objectbox.dart';

@Entity()
class SummaryEntity {
  int id;
  bool isMeeting;
  String? subject;
  String? content;

  @HnswIndex(dimensions: 1536, distanceType: VectorDistanceType.cosine)
  @Property(type: PropertyType.floatVector)
  List<double>? vector;

  @Index()
  int startTime;

  @Index()
  int endTime;

  @Index()
  int? createdAt;
  String? audioPath;

  String? title;

  SummaryEntity({
    this.id = 0,
    this.isMeeting = false,
    this.subject,
    this.content,
    this.vector,
    this.startTime = 0,
    this.endTime = 0,
    int? createdAt,
    this.audioPath,
    String? title,
  }) : createdAt = createdAt ?? DateTime.now().millisecondsSinceEpoch,
        title = title ?? (startTime > 0
          ? '${DateTime.fromMillisecondsSinceEpoch(startTime).toDateFormatString()} record'
          : 'Untitled Record');
}
