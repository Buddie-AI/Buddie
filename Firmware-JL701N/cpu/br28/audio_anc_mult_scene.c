
/*******************************************************************
						  ANC 多滤波器场景
*******************************************************************/
#include "audio_anc.h"
#include "audio_anc_mult_scene.h"


#if ANC_MULT_ORDER_ENABLE

#if 0
#define anc_mult_log	printf
#else
#define anc_mult_log(...)
#endif/*log_en*/

//ANC重组数据流程测试
#define ANC_MULT_DATA_TEST		0	//1 测试读取 ；2 测试重组

typedef struct {
    u8 yorder;
    anc_mult_gain_t *gains;	//gains, 复用flash位置
    anc_fr_t *fr;			//滤波器，复用flash位置
} anc_mult_param_t;

typedef struct {
    struct list_head entry;
    u16 scene_id;				//场景号
    anc_mult_param_t lff;
    anc_mult_param_t lfb;
    anc_mult_param_t ltrans;
    anc_mult_param_t lcmp;
    anc_mult_param_t rff;
    anc_mult_param_t rfb;
    anc_mult_param_t rtrans;
    anc_mult_param_t rcmp;
} anc_mult_bulk_t;

typedef struct {
    struct list_head head;
    audio_anc_t *param;
    u16 cur_scene;
    double *lff_coeff;
    double *lfb_coeff;
    double *lcmp_coeff;
    double *ltrans_coeff;
    double *rff_coeff;
    double *rfb_coeff;
    double *rcmp_coeff;
    double *rtrans_coeff;
} anc_mult_hdl_t;

//场景数据子结构;
typedef struct {
    u16 scene_id;
    u16 cnt;
    u8 dat[0];	//小心访问非对齐异常
} anc_mult_scene_file_t;

/*
   包含关系
	anc_coeff_t
	->anc_mult_scene_file_t
	->struct anc_param_head_t
	->data;
*/
anc_mult_hdl_t *mult_hdl = NULL;

static anc_coeff_t *anc_mult_cfg_test(int *db_len);
static anc_coeff_t *anc_mult_cfg_part_test(int *db_len);


//多滤波器数据填充
static int anc_mult_scene_param_read(struct list_head *list, u8 *dat)
{
    int i = 0;
    int file_offset = 0;
    u8 *temp_dat;
    anc_mult_scene_file_t *file = (anc_mult_scene_file_t *)dat;
    anc_mult_scene_file_t file_align;
    memcpy((u8 *)&file_align, (u8 *)file, sizeof(anc_mult_scene_file_t));
    struct anc_param_head_t id_head[file_align.cnt];

    anc_mult_bulk_t *bulk = zalloc(sizeof(anc_mult_bulk_t));

    bulk->scene_id = file_align.scene_id;

    /* put_buf(db_coeff->dat, db_coeff->cnt * 6); */
    anc_mult_log("anc_mult_scene_id %d\n", file_align.scene_id);
    temp_dat = file->dat + (file_align.cnt * 6);
    for (i = 0; i < file_align.cnt; i++) {
        memcpy(&id_head[i], file->dat + (i * 6), 6); //结构体为8字节，这里只有6字节
        anc_mult_log("id:0X%x, offset:%d,len:%d\n", id_head[i].id, id_head[i].offset, id_head[i].len);
        switch (id_head[i].id) {
        case ANC_L_FF_FGQ:
            anc_mult_log("ANC_L_FF_FGQ\n");
            bulk->lff.fr = (anc_fr_t *)(temp_dat + id_head[i].offset);
            bulk->lff.yorder = id_head[i].len / 13;
            break;
        case ANC_L_FB_FGQ:
            anc_mult_log("ANC_L_FB_FGQ\n");
            bulk->lfb.fr = (anc_fr_t *)(temp_dat + id_head[i].offset);
            bulk->lfb.yorder = id_head[i].len / 13;
            break;
        case ANC_L_CMP_FGQ:
            anc_mult_log("ANC_L_CMP_FGQ\n");
            bulk->lcmp.fr = (anc_fr_t *)(temp_dat + id_head[i].offset);
            bulk->lcmp.yorder = id_head[i].len / 13;
            break;
        case ANC_L_TRANS_FGQ:
            anc_mult_log("ANC_L_TRANS_FGQ\n");
            bulk->ltrans.fr = (anc_fr_t *)(temp_dat + id_head[i].offset);
            bulk->ltrans.yorder = id_head[i].len / 13;
            break;

        case ANC_L_FF_PARM:
            anc_mult_log("ANC_L_FF_PARM\n");
            bulk->lff.gains = (anc_mult_gain_t *)(temp_dat + id_head[i].offset);
            break;
        case ANC_L_FB_PARM:
            anc_mult_log("ANC_L_FB_PARM\n");
            bulk->lfb.gains = (anc_mult_gain_t *)(temp_dat + id_head[i].offset);
            break;
        case ANC_L_CMP_PARM:
            anc_mult_log("ANC_L_CMP_PARM\n");
            bulk->lcmp.gains = (anc_mult_gain_t *)(temp_dat + id_head[i].offset);
            break;
        case ANC_L_TRANS_PARM:
            anc_mult_log("ANC_L_TRANS_PARM\n");
            bulk->ltrans.gains = (anc_mult_gain_t *)(temp_dat + id_head[i].offset);
            break;

        case ANC_R_FF_FGQ:
            anc_mult_log("ANC_R_FF_FGQ\n");
            bulk->rff.fr = (anc_fr_t *)(temp_dat + id_head[i].offset);
            bulk->rff.yorder = id_head[i].len / 13;
            break;
        case ANC_R_FB_FGQ:
            anc_mult_log("ANC_R_FB_FGQ\n");
            bulk->rfb.fr = (anc_fr_t *)(temp_dat + id_head[i].offset);
            bulk->rfb.yorder = id_head[i].len / 13;
            break;
        case ANC_R_CMP_FGQ:
            anc_mult_log("ANC_R_CMP_FGQ\n");
            bulk->rcmp.fr = (anc_fr_t *)(temp_dat + id_head[i].offset);
            bulk->rcmp.yorder = id_head[i].len / 13;
            break;
        case ANC_R_TRANS_FGQ:
            anc_mult_log("ANC_R_TRANS_FGQ\n");
            bulk->rtrans.fr = (anc_fr_t *)(temp_dat + id_head[i].offset);
            bulk->rtrans.yorder = id_head[i].len / 13;
            break;

        case ANC_R_FF_PARM:
            anc_mult_log("ANC_R_FF_PARM\n");
            bulk->rff.gains = (anc_mult_gain_t *)(temp_dat + id_head[i].offset);
            break;
        case ANC_R_FB_PARM:
            anc_mult_log("ANC_R_FB_PARM\n");
            bulk->rfb.gains = (anc_mult_gain_t *)(temp_dat + id_head[i].offset);
            break;
        case ANC_R_CMP_PARM:
            anc_mult_log("ANC_R_CMP_PARM\n");
            bulk->rcmp.gains = (anc_mult_gain_t *)(temp_dat + id_head[i].offset);
            break;
        case ANC_R_TRANS_PARM:
            anc_mult_log("ANC_R_TRANS_PARM\n");
            bulk->rtrans.gains = (anc_mult_gain_t *)(temp_dat + id_head[i].offset);
            break;
        case ANC_L_ADAP_GOLD_CURVE:
        case ANC_R_ADAP_GOLD_CURVE:
            if (mult_hdl->param->adaptive) {
                anc_mult_log("ANC_L/R_ADAP_GOLD_CURVE\n");
                mult_hdl->param->adaptive->ref_ff_curve = temp_dat + id_head[i].offset;
                mult_hdl->param->adaptive->ref_ff_curve_len = id_head[i].len;
            }
            break;
        default:
            break;
        }
        /* put_buf((u8 *)(temp_dat + id_head[i].offset), id_head[i].len); */
    }

    //加入链表
    list_add_tail(&bulk->entry, list);
    //下一个场景的数据偏移量
    file_offset = ((u32)temp_dat - (u32)dat) + id_head[file_align.cnt - 1].offset + id_head[file_align.cnt - 1].len;
    anc_mult_log("next scene offset %d\n", file_offset);
    return file_offset;
}

static int anc_mult_scene_list_del(struct list_head *list)
{
    anc_mult_bulk_t *bulk;
    anc_mult_bulk_t *temp;
    list_for_each_entry_safe(bulk, temp, list, entry) {
        list_del(&bulk->entry);
        free(bulk);
    }
}


//多滤波器文件读取
int anc_mult_coeff_file_fill(anc_coeff_t *db_coeff)
{
    int offset = 0;
    u8 *temp_dat;
#if ANC_MULT_DATA_TEST == 1
    int len;
    db_coeff = anc_mult_cfg_test(&len);
#endif/*ANC_MULT_DATA_TEST*/
    if (mult_hdl && db_coeff) {
        if ((db_coeff->version & 0xF0) == 0xA0) {
            anc_mult_log("%s scene_cnt %d\n", __func__, db_coeff->cnt);
            anc_mult_scene_list_del(&mult_hdl->head);	//先卸载所有链表
            audio_anc_mult_scene_max_set(db_coeff->cnt);
            temp_dat = db_coeff->dat;
            for (int i = 0; i < db_coeff->cnt; i++) {
                //temp_dat表示当前场景数据指针，以下函数会返回 下一个场景相对当前temp_dat的偏移量
                temp_dat += anc_mult_scene_param_read(&mult_hdl->head, temp_dat);
            }
            return 0;
        }
    }
    return 1;
}

//多滤波器初始化API
void anc_mult_init(audio_anc_t *param)
{
    mult_hdl = zalloc(sizeof(anc_mult_hdl_t));
    mult_hdl->param = param;

    //链表初始化
    INIT_LIST_HEAD(&mult_hdl->head);
}

extern void audio_anc_biquad2ab_double(anc_fr_t *iir, double *out_coeff, u8 order, int alogm);
static int anc_mult_cfg_analsis(anc_mult_param_t *p, double **coeff, float *gain, int alogm)
{
    if (*coeff) {
        free(*coeff);
        *coeff = NULL;
    }
    if (!p->yorder) {
        //没有滤波器, 退出
        return -1;
    }
    *coeff = (double *)malloc(p->yorder * 40);
    /* g_printf("coeff %x\n", (u32)*coeff); */
    audio_anc_biquad2ab_double(p->fr, *coeff, p->yorder, alogm);
    float temp;
    memcpy((u8 *)&temp, (u8 *)&p->gains->iir_gain, 4);
    if (temp < 0) {
        *gain = 0 - temp;
        return 1;	//返回符号位
    } else {
        *gain = temp;
        return 0;	//返回符号位
    }
}

//场景ID校验
int audio_anc_mult_scene_id_check(u16 scene_id)
{
    if (!mult_hdl) {
        return 1;
    }
    anc_mult_bulk_t *bulk;
    list_for_each_entry(bulk, &mult_hdl->head, entry) {
        if (bulk->scene_id == scene_id) {
            return 0;
        }
    }
    return 1;
}

//目标场景, 转成IIR滤波器，并设置到lib coeff/gain变量
int anc_mult_scene_set(u16 scene_id)
{
    int ret = 1;
    if (!mult_hdl) {
        return 1;
    }
    anc_mult_log("anc_mult_scene_set cur scene_id %d\n", scene_id);
    u8 gain_sign = 0;
    audio_anc_t *param = mult_hdl->param;
    mult_hdl->cur_scene = scene_id;
    anc_mult_bulk_t *bulk;
    anc_mult_bulk_t *cmp_bulk;
    anc_mult_bulk_t *trans_bulk;
    list_for_each_entry(bulk, &mult_hdl->head, entry) {
        if (bulk->scene_id == scene_id) {
            if (anc_mult_cfg_analsis(&bulk->lff, &mult_hdl->lff_coeff, &param->gains.l_ffgain, param->gains.alogm) > 0) {
                gain_sign |= ANCL_FF_SIGN;
            }
            param->lff_coeff = mult_hdl->lff_coeff;
            param->lff_yorder = bulk->lff.yorder;
            /* g_printf("param->lff_yorder %d\n", param->lff_yorder); */
            /* put_buf((u8 *)mult_hdl->lff_coeff, bulk->lff.yorder * 40); */
            /* g_printf("gain %d/100, gain_sign %x\n", (int)(param->gains.l_ffgain * 100.0f), gain_sign); */

            if (anc_mult_cfg_analsis(&bulk->lfb, &mult_hdl->lfb_coeff, &param->gains.l_fbgain, param->gains.alogm) > 0) {
                gain_sign |= ANCL_FB_SIGN;
            }
            param->lfb_coeff = mult_hdl->lfb_coeff;
            param->lfb_yorder = bulk->lfb.yorder;

            if (anc_mult_cfg_analsis(&bulk->rff, &mult_hdl->rff_coeff, &param->gains.r_ffgain, param->gains.alogm) > 0) {
                gain_sign |= ANCR_FF_SIGN;
            }
            param->rff_coeff = mult_hdl->rff_coeff;
            param->rff_yorder = bulk->rff.yorder;

            if (anc_mult_cfg_analsis(&bulk->rfb, &mult_hdl->rfb_coeff, &param->gains.r_fbgain, param->gains.alogm) > 0) {
                gain_sign |= ANCR_FB_SIGN;
            }
            param->rfb_coeff = mult_hdl->rfb_coeff;
            param->rfb_yorder = bulk->rfb.yorder;

#if !ANC_MULT_ORDER_CMP_ONLY_USE_ID1	//CMP支持多场景
            cmp_bulk = bulk;
#endif/*ANC_MULT_ORDER_CMP_ONLY_USE_ID1*/
#if !ANC_MULT_ORDER_TRANS_ONLY_USE_ID1	//通透支持多场景
            trans_bulk = bulk;
#endif/*ANC_MULT_ORDER_CMP_ONLY_USE_ID1*/
            ret = 0;
        }
        if (bulk->scene_id == 1) {
#if ANC_MULT_ORDER_CMP_ONLY_USE_ID1		//CMP固定使用场景1的参数
            cmp_bulk = bulk;
#endif/*ANC_MULT_ORDER_CMP_ONLY_USE_ID1*/
#if ANC_MULT_ORDER_TRANS_ONLY_USE_ID1	//通透固定使用场景1的参数
            trans_bulk = bulk;
#endif/*ANC_MULT_ORDER_CMP_ONLY_USE_ID1*/
        }
    }

    if (anc_mult_cfg_analsis(&cmp_bulk->lcmp, &mult_hdl->lcmp_coeff, &param->gains.l_cmpgain, param->gains.alogm) > 0) {
        gain_sign |= ANCL_CMP_SIGN;
    }
    param->lcmp_coeff = mult_hdl->lcmp_coeff;
    param->lcmp_yorder = cmp_bulk->lcmp.yorder;

    if (anc_mult_cfg_analsis(&cmp_bulk->rcmp, &mult_hdl->rcmp_coeff, &param->gains.r_cmpgain, param->gains.alogm) > 0) {
        gain_sign |= ANCR_CMP_SIGN;
    }
    param->rcmp_coeff = mult_hdl->rcmp_coeff;
    param->rcmp_yorder = cmp_bulk->rcmp.yorder;

    if (anc_mult_cfg_analsis(&trans_bulk->ltrans, &mult_hdl->ltrans_coeff, &param->gains.l_transgain, param->gains.trans_alogm) > 0) {
        gain_sign |= ANCL_TRANS_SIGN;
    }
    param->ltrans_coeff = mult_hdl->ltrans_coeff;
    param->ltrans_yorder = trans_bulk->ltrans.yorder;

    if (anc_mult_cfg_analsis(&trans_bulk->rtrans, &mult_hdl->rtrans_coeff, &param->gains.r_transgain, param->gains.trans_alogm) > 0) {
        gain_sign |= ANCR_TRANS_SIGN;
    }
    param->rtrans_coeff = mult_hdl->rtrans_coeff;
    param->rtrans_yorder = trans_bulk->rtrans.yorder;

    param->gains.gain_sign = gain_sign;
    audio_anc_max_yorder_verify(param);
#if ANC_EAR_ADAPTIVE_EN
    //ANC自适应开发者模式，映射左右参数
    audio_anc_param_map(1, 1);
#endif/*ANC_EAR_ADAPTIVE_EN*/
    return ret;
}

//释放IIR滤波器空间
void audio_anc_mult_scene_coeff_free(void)
{
    /*
       空间暂不释放:
       频繁切场景时或与自适应交叉使用时，set 和 free的动作可能不是一一对应的，
       导致最后一个信号量响应时，滤波器已经被提前释放了。
    */
    return;
    if (!mult_hdl) {
        return;
    }
    if (mult_hdl->lff_coeff) {
        free(mult_hdl->lff_coeff);
        mult_hdl->param->lff_coeff = NULL;
        mult_hdl->lff_coeff = NULL;
    }
    if (mult_hdl->lfb_coeff) {
        free(mult_hdl->lfb_coeff);
        mult_hdl->param->lfb_coeff = NULL;
        mult_hdl->lfb_coeff = NULL;
    }
    if (mult_hdl->ltrans_coeff) {
        free(mult_hdl->ltrans_coeff);
        mult_hdl->param->ltrans_coeff = NULL;
        mult_hdl->ltrans_coeff = NULL;
    }
    if (mult_hdl->lcmp_coeff) {
        free(mult_hdl->lcmp_coeff);
        mult_hdl->param->lcmp_coeff = NULL;
        mult_hdl->lcmp_coeff = NULL;
    }
    if (mult_hdl->rff_coeff) {
        free(mult_hdl->rff_coeff);
        mult_hdl->param->rff_coeff = NULL;
        mult_hdl->rff_coeff = NULL;
    }
    if (mult_hdl->rfb_coeff) {
        free(mult_hdl->rfb_coeff);
        mult_hdl->param->rfb_coeff = NULL;
        mult_hdl->rfb_coeff = NULL;
    }
    if (mult_hdl->rtrans_coeff) {
        free(mult_hdl->rtrans_coeff);
        mult_hdl->param->rtrans_coeff = NULL;
        mult_hdl->rtrans_coeff = NULL;
    }
    if (mult_hdl->rcmp_coeff) {
        free(mult_hdl->rcmp_coeff);
        mult_hdl->param->rcmp_coeff = NULL;
        mult_hdl->rcmp_coeff = NULL;
    }
}

/*ANC滤波器格式校验*/
static int anc_mult_coeff_check(anc_coeff_t *db_coeff, u16 len)
{
    u32 coeff_len = 0;
    int i;

    if ((db_coeff->version & 0xF0) != 0xA0) {
        return 1;
    }
    if (db_coeff->cnt > 5) {	//理论上最大ID个数
        return 1;
    }

    anc_mult_scene_file_t *file = (anc_mult_scene_file_t *)db_coeff->dat;
    struct anc_param_head_t coeff_head[file->cnt];

    for (i = 0; i < file->cnt; i++) {
        memcpy(&coeff_head[i], file->dat + (i * 6), 6); //结构体为8字节，这里只有6字节
        if (coeff_len != coeff_head[i].offset) {
            return 1;
        }
        coeff_len +=  coeff_head[i].len;
    }
    return 0;
}

const static u8 anc_mult_part_id[8][2] = {
    {ANC_L_FF_PARM, ANC_L_FF_FGQ},
    {ANC_L_FB_PARM, ANC_L_FB_FGQ},
    {ANC_L_CMP_PARM, ANC_L_CMP_FGQ},
    {ANC_L_TRANS_PARM, ANC_L_TRANS_FGQ},

    {ANC_R_FF_PARM, ANC_R_FF_FGQ},
    {ANC_R_FB_PARM, ANC_R_FB_FGQ},
    {ANC_R_CMP_PARM, ANC_R_CMP_FGQ},
    {ANC_R_TRANS_PARM, ANC_R_TRANS_FGQ}
};

//部分数据文件-头部信息重组
static void anc_mult_part_head_recombination(u16 *cnt, struct anc_param_head_t *id_head, anc_mult_param_t *p, anc_mult_param_t *new_p, u8 type)
{
    u8 order = (new_p->fr) ? new_p->yorder : p->yorder;

    if ((p->gains != NULL) || (new_p->gains != NULL)) {
        id_head[*cnt].id = anc_mult_part_id[type][0];
        id_head[*cnt].len = sizeof(anc_mult_gain_t);
        if (*cnt) {
            id_head[*cnt].offset = id_head[(*cnt) - 1].len + id_head[(*cnt) - 1].offset;
        }
        (*cnt)++;
    }
    if (order) {
        id_head[*cnt].id = anc_mult_part_id[type][1];
        id_head[*cnt].len = order * sizeof(anc_fr_t);
        if (*cnt) {
            id_head[*cnt].offset = id_head[(*cnt) - 1].len + id_head[(*cnt) - 1].offset;
        }
        (*cnt)++;
    }
}

//部分数据文件-数据信息重组
static void anc_mult_part_data_recombination(anc_mult_param_t *new_p, anc_mult_param_t *p, anc_mult_param_t *t_p, u8 type)
{
    if (t_p->gains != NULL) {
        memcpy((u8 *)new_p->gains, (u8 *)t_p->gains, sizeof(anc_mult_gain_t));
    } else if (p->gains != NULL) {
        memcpy((u8 *)new_p->gains, (u8 *)p->gains, sizeof(anc_mult_gain_t));
    }
    if (t_p->fr != NULL) {
        memcpy((u8 *)new_p->fr, (u8 *)t_p->fr, t_p->yorder * sizeof(anc_fr_t));
    } else if (p->fr != NULL) {
        memcpy((u8 *)new_p->fr, (u8 *)p->fr, p->yorder * sizeof(anc_fr_t));
    }
}

//部分数据文件-写，不支持新增场景
static int anc_mult_part_coeff_write(anc_coeff_t *target_coeff, u16 len)
{
    u16 head_cnt, i, new_id_flag;
    u16 scene_cnt = 0;
    int ret = 0;
    int new_len = sizeof(anc_coeff_t);
    anc_coeff_t *old_coeff = (anc_coeff_t *)anc_db_get(ANC_DB_COEFF, &mult_hdl->param->coeff_size);
    u8 *temp_dat;

    anc_mult_bulk_t *new_bulk;
    anc_mult_bulk_t *target_bulk;
    anc_mult_bulk_t *old_bulk;
    anc_mult_bulk_t *t_bulk;

    if ((old_coeff->version & 0xF0) != 0xA0) {
        return -1;
    }

    anc_mult_scene_file_t *file[old_coeff->cnt];

    //1、解析目标数据信息
    struct list_head target_list;
    INIT_LIST_HEAD(&target_list);

    if ((target_coeff->version & 0xF0) != 0xA0) {
        return -1;
    }
    temp_dat = target_coeff->dat;
    for (i = 0; i < target_coeff->cnt; i++) {
        //temp_dat表示当前场景数据指针，以下函数会返回 下一个场景相对当前temp_dat的偏移量
        temp_dat += anc_mult_scene_param_read(&target_list, temp_dat);
    }

    //检查是否属于新场景(目前不支持新加场景重组)
    list_for_each_entry(target_bulk, &target_list, entry) {
        new_id_flag = 1;
        list_for_each_entry(old_bulk, &mult_hdl->head, entry) {
            if (target_bulk->scene_id == old_bulk->scene_id) {
                new_id_flag = 0;
                break;
            }
        }
        if (new_id_flag) {
            ret = -2;
            goto __free2;
        }
    }

    //2、重组新数据头信息
    list_for_each_entry(old_bulk, &mult_hdl->head, entry) {
        t_bulk = old_bulk;
        list_for_each_entry(target_bulk, &target_list, entry) {
            if (target_bulk->scene_id == old_bulk->scene_id) {
                t_bulk = target_bulk;
                break;
            }
        }
        file[scene_cnt] = zalloc(sizeof(struct anc_param_head_t) * 16 + sizeof(anc_mult_scene_file_t));
        struct anc_param_head_t *id_head = (struct anc_param_head_t *)file[scene_cnt]->dat;
        head_cnt = 0;
        anc_mult_part_head_recombination(&head_cnt, id_head, &old_bulk->lff,    &t_bulk->lff,    0);
        anc_mult_part_head_recombination(&head_cnt, id_head, &old_bulk->lfb,    &t_bulk->lfb,    1);
        anc_mult_part_head_recombination(&head_cnt, id_head, &old_bulk->lcmp,   &t_bulk->lcmp,   2);
        anc_mult_part_head_recombination(&head_cnt, id_head, &old_bulk->ltrans, &t_bulk->ltrans, 3);

        anc_mult_part_head_recombination(&head_cnt, id_head, &old_bulk->rff,    &t_bulk->rff,    4);
        anc_mult_part_head_recombination(&head_cnt, id_head, &old_bulk->rfb,    &t_bulk->rfb,    5);
        anc_mult_part_head_recombination(&head_cnt, id_head, &old_bulk->rcmp,   &t_bulk->rcmp,   6);
        anc_mult_part_head_recombination(&head_cnt, id_head, &old_bulk->rtrans, &t_bulk->rtrans, 7);
        file[scene_cnt]->scene_id = old_bulk->scene_id;
        file[scene_cnt]->cnt = head_cnt;
        scene_cnt++;
        //file+id_head+data(len+offset)
        new_len += sizeof(anc_mult_scene_file_t) + (sizeof(struct anc_param_head_t) * head_cnt) + \
                   id_head[head_cnt - 1].len + id_head[head_cnt - 1].offset;
        /* printf("last len%d, cnt%d, offset%d, len%d\n", new_len, head_cnt, id_head[head_cnt - 1].len, id_head[head_cnt - 1].offset);	 */
    }
    //重组后数据长度
    anc_mult_log("PART FILE new_file_len %d\n", new_len);

    //3、申请新数据空间
    anc_coeff_t *new_coeff = (anc_coeff_t *)zalloc(new_len);
    struct list_head new_list;
    INIT_LIST_HEAD(&new_list);

    int new_offset = 0;
    if (!new_coeff) {
        ret = -1;
        goto __free1;
    }
    memcpy((u8 *)new_coeff, (u8 *)old_coeff, sizeof(anc_coeff_t));
    temp_dat = new_coeff->dat;
    //4、拷贝数据头至新数据空间
    for (i = 0; i < new_coeff->cnt; i++) {
        memcpy(temp_dat, (u8 *)file[i], sizeof(anc_mult_scene_file_t));
        memcpy(temp_dat + sizeof(anc_mult_scene_file_t), file[i]->dat, \
               sizeof(struct anc_param_head_t) * file[i]->cnt);

        /* put_buf((u8*)new_coeff, new_len); */
        new_offset = anc_mult_scene_param_read(&new_list, temp_dat);
        temp_dat += new_offset;
    }
    //5、重组新数据
    list_for_each_entry(old_bulk, &mult_hdl->head, entry) {
        t_bulk = old_bulk;
        list_for_each_entry(target_bulk, &target_list, entry) {
            if (target_bulk->scene_id == old_bulk->scene_id) {
                t_bulk = target_bulk;
                break;
            }
        }
        list_for_each_entry(new_bulk, &new_list, entry) {
            if (new_bulk->scene_id == old_bulk->scene_id) {
                anc_mult_part_data_recombination(&new_bulk->lff,    &old_bulk->lff,    &t_bulk->lff,    0);
                anc_mult_part_data_recombination(&new_bulk->lfb,    &old_bulk->lfb,    &t_bulk->lfb,    1);
                anc_mult_part_data_recombination(&new_bulk->lcmp,   &old_bulk->lcmp,   &t_bulk->lcmp,   2);
                anc_mult_part_data_recombination(&new_bulk->ltrans, &old_bulk->ltrans, &t_bulk->ltrans, 3);

                anc_mult_part_data_recombination(&new_bulk->rff,    &old_bulk->rff,    &t_bulk->rff,    4);
                anc_mult_part_data_recombination(&new_bulk->rfb,    &old_bulk->rfb,    &t_bulk->rfb,    5);
                anc_mult_part_data_recombination(&new_bulk->rcmp,   &old_bulk->rcmp,   &t_bulk->rcmp,   6);
                anc_mult_part_data_recombination(&new_bulk->rtrans, &old_bulk->rtrans, &t_bulk->rtrans, 7);
            }
        }
    }
    //重组后数据内容
    /* put_buf((u8*)new_coeff, new_len); */

    //6、写入flash
    mult_hdl->param->write_coeff_size = new_len;	//更新滤波器长度
    ret = anc_db_put(mult_hdl->param, NULL, new_coeff);

    //7、释放新数据链表、ram
    anc_mult_scene_list_del(&new_list);
    free(new_coeff);

__free1:
    //8、释放重组临时ram
    for (i = 0; i < old_coeff->cnt; i++) {
        free(file[i]);
    }
__free2:
    //9、释放目标数据解析链表
    anc_mult_scene_list_del(&target_list);
    return ret;
}

//单独增益ID设置-兼容之前的命令
void audio_anc_mult_gains_id_set(u8 gain_id, int data)
{
    if (!mult_hdl) {
        return;
    }
    anc_mult_bulk_t *bulk;
    u8 *temp_dat;
    float fgain = *((float *)&data);
    int i;

    struct list_head list;
    INIT_LIST_HEAD(&list);

    anc_coeff_t *db_coeff = (anc_coeff_t *)anc_db_get(ANC_DB_COEFF, &mult_hdl->param->coeff_size);

    if ((db_coeff->version & 0xF0) != 0xA0) {
        return;
    }
    anc_coeff_t *coeff = malloc(mult_hdl->param->coeff_size);
    memcpy((u8 *)coeff, (u8 *)db_coeff, mult_hdl->param->coeff_size);

    temp_dat = coeff->dat;
    for (i = 0; i < coeff->cnt; i++) {
        temp_dat += anc_mult_scene_param_read(&list, temp_dat);
    }
    list_for_each_entry(bulk, &list, entry) {
        if (bulk->scene_id == audio_anc_mult_scene_get()) {
            break;
        }
    }
    switch (gain_id) {
    case ANC_FFGAIN_SET:
        if (bulk->lff.gains) {
            bulk->lff.gains->iir_gain = (bulk->lff.gains->iir_gain > 0) ? fgain : 0 - fgain;
        }
        break;
    case ANC_FBGAIN_SET:
        if (bulk->lfb.gains) {
            bulk->lfb.gains->iir_gain = (bulk->lfb.gains->iir_gain > 0) ? fgain : 0 - fgain;
        }
        break;
    case ANC_TRANS_GAIN_SET:
        if (bulk->ltrans.gains) {
            bulk->ltrans.gains->iir_gain = (bulk->ltrans.gains->iir_gain > 0) ? fgain : 0 - fgain;
        }
        break;
    case ANC_CMP_GAIN_SET:
        if (bulk->lcmp.gains) {
            bulk->lcmp.gains->iir_gain = (bulk->lcmp.gains->iir_gain > 0) ? fgain : 0 - fgain;
        }
        break;
    case ANC_RFF_GAIN_SET:
        if (bulk->rff.gains) {
            bulk->rff.gains->iir_gain = (bulk->rff.gains->iir_gain > 0) ? fgain : 0 - fgain;
        }
        break;
    case ANC_RFB_GAIN_SET:
        if (bulk->rfb.gains) {
            bulk->rfb.gains->iir_gain = (bulk->rfb.gains->iir_gain > 0) ? fgain : 0 - fgain;
        }
        break;
    case ANC_RTRANS_GAIN_SET:
        if (bulk->rtrans.gains) {
            bulk->rtrans.gains->iir_gain = (bulk->rtrans.gains->iir_gain > 0) ? fgain : 0 - fgain;
        }
        break;
    case ANC_RCMP_GAIN_SET:
        if (bulk->rcmp.gains) {
            bulk->rcmp.gains->iir_gain = (bulk->rcmp.gains->iir_gain > 0) ? fgain : 0 - fgain;
        }
        break;
    case ANC_GAIN_SIGN_SET:
        anc_mult_param_t *p;
        for (i = 0; i < 8; i++) {
            p = &bulk->lff + i;
            if (!p->gains) {
                continue;
            }
            if (((data & BIT(i)) && (p->gains->iir_gain > 0)) ||  \
                ((!(data & BIT(i))) && (p->gains->iir_gain < 0))) {
                p->gains->iir_gain = 0 - p->gains->iir_gain;
            }
        }
        break;
    }

    mult_hdl->param->write_coeff_size = mult_hdl->param->coeff_size;
    if (anc_db_put(mult_hdl->param, NULL, (anc_coeff_t *)coeff)) {
        anc_mult_log("anc_coeff_write err!");
    }
    audio_anc_mult_coeff_file_read();

    free(coeff);
}

//获取并解析flash anc_coeff.bin，实时更新参数效果
int audio_anc_mult_coeff_file_read(void)
{
    if (!mult_hdl) {
        return 1;
    }
    audio_anc_t *param = mult_hdl->param;
    //读flash
    anc_coeff_t *db_coeff = (anc_coeff_t *)anc_db_get(ANC_DB_COEFF, &param->coeff_size);
    anc_mult_log("anc_db coeff_size %d\n", param->coeff_size);
    //更新参数链表
    int ret = anc_mult_coeff_file_fill(db_coeff);

    //更新效果
    if (param->mode != ANC_OFF) {
        /* g_printf("read cur scene_id %d\n",mult_hdl->cur_scene); */
        anc_mult_scene_set(mult_hdl->cur_scene);
        audio_anc_reset(param, 0);
    }
    return ret;
}

//anc_coeff.bin在线更新，支持全部数据文件写、部分数据文件写
int audio_anc_mult_coeff_write(ANC_coeff_fill_t type, int *coeff, u16 len)
{
    int ret = 0;
    u8 single_type = 0;
#if ANC_MULT_DATA_TEST == 2  //流程测试，确认重组数据正常
    int test_len;
    if (type == ANC_MULT_COEFF_FILL_PART) {
        coeff = (int *)anc_mult_cfg_part_test((int)&test_len);
    } else {
        coeff = (int *)anc_mult_cfg_test((int)&test_len);
    }
    len = test_len;
    mult_hdl->param->write_coeff_size = len;
#endif/*ANC_MULT_DATA_TEST*/

    anc_coeff_t *db_coeff = (anc_coeff_t *)coeff;

    anc_coeff_t *tmp_coeff = NULL;
    if (!mult_hdl) {
        return -1;
    }
    anc_mult_log("anc_coeff_write:0x%x, len:%d", (u32)coeff, len);
    ret = anc_mult_coeff_check(db_coeff, len);
    if (ret) {
        r_printf("err %d\n", ret);
        return ret;
    }
    if (type == ANC_MULT_COEFF_FILL_PART) {
        ret = anc_mult_part_coeff_write(db_coeff, len);
    } else if (type == ANC_MULT_COEFF_FILL_ALL) {
        ret = anc_db_put(mult_hdl->param, NULL, (anc_coeff_t *)coeff);
    }
    if (ret) {
        anc_mult_log("anc_coeff_write err:%d", ret);
        return -1;
    }
    ret = audio_anc_mult_coeff_file_read();
    return ret;
}


#if ANC_MULT_DATA_TEST

static anc_coeff_t *anc_mult_cfg_test(int *db_len)
{
    int i, j;
    int offset = 0;
    int len = 4 + 2 * (4 + 6 + 6) + 4 + 4 + 26 + 39;
    anc_coeff_t *db = (anc_coeff_t *)zalloc(len);
    anc_mult_log("test_db_len %d\n", len);
    db->version = ANC_VERSION_BR28_MULT;
    db->cnt = 2;
    u8 *scene_temp = db->dat;
    u8 *temp_dat;
    struct anc_param_head_t *id_head;
    anc_fr_t fr;
    anc_mult_gain_t *gain;

    for (j = 0; j < db->cnt; j++) {
        anc_mult_scene_file_t *file = (anc_mult_scene_file_t *)scene_temp;

        switch (j) {
        case 0:
            file->cnt = 2;
            file->scene_id = 1;
            temp_dat = file->dat + (file->cnt * 6);
            id_head = (struct anc_param_head_t *)file->dat;
            id_head[0].id = ANC_L_FF_PARM;
            id_head[0].offset = 0;
            id_head[0].len = 4;
            gain = (anc_mult_gain_t *)temp_dat;
            gain->iir_gain = 2.0f;

            id_head[1].id = ANC_L_FF_FGQ;
            id_head[1].offset = 4;
            id_head[1].len = 26;
            fr.type = ANC_IIR_BAND_PASS;
            fr.a[0] = 500.0f;
            fr.a[1] = 10.0f;
            fr.a[2] = 3.0f;
            memcpy(temp_dat + id_head[1].offset, (u8 *)&fr, 13);
            memcpy(temp_dat + id_head[1].offset + 13, (u8 *)&fr, 13);
            offset = ((u32)temp_dat - (u32)scene_temp) + id_head[1].offset + id_head[1].len;
            break;

        case 1:
            file->cnt = 2;
            file->scene_id = 2;
            temp_dat = file->dat + (file->cnt * 6);
            id_head = (struct anc_param_head_t *)file->dat;
            id_head[0].id = ANC_L_FF_PARM;
            id_head[0].offset = 0;
            id_head[0].len = 4;
            gain = (anc_mult_gain_t *)temp_dat;
            gain->iir_gain = -3.0f;

            id_head[1].id = ANC_L_FF_FGQ;
            id_head[1].offset = 4;
            id_head[1].len = 39;
            fr.type = ANC_IIR_BAND_PASS;
            fr.a[0] = 5000.0f;
            fr.a[1] = 10.0f;
            fr.a[2] = 3.0f;
            memcpy(temp_dat + id_head[1].offset, (u8 *)&fr, 13);
            memcpy(temp_dat + id_head[1].offset + 13, (u8 *)&fr, 13);
            memcpy(temp_dat + id_head[1].offset + 26, (u8 *)&fr, 13);
            offset = ((u32)temp_dat - (u32)scene_temp) + id_head[1].offset + id_head[1].len;
            break;
        }
        scene_temp += offset;
    }
    *db_len = len;
    put_buf((u8 *)db, *db_len);
    return db;
}


anc_coeff_t *anc_mult_cfg_part_test(int *db_len)
{
    int i, j;
    int offset = 0;
    /* int len = 4 + 2 * (4 + 6 ) + 4 + 4 + 26 + 39; */
    int len = 4 + 2 * (4 + 6) + 6 + 4 + 4 + 39;
    anc_coeff_t *db = (anc_coeff_t *)zalloc(len);
    anc_mult_log("test_db_len %d\n", len);
    db->version = ANC_VERSION_BR28_MULT;
    db->cnt = 2;
    u8 *scene_temp = db->dat;
    u8 *temp_dat;
    struct anc_param_head_t *id_head;
    anc_fr_t fr;
    anc_mult_gain_t *gain;

    for (j = 0; j < db->cnt; j++) {
        anc_mult_scene_file_t *file = (anc_mult_scene_file_t *)scene_temp;

        switch (j) {
        case 0:
            file->cnt = 1;
            file->scene_id = 1;
            temp_dat = file->dat + (file->cnt * 6);
            id_head = (struct anc_param_head_t *)file->dat;
            id_head[0].id = ANC_L_FF_PARM;
            id_head[0].offset = 0;
            id_head[0].len = 4;
            gain = (anc_mult_gain_t *)temp_dat;
            gain->iir_gain = 1.0f;

            /* id_head[1].id = ANC_L_FF_FGQ; */
            /* id_head[1].offset = 4; */
            /* id_head[1].len = 26; */
            /* fr.type = ANC_IIR_BAND_PASS; */
            /* fr.a[0] = 500.0f; */
            /* fr.a[1] = 10.0f; */
            /* fr.a[2] = 3.0f; */
            /* memcpy(temp_dat + id_head[1].offset, (u8 *)&fr, 13); */
            /* memcpy(temp_dat + id_head[1].offset + 13, (u8 *)&fr, 13); */
            offset = ((u32)temp_dat - (u32)scene_temp) + id_head[0].offset + id_head[0].len;
            break;

        case 1:
            file->cnt = 2;
            file->scene_id = 2;
            temp_dat = file->dat + (file->cnt * 6);
            id_head = (struct anc_param_head_t *)file->dat;
            id_head[0].id = ANC_L_FF_PARM;
            id_head[0].offset = 0;
            id_head[0].len = 4;
            gain = (anc_mult_gain_t *)temp_dat;
            gain->iir_gain = 1.0f;

            id_head[1].id = ANC_L_FF_FGQ;
            id_head[1].offset = 4;
            /* id_head[1].len = 39; */
            id_head[1].len = 26;
            fr.type = ANC_IIR_BAND_PASS;
            fr.a[0] = 1.0f;
            fr.a[1] = 1.0f;
            fr.a[2] = 1.0f;
            memcpy(temp_dat + id_head[1].offset, (u8 *)&fr, 13);
            memcpy(temp_dat + id_head[1].offset + 13, (u8 *)&fr, 13);
            /* memcpy(temp_dat + id_head[1].offset + 26, (u8 *)&fr, 13); */
            offset = ((u32)temp_dat - (u32)scene_temp) + id_head[0].offset + id_head[0].len;
            break;
        }
        scene_temp += offset;
    }
    *db_len = len;
    put_buf((u8 *)db, *db_len);
    return db;
}

#endif/*ANC_MULT_DATA_TEST*/










#endif/*ANC_MULT_ORDER_ENABLE*/
