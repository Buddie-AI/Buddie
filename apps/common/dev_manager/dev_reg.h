#ifndef __DEV_REG_H__
#define __DEV_REG_H__

#include "system/includes.h"
#include "typedef.h"

///设备参数控制句柄
struct __dev_reg {
    char *logo;//设备选择使用的逻辑盘符
    char *name;//设备名称，底层匹配设备节点使用
    char *storage_path;//设备路径，文件系统mount时使用
    char *root_path;//设备文件系统根目录
    char *fs_type;//文件系统类型,如：fat, sdfile
};


extern const struct __dev_reg dev_reg[];

#endif//__DEV_REG_H__


