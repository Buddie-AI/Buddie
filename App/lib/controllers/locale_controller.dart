import 'package:flutter/cupertino.dart';
import 'package:intl/intl.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'dart:developer' as dev;

import '../generated/l10n.dart';

class LocaleNotifier extends ChangeNotifier {
  static const String sp_key = 'locale';

  static const String locale_en = 'en';
  static const String locale_zh = 'zh';

  String _locale = locale_en;

  String get locale => _locale;

  LocaleNotifier() {
    loadLocalePreference();
  }

  Future<void> toggleLocale() async {
    _locale == locale_en ? _setMode(locale_zh) : _setMode(locale_en);
  }

  /// en/zh
  String getCurrentLocale() {
    String locale = Intl.getCurrentLocale();
    dev.log('getCurrentLocale:$locale');
    return locale;
  }

  void _setMode(String locale) async {
    _locale = locale;
    S.load(Locale(_locale));
    notifyListeners();
    await _saveThemePreference(_locale);
  }

  Future<void> loadLocalePreference() async {
    final prefs = await SharedPreferences.getInstance();
    final locale = prefs.getString(sp_key);
    if (locale != null) {
      _setMode(locale);
    }
  }

  Future<void> _saveThemePreference(String locale) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString(sp_key, locale);
  }
}
