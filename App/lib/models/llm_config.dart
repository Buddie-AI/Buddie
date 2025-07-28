import 'package:objectbox/objectbox.dart';

@Entity()
class LlmConfigEntity {
  int id;
  String? provider;
  String? model;
  String? apiKey;
  String? baseUrl;

  LlmConfigEntity({
    this.id = 0,
    this.provider,
    this.model,
    this.apiKey,
    this.baseUrl,
  });
}
