#include "app_config.h"
#include "syscfg_id.h"
#include "le_rcsp_adv_module.h"

#include "adv_hearing_aid_setting.h"
#include "rcsp_adv_tws_sync.h"
#include "JL_rcsp_protocol.h"
#include "rcsp_adv_bluetooth.h"
#include "spp_user.h"

#if (RCSP_ADV_EN && RCSP_ADV_ASSISTED_HEARING)

static u8 g_dha_fitting_data[3 + 2 * 4 * DHA_FITTING_CHANNEL_MAX] = {0};
static u8 g_dha_fitting_info[3 + 2 + 2 * DHA_FITTING_CHANNEL_MAX + 1] = {0};
static bool g_aidIsOperating = false;
static u8 g_aidOpCode = -1;
static u8 g_aidOpCode_SN = -1;

extern int tws_api_get_role(void);
extern u8 deal_adv_setting_string_item(u8 *des, u8 *src, u8 src_len, u8 type);
extern u16 get_adv_sync_tws_len(void);
static u16 adv_hearing_aid_convert(u8 *data);

static void hearing_aid_update_handle(u8 *data, u16 data_len)
{
#if (TCFG_AUDIO_HEARING_AID_ENABLE && TCFG_AUDIO_DHA_FITTING_ENABLE)
    hearing_aid_fitting_parse(g_dha_fitting_data, data_len);
#endif /*TCFG_AUDIO_HEARING_AID_ENABLE && TCFG_AUDIO_DHA_FITTING_ENABLE*/
}

static void hearing_aid_update(void)
{
    // 跟进不同命令类型调用设置函数
    printf("%s\n", __func__);
    /* u16 data_len = get_adv_sync_tws_len(); */
    /* if (sizeof(g_dha_fitting_data) == data_len) { */
    /*     data_len = adv_hearing_aid_convert(g_dha_fitting_data); */
    /*     put_buf(g_dha_fitting_data, data_len); */
    /* } */
#if ASSISTED_HEARING_CUSTOM_TRASNDATA
    // 从机才触发
    u16 data_len = get_adv_sync_tws_len();
#else
    u16 data_len = adv_hearing_aid_convert(g_dha_fitting_data);
#endif
    put_buf(g_dha_fitting_data, data_len);
    hearing_aid_update_handle(g_dha_fitting_data, data_len);
}

#pragma pack(1)
struct hearing_aid_playload {
    u8 cmd;
    u16 data_len;
    dha_fitting_info_t data_info;
};
#pragma pack()

void get_dha_fitting_info(u8 *dha_fitting_info)
{
    printf("%s\n", __func__);
    struct hearing_aid_playload playload;
    playload.cmd = DHA_FITTING_CMD_INFO;
    // 如果当前是主动拿，就把所有信息返回

    // 获取0x50对应的数据
    /* u8 tmp_data[] = {0, DHA_FITTING_CHANNEL_MAX, 0x12, 0x21, 0x23, 0x32, 0x34, 0x43, 0x45, 0x54, 0x56, 0x65, 0x67, 0x76}; // 例子 */
    /* playload.data_len = sizeof(tmp_data); */
    /* memcpy(&playload.data_info, tmp_data, sizeof(tmp_data)); */

    // 获取0x50对应的数据
    dha_fitting_info_t dha_info;
#if (TCFG_AUDIO_HEARING_AID_ENABLE && TCFG_AUDIO_DHA_FITTING_ENABLE)
    playload.data_len = get_hearing_aid_fitting_info(&dha_info);
#endif /*TCFG_AUDIO_HEARING_AID_ENABLE && TCFG_AUDIO_DHA_FITTING_ENABLE*/

    memcpy(&playload.data_info, &dha_info, sizeof(dha_info));

    memcpy(g_dha_fitting_info, &playload, sizeof(playload));

    // 如果当前是从机通知，就直接memcpy过去，不需要填充前三个头
    u16 data_len = adv_hearing_aid_convert(g_dha_fitting_info);
    put_buf(g_dha_fitting_info, data_len);

    memcpy(dha_fitting_info, g_dha_fitting_info, sizeof(g_dha_fitting_info) - 1);
}

static void hearing_aid_sync(void)
{
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) {
        update_adv_setting(BIT(ATTR_TYPE_ASSISTED_HEARING));
    }
#endif
}

static void big_to_little(u8 *data, u8 data_len)
{
    u8 tmp_data = 0;
    for (u8 i = 0; i < (data_len / 2); i++) {
        tmp_data = data[i];
        data[i] = data[data_len - i - 1];
        data[data_len - i - 1] = tmp_data;
    }
}

static u16 adv_hearing_aid_convert(u8 *data)
{
    u16 offset = 0;
    u16 data_len = 0;
    u8 cmd = data[offset++];

    dha_fitting_adjust_t *dha_data; // 0x51
    dha_fitting_info_t *dha_info; // 0x50
    switch (cmd) {
    case DHA_FITTING_CMD_INFO:
        big_to_little(data + offset, 2);
        data_len = data[offset++] << 8 | data[offset++];
        dha_info = (dha_fitting_adjust_t *)(data + offset);
        offset += 2;
        for (; offset < data_len + 3; offset += 2) {
            big_to_little(data + offset, 2);
        }
        break;
    case DHA_FITTING_CMD_ADJUST:
        data_len = data[offset] << 8 | data[offset + 1];
        big_to_little(data + offset, 2);
        offset += 2;
        dha_data = (dha_fitting_adjust_t *)(data + offset);
        offset += 2;
        big_to_little((u8 *)&dha_data->freq, sizeof(dha_data->freq));
        offset += 2;
        big_to_little((u8 *)&dha_data->gain, sizeof(dha_data->gain));
        offset += 4;
        break;
    case DHA_FITTING_CMD_UPDATE:
        data_len = data[offset] << 8 | data[offset + 1];
        big_to_little(data + offset, 2);
        offset += 3;
        for (; offset < data_len + 3; offset += 4) {
            big_to_little(g_dha_fitting_data + offset, 4);
        }
        break;
    case DHA_FITTING_CMD_STATE:
        big_to_little(data + offset, 2);
        data_len = data[offset++] << 8 | data[offset++];
        offset += 1;
        break;
    }
    if (data_len + 3 == offset) {
        return offset;
    }
    return 0;
}

void set_hearing_aid_setting(u8 *hear_aid_setting)
{
    memcpy(g_dha_fitting_data, hear_aid_setting, sizeof(g_dha_fitting_data));
}

void get_hearing_aid_setting(u8 *hear_aid_setting)
{
    memcpy(hear_aid_setting, g_dha_fitting_data, sizeof(g_dha_fitting_data));
}

u32 hearing_aid_rcsp_notify(u8 *data, u16 data_len)
{
    u32 ret = 0;
    u8 *buf = NULL;

    if (0 == adv_hearing_aid_convert(data)) {
        return -1;
    }

    buf = zalloc(data_len + 3);
    if (NULL == buf) {
        return -1;
    }
    buf[0] = 0xFF;
    data_len = add_one_attr(buf + 1, data_len + 2, 0, COMMON_INFO_ATTR_ASSISTED_HEARING, data, data_len);

    ret = JL_CMD_send(JL_OPCODE_SYS_INFO_AUTO_UPDATE, buf, data_len + 1, JL_NOT_NEED_RESPOND);

    if (buf) {
        free(buf);
    }
    return ret;
}

u32 hearing_aid_rcsp_response(u8 *data, u16 data_len)
{
    /*不开辅听功能时，不回复app信息*/
#if (TCFG_AUDIO_HEARING_AID_ENABLE && TCFG_AUDIO_DHA_FITTING_ENABLE)
    if (get_hearing_aid_state() == 0)
#endif /*(TCFG_AUDIO_HEARING_AID_ENABLE && TCFG_AUDIO_DHA_FITTING_ENABLE)*/
    {
        return 0;
    }

    if ((TWS_ROLE_MASTER == tws_api_get_role()) && (g_aidOpCode >= 0) && (g_aidOpCode_SN >= 0)) {
        u32 ret = 0;
        ret = JL_CMD_response_send(g_aidOpCode, JL_PRO_STATUS_SUCCESS, g_aidOpCode_SN, data, data_len);
        g_aidIsOperating = false;
        g_aidOpCode = -1;
        g_aidOpCode_SN = -1;

        return ret;
    } else {
        return -1;
    }
}

void deal_hearing_aid_setting(u8 opCode, u8 opCode_SN, u8 *hear_aid_setting, u8 write_en, u8 type_sync)
{
    if (TWS_ROLE_MASTER == tws_api_get_role()) {
        set_hearing_aid_operating_flag();
    }
    if (g_aidIsOperating) {
        return;
    }
    if (TWS_ROLE_MASTER == tws_api_get_role()) {
        g_aidIsOperating = true;
        g_aidOpCode = opCode;
        g_aidOpCode_SN = opCode_SN;
    }
    if (hear_aid_setting) {
        u16 len = hear_aid_setting[1] << 8 | hear_aid_setting[2];
        memcpy(g_dha_fitting_data, hear_aid_setting, len + 2);
    }

    if (type_sync) {
        hearing_aid_sync();
    }

#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) {
        if (TWS_ROLE_MASTER == tws_api_get_role()) {
            hearing_aid_update();
        } else if (0 == type_sync) {
            hearing_aid_update();
        }
    } else
#endif
    {
        hearing_aid_update();
    }
}

#if ASSISTED_HEARING_CUSTOM_TRASNDATA
static struct spp_operation_t *spp_opt = NULL;
static void app_third_party_spp_rx_cbk(void *priv, u8 *buf, u16 len)
{
    app_third_party_hearing_aid_handle(buf, len);
}

void adv_hearing_aid_init(void)
{
    spp_get_operation_table(&spp_opt);
    if (spp_opt && spp_opt->regist_recieve_cbk) {
        spp_opt->regist_recieve_cbk(NULL, app_third_party_spp_rx_cbk);
    }
}

int app_third_party_hearing_aid_handle(u8 *data, u16 len)
{
#if TCFG_USER_TWS_ENABLE
    u8 *tmp_buf = NULL;
    u16 offset = 1;
    if (len < sizeof(g_dha_fitting_data)) {
        return -1;
    }
    tmp_buf = zalloc(1 + 1 + len);
    if (NULL == tmp_buf) {
        return -1;
    }

    offset += deal_adv_setting_string_item(tmp_buf + offset, data, len, ATTR_TYPE_ASSISTED_HEARING);
    tmp_buf[0] = offset;

    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        tws_api_send_data_to_sibling(tmp_buf, offset, TWS_FUNC_ID_ADV_SETTING_SYNC);
    }

    if (tmp_buf) {
        free(tmp_buf);
    }
#endif
    // 处理函数
    hearing_aid_update_handle(data, len);
    return 0;
}

extern int custom_app_send_user_data(u8 *data, u16 len);
int app_third_party_hearing_aid_resp(u8 *data, u16 len)
{
    if (custom_app_send_user_data(data, len)) {
        if (spp_opt && spp_opt->send_data) {
            return spp_opt->send_data(NULL, data, (u32)len);
        }
    }
    return 0;
}
#else
int app_third_party_hearing_aid_handle(u8 *data, u16 len)
{
    return 0;
}
int app_third_party_hearing_aid_resp(u8 *data, u16 len)
{
    return 0;
}
void adv_hearing_aid_init(void)
{

}
#endif

#endif

