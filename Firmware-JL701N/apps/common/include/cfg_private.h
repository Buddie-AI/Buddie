#ifndef _CFG_PRIVATE_H
#define _CFG_PRIVATE_H

int cfg_private_delete_file(FILE *file);
int cfg_private_write(FILE *file, void *buf, u32 len);
FILE *cfg_private_open(const char *path, const char *mode);
int cfg_private_read(FILE *file, void *buf, u32 len);
int cfg_private_close(FILE *file);
int cfg_private_seek(FILE *file, int offset, int fromwhere);
void cfg_private_erase(u32 addr, u32 len);

#endif
