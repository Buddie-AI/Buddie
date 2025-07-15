#include "dev_manager.h"
#include "dev_reg.h"
#include "app_config.h"
#if TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0
#include "dev_multiplex_api.h"
#endif
#include "spi/nor_fs.h"
#include "dev_update.h"

// *INDENT-OFF*

#if (TCFG_DEV_MANAGER_ENABLE)

#define DEV_MANAGER_TASK_NAME							"dev_mg"
#define DEV_MANAGER_SCAN_DISK_MAX_DEEPTH				9

///设备管理总控制句柄
struct __dev_manager {
	struct list_head 	list;
	OS_MUTEX			mutex;
	OS_SEM			    sem;
	u32 				counter;
};
static struct __dev_manager dev_mg;
#define __this	(&dev_mg)


///设备节点结构体
struct __dev {
	struct list_head 	entry;
	struct __dev_reg 	*parm;//设备参数
	struct imount 		*fmnt;
	volatile u8			valid:	1;//有效设备标记， 这里有效是指是否有可播放文件
	volatile u32		active_stamp;///活动设备时间戳，通过时间戳记录当前最后活动设备
};


static u32 __dev_manager_get_time_stamp(void)
{
	u32 counter = __this->counter;
	__this->counter ++;
	return counter;
}

int __dev_manager_add(char *logo, u8 need_mount)
{
	if (logo == NULL) {
		return DEV_MANAGER_ADD_ERR_PARM;
	}
	int i;
	printf("%s add start\n", logo);
	struct __dev_reg *p = NULL;
	struct __dev_reg *n;

	for(n=(struct __dev_reg *)dev_reg; n->logo != NULL; n++){
		if (!strcmp(n->logo, logo)) {
			p = n;
			break;
		}
	}

	if (p) {
		///挂载文件系统
		if (dev_manager_list_check_by_logo(logo)) {
			printf("dev online aready, err!!!\n");
			return DEV_MANAGER_ADD_IN_LIST_AREADY;
		}
		struct __dev *dev = (struct __dev *)zalloc(sizeof(struct __dev));
		if(dev == NULL){
			return DEV_MANAGER_ADD_ERR_NOMEM;
		}
		os_mutex_pend(&__this->mutex, 0);
		if(need_mount){
			dev->fmnt = mount(p->name, p->storage_path, p->fs_type, 3, NULL);
		}
		dev->parm = p;
		dev->valid = (dev->fmnt ? 1 : 0);
		dev->active_stamp = __dev_manager_get_time_stamp();//timer_get_ms();
		list_add_tail(&dev->entry, &__this->list);
		os_mutex_post(&__this->mutex);
		printf("%s, %s add ok, dev->fmnt = %x,  %d\n", __FUNCTION__, logo, (int)dev->fmnt, dev->active_stamp);
		if(dev->fmnt == NULL){
			return DEV_MANAGER_ADD_ERR_MOUNT_FAIL;
		}
		return DEV_MANAGER_ADD_OK;
	}
	printf("dev_manager_add can not find logo %s\n",logo);
	return DEV_MANAGER_ADD_ERR_NOT_FOUND;
}

static int __dev_manager_del(char *logo)
{
	if (logo == NULL) {
		return -1;
	}
	struct __dev *dev, *n;
	os_mutex_pend(&__this->mutex, 0);
	list_for_each_entry_safe(dev, n, &__this->list, entry) {
		if (!strcmp(dev->parm->logo, logo)) {
			///卸载文件系统
			if(dev->fmnt){
				unmount(dev->parm->storage_path);
			}
			list_del(&dev->entry);
			free(dev);
			printf("%s, %s del ok\n", __FUNCTION__, logo);
			break;
		}
	}
	os_mutex_post(&__this->mutex);
	return 0;
}

//*----------------------------------------------------------------------------*/
/**@brief    设备增加接口
   @param	 logo:逻辑盘符，如：sd0/sd1/udisk0等
   @return   0:成功，非0是失败
   @note
*/
/*----------------------------------------------------------------------------*/
int dev_manager_add(char *logo)
{
	if(logo == NULL){
		return -1;
	}
	int ret = 0;
#if TCFG_RECORD_FOLDER_DEV_ENABLE
	char rec_dev_logo[16] = {0};
	sprintf(rec_dev_logo, "%s%s", logo, "_rec");
	ret = __dev_manager_add(rec_dev_logo, 1);
	if(ret == DEV_MANAGER_ADD_OK){
		ret = __dev_manager_add(logo, 1);
		if(ret){
			__dev_manager_del(logo);
			__dev_manager_del(rec_dev_logo);
		}
	}else if(ret == DEV_MANAGER_ADD_ERR_NOT_FOUND){
		ret = __dev_manager_add(logo, 1);
	}
	else{
		ret = __dev_manager_add(logo, 0);
	}
#else
	ret = __dev_manager_add(logo, 1);
#endif//TCFG_RECORD_FOLDER_DEV_ENABLE

	return ret;
}

//*----------------------------------------------------------------------------*/
/**@brief    设备删除接口
   @param	 logo:逻辑盘符，如：sd0/sd1/udisk0等
   @return   0:成功，非0是失败
   @note
*/
/*----------------------------------------------------------------------------*/
int dev_manager_del(char *logo)
{
	if(logo == NULL){
		return -1;
	}
	__dev_manager_del(logo);
#if TCFG_RECORD_FOLDER_DEV_ENABLE
    char rec_dev_logo[16] = {0};
	sprintf(rec_dev_logo, "%s%s", logo, "_rec");
	__dev_manager_del(rec_dev_logo);
#endif//TCFG_RECORD_FOLDER_DEV_ENABLE

	return 0;
}
//*----------------------------------------------------------------------------*/
/**@brief    通过设备节点检查设备是否在线
   @param	 dev:设备节点
   @return   成功返回设备节点， 失败返回NULL
   @note     通过设备节点检查设备是否在设备链表中
*/
/*----------------------------------------------------------------------------*/
struct __dev *dev_manager_check(struct __dev *dev)
{
	if (dev == NULL) {
		return NULL;
	}
	struct __dev *p;
	list_for_each_entry(p, &__this->list, entry) {
		if(!(p->fmnt)){
			continue;
		}
		if (dev == p) {
			return p;
		}
	}
	return NULL;
}
//*----------------------------------------------------------------------------*/
/**@brief    通过盘符检查设备是否在线
   @param	 logo:逻辑盘符，如:sd0/sd1/udisk0
   @return   成功返回设备节点， 失败返回NULL
   @note     通过设备节点检查设备是否在设备链表中
*/
/*----------------------------------------------------------------------------*/
struct __dev *dev_manager_check_by_logo(char *logo)
{
	if (logo == NULL) {
		return NULL;
	}
	struct __dev *dev;
	list_for_each_entry(dev, &__this->list, entry) {
		if(!(dev->fmnt)){
			continue;
		}
		if (!strcmp(dev->parm->logo, logo)) {
			return dev;
		}
	}
	return NULL;
}
//*----------------------------------------------------------------------------*/
/**@brief    获取设备总数
   @param	 valid:
   					1：有效可播放设备, 0;所有设备,包括有可播放设备及无可播放设备
   @return   设备总数
   @note     根据使用情景决定接口参数
*/
/*----------------------------------------------------------------------------*/
u32 dev_manager_get_total(u8 valid)
{
	u32 total_valid = 0;
	u32 total = 0;
	struct __dev *dev;
	os_mutex_pend(&__this->mutex, 0);
	list_for_each_entry(dev, &__this->list, entry) {
		if(!(dev->fmnt)){
			continue;
		}
		if (dev->valid) {
			total_valid++;
		}
		total ++;
	}
	os_mutex_post(&__this->mutex);
	return (valid ? total_valid : total);
}
//*----------------------------------------------------------------------------*/
/**@brief    获取设备列表第一个设备
   @param	 valid:
   					1：有效可播放设备, 0;所有设备,包括有可播放设备及无可播放设备
   @return   成功返回设备设备节点,失败返回NULL
   @note     根据使用情景决定接口参数
*/
/*----------------------------------------------------------------------------*/
struct __dev *dev_manager_find_first(u8 valid)
{
	struct __dev *dev = NULL;
	os_mutex_pend(&__this->mutex, 0);
	list_for_each_entry(dev, &__this->list, entry) {
		if(!(dev->fmnt)){
			continue;
		}
		if(valid){
			if (dev->valid) {
				os_mutex_post(&__this->mutex);
				return dev;
			}
		}else{
			os_mutex_post(&__this->mutex);
			return dev;
		}
	}
	os_mutex_post(&__this->mutex);
	return NULL;
}
//*----------------------------------------------------------------------------*/
/**@brief    获取设备列表最后一个设备
   @param	 valid:
   					1：有效可播放设备中查找
					0：所有设备,包括有可播放设备及无可播放设备中查找
   @return   成功返回设备设备节点,失败返回NULL
   @note     根据使用情景决定接口参数
*/
/*----------------------------------------------------------------------------*/
struct __dev *dev_manager_find_last(u8 valid)
{
	struct __dev *dev = NULL;
	os_mutex_pend(&__this->mutex, 0);
	list_for_each_entry_reverse(dev, &__this->list, entry) {
		if(!(dev->fmnt)){
			continue;
		}
		if(valid){
			if (dev->valid) {
				os_mutex_post(&__this->mutex);
				return dev;
			}
		}else{
			os_mutex_post(&__this->mutex);
			return dev;
		}
	}
	os_mutex_post(&__this->mutex);
	return NULL;
}
//*----------------------------------------------------------------------------*/
/**@brief    获取上一个设备节点
   @param
   			dev:当前设备节点
			valid:
   					1：有效可播放设备中查找,
					0：所有设备,包括有可播放设备及无可播放设备中查找
   @return   成功返回设备设备节点,失败返回NULL
   @note     根据当前设置的参数设备节点，找链表中的上一个设备
*/
/*----------------------------------------------------------------------------*/
struct __dev *dev_manager_find_prev(struct __dev *dev, u8 valid)
{
	if (dev == NULL) {
		return NULL;
	}
	struct __dev *p = NULL;
	os_mutex_pend(&__this->mutex, 0);
	if (dev_manager_check(dev) == NULL) {
		///传入的参数无效， 返回活动设备
		os_mutex_post(&__this->mutex);
		return dev_manager_find_active(valid);
	}
	list_for_each_entry(p, &dev->entry, entry) {
        if((void*)p == (void*)(&__this->list)){
            continue;
        }
        if(!(p->fmnt)){
			continue;
		}
		if(valid){
			if (p->valid) {
				os_mutex_post(&__this->mutex);
				return p;
			}
		}else{
			os_mutex_post(&__this->mutex);
			return p;
		}
	}
	os_mutex_post(&__this->mutex);
	return NULL;
}
//*----------------------------------------------------------------------------*/
/**@brief    获取下一个设备节点
   @param
   			dev:当前设备节点
			valid:
   					1：有效可播放设备中查找,
					0：所有设备,包括有可播放设备及无可播放设备中查找
   @return   成功返回设备设备节点,失败返回NULL
   @note     根据当前设置的参数设备节点，找链表中的下一个设备
*/
/*----------------------------------------------------------------------------*/
struct __dev *dev_manager_find_next(struct __dev *dev, u8 valid)
{
	if (dev == NULL) {
		return NULL;
	}
	struct __dev *p = NULL;
	os_mutex_pend(&__this->mutex, 0);
	if (dev_manager_check(dev) == NULL) {
		///传入的参数无效， 返回活动设备
		os_mutex_post(&__this->mutex);
		return dev_manager_find_active(valid);
	}

	list_for_each_entry_reverse(p, &dev->entry, entry) {
        if((void*)p == (void*)(&__this->list)){
            continue;
        }
		if(!(p->fmnt)){
			continue;
		}
		if(valid){
			if (p->valid) {
				os_mutex_post(&__this->mutex);
				return p;
			}
		}else{
			os_mutex_post(&__this->mutex);
			return p;
		}
	}
	os_mutex_post(&__this->mutex);
	return NULL;
}
//*----------------------------------------------------------------------------*/
/**@brief    获取当前活动设备节点
   @param
			valid:
   					1：有效可播放设备中查找,
					0：所有设备,包括有可播放设备及无可播放设备中查找
   @return   成功返回设备设备节点,失败返回NULL
   @note
*/
/*----------------------------------------------------------------------------*/
struct __dev *dev_manager_find_active(u8 valid)
{
	struct __dev *dev = NULL;
	struct __dev *active = NULL;
	os_mutex_pend(&__this->mutex, 0);
	list_for_each_entry(dev, &__this->list, entry) {
		if(!(dev->fmnt)){
			continue;
		}
		if(valid){
			if (dev->valid) {
				if (active) {
					if (active->active_stamp < dev->active_stamp) {
						active = dev;
					}
				} else {
					active = dev;
				}
			}
		}else{
			if (active) {
				if (active->active_stamp < dev->active_stamp) {
					active = dev;
				}
			} else {
				active = dev;
			}

		}
	}
	os_mutex_post(&__this->mutex);
	return active;
}
//*----------------------------------------------------------------------------*/
/**@brief    获取指定设备节点
   @param
   			logo：指定逻辑盘符，如：sd0/sd1/udisk0
			valid:
   					1：有效可播放设备中查找,
					0：所有设备,包括有可播放设备及无可播放设备中查找
   @return   成功返回设备设备节点,失败返回NULL
   @note
*/
/*----------------------------------------------------------------------------*/
struct __dev *dev_manager_find_spec(char *logo, u8 valid)
{
	if (logo == NULL) {
		return NULL;
	}
	struct __dev *dev = NULL;
	os_mutex_pend(&__this->mutex, 0);
	list_for_each_entry(dev, &__this->list, entry) {
		if(!(dev->fmnt)){
			continue;
		}
		if (!strcmp(dev->parm->logo, logo)) {
			if(valid){
				if (dev->valid) {
					os_mutex_post(&__this->mutex);
					return dev;
				}
			}else{
				os_mutex_post(&__this->mutex);
				return dev;
			}
		}
	}
	os_mutex_post(&__this->mutex);
	return NULL;
}
//*----------------------------------------------------------------------------*/
/**@brief    获取指定序号设备节点
   @param
   			index：指定序号，指的是在设备链表中的顺序
			valid:
   					1：有效可播放设备中查找,
					0：所有设备,包括有可播放设备及无可播放设备中查找
   @return   成功返回设备设备节点,失败返回NULL
   @note
*/
/*----------------------------------------------------------------------------*/
struct __dev *dev_manager_find_by_index(u32 index, u8 valid)
{
	struct __dev *dev = NULL;
	u32 i = 0;
	os_mutex_pend(&__this->mutex, 0);
	list_for_each_entry(dev, &__this->list, entry) {
		if(!(dev->fmnt)){
			continue;
		}
		if(valid){
			if (dev->valid) {
				if (i == index) {
					os_mutex_post(&__this->mutex);
					return dev;
				}
				i++;
			}
		}else{
			if (i == index) {
				os_mutex_post(&__this->mutex);
				return dev;
			}
			i++;
		}
	}
	os_mutex_post(&__this->mutex);
	return NULL;
}
//*----------------------------------------------------------------------------*/
/**@brief   设备扫盘释放
   @param
   			fsn：扫描句柄
   @return  无
   @note
*/
/*----------------------------------------------------------------------------*/
void dev_manager_scan_disk_release(struct vfscan *fsn)
{
	if (fsn) {
		fscan_release(fsn);
	}
}
//*----------------------------------------------------------------------------*/
/**@brief   设备扫盘
   @param
   			dev：设备节点
   			path：指定扫描目录
   			parm：扫描参数，包括文件后缀等
   			cycle_mode：播放循环模式
			callback：扫描打断回调
   @return  成功返回扫描控制句柄，失败返回NULL
   @note
*/
/*----------------------------------------------------------------------------*/
struct vfscan *dev_manager_scan_disk(struct __dev *dev, const char *path, const char *parm, u8 cycle_mode, struct __scan_callback *callback)
{
	if (dev_manager_check(dev) == NULL) {
		return NULL;
	}
#if TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0

#if (TCFG_DM_MULTIPLEX_WITH_SD_PORT == 0)     //0:sd0  1:sd1 //dm 参与复用的sd配置
        if(!memcmp(dev->parm->logo ,"sd0",strlen("sd0"))){
#else
        if(!memcmp(dev->parm->logo ,"sd1",strlen("sd1"))){
#endif
            dev_usb_change_sd();
        }

    if(!memcmp(dev->parm->logo ,"udisk",strlen("udisk")))
        dev_sd_change_usb();

    if (dev_manager_online_check(dev, 1) == NULL) {
        printf("mult remount fail !!!\n");
        return NULL;
    }
#endif
	char *fsn_path = NULL;
	char *tmp_path = NULL;
	if (path) {
		if (*path == '/') {
			path++;
		}
		tmp_path = zalloc(strlen(dev->parm->root_path) + strlen(path) + 1);
		if (tmp_path == NULL) {
			return NULL;
		}
		sprintf(tmp_path, "%s%s", dev->parm->root_path, path);
		fsn_path = tmp_path;
	} else {
		fsn_path = dev->parm->root_path;
	}
	printf("fsn_path = %s, scan parm = %s\n", fsn_path, parm);
	struct vfscan *fsn;
	/* clock_add_set(SCAN_DISK_CLK); */
	if(callback && callback->enter){
		callback->enter(dev);//扫描前处理， 可以在注册的回调里提高系统时钟等处理
	}
	fsn = fscan_interrupt(
			(const char *)fsn_path,
			parm,
			DEV_MANAGER_SCAN_DISK_MAX_DEEPTH,
			((callback) ? callback->scan_break : NULL));
	/* clock_remove_set(SCAN_DISK_CLK); */
	if(callback && callback->exit){
		callback->exit(dev);//扫描后处理， 可以在注册的回调里还原到enter前的状态
	}
	if (fsn) {
		if (fsn->file_number == 0) {
			printf("dev nofile\n");
#if (TCFG_DEV_UPDATE_IF_NOFILE_ENABLE)
			///没有文件找升级文件
			dev_update_check(dev->parm->logo);
#endif/*TCFG_DEV_UPDATE_IF_NOFILE_ENABLE*/
			///没有文件,释放fsn， 减少外面流程的处理
			dev_manager_scan_disk_release(fsn);
			fsn = NULL;
		} else {
			fsn->cycle_mode = cycle_mode;
		}
	}

	if (tmp_path) {
		free(tmp_path);
	}
	return fsn;
}
//*----------------------------------------------------------------------------*/
/**@brief   通过设备节点标记指定设备是否有效
   @param
   			dev：设备节点
			flag:
   					1：设备有效,
					0：设备无效
   @return
   @note    这里有无效是指是否有可播放文件
*/
/*----------------------------------------------------------------------------*/
void dev_manager_set_valid(struct __dev *dev, u8 flag)
{
	os_mutex_pend(&__this->mutex, 0);
	if (dev_manager_check(dev)) {
		dev->valid = flag;
	}
	os_mutex_post(&__this->mutex);
}
//*----------------------------------------------------------------------------*/
/**@brief   通过逻辑盘符标记指定设备是否有效
   @param
   			logo：逻辑盘符，如：sd0/sd1/udisk0
			flag:
   					1：设备有效,
					0：设备无效
   @return
   @note    这里有无效是指是否有可播放文件
*/
/*----------------------------------------------------------------------------*/
void dev_manager_set_valid_by_logo(char *logo, u8 flag)
{
	if (logo == NULL) {
		return ;
	}
	struct __dev *dev = NULL;
	os_mutex_pend(&__this->mutex, 0);
	dev = dev_manager_check_by_logo(logo);
	if (dev) {
		dev->valid = flag;
	}
	os_mutex_post(&__this->mutex);
}
//*----------------------------------------------------------------------------*/
/**@brief   激活指定设备节点的设备
   @param
   			dev：设备节点
   @return
   @note    该接口可以将设备变为最新活动设备
*/
/*----------------------------------------------------------------------------*/
void dev_manager_set_active(struct __dev *dev)
{
	os_mutex_pend(&__this->mutex, 0);
	if (dev_manager_check(dev)) {
		dev->active_stamp = __dev_manager_get_time_stamp();//timer_get_ms();
	}
	os_mutex_post(&__this->mutex);
}
//*----------------------------------------------------------------------------*/
/**@brief   激活指定逻辑盘符的设备
   @param
   			logo：逻辑盘符，如：sd0/sd1/udisk0
   @return
   @note    该接口可以将设备变为最新活动设备
*/
/*----------------------------------------------------------------------------*/
void dev_manager_set_active_by_logo(char *logo)
{
	if (logo == NULL) {
		return ;
	}
	struct __dev *dev = NULL;
	os_mutex_pend(&__this->mutex, 0);
	dev = dev_manager_check_by_logo(logo);
	if (dev) {
		dev->active_stamp = __dev_manager_get_time_stamp();//timer_get_ms();
	}
	os_mutex_post(&__this->mutex);
}
//*----------------------------------------------------------------------------*/
/**@brief   获取设备节点的逻辑盘符
   @param
   			dev：设备节点
   @return  成功返回逻辑盘符，如：sd0/sd1/udisk0，失败返回NULL
   @note
*/
/*----------------------------------------------------------------------------*/
char *dev_manager_get_logo(struct __dev *dev)
{
	char *logo = NULL;
	os_mutex_pend(&__this->mutex, 0);
	if (dev_manager_check(dev) == NULL) {
		os_mutex_post(&__this->mutex);
		return NULL;
	}
	logo = dev->parm->logo;
	os_mutex_post(&__this->mutex);
	return logo;
}
//*----------------------------------------------------------------------------*/
/**@brief   获取物理设备节点的逻辑盘符(去掉_rec后缀)
   @param
   			dev：设备节点
   @return  成功返回逻辑盘符，如：sd0/sd1/udisk0，失败返回NULL
   @note    物理逻辑盘符是指非录音文件夹设备盘符(录音文件夹设备如：sd0_rec)
*/
/*----------------------------------------------------------------------------*/
char *dev_manager_get_phy_logo(struct __dev *dev)
{
    char *logo = dev_manager_get_logo(dev);
    char phy_dev_logo[16] = {0};
    if (logo) {
        char *str = strstr(logo, "_rec");
        if (str) {
            strncpy(phy_dev_logo, logo, strlen(logo) - strlen(str));
			struct __dev *phy_dev = dev_manager_find_spec(phy_dev_logo, 0);
			return dev_manager_get_logo(phy_dev);
        }
	}
	return logo;
}
//*----------------------------------------------------------------------------*/
/**@brief   获取录音文件夹设备节点的逻辑盘符(追加_rec后缀)
   @param
   			dev：设备节点
   @return  成功返回逻辑盘符，如：sd0_rec/sd1_rec/udisk0_rec，失败返回NULL
   @note
*/
/*----------------------------------------------------------------------------*/
char *dev_manager_get_rec_logo(struct __dev *dev)
{
    char *logo = dev_manager_get_logo(dev);
    char rec_dev_logo[16] = {0};
    if (logo) {
        char *str = strstr(logo, "_rec");
		if (!str) {
			sprintf(rec_dev_logo, "%s%s", logo, "_rec");
			struct __dev *rec_dev = dev_manager_find_spec(rec_dev_logo, 0);
			return dev_manager_get_logo(rec_dev);
        }
	}
	return logo;
}
//*----------------------------------------------------------------------------*/
/**@brief   通过设备节点获取设备文件系统根目录
   @param
   			dev：设备节点
   @return  成功返回根目录，失败返回NULL
   @note
*/
/*----------------------------------------------------------------------------*/
char *dev_manager_get_root_path(struct __dev *dev)
{
	char *path = NULL;
	os_mutex_pend(&__this->mutex, 0);
	if (dev_manager_check(dev) == NULL) {
		os_mutex_post(&__this->mutex);
		return NULL;
	}
	path = dev->parm->root_path;
	os_mutex_post(&__this->mutex);
	return path;
}
//*----------------------------------------------------------------------------*/
/**@brief   通过逻辑盘符获取设备文件系统根目录
   @param
   			logo：逻辑盘符，如：sd0/sd1/udisk0
   @return  成功返回根目录，失败返回NULL
   @note
*/
/*----------------------------------------------------------------------------*/
char *dev_manager_get_root_path_by_logo(char *logo)
{
	char *path = NULL;
	os_mutex_pend(&__this->mutex, 0);
	struct __dev *dev = dev_manager_check_by_logo(logo);
	if (dev == NULL) {
		os_mutex_post(&__this->mutex);
		return NULL;
	}
	path = dev->parm->root_path;
	os_mutex_post(&__this->mutex);
	return path;
}

//*----------------------------------------------------------------------------*/
/**@brief   通过设备节点获取设备mount信息
   @param
            dev：设备节点
   @return  成功返回对应指针，失败返回NULL
   @note
*/
/*----------------------------------------------------------------------------*/
struct imount *dev_manager_get_mount_hdl(struct __dev *dev)
{
    if (dev == NULL) {
        return NULL;
    }else{
        return dev->fmnt;
    }
}
//*----------------------------------------------------------------------------*/
/**@brief   通过逻辑盘符判断设备是否在线
   @param
   			logo：逻辑盘符，如：sd0/sd1/udisk0
			valid：
				1：检查有效可播放设备
				0：检查所有设备
   @return  1：在线 0：不在线
   @note
*/
/*----------------------------------------------------------------------------*/
int dev_manager_online_check_by_logo(char *logo, u8 valid)
{
	struct __dev *dev = dev_manager_find_spec(logo, valid);
	return (dev ? 1 : 0);
}
//*----------------------------------------------------------------------------*/
/**@brief   通过设备节点判断设备是否在线
   @param
   			dev：设备节点
			valid：
				1：检查有效可播放设备
				0：检查所有设备
   @return  1：在线 0：不在线
   @note
*/
/*----------------------------------------------------------------------------*/
int dev_manager_online_check(struct __dev *dev, u8 valid)
{
	if (dev_manager_check(dev) == NULL) {
		return 0;
	}else{
		if(valid){
			return (dev->valid ? 1 : 0);
		}else{
			return 1;
		}
	}
}

//*----------------------------------------------------------------------------*/
/**@brief   通过逻辑盘符判断设备是否在设备链表中
   @param
   			logo：逻辑盘符，如：sd0/sd1/udisk0
   @return  1：在设备链表中， 0：不在设备链表中
   @note	该接口会检查所有在设备链表中的设备，忽略mount，valid等状态
*/
/*----------------------------------------------------------------------------*/
struct __dev *dev_manager_list_check_by_logo(char *logo)
{
	if (logo == NULL) {
		return 0;
	}
	struct __dev *dev;
	os_mutex_pend(&__this->mutex, 0);
	list_for_each_entry(dev, &__this->list, entry) {
		if (!strcmp(dev->parm->logo, logo)) {
			os_mutex_post(&__this->mutex);
			return dev;
		}
	}
	os_mutex_post(&__this->mutex);
	return NULL;
}

//*----------------------------------------------------------------------------*/
/**@brief   检查链表中没有挂载的设备并重新挂载
   @param
   			logo：逻辑盘符，如：sd0/sd1/udisk0
   @return  1：在设备链表中， 0：不在设备链表中
   @note	该接口会检查所有在设备链表中的设备，忽略mount，valid等状态
*/
/*----------------------------------------------------------------------------*/
void dev_manager_list_check_mount(void)
{
	struct __dev *dev;
	os_mutex_pend(&__this->mutex, 0);
	list_for_each_entry(dev, &__this->list, entry) {

        if(!strcmp(dev->parm->logo,"virfat_flash")){
            continue;
        }

        if(dev->fmnt){
            unmount(dev->parm->storage_path);
            dev->fmnt = NULL;
        }

		if(dev->fmnt == NULL){
			struct __dev_reg *p = dev->parm;
			dev->fmnt = mount(p->name, p->storage_path, p->fs_type, 3, NULL);
			dev->valid = (dev->fmnt ? 1 : 0);
		}
	}
	os_mutex_post(&__this->mutex);
}

//*----------------------------------------------------------------------------*/
/**@brief   设备挂载
   @param
   			logo：逻辑盘符，如：sd0/sd1/udisk0
   @return  0：成功， -1：失败
   @note	需要主动mount设备可以调用改接口
*/
/*----------------------------------------------------------------------------*/
static int __dev_manager_mount(char *logo)
{
	int ret = 0;
	os_mutex_pend(&__this->mutex, 0);
	struct __dev *dev = dev_manager_list_check_by_logo(logo);
	if (dev == NULL) {
		os_mutex_post(&__this->mutex);
		return -1;
	}
	if(dev->fmnt == NULL){
		struct __dev_reg *p = dev->parm;
		dev->fmnt = mount(p->name, p->storage_path, p->fs_type, 3, NULL);
		dev->valid = (dev->fmnt ? 1 : 0);
	}
	ret = (dev->valid ? 0 : -1);
	os_mutex_post(&__this->mutex);
	return ret;
}
//*----------------------------------------------------------------------------*/
/**@brief   设备卸载
   @param
   			logo：逻辑盘符，如：sd0/sd1/udisk0
   @return  0：成功， -1：失败
   @note	需要主动unmount设备可以调用改接口
*/
/*----------------------------------------------------------------------------*/
static int __dev_manager_unmount(char *logo)
{
	os_mutex_pend(&__this->mutex, 0);
	struct __dev *dev = dev_manager_check_by_logo(logo);
	if (dev == NULL) {
		os_mutex_post(&__this->mutex);
		return -1;
	}
	if(dev->fmnt){
		unmount(dev->parm->storage_path);
		dev->fmnt = NULL;
	}
	dev->valid = 0;
	os_mutex_post(&__this->mutex);
	return 0;
}

//*----------------------------------------------------------------------------*/
/**@brief   设备挂载
   @param
   			logo：逻辑盘符，如：sd0/sd1/udisk0
   @return  0：成功， -1：失败
   @note	慎用
*/
/*----------------------------------------------------------------------------*/
int dev_manager_mount(char *logo)
{
	int ret = __dev_manager_mount(logo);
	if(ret == 0){
#if TCFG_RECORD_FOLDER_DEV_ENABLE
		char rec_dev_logo[16] = {0};
		sprintf(rec_dev_logo, "%s%s", logo, "_rec");
		ret = __dev_manager_mount(rec_dev_logo);
		if(ret){
			__dev_manager_unmount(logo);
		}
#endif
	}
	return ret;
}

//*----------------------------------------------------------------------------*/
/**@brief   设备卸载
   @param
   			logo：逻辑盘符，如：sd0/sd1/udisk0
   @return  0：成功， -1：失败
   @note	慎用
*/
/*----------------------------------------------------------------------------*/
int dev_manager_unmount(char *logo)
{
	int err = 0;
#if TCFG_RECORD_FOLDER_DEV_ENABLE
	char rec_dev_logo[16] = {0};
	sprintf(rec_dev_logo, "%s%s", logo, "_rec");
	__dev_manager_unmount(rec_dev_logo);
#endif
	err = __dev_manager_unmount(logo);
	return err;
}


//*----------------------------------------------------------------------------*/
/**@brief   设备检测线程处理
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
extern void hidden_file(u8 flag);
static void dev_manager_task(void *p)
{
	int res = 0;
	int msg[8] = {0};
	///过滤隐藏 和 .开头名名的字文件
	hidden_file(1);
	///设备初始化，
	devices_init();

#if TCFG_SD_ALWAY_ONLINE_ENABLE
#if (defined(TCFG_SD0_ENABLE) && (TCFG_SD0_ENABLE))
	extern void force_set_sd_online(char *sdx);
	force_set_sd_online("sd0");
	int err = dev_manager_add("sd0");
#elif (defined(TCFG_SD1_ENABLE) && (TCFG_SD1_ENABLE))
	extern void force_set_sd_online(char *sdx);
	force_set_sd_online("sd1");
	int err = dev_manager_add("sd1");
#endif
	if (err != 0) {
		printf("sd add fail\n");
	} else {
#if (TCFG_SD_ALWAY_ONLINE_ENABLE && (TCFG_SD0_ENABLE || TCFG_SD1_ENABLE))
        extern void sdx_dev_detect_timer_del();
        sdx_dev_detect_timer_del();
#endif
    }
#endif

#if SDFILE_STORAGE && TCFG_CODE_FLASH_ENABLE
	dev_manager_add(SDFILE_DEV);
#endif

#if TCFG_NOR_REC
	dev_manager_add("rec_nor");
#endif

#if TCFG_NOR_FAT
    dev_manager_add("fat_nor");
    dev_manager_set_valid_by_logo("fat_nor", 0);///将设备设置为无效设备
#endif

#if TCFG_NOR_FS
	dev_manager_add("res_nor");
#endif

#if TCFG_VIR_UDISK_ENABLE
	dev_manager_add("vir_udisk0");
#endif

#if TCFG_VIRFAT_FLASH_ENABLE
	dev_manager_add("virfat_flash");
    dev_manager_set_valid_by_logo("virfat_flash", 0);///将设备设置为无效设备
#endif

#if TCFG_NANDFLASH_DEV_ENABLE
	dev_manager_add("nandflash_ftl");
    dev_manager_set_valid_by_logo("nandflash_ftl", 0);///将设备设置为无效设备
#endif

#if FLASH_INSIDE_REC_ENABLE
    set_rec_capacity(512*1024);//需要先设置容量,注意要小于Ini文件设置大小.
    _sdfile_rec_init();
	dev_manager_add("rec_sdfile");
#endif

	os_sem_post(&__this->sem);

	while (1) {
		res = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
		switch (res) {
			case OS_TASKQ:
				switch (msg[0]) {
					case Q_EVENT:
						break;
					case Q_MSG:
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
	}
}

void dev_manager_var_init()
{
	memset(__this, 0, sizeof(struct __dev_manager));
	INIT_LIST_HEAD(&__this->list);
	os_mutex_create(&__this->mutex);
}

#endif/*TCFG_DEV_MANAGER_ENABLE*/
//*----------------------------------------------------------------------------*/
/**@brief   设备管理器初始化
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void dev_manager_init(void)
{
#if (TCFG_DEV_MANAGER_ENABLE)
    dev_manager_var_init();

	os_sem_create(&__this->sem, 0);

	int err = task_create(dev_manager_task, NULL, DEV_MANAGER_TASK_NAME);
	if (err != OS_NO_ERR) {
		ASSERT(0, "task_create fail!!! %x\n", err);
	}
	os_sem_pend(&__this->sem, 0);
#else
	devices_init();
#endif
}

