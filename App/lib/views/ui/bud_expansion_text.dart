import 'package:flutter/material.dart';

class BudExpansionText extends StatelessWidget {
  final bool expanded;
  final int maxLines;
  final String text;
  final TextStyle style;

  const BudExpansionText({
    super.key,
    this.expanded = true,
    this.maxLines = 2,
    required this.text,
    required this.style,
  });

  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(
      builder: (BuildContext context, BoxConstraints constraints) {
        final double maxWidth = constraints.maxWidth;
        final TextSpan textSpan = TextSpan(
          text: text,
          style: style,
        );

        final TextPainter textPainter = TextPainter(
          text: textSpan,
          maxLines: maxLines,
          textDirection: TextDirection.ltr,
        )..layout(maxWidth: maxWidth);
        if (!textPainter.didExceedMaxLines) {
          return SelectableText.rich(textSpan);
        }

        /// ...
        final TextSpan ellipsisTextSpan = TextSpan(
          text: '...',
          style: style,
        );

        final TextPainter linkPainter = TextPainter(
          text: ellipsisTextSpan,
          textDirection: TextDirection.ltr,
        )..layout(maxWidth: maxWidth);
        final position = textPainter.getPositionForOffset(Offset(maxWidth - linkPainter.width, textPainter.height));
        final int endOffset = textPainter.getOffsetBefore(position.offset) ?? position.offset;
        String clipText = text.substring(0, endOffset - 3);

        return SelectableText.rich(
          maxLines: expanded ? null : maxLines,
          TextSpan(
            text: expanded ? text : clipText,
            style: style,
            children: [
              if (!expanded) ellipsisTextSpan,
            ],
          ),
        );
      },
    );
  }
}
