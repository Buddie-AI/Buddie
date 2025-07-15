#include "system/app_core.h"
#include "system/includes.h"
#include "app_config.h"
#include "music/music_id3.h"
#include "system/fs/fs.h"

#if ((TCFG_DEC_ID3_V1_ENABLE) || (TCFG_DEC_ID3_V2_ENABLE))

#define LOG_TAG_CONST       APP_MUSIC
#define LOG_TAG             "[APP_MUSIC_ID3]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define ID3_ANALYSIS_EN		1	// ID3解析

struct id3_v1_hdl {
    char header[3];		// TAG
    char title[30]; 	// 标题
    char artist[30];	// 作者
    char album[30];		// 专辑
    char year[4];		// 出品年代
    char comment[30];	// 备注
    char genre;			// 类型
};

struct id3_v2_hdl {
    char header[3];		// ID3
    char ver;			// 版本号
    char revision;		// 副版本号
    char flag;			// 标志
    u8   size[4];		// 标签大小，不包括标签头的10个字节
};

struct id3_v2_frame {
    char frameID[4];	// 帧标识
    u8   size[4]; 		// 帧大小，不包括帧头10个字节，不小于1
    char flags[2];		// 标志
};

u8 tpe1_data[512];
u32 tpe1_data_len;

#define ID3_SIZEOF_ALIN(var,al)     ((((var)+(al)-1)/(al))*(al))
#define ID3_V1_HEADER_SIZE			(128) // sizeof(struct id3_v1_hdl)
#define ID3_V2_HEADER_SIZE			(10)  // sizeof(struct id3_v2_hdl)
#define ID3_V2_HEADER_SIZE_MAX		(1024 * 2)

#define ID3_V2_FRAME_SIZE			(10)  // sizeof(struct id3_v2_frame)

static u8 id3_v1_match(u8 *buf)
{
    if (buf[0] == 'T' && buf[1] == 'A' && buf[2] == 'G') {
        return true;
    } else {
        return false;
    }
}

static u8 id3_v2_match(u8 *buf)
{
    //put_buf(buf, 16);
    if ((buf[0] == 'I')
        && (buf[1] == 'D')
        && (buf[2] == '3')
        && (buf[3] != 0xff)
        && (buf[4] != 0xff)
        && ((buf[6] & 0x80) == 0)
        && ((buf[7] & 0x80) == 0)
        && ((buf[8] & 0x80) == 0)
        && ((buf[9] & 0x80) == 0)) {
        return true;
    } else {
        return false;
    }
}

static u32 id3v2_get_tag_len(u8 *buf)
{
    u32 len = (((u32)buf[6] & 0x7f) << 21) +
              (((u32)buf[7] & 0x7f) << 14) +
              (((u32)buf[8] & 0x7f) << 7) +
              (buf[9] & 0x7f) +
              ID3_V2_HEADER_SIZE;

    if (buf[5] & 0x10) {
        len += ID3_V2_HEADER_SIZE;
    }

    log_info("----id3v2_len%d---\n", len);
    return len;
}

#if ID3_ANALYSIS_EN

static void id3_v1_analysis(MP3_ID3_OBJ *obj)
{
    struct id3_v1_hdl *hdl = (struct id3_v1_hdl *)obj->id3_buf;
    /* log_info("header:%s \n", hdl->header); */
    log_info("title:%s \n", hdl->title);
    log_info("artist:%s \n", hdl->artist);
    log_info("album:%s \n", hdl->album);
    log_info("year:%s \n", hdl->year);
    log_info("comment:%s \n", hdl->comment);
    log_info("genre:%d \n", hdl->genre);
}

static const u8 ID3_V2_FRAME_ID[][4] = {
    "TIT2",	// 标题
    "TPE1",	// 作者
    "TALB",	// 专辑
    "TYER",	// 年代
    "TCON",	// 类型
    "COMM",	// 备注
};

static void id3_v2_analysis(void *file, void *buf)
{
#define FRAME_DAT_MAX		512
    u8 *frame_dat = malloc(FRAME_DAT_MAX);
    u8  frame_hdl[ID3_V2_FRAME_SIZE];
    u32 frame_len;
    struct id3_v2_hdl *hdl = (struct id3_v2_hdl *)buf;
    /* log_info("header:%s \n", hdl->header); */

    if (!frame_dat) {
        return ;
    }

    u32 addr = ID3_V2_HEADER_SIZE;
    u32 addr_end = id3v2_get_tag_len(buf) + ID3_V2_HEADER_SIZE;
    log_info("frame total len:0x%x ", addr_end);
    memset(tpe1_data, 0, sizeof(tpe1_data));
    while (addr < addr_end) {
        fseek(file, addr, SEEK_SET);
        fread(file, frame_hdl, ID3_V2_FRAME_SIZE);

        int idx;
        struct id3_v2_frame *frame = (struct id3_v2_frame *)frame_hdl;
        frame_len = (((u32)frame->size[0]) << 24) + (((u32)frame->size[1]) << 16) + (((u32)frame->size[2]) << 8) + (u32)frame->size[3];

        log_info_hexdump(frame, ID3_V2_FRAME_SIZE);
        log_info("frame ID:%s ", frame->frameID);
        log_info("frame len:0x%x ", frame_len);
        log_info("addr :0x%x ", addr);

        if (!frame_len) {
            break;
        }
        if (frame_len <= FRAME_DAT_MAX) {
            memset(frame_dat, 0, FRAME_DAT_MAX);
            fread(file, frame_dat, frame_len);
        }

        for (idx = 0; idx < sizeof(ID3_V2_FRAME_ID) / 4; idx++) {
            if (!memcmp(frame->frameID, ID3_V2_FRAME_ID[idx], 4)) {
                log_info("find frameID \n");
                break;
            }
        }
        if (frame_len <= FRAME_DAT_MAX) {
            log_info("frame dat:%s, \n", frame_dat);
            log_info_hexdump(frame_dat, frame_len);
        }

        if (!memcmp(frame->frameID, "TPE1", 4)) {
            frame_dat++;
            sprintf(tpe1_data, "%s", frame_dat);
            tpe1_data_len = frame_len;
            log_info_hexdump(tpe1_data, tpe1_data_len - 1);
            frame_dat--;
        }
        addr += ID3_V2_FRAME_SIZE + frame_len;
    }

    free(frame_dat);
}
#endif

void id3_obj_post(MP3_ID3_OBJ **obj)
{
    if (obj == NULL || (*obj) == NULL) {
        return ;
    }
    free(*obj);
    *obj = NULL;
}

MP3_ID3_OBJ *id3_v1_obj_get(void *file)
{
    if (file == NULL) {
        return NULL;
    }

    MP3_ID3_OBJ *obj = NULL;
    u8 *need_buf = NULL;
    u32 need_buf_size;
    u32 cur_fptr = fpos(file);
    u32 file_len = flen(file);

    need_buf_size = ID3_SIZEOF_ALIN(sizeof(MP3_ID3_OBJ), 4)
                    + ID3_SIZEOF_ALIN(ID3_V1_HEADER_SIZE, 4);
    need_buf = (u8 *)zalloc(need_buf_size);
    if (need_buf == NULL) {
        log_error("malloc err !!\n");
        return NULL;
    }

    obj = (MP3_ID3_OBJ *)need_buf;
    obj->id3_buf = (u8 *)(need_buf + ID3_SIZEOF_ALIN(sizeof(MP3_ID3_OBJ), 4));
    obj->id3_len = ID3_V1_HEADER_SIZE;

    fseek(file, (file_len - ID3_V1_HEADER_SIZE), SEEK_SET);
    fread(file, obj->id3_buf, ID3_V1_HEADER_SIZE);

    if (id3_v1_match(obj->id3_buf) == true) {
        log_info("find id3 v1 !!\n");
        put_buf(obj->id3_buf, obj->id3_len);
    } else {
        goto __id3_v1_err;
    }

#if ID3_ANALYSIS_EN
    id3_v1_analysis(obj);
#else

    fseek(file, cur_fptr, SEEK_SET);

    return obj;
#endif

__id3_v1_err:
    fseek(file, cur_fptr, SEEK_SET);

    if (need_buf) {
        free(need_buf);
    }
    return NULL;
}


MP3_ID3_OBJ *id3_v2_obj_get(void *file)
{
    if (file == NULL) {
        return NULL;
    }
    MP3_ID3_OBJ *obj = NULL;
    u8 *need_buf = NULL;
    u32 need_buf_size;
    u8 tmp_buf[ID3_V2_HEADER_SIZE];
    u32 cur_fptr = fpos(file);

    fseek(file, 0, SEEK_SET);
    fread(file, tmp_buf, ID3_V2_HEADER_SIZE);

    if (id3_v2_match(tmp_buf) == true) {
        log_info("find id3 v2 !!\n");

#if ID3_ANALYSIS_EN
        id3_v2_analysis(file, (MP3_ID3_OBJ *)tmp_buf);
#else

        u32 tag_len = id3v2_get_tag_len(tmp_buf);

        if (tag_len > (ID3_V2_HEADER_SIZE_MAX)) {
            log_error("id3 v2 info is too big!!\n", tag_len);
            goto __id3_v2_err;
        }

        need_buf_size = ID3_SIZEOF_ALIN(sizeof(MP3_ID3_OBJ), 4)
                        + ID3_SIZEOF_ALIN(tag_len, 4);

        need_buf = (u8 *)zalloc(need_buf_size);
        if (need_buf == NULL) {
            log_error("no space for id3 v2  %d\n", tag_len);
            goto __id3_v2_err;
        }

        obj = (MP3_ID3_OBJ *)need_buf;
        obj->id3_buf = (u8 *)(need_buf + ID3_SIZEOF_ALIN(sizeof(MP3_ID3_OBJ), 4));
        obj->id3_len = tag_len;

        fseek(file, 0, SEEK_SET);
        fread(file, obj->id3_buf, obj->id3_len);

        log_info("id3_v2 size = %d\n", obj->id3_len);
        put_buf(obj->id3_buf, obj->id3_len);
#endif

    } else {
        goto __id3_v2_err;
    }
    fseek(file, cur_fptr, SEEK_SET);

    return obj;

__id3_v2_err:
    fseek(file, cur_fptr, SEEK_SET);

    if (need_buf) {
        free(need_buf);
    }
    return NULL;
}

#endif

