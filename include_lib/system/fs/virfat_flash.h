#ifndef VIRTUAL_DISK_H
#define VIRTUAL_DISK_H

#include "system/includes.h"


#define VIR_FILE_TYPE_MP3   1
#define VIR_FILE_TYPE_WAV   2

extern const struct device_operations virfat_flash_dev_ops;
extern const struct device_operations private_sfc_dev_ops;

typedef enum {
    FS_FAT12    =   0x01,
    FS_FAT16    =   0x02,
    FS_FAT32    =   0x04,
    FS_EXFAT    =   0x14,
} FS_TYPE;

#define JLFAT_DIRINFO_ENABLE 0
#if JLFAT_DIRINFO_ENABLE
typedef struct jlfat_dirinfo {
    u8 name[12]; //文件名字
    int fsize; // 文件大小
    u16 fix_date; //修改日期
    u16 fix_time; //修改时间
    u8 attr; //文件属性
    int sclust; //文件起始簇
} JLFAT_DIRINFO;
#endif

typedef struct _virfat_flash_t {
    u32 fatbase;
    // u32 fatbase2;
    u16 fatsize;
    u32 dirbase;
    u32 database;
    u32 clustSize;
    u32 max_clust;
    u32 capacity;
    u8  csize;
    FS_TYPE fs_type;
    char re_devname[24];
    u32 flash_fatfs_addr;
    u32 fatfs_len;
} virfat_flash_t;

typedef struct _dir_entry {
    u8 name[12];
    u32 fileSize;
} dir_entry_t;


#if 0
extern void virfat_flash_rand_file_size();
extern void virtual_disk_reduce_file_size(tu8 file_num);
extern void virtual_disk_remove_files(tu8 file_type);
extern void virtual_disk_reduce_file_size(tu8 file_num);
extern u8 virfat_flash_init(void *buf_512, void (*fun_read_file)(u32, u32, u8 *, u32));
#endif

#define MIN_FAT16	4086	/* Minimum number of clusters for FAT16 */
#define	MIN_FAT32	65526	/* Minimum number of clusters for FAT32 */

#define	AM_RDO	0x01	/* Read only */
#define	AM_HID	0x02	/* Hidden */
#define	AM_SYS	0x04	/* System */
#define	AM_VOL	0x08	/* Volume label */
#define AM_LFN	0x0F	/* LFN entry */
#define AM_DIR	0x10	/* Directory */
#define AM_ARC	0x20	/* Archive */
#define AM_FCH	0x80    /* exFAT下,文件簇连续标志 */
#define AM_FRG	0x40    /* exFAT下,文件簇BITMAP 有碎片 */
#define AM_LFN0_START	0x1
#define AM_LFN0_LEN	    10
#define AM_LFN1_START	0xE
#define AM_LFN1_LEN	    12
#define AM_LFN2_START	0x1C
#define AM_LFN2_LEN	    4
#define AM_LFN_LEN	    26

#define AM_EX_DIR_STRETCHED 0x08

#define BS_jmpBoot			0
#define BS_OEMName			3
#define BPB_BytsPerSec_l	11
#define BPB_BytsPerSec_h	12
#define BPB_SecPerClus		13
#define BPB_RsvdSecCnt		14
#define BPB_NumFATs			16
#define BPB_RootEntCnt		17
#define BPB_TotSec16		19
#define BPB_Media			21
#define BPB_FATSz16			22
#define BPB_SecPerTrk		24
#define BPB_NumHeads		26
#define BPB_HiddSec			28
#define BPB_TotSec32		32
#define BS_55AA				510

#define BS_DrvNum			36
#define BS_BootSig			38
#define BS_VolID			39
#define BS_VolLab			43
#define BS_FilSysType		54

#define BPB_FATSz32			36
#define BPB_ExtFlags		40
#define BPB_FSVer			42
#define BPB_RootClus		44
#define BPB_FSInfo			48
#define BPB_BkBootSec		50
#define BS_DrvNum32			64
#define BS_BootSig32		66
#define BS_VolID32			67
#define BS_VolLab32			71
#define BS_FilSysType32		82
#define BS_FileSysTypeexFAT				5
#define BPB_FatOffset					80
#define BPB_FatLength					84
#define BPB_ClusterHeapOffset			88
#define BPB_ClusterCount				92
#define BPB_FirstClusterOfRootDirectory	96
#define BPB_VolumeFlags					106
#define BPB_BytesPerSectorShift			108
#define BPB_SectorsPerClusterShift		109
#define BPB_NumberOfFats				110
#define MBR_Table			            446
#define	FSI_LeadSig			0	/* FSI: Leading signature (4) */
#define	FSI_StrucSig		484	/* FSI: Structure signature (4) */
#define	FSI_Free_Count		488	/* FSI: Number of free clusters (4) */
#define	FSI_Nxt_Free		492	/* FSI: Last allocated cluster (4) */

///for FAT12/FAT16/FAT32
#define	DIR_Name			0	/* Short file name (11) */
#define	DIR_Attr			11	/* Attribute (1) */
#define	DIR_NTres			12	/* NT flag (1) */
#define DIR_CrtTimeTenth	13	/* Created time sub-second (1) */
#define	DIR_CrtTime			14	/* Created time (2) */
#define	DIR_CrtDate			16	/* Created date (2) */
#define DIR_LstAccDate		18	/* Last accessed date (2) */
#define	DIR_FstClusHI		20	/* Higher 16-bit of first cluster (2) */
#define	DIR_WrtTime			22	/* Modified time (2) */
#define	DIR_WrtDate			24	/* Modified date (2) */
#define	DIR_FstClusLO		26
#define	DIR_FileSize		28
#define	LDIR_Attr			11	/* LFN attribute (1) */
#define	LDIR_Type			12	/* LFN type (1) */
#define	LDIR_Chksum			13	/* Sum of corresponding SFN entry */
////for exFAT
#define DIR_FileChainFlags	1
#define DIR_NameLen		    3
#define DIR_AttrexFAT		4
#define DIR_FileSizeexFAT   8
#define DIR_FstClustexFAT	20
#define DIR_FileSize2exFAT  24



#endif

