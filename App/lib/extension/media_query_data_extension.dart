import 'dart:math';

import 'package:flutter/material.dart';

extension MediaQueryDataExtension on MediaQueryData {
  double get fixedBottom {
    return fixBottom(34);
  }

  double fixBottom(double bottom) {
    double fix = max(padding.bottom, bottom);
    return fix;
  }
}
