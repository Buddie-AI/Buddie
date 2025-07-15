/*****************************************************************
>file name : spatial_audio_test.c
>create time : Thu 19 May 2022 03:47:18 PM CST
*****************************************************************/
#include "app_config.h"
#include "tone_player.h"

#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
extern int a2dp_spatial_audio_open(void);
extern void a2dp_spatial_audio_close(void);
extern void a2dp_spatial_audio_head_tracked_en(u8 en);
extern void a2dp_spatial_audio_setup(u8 en, u8 head_track);
extern void spatial_audio_set_effect_mode(int mode);

static int test_mode = 0;
static u8 first_in = 1;
int a2dp_spatial_audio_test(void)
{
    if (++test_mode > 4) {
        test_mode = 0;
    }

    if (first_in) {
        first_in = 0;
        test_mode = 0;
    }

    spatial_audio_set_effect_mode(test_mode);
    if (test_mode == 0) {
        printf("SpatialAudio:open_fixed");
        a2dp_spatial_audio_setup(1, 0);
        //tone_play_index(IDEX_TONE_SA_FIXED,1);
        tone_play_index(IDEX_TONE_NORMAL, 1);
    } else if (test_mode == 1) {
        printf("SpatialAudio:open_tracked");
        a2dp_spatial_audio_setup(1, 1);
        tone_play_index(IDEX_TONE_NORMAL, 1);
    }

    else if (test_mode == 2) {
        printf("SpatialAudio:open_tracked");
        a2dp_spatial_audio_setup(1, 0);
        tone_play_index(IDEX_TONE_NORMAL, 1);
    } else if (test_mode == 3) {
        printf("SpatialAudio:open_tracked");
        a2dp_spatial_audio_setup(1, 1);
        //tone_play_index(IDEX_TONE_SA_TRACKED,1);
        tone_play_index(IDEX_TONE_NORMAL, 1);
    }
    /*
    else if (test_mode == 4){
    	printf("SpatialAudio:open_tracked");
    	a2dp_spatial_audio_setup(1,0);
         tone_play_index(IDEX_TONE_NUM_4,1);
    } else if (test_mode == 5){
    	printf("SpatialAudio:open_tracked");
    	a2dp_spatial_audio_setup(1,1);
         tone_play_index(IDEX_TONE_NUM_5,1);
    }
    */
    else {
        printf("SpatialAudio:close");
        a2dp_spatial_audio_setup(0, 0);
        tone_play_index(IDEX_TONE_NORMAL, 1);
    }
    return 0;
}

#endif/*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/

