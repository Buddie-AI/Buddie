#ifndef __MUSIC_PLAYER_H__
#define __MUSIC_PLAYER_H__
#include "system/app_core.h"
#include "system/includes.h"
#include "server/server_core.h"
#include "media/audio_decoder.h"
#include "dev_manager/dev_manager.h"
#include "file_operate/file_manager.h"
#include "audio_dec/audio_dec_file.h"

///解码错误码表
enum {
    MUSIC_PLAYER_ERR_NULL = 0x0,		///没有错误， 不需要做任何
    MUSIC_PLAYER_SUCC	  = 0x1,		///播放成功， 可以做相关显示、断点记忆等处理
    MUSIC_PLAYER_ERR_PARM,	    		///参数错误
    MUSIC_PLAYER_ERR_POINT,	    		///参数指针错误
    MUSIC_PLAYER_ERR_NO_RAM,			///没有ram空间错误
    MUSIC_PLAYER_ERR_DECODE_FAIL,		///解码器启动失败错误
    MUSIC_PLAYER_ERR_DEV_NOFOUND,		///没有找到指定设备
    MUSIC_PLAYER_ERR_DEV_OFFLINE,		///设备不在线错误
    MUSIC_PLAYER_ERR_DEV_READ,			///设备读错误
    MUSIC_PLAYER_ERR_FSCAN,				///设备扫描失败
    MUSIC_PLAYER_ERR_FILE_NOFOUND,		///没有找到文件
    MUSIC_PLAYER_ERR_RECORD_DEV,		///录音文件夹操作错误
    MUSIC_PLAYER_ERR_FILE_READ,			///文件读错误
};

///断点信息结构体
struct __breakpoint {
    ///文件信息
    u32 fsize;
    u32 sclust;
    ///解码信息
    struct audio_dec_breakpoint dbp;
};

///回调接口结构体
struct __player_cb {
    ///解码成功回调
    void (*start)(void *priv, int parm);
    ///解码结束回调
    void (*end)(void *priv, int parm);
    ///解码错误回调
    void (*err)(void *priv, int parm);
};

///参数设置结构体
struct __player_parm {
    const struct __player_cb *cb;
    ///其他扩展
    const struct __scan_callback *scan_cb;//扫盘打断回调
};

//music_player创建接口
bool music_player_creat(void *priv, struct __player_parm *parm);
//music_player释放接口
void music_player_destroy(void);
//music_player播放结束处理
int music_player_end_deal(int parm);
//music_player播放器解码错误处理
int music_player_decode_err_deal(int event);
//music_player解码器启动接口
int music_player_decode_start(FILE *file, struct audio_dec_breakpoint *dbp);
//music_player获取当前播放设备断点信息
bool music_player_get_playing_breakpoint(struct __breakpoint *bp, u8 flag);
//music_player获取当前播放设备文件总数
u16 music_player_get_file_total(void);
//music_player获取当前播放文件序号
u16 music_player_get_file_cur(void);
//music_player获取当前播放文件所在文件夹的文件总数
u16 music_player_get_fileindir_number(void);
//music_player获取当前播放文件所在文件夹
u16 music_player_get_dir_cur(void);
//music_player获取文件夹总数
u16 music_player_get_dir_total(void);
//music_player获取当前文件句柄
FILE *music_player_get_file_hdl(void);
//music_player获取当前文件簇号
u32 music_player_get_file_sclust(void);
//music_player获取当前文件长度
u32 music_player_get_file_len(void);
//music_player获取现在的使用设备
char *music_player_get_dev_cur(void);
//music_player获取当前播放对应的music设备
char *music_player_get_cur_music_dev(void);
//music_player获取当前播放设备下一个设备
char *music_player_get_dev_next(u8 auto_next);
//music_player获取当前播放设备上一个设备
char *music_player_get_dev_prev(void);
//music_player获取当前播放状态
int music_player_get_play_status(void);
//music_player获取当前播放歌曲时间
int music_player_get_dec_cur_time(void);
//music_player获取当前播放歌曲总时间
int music_player_get_dec_total_time(void);
//music_player获取当前播放循环模式
u8 music_player_get_repeat_mode(void);
//music_player获取当前录音区分播放状态
bool music_player_get_record_play_status(void);
//music_player从设备列表里面往前或往后找设备，并且过滤掉指定字符串的设备
char *music_player_get_dev_flit(char *flit, u8 direct);
//music_player播放/暂停
int music_player_pp(void);
//music_player快进
void music_player_ff(int step);
//music_player快退
void music_player_fr(int step);
//music_player设置播放循环模式
int  music_player_set_repeat_mode(u8 mode);
//music_player切换循环模式
int  music_player_change_repeat_mode(void);
//music_player解码停止
void music_player_stop(u8 fsn_release);
//music_player删除当前播放文件,并播放下一曲
int music_player_delete_playing_file(void);
//music_player播放上一曲
int music_player_play_prev(void);
//music_player播放下一曲
int music_player_play_next(void);
//music_player播放第一曲
int music_player_play_first_file(char *logo);
//music_player播放最后一曲
int music_player_play_last_file(char *logo);
//music_player自动播放下一曲
int music_player_play_auto_next(void);
//music_player上一个文件夹
int music_player_play_folder_prev(void);
//music_player下一个文件夹
int music_player_play_folder_next(void);
//music_player上一个设备
int music_player_play_devcie_prev(struct __breakpoint *bp);
//music_player下一个设备
int music_player_play_devcie_next(struct __breakpoint *bp);
//music_player断点播放指定设备
int music_player_play_by_breakpoint(char *logo, struct __breakpoint *pb);
//music_player序号播放指定设备
int music_player_play_by_number(char *logo, u32 number);
//music_player簇号播放指定设备
int music_player_play_by_sclust(char *logo, u32 sclust);
//music_player路径播放指定设备
int music_player_play_by_path(char *logo, const char *path);
//music_player录音区分切换播放
int music_player_play_record_folder(char *logo, struct __breakpoint *bp);
//music_player扫盘
int music_player_scan_disk();
//获取当前设备的物理设备
const char *music_player_get_phy_dev(int *len);
//歌词分析
int music_player_lrc_analy_start();

//music_player获取当前vfscan句柄
struct vfscan *music_player_get_vfscan_hdl(void);
#endif//__MUSIC_PLAYER_H__

