extension DurationExtension on Duration {
  /// milliseconds
  String toTimeFormatString() {
    int hours = inHours;
    int minutes = inMinutes % 60;
    int seconds = inSeconds % 60;
    String result = '$hours:${minutes.toString().padLeft(2, '0')}:${seconds.toString().padLeft(2, '0')}';
    return result;
  }
}
