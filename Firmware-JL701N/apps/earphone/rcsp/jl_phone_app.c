
#include "app_config.h"
#include "btstack/avctp_user.h"
#include "adv_music_info_setting.h"

#if (JL_EARPHONE_APP_EN)

int jl_phone_app_init()
{
#if RCSP_ADV_MUSIC_INFO_ENABLE
    bt_music_info_handle_register(rcsp_adv_music_info_deal);
#endif
    return 0;
}
#endif
