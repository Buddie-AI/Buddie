#include "includes.h"
#include "app_config.h"
#include "norflash_sfc.h"

#if (defined(TCFG_NORFLASH_SFC_DEV_ENABLE) && TCFG_NORFLASH_SFC_DEV_ENABLE)

#undef LOG_TAG_CONST
#define LOG_TAG     "[FLASH_SFC]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"


#define MAX_NORFLASH_PART_NUM       4

struct norflash_partition {
    const char *name;
    u32 start_addr;
    u32 size;
    struct device device;
};

static struct norflash_partition nor_part[MAX_NORFLASH_PART_NUM];

struct norflash_info {
    u32 flash_id;
    u32 flash_capacity;
    int spi_num;
    int spi_err;
    u8 spi_cs_io;
    u8 spi_r_width;
    u8 part_num;
    u8 open_cnt;
    struct norflash_partition *const part_list;
    OS_MUTEX mutex;
    u32 max_end_addr;
};

static struct norflash_info _norflash = {
    .spi_num = (int) - 1,
    .part_list = nor_part,
};

static int _norflash_read(u32 addr, u8 *buf, u32 len, u8 cache);
static int _norflash_eraser(u8 eraser, u32 addr);
static void _norflash_cache_sync_timer(void *priv);
static int _norflash_write_pages(u32 addr, u8 *buf, u32 len);

static struct norflash_partition *norflash_find_part(const char *name)
{
    struct norflash_partition *part = NULL;
    u32 idx;
    for (idx = 0; idx < MAX_NORFLASH_PART_NUM; idx++) {
        part = &_norflash.part_list[idx];
        if (part->name == NULL) {
            continue;
        }
        if (!strcmp(part->name, name)) {
            return part;
        }
    }
    return NULL;
}

static struct norflash_partition *norflash_new_part(const char *name, u32 addr, u32 size)
{
    struct norflash_partition *part;
    u32 idx;
    for (idx = 0; idx < MAX_NORFLASH_PART_NUM; idx++) {
        part = &_norflash.part_list[idx];
        if (part->name == NULL) {
            break;
        }
    }
    if (part->name != NULL) {
        log_error("create norflash part fail\n");
        return NULL;
    }
    memset(part, 0, sizeof(*part));
    part->name = name;
    part->start_addr = addr;
    part->size = size;
    if (part->start_addr + part->size > _norflash.max_end_addr) {
        _norflash.max_end_addr = part->start_addr + part->size;
    }
    _norflash.part_num++;
    return part;
}

static void norflash_delete_part(const char *name)
{
    struct norflash_partition *part;
    u32 idx;
    for (idx = 0; idx < MAX_NORFLASH_PART_NUM; idx++) {
        part = &_norflash.part_list[idx];
        if (part->name == NULL) {
            continue;
        }
        if (!strcmp(part->name, name)) {
            part->name = NULL;
            _norflash.part_num--;
        }
    }
}

static int norflash_verify_part(struct norflash_partition *p)
{
    struct norflash_partition *part = NULL;
    u32 idx;
    for (idx = 0; idx < MAX_NORFLASH_PART_NUM; idx++) {
        part = &_norflash.part_list[idx];
        if (part->name == NULL) {
            continue;
        }
        if ((p->start_addr >= part->start_addr) && (p->start_addr < part->start_addr + part->size)) {
            if (strcmp(p->name, part->name) != 0) {
                return -1;
            }
        }
    }
    return 0;
}



#define FLASH_CACHE_ENABLE  		1

#if FLASH_CACHE_ENABLE
static u32 flash_cache_addr;
static u8 *flash_cache_buf = NULL; //缓存4K的数据，与flash里的数据一样。
static u8 flash_cache_is_dirty;
static u16 flash_cache_timer;

#define FLASH_CACHE_SYNC_T_INTERVAL     60

static int _check_0xff(u8 *buf, u32 len)
{
    for (u32 i = 0; i < len; i ++) {
        if ((*(buf + i)) != 0xff) {
            return 1;
        }
    }
    return 0;
}
#endif


static u32 _pow(u32 num, int n)
{
    u32 powint = 1;
    int i;
    for (i = 1; i <= n; i++) {
        powint *= num;
    }
    return powint;
}

static u32 _norflash_read_id()
{
    return sfc_spi_read_id();
}

static int _norflash_init(const char *name, struct norflash_sfc_dev_platform_data *pdata)
{
    log_info("norflash_sfc_init >>>>");

    struct norflash_partition *part;
    if (_norflash.spi_num == (int) - 1) {
        //_norflash.spi_num = pdata->spi_hw_num;
        _norflash.flash_id = 0;
        _norflash.flash_capacity = 0;
        os_mutex_create(&_norflash.mutex);
        _norflash.max_end_addr = 0;
        _norflash.part_num = 0;
        sfc_spi_init(pdata->sfc_spi_pdata);
    }

    part = norflash_find_part(name);

    if (!part) {
        part = norflash_new_part(name, pdata->start_addr, pdata->size);
        ASSERT(part, "not enough norflash partition memory in array\n");
        ASSERT(norflash_verify_part(part) == 0, "norflash partition %s overlaps\n", name);
        log_info("norflash new partition %s\n", part->name);
    } else {
        ASSERT(0, "norflash partition name already exists\n");
    }
    return 0;
}

static void clock_critical_enter()
{

}
static void clock_critical_exit()
{
    /* if (!(_norflash.flash_id == 0 || _norflash.flash_id == 0xffff)) { */
    /* spi_set_baud(_norflash.spi_num, spi_get_baud(_norflash.spi_num)); */
    /* } */
}
CLOCK_CRITICAL_HANDLE_REG(spi_norflash, clock_critical_enter, clock_critical_exit);

static int _norflash_open(void *arg)
{
    int reg = 0;
    os_mutex_pend(&_norflash.mutex, 0);
    log_info("norflash open\n");
    if (!_norflash.open_cnt) {
        _norflash.flash_id = _norflash_read_id();

        log_info("norflash_read_id: 0x%x\n", _norflash.flash_id);
        if ((_norflash.flash_id == 0) || (_norflash.flash_id == 0xffffff)) {
            log_error("read norflash id error !\n");
            reg = -ENODEV;
            goto __exit;
        }
        _norflash.flash_capacity = 64 * _pow(2, (_norflash.flash_id & 0xff) - 0x10) * 1024;
        log_info("norflash_capacity: 0x%x\n", _norflash.flash_capacity);

#if FLASH_CACHE_ENABLE
        flash_cache_buf = (u8 *)malloc(4096);
        ASSERT(flash_cache_buf, "flash_cache_buf is not ok\n");
        flash_cache_addr = 4096;//先给一个大于4096的数
        _norflash_read(0, flash_cache_buf, 4096, 1);
        flash_cache_addr = 0;
#endif
        log_info("norflash open success !\n");
    }
    if (_norflash.flash_id == 0 || _norflash.flash_id == 0xffffff)  {
        log_error("re-open norflash id error !\n");
        reg = -EFAULT;
        goto __exit;
    }
    ASSERT(_norflash.max_end_addr <= _norflash.flash_capacity, "max partition end address is greater than flash capacity\n");
    _norflash.open_cnt++;

__exit:
    os_mutex_post(&_norflash.mutex);
    return reg;
}

int _norflash_close(void)
{
    os_mutex_pend(&_norflash.mutex, 0);
    log_info("norflash close\n");
    if (_norflash.open_cnt) {
        _norflash.open_cnt--;
    }
    if (!_norflash.open_cnt) {
#if FLASH_CACHE_ENABLE
        if (flash_cache_is_dirty) {
            flash_cache_is_dirty = 0;
            _norflash_eraser(IOCTL_ERASE_SECTOR, flash_cache_addr);
            _norflash_write_pages(flash_cache_addr, flash_cache_buf, 4096);
        }
        free(flash_cache_buf);
        flash_cache_buf = NULL;
#endif
        log_info("norflash close done\n");
    }
    os_mutex_post(&_norflash.mutex);
    return 0;
}

static int _norflash_read(u32 addr, u8 *buf, u32 len, u8 cache)
{
    int ret = 0;

    ret = sfc_spi_read(addr, buf, len);
    if (ret != len) {
        ret = -1;
    } else {
        ret = 0;
    }

    return ret;
}

static int _norflash_write_pages(u32 addr, u8 *buf, u32 len)
{
    /* y_printf("flash write addr = %d, num = %d\n", addr, len); */
    int ret = 0;
    ret = sfc_spi_write_pages(addr, buf, len);
    if (ret != len) {
        ret = -1;
    } else {
        ret = 0;
    }

    return ret;
}

#if FLASH_CACHE_ENABLE
static void _norflash_cache_sync_timer(void *priv)
{
    int reg = 0;
    os_mutex_pend(&_norflash.mutex, 0);
    if (flash_cache_is_dirty) {
        flash_cache_is_dirty = 0;
        reg = _norflash_eraser(IOCTL_ERASE_SECTOR, flash_cache_addr);
        if (reg) {
            goto __exit;
        }
        reg = _norflash_write_pages(flash_cache_addr, flash_cache_buf, 4096);
    }
    if (flash_cache_timer) {
        sys_timeout_del(flash_cache_timer);
        flash_cache_timer = 0;
    }
__exit:
    os_mutex_post(&_norflash.mutex);
}
#endif

int _norflash_write(u32 addr, void *buf, u32 len, u8 cache)
{
    int reg = 0;
    os_mutex_pend(&_norflash.mutex, 0);

    u8 *w_buf = (u8 *)buf;
    u32 w_len = len;

    /* y_printf("flash write addr = %d, num = %d\n", addr, len); */
#if FLASH_CACHE_ENABLE
    if (!cache) {
        reg = _norflash_write_pages(addr, w_buf, w_len);
        goto __exit;
    }
    u32 align_addr = addr / 4096 * 4096;
    u32 align_len = 4096 - (addr - align_addr);
    align_len = w_len > align_len ? align_len : w_len;
    if (align_addr != flash_cache_addr) {
        if (flash_cache_is_dirty) {
            flash_cache_is_dirty = 0;
            reg = _norflash_eraser(IOCTL_ERASE_SECTOR, flash_cache_addr);
            if (reg) {
                line_inf;
                goto __exit;
            }
            reg = _norflash_write_pages(flash_cache_addr, flash_cache_buf, 4096);
            if (reg) {
                line_inf;
                goto __exit;
            }
        }
        _norflash_read(align_addr, flash_cache_buf, 4096, 0);
        flash_cache_addr = align_addr;
    }
    memcpy(flash_cache_buf + (addr - align_addr), w_buf, align_len);
    if ((addr + align_len) % 4096) {
        flash_cache_is_dirty = 1;
        if (flash_cache_timer) {
            sys_timer_re_run(flash_cache_timer);
        } else {
            flash_cache_timer = sys_timeout_add(0, _norflash_cache_sync_timer, FLASH_CACHE_SYNC_T_INTERVAL);
        }
    } else {
        flash_cache_is_dirty = 0;
        reg = _norflash_eraser(IOCTL_ERASE_SECTOR, align_addr);
        if (reg) {
            line_inf;
            goto __exit;
        }
        reg = _norflash_write_pages(align_addr, flash_cache_buf, 4096);
        if (reg) {
            line_inf;
            goto __exit;
        }
    }
    addr += align_len;
    w_buf += align_len;
    w_len -= align_len;
    while (w_len) {
        u32 cnt = w_len > 4096 ? 4096 : w_len;
        _norflash_read(addr, flash_cache_buf, 4096, 0);
        flash_cache_addr = addr;
        memcpy(flash_cache_buf, w_buf, cnt);
        if ((addr + cnt) % 4096) {
            flash_cache_is_dirty = 1;
            if (flash_cache_timer) {
                sys_timer_re_run(flash_cache_timer);
            } else {
                flash_cache_timer = sys_timeout_add(0, _norflash_cache_sync_timer, FLASH_CACHE_SYNC_T_INTERVAL);
            }
        } else {
            flash_cache_is_dirty = 0;
            reg = _norflash_eraser(IOCTL_ERASE_SECTOR, addr);
            if (reg) {
                line_inf;
                goto __exit;
            }
            reg = _norflash_write_pages(addr, flash_cache_buf, 4096);
            if (reg) {
                line_inf;
                goto __exit;
            }
        }
        addr += cnt;
        w_buf += cnt;
        w_len -= cnt;
    }
#else
    reg = _norflash_write_pages(addr, w_buf, w_len);
#endif
__exit:
    os_mutex_post(&_norflash.mutex);
    return reg;
}

static int _norflash_eraser(u8 eraser, u32 addr)
{
    int ret = 0;

    ret = sfc_spi_eraser(eraser, addr);

    return ret;
}

int _norflash_ioctl(u32 cmd, u32 arg, u32 unit, void *_part)
{
    int reg = 0;
    struct norflash_partition *part = _part;
    os_mutex_pend(&_norflash.mutex, 0);
    switch (cmd) {
    case IOCTL_GET_STATUS:
        *(u32 *)arg = 1;
        break;
    case IOCTL_GET_ID:
        *((u32 *)arg) = _norflash.flash_id;
        break;
    case IOCTL_GET_CAPACITY:
        if (_norflash.flash_capacity == 0)  {
            *(u32 *)arg = 0;
        } else if (_norflash.part_num == 1 && part->start_addr == 0) {
            *(u32 *)arg = _norflash.flash_capacity / unit;
        } else {
            *(u32 *)arg = part->size / unit;
        }
        break;
    case IOCTL_GET_BLOCK_SIZE:
        *(u32 *)arg = 512;
        break;
    case IOCTL_ERASE_PAGE:
        reg = _norflash_eraser(IOCTL_ERASE_PAGE, arg * unit + part->start_addr);
        break;
    case IOCTL_ERASE_SECTOR:
        reg = _norflash_eraser(IOCTL_ERASE_SECTOR, arg * unit + part->start_addr);
        break;
    case IOCTL_ERASE_BLOCK:
        reg = _norflash_eraser(IOCTL_ERASE_BLOCK, arg * unit + part->start_addr);
        break;
    case IOCTL_ERASE_CHIP:
        reg = _norflash_eraser(IOCTL_ERASE_CHIP, 0);
        break;
    case IOCTL_FLUSH:
        break;
    case IOCTL_CMD_RESUME:
        break;
    case IOCTL_CMD_SUSPEND:
        break;
    case IOCTL_GET_PART_INFO:
        u32 *info = (u32 *)arg;
        u32 *start_addr = &info[0];
        u32 *part_size = &info[1];
        *start_addr = part->start_addr;
        *part_size = part->size;
        break;
    default:
        reg = -EINVAL;
        break;
    }
__exit:
    os_mutex_post(&_norflash.mutex);
    return reg;
}


/*************************************************************************************
 *                                  挂钩 device_api
 ************************************************************************************/

static int norflash_sfc_dev_init(const struct dev_node *node, void *arg)
{
    struct norflash_sfc_dev_platform_data *pdata = arg;

    return _norflash_init(node->name, pdata);
}

static int norflash_sfc_dev_open(const char *name, struct device **device, void *arg)
{
    struct norflash_partition *part;
    part = norflash_find_part(name);
    if (!part) {
        log_error("no norflash partition is found\n");
        return -ENODEV;
    }
    *device = &part->device;
    (*device)->private_data = part;
    if (atomic_read(&part->device.ref)) {
        return 0;
    }
    return _norflash_open(arg);
}
static int norflash_sfc_dev_close(struct device *device)
{
    return _norflash_close();
}
static int norflash_sfc_dev_read(struct device *device, void *buf, u32 len, u32 offset)
{
    int reg;
    /* printf("flash read sector = %d, num = %d\n", offset, len); */
    offset = offset * 512;
    len = len * 512;
    struct norflash_partition *part;
    part = (struct norflash_partition *)device->private_data;
    if (!part) {
        log_error("norflash partition invalid\n");
        return -EFAULT;
    }
    offset += part->start_addr;
    reg = _norflash_read(offset, buf, len, 1);
    if (reg) {
        r_printf(">>>[r error]:\n");
        len = 0;
    }

    len = len / 512;
    return len;
}
static int norflash_sfc_dev_write(struct device *device, void *buf, u32 len, u32 offset)
{
    /* printf("flash write sector = %d, num = %d\n", offset, len); */
    int reg = 0;
    offset = offset * 512;
    len = len * 512;
    struct norflash_partition *part = device->private_data;
    if (!part) {
        log_error("norflash partition invalid\n");
        return -EFAULT;
    }
    offset += part->start_addr;
    reg = _norflash_write(offset, buf, len, 1);
    if (reg) {
        r_printf(">>>[w error]:\n");
        len = 0;
    }
    len = len / 512;
    return len;
}
static bool norflash_sfc_dev_online(const struct dev_node *node)
{
    return 1;
}
static int norflash_sfc_dev_ioctl(struct device *device, u32 cmd, u32 arg)
{
    struct norflash_partition *part = device->private_data;
    if (!part) {
        log_error("norflash partition invalid\n");
        return -EFAULT;
    }
    return _norflash_ioctl(cmd, arg, 512, part);
}

/*
 * 1. 外部调用时以512字节为单位的地址和长度，且需要驱动write自己处理擦除，
 * 请使用norflash_sfc_dev_ops接口，否则使用本文件内的其他ops
 *
 * 2. 如果不需要驱动自己处理擦除，可以把宏FLASH_CACHE_ENABLE清零，或者把
 * norflash_sfc_dev_read()里面调用的_norflash_read()的实参cache填0，
 * norflash_sfc_dev_write()同理
 *
 * 3. norflash_sfc_dev_ops可以被多个设备名注册，每个设备名被认为是不同分区，所以
 * 需要填不同的分区起始地址和大小，若分区地址有重叠或者最大分区结束地址大于
 * flash容量，会触发ASSERT()
 *
 * 4. 关于IOCTL_GET_CAPACITY，有多个分区注册时返回分区的大小，如果只注册了1
 * 个分区，分区起始地址 == 0时返回flash容量，起始地址 != 0时返回分区大小，
 * norflash_sfc_dev_ops返回的长度以512字节为单位
 *
 * 5. 本文件内的各个ops可以同时使用
 */
//按512字节单位读写
const struct device_operations norflash_sfc_dev_ops = {
    .init   = norflash_sfc_dev_init,
    .online = norflash_sfc_dev_online,
    .open   = norflash_sfc_dev_open,
    .read   = norflash_sfc_dev_read,
    .write  = norflash_sfc_dev_write,
    .ioctl  = norflash_sfc_dev_ioctl,
    .close  = norflash_sfc_dev_close,
};


static int norflash_sfc_fs_dev_init(const struct dev_node *node, void *arg)
{
    struct norflash_sfc_dev_platform_data *pdata = arg;

    return _norflash_init(node->name, pdata);
}

static int norflash_sfc_fs_dev_open(const char *name, struct device **device, void *arg)
{
    struct norflash_partition *part;
    part = norflash_find_part(name);
    if (!part) {
        log_error("no norflash partition is found\n");
        return -ENODEV;
    }
    *device = &part->device;
    (*device)->private_data = part;
    if (atomic_read(&part->device.ref)) {
        return 0;
    }
    return _norflash_open(arg);
}
static int norflash_sfc_fs_dev_close(struct device *device)
{
    return _norflash_close();
}
static int norflash_sfc_fs_dev_read(struct device *device, void *buf, u32 len, u32 offset)
{
    int reg;
    /* printf("flash read sector = %d, num = %d\n", offset, len); */
    struct norflash_partition *part;
    part = (struct norflash_partition *)device->private_data;
    if (!part) {
        log_error("norflash partition invalid\n");
        return -EFAULT;
    }
    offset += part->start_addr;
    reg = _norflash_read(offset, buf, len, 0);
    if (reg) {
        r_printf(">>>[r error]:\n");
        len = 0;
    }

    return len;
}

static int norflash_sfc_fs_dev_write(struct device *device, void *buf, u32 len, u32 offset)
{
    //printf("flash write addr = 0x%x, len = 0x%x\n", offset, len);
    int reg = 0;
    struct norflash_partition *part = device->private_data;
    if (!part) {
        log_error("norflash partition invalid\n");
        return -EFAULT;
    }
    offset += part->start_addr;
    reg = _norflash_write(offset, buf, len, 0);
    if (reg) {
        r_printf(">>>[w error]:\n");
        len = 0;
    }
    return len;
}
static bool norflash_sfc_fs_dev_online(const struct dev_node *node)
{
    return 1;
}

static int norflash_sfc_fs_dev_ioctl(struct device *device, u32 cmd, u32 arg)
{
    struct norflash_partition *part = device->private_data;
    if (!part) {
        log_error("norflash partition invalid\n");
        return -EFAULT;
    }
    return _norflash_ioctl(cmd, arg, 1, part);
}

/*
 * 1. 外部调用时以1字节为单位的地址和长度，且驱动write自己不处理擦除，
 * 请使用norflash_sfc_fs_dev_ops接口，否则使用本文件内的其他ops。注意：有些文件系
 * 统需要满足这个条件的驱动，如果期望修改成驱动内部处理擦除，需要测试所
 * 有关联的文件系统是否支持，或者新建一个符合需求的ops
 *
 * 2. 如果需要驱动自己处理擦除，需要把宏FLASH_CACHE_ENABLE置1，且
 * norflash_sfc_fs_dev_read()里面调用的_norflash_read()的实参cache填1，
 * norflash_sfc_fs_dev_write()同理
 *
 * 3. norflash_sfc_fs_dev_ops可以被多个设备名注册，每个设备名被认为是不同分区，所以
 * 需要填不同的分区起始地址和大小，若分区地址有重叠或者最大分区结束地址大于
 * flash容量，会触发ASSERT()
 *
 * 4. 关于IOCTL_GET_CAPACITY，有多个分区注册时返回分区的大小，如果只注册了1
 * 个分区，分区起始地址 == 0时返回flash容量，起始地址 != 0时返回分区大小
 * norflash_sfc_fs_dev_ops返回的长度以1字节为单位
 *
 * 5. 本文件内的各个ops可以同时使用
 */
//按1字节单位读写
const struct device_operations norflash_sfc_fs_dev_ops = {
    .init   = norflash_sfc_fs_dev_init,
    .online = norflash_sfc_fs_dev_online,
    .open   = norflash_sfc_fs_dev_open,
    .read   = norflash_sfc_fs_dev_read,
    .write  = norflash_sfc_fs_dev_write,
    .ioctl  = norflash_sfc_fs_dev_ioctl,
    .close  = norflash_sfc_fs_dev_close,
};

/*
 * 对ops的读写单位有另外需求，或者驱动内部是否支持擦除，可以参照上面的ops，
 * 不同条件自由组合，建立新的ops
 */

#endif /* #if (defined(TCFG_NORFLASH_SFC_DEV_ENABLE) && TCFG_NORFLASH_SFC_DEV_ENABLE) */
