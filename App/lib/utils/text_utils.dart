import 'dart:math';

// Edit Distance Calculation
double calculateEditDistance(String text1, String text2) {
  final lower1 = text1.toLowerCase();
  final lower2 = text2.toLowerCase();

  int len1 = lower1.length;
  int len2 = lower2.length;

  List<List<int>> dp = List.generate(
      len1 + 1, (_) => List<int>.filled(len2 + 1, 0, growable: false),
      growable: false);

  for (int i = 0; i <= len1; i++) dp[i][0] = i;
  for (int j = 0; j <= len2; j++) dp[0][j] = j;

  for (int i = 1; i <= len1; i++) {
    for (int j = 1; j <= len2; j++) {
      if (lower1[i - 1] == lower2[j - 1]) {
        dp[i][j] = dp[i - 1][j - 1];
      } else {
        dp[i][j] = 1 +
            min(dp[i - 1][j - 1],
                min(dp[i - 1][j],
                    dp[i][j - 1]));
      }
    }
  }

  int editDistance = dp[len1][len2];
  int maxLength = max(len1, len2);
  return 1.0 - (editDistance / maxLength);
}