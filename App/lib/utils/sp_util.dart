import 'package:shared_preferences/shared_preferences.dart';

class SPUtil {
  static Future<int?> getInt(String key) async {
    SharedPreferences instance = await SharedPreferences.getInstance();
    int? result = instance.getInt(key);
    return result;
  }

  static Future<bool> setInt(String key, int value) async {
    SharedPreferences instance = await SharedPreferences.getInstance();
    bool result = await instance.setInt(key, value);
    return result;
  }

  static Future<String?> getString(String key) async {
    SharedPreferences instance = await SharedPreferences.getInstance();
    String? result = instance.getString(key);
    return result;
  }

  static Future<bool> setString(String key, String value) async {
    SharedPreferences instance = await SharedPreferences.getInstance();
    bool result = await instance.setString(key, value);
    return result;
  }

  static Future<bool> remove(String key) async {
    SharedPreferences instance = await SharedPreferences.getInstance();
    bool result = await instance.remove(key);
    return result;
  }

  static Future<bool> setBool(String key, bool value) async {
    SharedPreferences instance = await SharedPreferences.getInstance();
    bool result = await instance.setBool(key, value);
    return result;
  }

  static Future<bool?> getBool(String key) async {
    SharedPreferences instance = await SharedPreferences.getInstance();
    bool? result = instance.getBool(key);
    return result;
  }
}
