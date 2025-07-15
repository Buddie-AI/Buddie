#ifndef _STORAGE_DEV_H_
#define _STORAGE_DEV_H_

#include "typedef.h"

struct storage_dev {
    char *logo;
    char *dev_name;
    char *storage_path;
    char *root_path;
    char *fs_type;
    void *priv;
    u32  *counter;
};

enum {
    STORAGE_DEV_OK = 0,
    STORAGE_DEV_ALREADY = (('S' << 24) | ('G' << 16) | ('\0' << 8) | '\0'),
    STORAGE_DEV_IS_NOT_STORAGE,
    STORAGE_DEV_NO_FIND,
    STORAGE_DEV_EMPTY,
    STORAGE_DEV_FULL,
    STORAGE_DEV_CALLBACK,
    STORAGE_DEV_FAT_MOUNT_FAIL,
};

int storage_dev_add(void *logo);
int storage_dev_del(void *logo);
int storage_dev_total(void);

struct storage_dev *storage_dev_check(void *logo);
struct storage_dev *storage_dev_frist(void);
struct storage_dev *storage_dev_last(void);
struct storage_dev *storage_dev_last_active(void);
struct storage_dev *storage_dev_next(struct storage_dev *cur);
struct storage_dev *storage_dev_prev(struct storage_dev *cur);
struct storage_dev *storage_dev_last_active(void);
struct storage_dev *storage_dev_find_by_index(u16 index);//根据号码获取设备
void storage_dev_active_mark(void *logo);


// return 0-ok
typedef int (* storage_callback)(struct storage_dev *);

int storage_dev_add_ex(void *logo, storage_callback cb);
int storage_dev_del_ex(void *logo, storage_callback cb);
int storage_dev_total_ex(storage_callback cb);

struct storage_dev *storage_dev_check_ex(void *logo, storage_callback cb);
struct storage_dev *storage_dev_frist_ex(storage_callback cb);
struct storage_dev *storage_dev_last_ex(storage_callback cb);
struct storage_dev *storage_dev_next_ex(struct storage_dev *cur, storage_callback cb);
struct storage_dev *storage_dev_prev_ex(struct storage_dev *cur, storage_callback cb);
struct storage_dev *storage_dev_last_active_ex(storage_callback cb);


#define REGISTER_STORAGE_DEVICE(node) \
    const struct storage_dev node sec(.storage_device)

#define REGISTER_STORAGE_DEVICES(node) \
    const struct storage_dev node[] sec(.storage_device)

#endif

