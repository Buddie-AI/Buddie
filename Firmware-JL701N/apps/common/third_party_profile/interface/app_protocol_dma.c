#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "app_protocol_api.h"
#include "system/includes.h"
#include "vm.h"
#include "audio_config.h"
#include "app_power_manage.h"
#include "user_cfg.h"
#if APP_PROTOCOL_DMA_CODE

#if 1
#define APP_DMA_LOG       printf
#define APP_DMA_DUMP      put_buf
#else
#define APP_DMA_LOG(...)
#define APP_DMA_DUMP(...)
#endif

//*********************************************************************************//
//                                 DMA认证信息                                     //
//*********************************************************************************//
#define DMA_PRODUCT_INFO_TEST       0

#if DMA_PRODUCT_INFO_TEST
static const char *dma_product_id  = "abcdeYghv0fy6HFexl6bTIZU123Z4n6H";
static const char *dma_triad_id    = "A254BcBb000E00D200A0C0E3";
static const char *dma_secret      = "bab8d7e0dc616630";
#endif
static const char *dma_product_key = "rcPXDpFbBOxXaDL9GZ8qLTWq7Yz8rWFm";

#define DMA_PRODUCT_ID_LEN      65
#define DMA_PRODUCT_KEY_LEN     65
#define DMA_TRIAD_ID_LEN        32
#define DMA_SECRET_LEN          32

#define DMA_LEGAL_CHAR(c)       ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))

static u16 dma_get_one_info(const u8 *in, u8 *out)
{
    int read_len = 0;
    const u8 *p = in;

    while (DMA_LEGAL_CHAR(*p) && *p != ',') { //read Product ID
        *out++ = *p++;
        read_len++;
    }
    return read_len;
}

u8 read_dma_product_info_from_flash(u8 *read_buf, u16 buflen)
{
    u8 *rp = read_buf;
    const u8 *dma_ptr = (u8 *)app_protocal_get_license_ptr();

    if (dma_ptr == NULL) {
        return FALSE;
    }

    if (dma_get_one_info(dma_ptr, rp) != 32) {
        return FALSE;
    }
    dma_ptr += 33;

    rp = read_buf + DMA_PRODUCT_ID_LEN;
    memcpy(rp, dma_product_key, strlen(dma_product_key));

    rp = read_buf + DMA_PRODUCT_ID_LEN + DMA_PRODUCT_KEY_LEN;
    if (dma_get_one_info(dma_ptr, rp) != 24) {
        return FALSE;
    }
    dma_ptr += 25;

    rp = read_buf + DMA_PRODUCT_ID_LEN + DMA_PRODUCT_KEY_LEN + DMA_TRIAD_ID_LEN;
    if (dma_get_one_info(dma_ptr, rp) != 16) {
        return FALSE;
    }

    return TRUE;
}
__attribute__((weak))
void bt_update_testbox_addr(u8 *addr)
{

}
void dueros_dma_manufacturer_info_init()
{
    u8 read_buf[DMA_PRODUCT_ID_LEN + DMA_PRODUCT_KEY_LEN + DMA_TRIAD_ID_LEN + DMA_SECRET_LEN + 1] = {0};
    bool ret = FALSE;

    APP_DMA_LOG("dueros_dma_manufacturer_info_init\n");

#if DMA_PRODUCT_INFO_TEST
    memcpy(read_buf, dma_product_id, strlen(dma_product_id));
    memcpy(read_buf + DMA_PRODUCT_ID_LEN, dma_product_key, strlen(dma_product_key));
    memcpy(read_buf + DMA_PRODUCT_ID_LEN + DMA_PRODUCT_KEY_LEN, dma_triad_id, strlen(dma_triad_id));
    memcpy(read_buf + DMA_PRODUCT_ID_LEN + DMA_PRODUCT_KEY_LEN + DMA_TRIAD_ID_LEN, dma_secret, strlen(dma_secret));
    ret = TRUE;
#else
    //  ret = read_dma_product_info_from_flash(read_buf, sizeof(read_buf));
#endif

    if (ret == TRUE) {
        APP_DMA_LOG("read license success\n");
        APP_DMA_LOG("product id: %s\n", read_buf);
        APP_DMA_LOG("product key: %s\n", read_buf + DMA_PRODUCT_ID_LEN);
        APP_DMA_LOG("triad id: %s\n", read_buf + DMA_PRODUCT_ID_LEN + DMA_PRODUCT_KEY_LEN);
        APP_DMA_LOG("secret: %s\n", read_buf + DMA_PRODUCT_ID_LEN + DMA_PRODUCT_KEY_LEN + DMA_TRIAD_ID_LEN);
        app_protocol_set_info_group(DMA_HANDLER_ID, read_buf);
    } else {
        app_protocol_set_info_group(DMA_HANDLER_ID, NULL);
    }

#if 1
    u8 mac[] = {0xF4, 0x43, 0x8D, 0x29, 0x17, 0x02};
    u8 ble_mac[6];
    void bt_update_mac_addr(u8 * addr);
    void lmp_hci_write_local_address(const u8 * addr);
    void bt_update_testbox_addr(u8 * addr);
    extern int le_controller_set_mac(void *addr);
    //extern void lib_make_ble_address(u8 * ble_address, u8 * edr_address);
    bt_update_mac_addr(mac);
    lmp_hci_write_local_address(mac);
    bt_update_testbox_addr(mac);
    //lib_make_ble_address(ble_mac, mac);
    le_controller_set_mac(mac); //修改BLE地址
    APP_DMA_DUMP(mac, 6);
    APP_DMA_DUMP(ble_mac, 6);
#endif
}

//*********************************************************************************//
//                                 DMA提示音                                       //
//*********************************************************************************//
const char *dma_notice_tab[APP_RROTOCOL_TONE_MAX] = {
    [APP_PROTOCOL_TONE_CONNECTED_ALL_FINISH]		= TONE_RES_ROOT_PATH"tone/xd_ok.*",//所有连接完成【已连接，你可以按AI键来和我进行对话】
    [APP_PROTOCOL_TONE_PROTOCOL_CONNECTED]		= TONE_RES_ROOT_PATH"tone/xd_con.*",//小度APP已连接，经典蓝牙未连接【请在手机上完成蓝牙配对】
    [APP_PROTOCOL_TONE_CONNECTED_NEED_OPEN_APP]	= TONE_RES_ROOT_PATH"tone/xd_btcon.*",//经典蓝牙已连接，小度app未连接【已配对，请打开小度app进行连接】
    [APP_PROTOCOL_TONE_DISCONNECTED]				= TONE_RES_ROOT_PATH"tone/xd_dis.*",//经典蓝牙已断开【蓝牙已断开，请在手机上完成蓝牙配对】
    [APP_PROTOCOL_TONE_DISCONNECTED_ALL]			= TONE_RES_ROOT_PATH"tone/xd_alldis.*",//经典蓝牙和小度都断开了【蓝牙未连接，请用手机蓝牙和我连接吧】
    [APP_RROTOCOL_TONE_SPEECH_APP_START]	    	= TONE_NORMAL,
    [APP_RROTOCOL_TONE_SPEECH_KEY_START]	    	= TONE_NORMAL,
};

#endif

