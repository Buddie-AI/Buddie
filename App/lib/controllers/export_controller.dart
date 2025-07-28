import 'dart:convert';
import 'dart:io';

import 'package:app/utils/share_plus_util.dart';
import 'package:file_picker/file_picker.dart';
import 'package:path_provider/path_provider.dart';
import 'package:flutter/material.dart';

class ExportLatencyLog extends StatefulWidget {
  const ExportLatencyLog({super.key});

  @override
  State createState() => _ExportLatencyLogState();
}

class _ExportLatencyLogState extends State<ExportLatencyLog> {
  final TextEditingController _fileNameController = TextEditingController();
  String _fileName = 'latency_log.txt';

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

  Future<String?> _pickDirectory() async {
    final result = await FilePicker.platform.getDirectoryPath();
    return result;
  }

  Future<void> _exportData() async {
    String results;

    final directory = await getApplicationDocumentsDirectory();
    final logFile = File('${directory.path}/latency_report.txt');

    try {
      results = await logFile.readAsString();
    } catch (e) {
      print("Error reading file: $e");
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Error reading log file: $e')),
      );
      return;
    }

    final String tempDir = directory.path;
    String filePath = '$tempDir/$_fileName';
    if (!filePath.endsWith('.txt')) {
      filePath = '$filePath.txt';
    }

    try {
      File file = File(filePath);
      await file.writeAsBytes(utf8.encode(results));
      bool success = await SharePlusUtil.shareFile(path: filePath);
      if (success) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('File has been shared: ${file.path}')),
        );
      }
    } catch (e) {
      debugPrint('Error exporting TXT file: $e');
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Error exporting file: $e')),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      title: const Text('Export Latency Log'),
      content: SingleChildScrollView(
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            TextField(
              decoration: const InputDecoration(labelText: 'File Name'),
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
          child: const Text('Cancel'),
          onPressed: () {
            Navigator.of(context).pop();
          },
        ),
        TextButton(
          child: const Text('Confirm'),
          onPressed: () async {
            await _exportData();
            Navigator.of(context).pop();
          },
        ),
      ],
    );
  }
}
