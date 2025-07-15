#ifndef __BREAKPOINT_H__
#define __BREAKPOINT_H__

#include "system/includes.h"
#include "media/audio_decoder.h"
#include "music/music_player.h"

struct __breakpoint *breakpoint_handle_creat(void);
void breakpoint_handle_destroy(struct __breakpoint **bp);
bool breakpoint_vm_read(struct __breakpoint *bp, char *logo);
void breakpoint_vm_write(struct __breakpoint *bp, char *logo);

#endif//__BREAKPOINT_H__

