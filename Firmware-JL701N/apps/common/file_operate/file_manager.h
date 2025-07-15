#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

#include "system/includes.h"
#include "typedef.h"
#include "system/fs/fs.h"
#include "dev_manager.h"

FILE *file_manager_select(struct __dev *dev, struct vfscan *fs, int sel_mode, int arg, struct __scan_callback *callback);

#endif// __FILE_MANAGER_H__
