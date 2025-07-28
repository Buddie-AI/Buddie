extension MapExtension on Map {
  void moveToNextDot() {
    try {
      final nextString = "${this['text']}.";
      this['text'] = nextString.substring(0, nextString.length % 4);
    } catch (e) {
      print('Error: $e');
    }
  }
}