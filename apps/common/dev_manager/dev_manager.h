#ifndef __DEV_MANAGER_H__
#define __DEV_MANAGER_H__

#include "system/includes.h"
#include "typedef.h"
#include "system/fs/fs.h"

enum {
    DEV_MANAGER_ADD_OK = 0x0,
    DEV_MANAGER_ADD_IN_LIST_AREADY,
    DEV_MANAGER_ADD_ERR_PARM,
    DEV_MANAGER_ADD_ERR_NOMEM,
    DEV_MANAGER_ADD_ERR_NOT_FOUND,
    DEV_MANAGER_ADD_ERR_MOUNT_FAIL,
};


struct __dev;

struct __scan_callback {
    void (*enter)(struct __dev *dev);
    void (*exit)(struct __dev *dev);
    int (*scan_break)(void);
};

//dev_manager增加设备节点
int dev_manager_add(char *logo);
//dev_manager删除设备节点
int dev_manager_del(char *logo);
//dev_manager获取设备总数
u32 dev_manager_get_total(u8 valid);
//dev_manager通过设备节点检查设备是否在设备链表中
struct __dev *dev_manager_check(struct __dev *dev);
//dev_manager通过逻辑盘符检查设备是否在设备链表中
struct __dev *dev_manager_check_by_logo(char *logo);
//dev_manager查找第一个设备
struct __dev *dev_manager_find_first(u8 valid);
//dev_manager查找最后一个设备
struct __dev *dev_manager_find_last(u8 valid);
//dev_manager查找参数设备的前一个设备
struct __dev *dev_manager_find_prev(struct __dev *dev, u8 valid);
//dev_manager查找参数设备的后一个设备
struct __dev *dev_manager_find_next(struct __dev *dev, u8 valid);
//dev_manager查找最后活动设备
struct __dev *dev_manager_find_active(u8 valid);
//dev_manager查找指定逻辑盘符对应的设备
struct __dev *dev_manager_find_spec(char *logo, u8 valid);
//dev_manager查找指定序号设备
struct __dev *dev_manager_find_by_index(u32 index, u8 valid);
//dev_manager扫盘句柄释放
void dev_manager_scan_disk_release(struct vfscan *fsn);
//dev_manager扫盘
struct vfscan *dev_manager_scan_disk(struct __dev *dev, const char *path, const char *parm, u8 cycle_mode, struct __scan_callback *callback);
//dev_manager设定指定设备节点设备有效
void dev_manager_set_valid(struct __dev *dev, u8 flag);
//dev_manager设定指定设备节点设备为最后活动设备
void dev_manager_set_active(struct __dev *dev);
//dev_manager设定指定逻辑盘符的设备有效
void dev_manager_set_valid_by_logo(char *logo, u8 flag);
//dev_manager设定指定逻辑盘符的设备为最后活动设备
void dev_manager_set_active_by_logo(char *logo);
//dev_manager获取指定设备节点的逻辑盘符
char *dev_manager_get_logo(struct __dev *dev);
//获取物理设备节点的逻辑盘符(去掉_rec后缀)
char *dev_manager_get_phy_logo(struct __dev *dev);
//获取录音文件夹设备节点的逻辑盘符(追加_rec后缀)
char *dev_manager_get_rec_logo(struct __dev *dev);
//dev_manager获取指定设备节点的文件系统根目录
char *dev_manager_get_root_path(struct __dev *dev);
//dev_manager获取指定逻辑盘符的设备的文件系统根目录
char *dev_manager_get_root_path_by_logo(char *logo);
//dev_manager通过设备节点获取设备mount信息
struct imount *dev_manager_get_mount_hdl(struct __dev *dev);
//dev_manager通过设备节点检查设备是否在线
int dev_manager_online_check(struct __dev *dev, u8 valid);
//dev_manager通过逻辑盘符检查设备是否在线
int dev_manager_online_check_by_logo(char *logo, u8 valid);
//通过逻辑盘符判断设备是否在设备链表中
struct __dev *dev_manager_list_check_by_logo(char *logo);
//检查链表中没有挂载的设备并重新挂载
void dev_manager_list_check_mount(void);
//设备挂载
int dev_manager_mount(char *logo);
//设备卸载
int dev_manager_unmount(char *logo);

//dev_manager初始化
void dev_manager_init(void);

void dev_manager_var_init();

#endif//__DEV_MANAGER_H__

