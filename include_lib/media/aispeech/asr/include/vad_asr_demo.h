#ifndef _AISPEECH_VAD_ASR_DEMO_H
#define _AISPEECH_VAD_ASR_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

void *aispeech_vad_asr_init(void);
void aispeech_vad_asr_deinit(void);
int aispeech_vad_asr_feed(char *data, int data_len);

typedef int (*asr_output_handler)(int status, const char *json, int bytes);
void aispeech_asr_register_handler(asr_output_handler func);

#ifdef __cplusplus
}
#endif
#endif // _AISPEECH_VAD_ASR_MANAGER_H
