extension StringExtension on String {
  static final RegExp _customFormat = RegExp(r'^(?<year>\d{4})[/-]?(?<month>\d{1,2})[-/]?(?<day>\d{1,2})' // Day part.
  r'( (?<hour>\d{1,2}):(?<minute>\d{1,2}):(?<second>\d{1,2})$)?'); // Time part.

  DateTime? toDateTime() {
    RegExp re = _customFormat;
    RegExpMatch? match = re.firstMatch(this);
    if (match != null) {
      String? yearString = match.namedGroup('year');
      if (yearString != null) {
        int year = int.parse(yearString);
        String? monthString = match.namedGroup('month');
        String? dayString = match.namedGroup('day');
        String? hourString = match.namedGroup('hour');
        String? minuteString = match.namedGroup('minute');
        String? secondString = match.namedGroup('second');
        int month = monthString != null ? int.parse(monthString) : 1;
        int day = dayString != null ? int.parse(dayString) : 1;
        int hour = hourString != null ? int.parse(hourString) : 0;
        int minute = minuteString != null ? int.parse(minuteString) : 0;
        int second = secondString != null ? int.parse(secondString) : 0;
        DateTime dateTime = DateTime(year, month, day, hour, minute, second);
        return dateTime;
      }
    }
    return null;
  }

  String toCapitalize() => '${this[0].toUpperCase()}${substring(1)}';
}
