package com.tencent.cloud.asr.plugin.asr_plugin;

public class ASRControllerConfig {
    int appID = 0;
    int projectID = 0;
    String secretID = "";
    String secretKey = "";
    String token = null;

    String engine_model_type = "";
    int filter_dirty;
    int filter_modal;
    int filter_punc;
    int convert_num_mode;
    String hotword_id = "";
    String customization_id = "";
    int vad_silence_time;
    int needvad;
    int word_info;
    int reinforce_hotword;
    float noise_threshold;

    boolean is_compress;
    boolean silence_detect;
    int silence_detect_duration;
    boolean is_save_audio_file;
    String audio_file_path = "";
}
