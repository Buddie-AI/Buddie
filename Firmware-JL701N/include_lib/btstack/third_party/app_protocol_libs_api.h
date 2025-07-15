
#ifndef __APP_PRO_LIBS_API_H__
#define __APP_PRO_LIBS_API_H__

#include "typedef.h"


///***************************************///
//GMA的函数接口汇集,begin
int gma_prev_init(void);
int gma_opus_voice_mic_send(uint8_t *voice_buf, uint16_t voice_len);
/*gma的总初始化函数**/
void gma_all_init(void);
/*gma的总的释放函数**/
void gma_all_exit(void);
bool gma_connect_success(void);
void gma_set_active_ali_para(void *addr);
/*TWS的公共地址配置*/
void gma_set_sibling_mac_para(void *mac);
void gma_ble_adv_enable(u8 enable);
void gma_ble_ibeacon_adv(u8 enable);
//gma接收的数据处理函数
int gma_rx_loop(void);
/*gma命令和数据发送处理*/
int tm_data_send_process_thread(void);
int gma_start_voice_recognition(int flag);


int gma_disconnect(void *addr);

void gma_message_callback_register(int (*handler)(int opcode, u8 *data, u32 len));
void gma_is_tws_master_callback_register(bool (*handler)(void));
void gma_tx_resume_register(bool (*handler)(void));
void gma_rx_resume_register(bool (*handler)(void));
/*注册电量的获取回调函数*/
void gma_get_battery_callback_register(bool (*handler)(u8 battery_type, u8 *value));

//ota interface
int gma_ota_requset_next_packet(void *priv, u32 offset, u16 len);
void gma_replay_ota_result(u8 result);

int tws_ota_get_data_from_sibling(u8 opcode, u8 *data, u8 len);
u8 tws_ota_control(int type, ...);
void tws_ota_app_event_deal(u8 evevt);
//gma apis ends

///***************************************///
//DMA apis begin
extern int dueros_process();
extern int dma_all_init(void);
extern int dma_all_exit(void);
extern void dma_message_callback_register(int (*handler)(int id, int opcode, u8 *data, u32 len));
extern void dma_check_status_callback_register(int (*handler)(int state_flag));
extern void dma_tx_resume_register(bool (*handler)(void));
extern void dma_rx_resume_register(bool (*handler)(void));
extern void dma_ble_adv_enable(u8 enable);
extern u16 dma_speech_data_send(u8 *buf, u16 len);
extern int dueros_send_process(void);
extern void dma_set_product_id_key(void *data);
extern bool dma_pair_state();
extern int dma_start_voice_recognition(u8 en);
extern void dueros_dma_manufacturer_info_init();
extern int dma_disconnect(void *addr);
extern int dma_update_tws_state_to_lib(int state);
extern void dma_get_battery_callback_register(bool (*handler)(u8 battery_type, u8 *value));
extern void dma_set_pid(u32 pid);
extern void dma_tws_data_deal(u8 *data, int len);


//DMA apis ends

///***************************************///
//TME apis begins
extern void tme_get_battery_callback_register(bool (*handler)(u8 battery_type, u8 *value));
extern void tme_message_callback_register(int (*handler)(int id, int opcode, const u8 *data, u32 len));
extern void tme_is_tws_master_callback_register(bool (*handler)(void));
extern void tme_tx_resume_register(bool (*handler)(void));
extern void tme_rx_resume_register(bool (*handler)(void));
extern void TME_protocol_process(void *parm);
extern int tme_all_init(void);
extern int tme_all_exit(void);
/* extern u16  tme_speech_data_send(buf, len); */
extern void tme_ble_adv_enable(u8 enable);
extern void TME_send_packet_process(void);
extern void TME_recieve_packet_parse_process(void);
extern bool tme_connect_success(void);
extern int tme_send_voice_data(void *buf, u16 len);
extern int tme_start_voice_recognition(int flag);
extern void tme_set_configuration_info(void *addr);
extern int tme_protocol_disconnect(void *priv);
extern void tme_set_pid(u32 pid);
extern void tme_set_bid(u32 bid);
extern u32 TME_request_ota_data(void *priv, u32 offset, u16 len);
extern void TME_notify_file_size(u32 file_size);

//TME api ends

///***************************************///
//MMA api begins
extern void mma_all_init(void);
extern void mma_all_exit(void);
extern void mma_ble_adv_enable(u8 enable);
extern int XM_speech_data_send(u8 *buf, u16 len);
extern bool XM_protocal_auth_pass(void);
extern int mma_start_voice_recognition(int ctrl);
extern void mma_message_callback_register(int (*handler)(int id, int opcode, u8 *data, u32 len));
extern void mma_is_tws_master_callback_register(bool (*handler)(void));
extern void mma_tx_resume_register(void (*handler)(void));
extern void mma_rx_resume_register(void (*handler)(void));
extern void mma_set_verdor_id(u16 pid);
extern void mma_set_product_id(u16 pid);
extern void mma_set_local_version(u16 version);
extern int mma_protocol_loop_process();
extern u32 mma_request_ota_data(void *priv, u32 offset, u16 len);
extern int mma_notify_file_size(u32 size);
extern u32 mma_report_ota_status(u8 state);
extern int mma_disconnect(void *addr);
extern void mma_tws_data_deal(u8 *data, int len);
extern void mma_get_battery_callback_register(bool (*handler)(u8 battery_type, u8 *value));

//MMA API END

///***************************************///
//GFPS apis begins
extern int gfps_all_init();
extern int gfps_all_exit();
extern int gfps_disconnect(void);
extern int gfps_ble_adv_enable(int enable);
extern void gfps_set_model_id(uint8_t *model_id);
extern void gfps_set_anti_spoofing_public_key(char *public_key);
extern void gfps_set_anti_spoofing_private_key(char *private_key);
extern void gfps_get_battery_callback_register(bool (*handler)(u8 battery_type, u8 *value));
extern void gfps_message_callback_register(int (*handler)(int id, int opcode, u8 *data, u32 len));
extern void gfps_set_pair_mode(void);
extern void gfps_battery_update(void);
extern void gfps_personalized_name_set(u8 *data, u8 len);
extern void gfps_is_tws_master_callback_register(bool (*handler)(void));
extern void gfps_tws_data_deal(u8 *data, int len);
extern int gfps_update_tws_state_to_lib(int state);
extern void gfps_factory_reset(void);
extern void gfps_set_battery_ui_enable(uint8_t enable);
extern void update_channel_map_do_in_irq_flag_set(uint8_t en);
//GFPS API END
#endif /* __APP_PRO_LIBS_API_H__ */


