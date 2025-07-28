import 'package:app/controllers/record_controller.dart';
import 'package:app/views/about/about_screen.dart';
import 'package:app/views/home/home_chat_screen.dart';
import 'package:app/views/meeting/meeting_detail_screen.dart';
import 'package:app/views/meeting/meeting_list_screen.dart';

import 'package:app/views/setting/setting_screen.dart';
import 'package:app/views/entry/welcome_screen.dart';
import 'package:flutter_smart_dialog/flutter_smart_dialog.dart';
import 'package:go_router/go_router.dart';
import '../views/entry/loading_screen.dart';
import 'package:app/views/help_feedback/help_feedback_screen.dart';
import 'package:app/views/meeting/model/meeting_model.dart';
import 'package:flutter/material.dart';

class BudNavigatorObserver extends NavigatorObserver {
  @override
  void didPush(Route route, Route? previousRoute) {
    if (previousRoute != null) {
      FocusManager.instance.primaryFocus?.unfocus();
    }
    super.didPush(route, previousRoute);
  }
}

class RouteUtils {
  static late GoRouter goRoute;

  static Future<void> initializeRouter() async {
    goRoute = GoRouter(
      routes: routes,
      initialLocation: RouteName.loading,
      observers: [BudNavigatorObserver(), FlutterSmartDialog.observer],
    );
  }

  static List<GoRoute> routes = [
    GoRoute(
      path: RouteName.welcome,
      name: RouteName.welcome,
      builder: (_, state) =>
          WelcomeScreen(controller: state.extra as RecordScreenController?),
    ),
    GoRoute(
      path: RouteName.home_chat,
      name: RouteName.home_chat,
      builder: (_, state) =>
          HomeChatScreen(controller: state.extra as RecordScreenController?),
    ),
    GoRoute(
      path: RouteName.setting,
      name: RouteName.setting,
      builder: (_, state) => const SettingScreen(),
    ),
    GoRoute(
      path: RouteName.about,
      name: RouteName.about,
      builder: (_, state) => const AboutScreen(),
    ),
    GoRoute(
      path: RouteName.meeting_list,
      name: RouteName.meeting_list,
      builder: (_, state) => const MeetingListScreen(),
    ),
    GoRoute(
      path: RouteName.meeting_detail,
      name: RouteName.meeting_detail,
      builder: (_, state) =>
          MeetingDetailScreen(model: state.extra as MeetingModel),
    ),

    GoRoute(
      path: RouteName.loading,
      name: RouteName.loading,
      builder: (_, state) => const LoadingScreen(),
    ),
    GoRoute(
      path: RouteName.help_feedback,
      name: RouteName.help_feedback,
      builder: (_, state) =>  HelpFeedbackScreen(locale: state.extra as String),
    ),
  ];
}

class RouteName {
  static const String welcome = '/welcome';
  static const String setup = '/setup';

  static const String home_chat = '/home_chat';

  /// home
  static const String setting = '/setting';
  static const String voice_print = '/voice_print';
  static const String help_feedback = '/help_feedback';
  static const String about = '/about';
  static const String loading = '/loading';

  /// journal
  static const String journal = '/journal';
  static const String meeting_list = '/meeting_list';
  static const String daily_list = '/daily_list';
  static const String meeting_detail = '/meeting_detail';
  static const String todo_list = '/todo_list';
}
