/*****************************************************************
>file name : audio_track.h
>author : lichao
>create time : Tue 22 Dec 2020 04:18:39 PM CST
*****************************************************************/
#ifndef _AUDIO_TRACK_H_
#define _AUDIO_TRACK_H_

void *audio_local_sample_track_open(u8 channel, int sample_rate, int period);
int audio_local_sample_track_in_period(void *c, int samples);
int audio_local_sample_track_rate(void *c);
void audio_local_sample_track_close(void *c);

#endif
