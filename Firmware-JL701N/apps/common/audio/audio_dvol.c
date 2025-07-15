/*
 ****************************************************************
 *							AUDIO DIGITAL VOLUME
 * File  : audio_dvol.c
 * By    :
 * Notes : 数字音量模块，支持多通道音量控制
 ****************************************************************
 */

#include "audio_dvol.h"

#if 0
#define dvol_log	y_printf
#else
#define dvol_log(...)
#endif

#define DIGITAL_FADE_EN 	1
#define DIGITAL_FADE_STEP 	4

#define BG_DVOL_MAX			14
#define BG_DVOL_MID			10
#define BG_DVOL_MIN		    6
#define BG_DVOL_MAX_FADE	13	/*>= BG_DVOL_MAX:自动淡出BG_DVOL_MAX_FADE*/
#define BG_DVOL_MID_FADE	9	/*>= BG_DVOL_MID:自动淡出BG_DVOL_MID_FADE*/
#define BG_DVOL_MIN_FADE	5	/*>= BG_DVOL_MIN:自动淡出BG_DVOL_MIN_FADE*/

#define ASM_ENABLE			1
#define  L_sat(b,a)       __asm__ volatile("%0=sat16(%1)(s)":"=&r"(b) : "r"(a));
#define  L_sat32(b,a,n)       __asm__ volatile("%0=%1>>%2(s)":"=&r"(b) : "r"(a),"r"(n));

typedef struct {
    u8 bg_dvol_fade_out;
    u8 start;
    OS_MUTEX mutex;
    struct list_head dvol_head;
} dvol_t;
static dvol_t dvol_attr;

/*
 *数字音量级数 DEFAULT_DIGITAL_VOL_MAX
 *数组长度 DEFAULT_DIGITAL_VOL_MAX + 1
 */
#define DEFAULT_DIGITAL_VOL_MAX     (31)
const u16 default_dig_vol_table[DEFAULT_DIGITAL_VOL_MAX + 1] = {
    0	, //0
    93	, //1
    111	, //2
    132	, //3
    158	, //4
    189	, //5
    226	, //6
    270	, //7
    323	, //8
    386	, //9
    462	, //10
    552	, //11
    660	, //12
    789	, //13
    943	, //14
    1127, //15
    1347, //16
    1610, //17
    1925, //18
    2301, //19
    2751, //20
    3288, //21
    3930, //22
    4698, //23
    5616, //24
    6713, //25
    8025, //26
    9592, //27
    11466,//28
    15200,//29
    16000,//30
    16384 //31
};

static u16 digital_vol_max = DEFAULT_DIGITAL_VOL_MAX;
static u16 *dig_vol_table = default_dig_vol_table;

/*
*********************************************************************
*                  Audio Digital Volume Init
* Description: 数字音量模块初始化
* Arguments  : None.
* Return	 : 0 成功 其他 失败
* Note(s)    : None.
*********************************************************************
*/
int audio_digital_vol_init(u16 *vol_table, u16 vol_max)
{
    memset(&dvol_attr, 0, sizeof(dvol_attr));
    INIT_LIST_HEAD(&dvol_attr.dvol_head);
    os_mutex_create(&dvol_attr.mutex);

    if (vol_table != NULL) {
        dig_vol_table = vol_table;
        digital_vol_max = vol_max;
    }

    return 0;
}

/*背景音乐淡出使能*/
void audio_digital_vol_bg_fade(u8 fade_out)
{
    dvol_log("audio_digital_vol_bg_fade:%d", fade_out);
    dvol_attr.bg_dvol_fade_out = fade_out;
}

/*
*********************************************************************
*                  Audio Digital Volume Open
* Description: 数字音量模块打开
* Arguments  : dvol_idx 	数字音量通道索引，详见audio_dvol.h宏定义
*			   vol			当前数字音量等级
*			   vol_max		最大数字音量等级
*			   fade_step	淡入淡出步进
*			   vol_limit	数字音量限制，即可以给vol_max等分的最大音量
* Return	 : 数字音量通道句柄
* Note(s)    : fade_step一般不超过两级数字音量的最小差值
*				(1)通话如果用数字音量，一般步进小一点，音量调节的时候不会有杂音
*				(2)淡出的时候可以快一点，尽快淡出到0
*********************************************************************
*/
dvol_handle *audio_digital_vol_open(u8 dvol_idx, u8 vol, u8 vol_max, u16 fade_step, char vol_limit)
{
    dvol_log("dvol_open:%d-%d-%d-%d\n", vol, vol_max, fade_step, vol_limit);
    dvol_handle *dvol = NULL;
    dvol = zalloc(sizeof(dvol_handle));
    if (dvol) {
        u8 vol_level;
        dvol->fade 		= DIGITAL_FADE_EN;
        dvol->vol 		= (vol > vol_max) ? vol_max : vol;
        if (vol > vol_max) {
            printf("[warning]cur digital_vol(%d) > digital_vol_max(%d)!!", vol, vol_max);
        }
        dvol->vol_max 	= vol_max;
        if (vol_limit == -1) {
            dvol->vol_limit = digital_vol_max;
        } else {
            dvol->vol_limit = (vol_limit > digital_vol_max) ? digital_vol_max : vol_limit;
        }
        vol_level 		= dvol->vol * dvol->vol_limit / vol_max;
        dvol->vol_target = dig_vol_table[vol_level];
        dvol->vol_fade 	= dvol->vol_target;
        dvol->fade_step 	= fade_step;
        dvol->toggle 	= 1;
        dvol->idx = dvol_idx;
        local_irq_disable();
        list_add(&dvol->entry, &dvol_attr.dvol_head);
#if BG_DVOL_FADE_ENABLE
        dvol->vol_bk = -1;
        if (dvol_attr.bg_dvol_fade_out) {
            dvol_handle *hdl;
            list_for_each_entry(hdl, &dvol_attr.dvol_head, entry) {
                if ((hdl != dvol) && (hdl->idx)) {
                    hdl->vol_bk = hdl->vol;
                    if (hdl->vol >= BG_DVOL_MAX) {
                        hdl->vol -= BG_DVOL_MAX_FADE;
                    } else if (hdl->vol >= BG_DVOL_MID) {
                        hdl->vol -= BG_DVOL_MID_FADE;
                    } else if (hdl->vol >= BG_DVOL_MIN) {
                        hdl->vol -= BG_DVOL_MIN_FADE;
                    } else {
                        hdl->vol_bk = -1;
                        continue;
                    }
                    u8 vol_level = hdl->vol * dvol->vol_limit / hdl->vol_max;
                    hdl->vol_target = dig_vol_table[vol_level];
                    //y_printf("bg_dvol fade_out:%x,vol_bk:%d,vol_set:%d,tartget:%d",hdl,hdl->vol_bk,hdl->vol,hdl->vol_target);
                }
            }
        }
#endif/*BG_DVOL_FADE_ENABLE*/
        local_irq_enable();
        dvol_log("dvol_open[%x]:%x-%d-%d-%d\n", dvol_idx, dvol, dvol->vol, dvol->vol_max, fade_step);
    }
    return dvol;
}

/*
*********************************************************************
*                  Audio Digital Volume Close
* Description: 数字音量模块关闭
* Arguments  : dvol_idx 	数字音量通道索引，详见audio_dvol.h宏定义
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_digital_vol_close(u8 dvol_idx)
{
    dvol_log("dvol_close[%x]\n", dvol_idx);
    dvol_handle *dvol = NULL;
    u8 dvol_valid = 0;
    list_for_each_entry(dvol, &dvol_attr.dvol_head, entry) {
        if (dvol && (dvol->idx == dvol_idx)) {
            dvol_log("dvol_close[%x]:%x", dvol_idx, dvol);
            dvol_valid = 1;
            break;
        }
    }
    if (dvol_valid == 0) {
        return;
    }

    if (dvol) {
        local_irq_disable();
#if BG_DVOL_FADE_ENABLE
        dvol_handle *hdl;
        list_for_each_entry(hdl, &dvol_attr.dvol_head, entry) {
            if ((hdl != dvol) && (hdl->vol_bk >= 0)) {
                //y_printf("bg_dvol fade_in:%x,%d",hdl,hdl->vol_bk);
                hdl->vol =  hdl->vol_bk;
                u8 vol_level = hdl->vol_bk * dvol->vol_limit / hdl->vol_max;
                hdl->vol_target = dig_vol_table[vol_level];
                hdl->vol_bk = -1;
            }
        }
#endif
        list_del(&dvol->entry);
        free(dvol);
        dvol = NULL;
        local_irq_enable();
    }
}

/*
*********************************************************************
*                  Audio Digital Volume Get
* Description: 数字音量获取
* Arguments  : dvol_idx 	数字音量通道索引，详见audio_dvol.h宏定义
* Return	 : 对应的数字音量通道的当前音量等级.
* Note(s)    : None.
*********************************************************************
*/
int audio_digital_vol_get(u8 dvol_idx)
{
    dvol_handle *dvol = NULL;
    u8 dvol_valid = 0;
    local_irq_disable();
    list_for_each_entry(dvol, &dvol_attr.dvol_head, entry) {
        if (dvol && (dvol->idx == dvol_idx)) {
            dvol_valid = 1;
            break;
        }
    }
    local_irq_enable();
    if (dvol && dvol_valid) {
        return dvol->vol;
    }
    return 0;
}

/*
*********************************************************************
*                  Audio Digital Volume Set
* Description: 数字音量设置
* Arguments  : dvol_idx 数字音量通道索引，详见audio_dvol.h宏定义
*			   vol		目标音量等级
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_digital_vol_set(u8 dvol_idx, u8 vol)
{
    dvol_log("audio_digital_vol set:%d,%d\n", dvol_idx, vol);
    dvol_handle *dvol = NULL;
    u8 dvol_valid = 0;
    local_irq_disable();
    list_for_each_entry(dvol, &dvol_attr.dvol_head, entry) {
        if (dvol && (dvol->idx == dvol_idx)) {
            dvol_log("dvol_set[%x]:%x", dvol_idx, dvol);
            dvol_valid = 1;
            break;
        }
    }
    local_irq_enable();
    if ((dvol == NULL) || (dvol_valid == 0)) {
        return;
    }
    if (dvol->toggle == 0) {
        return;
    }
    dvol->vol = (vol > dvol->vol_max) ? dvol->vol_max : vol;
#if BG_DVOL_FADE_ENABLE
    if (dvol->vol_bk != -1) {
        dvol->vol_bk = vol;
    }
#endif
    dvol->fade = DIGITAL_FADE_EN;
    u8 vol_level = dvol->vol * dvol->vol_limit / dvol->vol_max;
    dvol->vol_target = dig_vol_table[vol_level];
    dvol_log("digital_vol:%d-%d-%d-%d\n", vol, vol_level, dvol->vol_fade, dvol->vol_target);
}
/*********************************************************************
*                  Audio Digital Volume Set
* Description: 数字音量设置,设置数字音量的当前值，不用淡入淡出
* Arguments  : dvol_idx 数字音量通道索引，详见audio_dvol.h宏定义
*			   vol		目标音量等级
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/

void audio_digital_vol_set_no_fade(u8 dvol_idx, u8 vol)
{
    dvol_log("audio_digital_vol set:%d,%d\n", dvol_idx, vol);
    dvol_handle *dvol = NULL;
    u8 dvol_valid = 0;
    local_irq_disable();
    list_for_each_entry(dvol, &dvol_attr.dvol_head, entry) {
        if (dvol && (dvol->idx == dvol_idx)) {
            dvol_log("dvol_set[%x]:%x", dvol_idx, dvol);
            dvol_valid = 1;
            break;
        }
    }
    local_irq_enable();
    if ((dvol == NULL) || (dvol_valid == 0)) {
        return;
    }
    if (dvol->toggle == 0) {
        return;
    }
    dvol->vol = (vol > dvol->vol_max) ? dvol->vol_max : vol;
#if BG_DVOL_FADE_ENABLE
    if (dvol->vol_bk != -1) {
        dvol->vol_bk = vol;
    }
#endif
    dvol->fade = DIGITAL_FADE_EN;
    u8 vol_level = dvol->vol * dvol->vol_limit / dvol->vol_max;
    dvol->vol_fade = dig_vol_table[vol_level];
    dvol_log("digital_vol:%d-%d-%d-%d\n", vol, vol_level, dvol->vol_fade, dvol->vol_target);
}

void audio_digital_vol_reset_fade(u8 dvol_idx)
{
    dvol_handle *dvol = NULL;
    u8 dvol_valid = 0;
    local_irq_disable();
    list_for_each_entry(dvol, &dvol_attr.dvol_head, entry) {
        if (dvol && (dvol->idx == dvol_idx)) {
            dvol_valid = 1;
            break;
        }
    }
    local_irq_enable();
    if (dvol && dvol_valid) {
        dvol->vol_fade = 0;
    }
}

/*
*********************************************************************
*                  Audio Digital Volume Process
* Description: 数字音量运算核心
* Arguments  : dvol_idx 数字音量通道索引，详见audio_dvol.h宏定义
*			   data		待运算数据
*			   len		待运算数据长度
* Return	 : 0 成功 其他 失败
* Note(s)    : None.
*********************************************************************
*/
/*rounding处理*/
#define MUL32_PSHIFT(x,y,s) (((x) * (y) + (1 << ((s)-1))) >> (s))

int audio_digital_vol_run(u8 dvol_idx, void *data, u32 len)
{
    s32 valuetemp;
    s16 *buf;

    os_mutex_pend(&dvol_attr.mutex, 0);
    dvol_handle *dvol = NULL;
    u8 dvol_valid = 0;
    local_irq_disable();
    list_for_each_entry(dvol, &dvol_attr.dvol_head, entry) {
        if (dvol && (dvol->idx == dvol_idx)) {
            dvol_valid = 1;
#if 0
            static dvol_handle *last_dvol = NULL;
            if (last_dvol != dvol) {
                dvol_log("dvol_run[%x]:%x", dvol_idx, dvol);
                last_dvol = dvol;
            }
#endif
            break;
        }
    }
    local_irq_enable();

    if ((dvol->toggle == 0) || (dvol_valid == 0)) {
        os_mutex_post(&dvol_attr.mutex);
        return -1;
    }

    buf = data;
    len >>= 1; //byte to point

    for (u32 i = 0; i < len; i += 2) {
        ///left channel
        if (dvol->fade) {
            if (dvol->vol_fade > dvol->vol_target) {
                dvol->vol_fade -= dvol->fade_step;
                if (dvol->vol_fade < dvol->vol_target) {
                    dvol->vol_fade = dvol->vol_target;
                }
            } else if (dvol->vol_fade < dvol->vol_target) {
                dvol->vol_fade += dvol->fade_step;
                if (dvol->vol_fade > dvol->vol_target) {
                    dvol->vol_fade = dvol->vol_target;
                }
            }
        } else {
            dvol->vol_fade = dvol->vol_target;
        }

        valuetemp = buf[i];
        if (valuetemp < 0) {
            /*负数先转换成正数，运算完再转换回去，是为了避免负数右移位引入1的误差，增加底噪*/
            valuetemp = -valuetemp;
            /*rounding处理（加入0.5），减少小信号时候的误差和谐波幅值*/
            valuetemp = (valuetemp * dvol->vol_fade + (1 << 13)) >> 14 ;
            valuetemp = -valuetemp;
        } else {
            valuetemp = (valuetemp * dvol->vol_fade + (1 << 13)) >> 14 ;
        }
        /*饱和处理*/
        buf[i] = (s16)data_sat_s16(valuetemp);

        ///right channel
        valuetemp = buf[i + 1];
        if (valuetemp < 0) {
            /*负数先转换成正数，运算完再转换回去，是为了避免负数右移位引入1的误差，增加底噪*/
            valuetemp = -valuetemp;
            valuetemp = (valuetemp * dvol->vol_fade + (1 << 13)) >> 14 ;
            valuetemp = -valuetemp;
        } else {
            valuetemp = (valuetemp * dvol->vol_fade + (1 << 13)) >> 14 ;
        }
        /*饱和处理*/
        buf[i + 1] = (s16)data_sat_s16(valuetemp);
    }
    os_mutex_post(&dvol_attr.mutex);
    return 0;
}

