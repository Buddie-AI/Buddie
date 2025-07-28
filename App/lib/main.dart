import 'package:app/controllers/locale_controller.dart';
import 'package:app/controllers/style_controller.dart';
import 'package:app/services/objectbox_service.dart';
import 'package:app/utils/route_utils.dart';
import 'package:app/config/default_config.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart' as flutter_blue;
import 'package:flutter_foreground_task/flutter_foreground_task.dart';
import 'package:flutter_local_notifications/flutter_local_notifications.dart';
import 'package:flutter_localizations/flutter_localizations.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:flutter_smart_dialog/flutter_smart_dialog.dart';
import 'package:provider/provider.dart';
import 'package:sentry_flutter/sentry_flutter.dart';

import 'generated/l10n.dart';

final FlutterLocalNotificationsPlugin flutterLocalNotificationsPlugin = FlutterLocalNotificationsPlugin();

Future<void> main() async {
  // Initialize port for communication between TaskHandler and UI.
  WidgetsFlutterBinding.ensureInitialized();

  await SystemChrome.setPreferredOrientations([DeviceOrientation.portraitUp]);

  const AndroidInitializationSettings initializationSettingsAndroid = AndroidInitializationSettings(
    '@mipmap/ic_launcher',
  );

  final DarwinInitializationSettings initializationSettingsIOS = DarwinInitializationSettings(
    requestAlertPermission: true,
    requestBadgePermission: true,
    requestSoundPermission: true,
  );

  final InitializationSettings initializationSettings = InitializationSettings(
    android: initializationSettingsAndroid,
    iOS: initializationSettingsIOS,
  );

  await flutterLocalNotificationsPlugin.initialize(initializationSettings);

  FlutterForegroundTask.initCommunicationPort();

  await ObjectBoxService.initialize();

  flutter_blue.FlutterBluePlus.setLogLevel(flutter_blue.LogLevel.error);
  flutter_blue.FlutterBluePlus.setOptions(restoreState: true);

  // 初始化默认配置
  await DefaultConfig.initialize();
  DefaultConfig.printConfigStatus();

  await RouteUtils.initializeRouter();
  await SentryFlutter.init(
    (options) {
      options.dsn = 'https://476fe26ce43858184b0f5309106671d6@o4507015727874048.ingest.us.sentry.io/4508811095375872';
      options.tracesSampleRate = 1.0;
      options.profilesSampleRate = 1.0;
      options.diagnosticLevel = SentryLevel.warning;
    },
    appRunner: () => runApp(
      SentryWidget(
        child: MultiProvider(
          providers: [
            ChangeNotifierProvider(create: (_) => ThemeNotifier()),
            ChangeNotifierProvider(create: (_) => LocaleNotifier()),
          ],
          child: MyApp(),
        ),
      ),
    ),
  );
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return ScreenUtilInit(
      designSize: const Size(375, 812),
      minTextAdapt: true,
      splitScreenMode: true,
      child: MaterialApp.router(
        title: 'Buddie',
        localizationsDelegates: const [
          S.delegate,
          GlobalMaterialLocalizations.delegate,
          GlobalWidgetsLocalizations.delegate,
          GlobalCupertinoLocalizations.delegate,
        ],
        supportedLocales: S.delegate.supportedLocales,
        theme: ThemeData(
          useMaterial3: true,
          colorScheme: ColorScheme.fromSeed(seedColor: const Color(0xFF00B2CA)),
          fontFamily: 'SourceHanSansCN',
          textTheme: const TextTheme(
            displayLarge: TextStyle(fontSize: 34, fontWeight: FontWeight.bold),
            displayMedium: TextStyle(fontSize: 24, fontWeight: FontWeight.w300),
            displaySmall: TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
          ),
        ),
        debugShowCheckedModeBanner: false,
        routerConfig: RouteUtils.goRoute,
        builder: FlutterSmartDialog.init(
          builder: (context, child) {
            return MediaQuery(
              data: MediaQuery.of(context).copyWith(textScaler: const TextScaler.linear(1.0)),
              child: child!,
            );
          },
        ),
      ),
    );
  }
}
