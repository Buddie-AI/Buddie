import 'dart:convert';
import 'dart:io';

import 'package:csv/csv.dart';
import 'package:flutter/material.dart';

import '../../../../generated/l10n.dart';
import '../../../../models/record_entity.dart';
import '../../../../services/objectbox_service.dart';
import '../../../../utils/path_provider_utils.dart';
import '../../../../utils/share_plus_util.dart';

class ExportDataDialog extends StatefulWidget {
  const ExportDataDialog({super.key});

  @override
  State createState() => _ExportDataDialogState();
}

class _ExportDataDialogState extends State<ExportDataDialog> {
  DateTimeRange? _selectedDateRange;
  final TextEditingController _fileNameController = TextEditingController();
  String _fileName = 'exported_data.csv';

  @override
  void initState() {
    super.initState();
    _fileNameController.text = _fileName;
  }

  @override
  void dispose() {
    _fileNameController.dispose();
    super.dispose();
  }

  Future<void> _pickDateRange() async {
    final DateTimeRange? picked = await showDateRangePicker(
      context: context,
      firstDate: DateTime(2000),
      lastDate: DateTime(2101),
      initialDateRange: _selectedDateRange,
    );

    if (picked != null && picked != _selectedDateRange) {
      setState(() {
        _selectedDateRange = picked;
      });
    }
  }

  Future<void> _exportData() async {
    List<RecordEntity>? results;
    if (_selectedDateRange == null) {
      results = ObjectBoxService().getRecords();
    } else {
      results = ObjectBoxService().getRecordsByTimeRange(
        _selectedDateRange!.start.millisecondsSinceEpoch,
        _selectedDateRange!.end.millisecondsSinceEpoch,
      );
    }

    List<List<dynamic>> rows = [];
    rows.add(['Role', 'Content', 'Timestamp']);
    for (var record in results!) {
      rows.add([
        record.role,
        record.content,
        DateTime.fromMillisecondsSinceEpoch(record.createdAt!).toString(),
      ]);
    }
    String csvData = const ListToCsvConverter().convert(rows);
    final String path = await PathProviderUtil.getAppSaveDirectory();
    String filePath = '$path/$_fileName';
    if (!filePath.endsWith('.csv')) {
      filePath = '$filePath.csv';
    }

    try {
      File file = File(filePath);
      await file.writeAsBytes(utf8.encode(csvData));
      bool success = await SharePlusUtil.shareFile(path: filePath);
      if (success) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('File has been saved to: ${file.path}')),
        );
      }
    } catch (e) {
      debugPrint('Error saving CSV file: $e');
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(SnackBar(content: Text('Error saving file: $e')));
    }
  }

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      title: Text(S.of(context).exportDataTitle),
      content: SingleChildScrollView(
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            ListTile(
              leading: const Icon(Icons.calendar_today),
              title: Text(S.of(context).exportDataDialogTip),
              subtitle: _selectedDateRange == null
                  ? Text(S.of(context).exportDataDialogTipNoDate)
                  : Text(
                      S
                          .of(context)
                          .exportDataDialogDateFromTo(
                            "${_selectedDateRange!.start}",
                            "${_selectedDateRange!.end}",
                          ),
                    ),
              onTap: _pickDateRange,
            ),
            TextField(
              decoration: InputDecoration(
                labelText: S.of(context).exportDataFileName,
              ),
              controller: _fileNameController,
              onChanged: (value) {
                setState(() {
                  _fileName = value;
                });
              },
            ),
          ],
        ),
      ),
      actions: <Widget>[
        TextButton(
          child: Text(S.of(context).buttonCancel),
          onPressed: () {
            Navigator.of(context).pop();
          },
        ),
        TextButton(
          child: Text(S.of(context).buttonConfirm),
          onPressed: () async {
            await _exportData();
            Navigator.of(context).pop();
          },
        ),
      ],
    );
  }
}
