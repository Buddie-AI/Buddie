import 'package:intl/intl.dart';

extension DatetimeExtension on DateTime {
  /// M - month
  /// m - minute
  /// H - hour(24)
  /// Default is 'yyyy-MM-dd HH:mm:ss'
  /// when onlyDate is true,the format is 'yyyy-MM-dd'
  String toDateFormatString({
    String? formats,
    String dateSplit = '-',
    String datetimeSplit = ' ',
    String timeSplit = ':',
    bool showTime = true,
  }) {
    if (formats != null) {
      return DateFormat(formats).format(this);
    }
    String datePattern = 'yyyy${dateSplit}MM${dateSplit}dd';
    String dateFormat = DateFormat(datePattern).format(this);
    if (showTime) {
      String timeFormat = toTimeFormatString(timeSplit: timeSplit);
      dateFormat = '$dateFormat$datetimeSplit$timeFormat';
    }
    return dateFormat;
  }

  String toTimeFormatString({
    String timeSplit = ':',
  }) {
    String timePattern = 'HH${timeSplit}mm${timeSplit}ss';
    String format = DateFormat(timePattern).format(this);
    return format;
  }

  DateTime get lastDayThisMonth {
    return DateTime(year, month + 1, 0, 23, 59, 59, 999);
  }

  String toDurationFormatString() {
    String result = '$hour:${minute.toString().padLeft(2, '0')}:${second.toString().padLeft(2, '0')}';
    return result;
  }
}
