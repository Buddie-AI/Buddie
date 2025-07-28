class TextProcessUtils {
  static String removeBracketsContent(String text) {
    return text.replaceAll(RegExp(r'\[.*?\]|\{.*?\}'), '');
  }

  static String clearIfRepeatedMoreThanFiveTimes(String text) {
    return text.replaceAllMapped(
      RegExp(r'(.+?)(?:\s*\1){4,}', caseSensitive: false),
          (match) => match.group(1)!,
    );
  }
}
