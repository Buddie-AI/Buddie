import 'dart:async';
import 'dart:io';

import 'package:app/controllers/style_controller.dart';
import 'package:app/extension/duration_extension.dart';
import 'package:app/utils/assets_util.dart';
import 'package:app/utils/path_provider_utils.dart';
import 'package:app/views/ui/bud_icon.dart';
import 'package:audio_waveforms/audio_waveforms.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:provider/provider.dart';

import '../../../generated/l10n.dart';

class AudioPlayer extends StatefulWidget {
  final String? filePath;
  final String? assetsPath;

  const AudioPlayer({super.key, this.filePath, this.assetsPath})
    : assert(filePath != null || assetsPath != null);

  @override
  State<AudioPlayer> createState() => _AudioPlayerState();
}

class _AudioPlayerState extends State<AudioPlayer> {
  final PlayerController _playerController = PlayerController();
  StreamSubscription? _playerStateSubscription;
  StreamSubscription? _currentDurationSubscription;

  /// milliseconds
  int _maxDuration = 0;
  int _currentDuration = 0;
  PlayerState? _playerState;
  bool _exists = false;

  @override
  void dispose() {
    _playerStateSubscription?.cancel();
    _currentDurationSubscription?.cancel();
    _playerController.dispose();
    super.dispose();
  }

  @override
  void initState() {
    super.initState();
    _init();
  }

  void _init() async {
    await _prepare();
    _addListen();
  }

  Future<void> _prepare() async {
    late File file;
    if (widget.filePath != null) {
      file = File(widget.filePath!);
    } else {
      String fileName = widget.assetsPath!.split('/').last;
      final audioFile = await rootBundle.load(widget.assetsPath!);
      String dir = await PathProviderUtil.getTemporaryPath();
      file = File('$dir/$fileName');
      await file.writeAsBytes(audioFile.buffer.asUint8List());
    }

    bool exists = await file.exists();
    setState(() {
      _exists = exists;
    });
    if (exists) {
      await _playerController.preparePlayer(path: widget.filePath!);
      _playerController.setFinishMode(finishMode: FinishMode.pause);

      await _getMaxDuration();
    } else {
      debugPrint('${file.path} does not exist');
    }
  }

  Future<void> _getMaxDuration() async {
    final fileLengthInDuration = await _playerController.getDuration(
      DurationType.max,
    );
    setState(() {
      _maxDuration = fileLengthInDuration;
    });
  }

  void _addListen() {
    _playerStateSubscription = _playerController.onPlayerStateChanged.listen((
      state,
    ) {
      setState(() {
        _playerState = state;
      });
    });
    _currentDurationSubscription = _playerController.onCurrentDurationChanged
        .listen((duration) {
          setState(() {
            _currentDuration = duration;
          });
        });
  }

  void _onClickPlay() async {
    if (_playerState == PlayerState.playing) {
      await _playerController.pausePlayer();
    } else {
      await _playerController.startPlayer();
    }
  }

  void _onClickReplay() {
    _seekTo(_currentDuration - 15 * 1000);
  }

  void _onClickForward() {
    _seekTo(_currentDuration + 15 * 1000);
  }

  void _seekTo(int progress) {
    int calProgress = 0;
    if (progress < 0) {
      calProgress = 0;
    } else if (progress > _maxDuration) {
      calProgress = _maxDuration;
    } else {
      calProgress = progress;
    }
    _playerController.seekTo(calProgress);
  }

  @override
  Widget build(BuildContext context) {
    return _exists
        ? LayoutBuilder(
            builder: (context, constraints) {
              final themeNotifier = Provider.of<ThemeNotifier>(context);
              bool isLightMode = themeNotifier.mode == Mode.light;
              Duration currentDuration = Duration(
                milliseconds: _currentDuration,
              );
              Duration maxDuration = Duration(milliseconds: _maxDuration);
              return Column(
                mainAxisAlignment: MainAxisAlignment.center,
                mainAxisSize: MainAxisSize.min,
                children: [
                  AudioFileWaveforms(
                    size: Size(double.infinity, 72.sp),
                    playerController: _playerController,
                    margin: EdgeInsets.symmetric(vertical: 2.sp),
                    decoration: BoxDecoration(
                      color: isLightMode
                          ? const Color(0xFFF2F4F7)
                          : const Color(0xFF232323),
                      borderRadius: BorderRadius.circular(4),
                    ),
                    continuousWaveform: true,
                    playerWaveStyle: const PlayerWaveStyle(
                      fixedWaveColor: Color(0xFFD9D9D9),
                      liveWaveColor: Color(0xFFD9D9D9),
                      seekLineColor: Color(0xFFEF6D70),
                      seekLineThickness: 1,
                      waveThickness: 1,
                      spacing: 3,
                      scaleFactor: 800,
                    ),
                  ),
                  DefaultTextStyle(
                    style: TextStyle(
                      fontWeight: FontWeight.w400,
                      fontSize: 10,
                      color: isLightMode
                          ? const Color(0xFF999999)
                          : const Color(0x66FFFFFF),
                    ),
                    child: Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        Text(currentDuration.toTimeFormatString()),
                        Text(maxDuration.toTimeFormatString()),
                      ],
                    ),
                  ),
                  Row(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      InkWell(
                        splashColor: Colors.transparent,
                        highlightColor: Colors.transparent,
                        onTap: _onClickReplay,
                        child: BudIcon(
                          icon: AssetsUtil.icon_reply_15,
                          size: 24.sp,
                        ),
                      ),
                      SizedBox(width: 24.sp),
                      InkWell(
                        splashColor: Colors.transparent,
                        highlightColor: Colors.transparent,
                        onTap: _onClickPlay,
                        child: (_playerState == PlayerState.playing)
                            ? Icon(Icons.pause_circle)
                            : BudIcon(
                                icon: AssetsUtil.icon_audio_play,
                                size: 24.sp,
                              ),
                      ),
                      SizedBox(width: 24.sp),
                      InkWell(
                        splashColor: Colors.transparent,
                        highlightColor: Colors.transparent,
                        onTap: _onClickForward,
                        child: BudIcon(
                          icon: AssetsUtil.icon_forward_15,
                          size: 24.sp,
                        ),
                      ),
                    ],
                  ),
                ],
              );
            },
          )
        : Text(S.of(context).audioPlayerTip);
  }
}
