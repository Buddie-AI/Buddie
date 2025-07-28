import 'package:flutter/cupertino.dart';
import 'package:shared_preferences/shared_preferences.dart';

enum Mode { light, dark }

class ThemeNotifier extends ChangeNotifier {
  Mode _mode = Mode.light;

  Mode get mode => _mode;

  ThemeNotifier() {
    loadThemePreference();
  }

  void setMode(String modeValue) {
    if (modeValue == "light") {
      _mode = Mode.light;
    } else if (modeValue == "dark") {
      _mode = Mode.dark;
    } else {
      print("Invalid mode");
    }
    notifyListeners();
  }

  Future<void> loadThemePreference() async {
    final prefs = await SharedPreferences.getInstance();
    final isDarkMode = prefs.getBool('isDarkMode') ?? false;
    setMode(isDarkMode ? "dark" : "light");
  }

  Future<void> toggleTheme() async {
    if (_mode == Mode.light) {
      setMode("dark");
      await _saveThemePreference(true);
    } else {
      setMode("light");
      await _saveThemePreference(false);
    }
  }

  Future<void> _saveThemePreference(bool isDarkMode) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setBool('isDarkMode', isDarkMode);
  }
}