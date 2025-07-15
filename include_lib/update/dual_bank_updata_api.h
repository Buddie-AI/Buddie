#ifndef _DUAL_BANK_API_H_
#define _DUAL_BANK_API_H_

/* @brief:Api for getting the buffer size for temporary storage
 */
u32 get_dual_bank_passive_update_max_buf(void);

/* @brief:Initializes the update task,and setting the crc value and file size of new fw;
 * @param fw_crc:crc value of new fw file
 * @param fw_size:total size of new fw file
 * @param priv:reserved
 * @param max_ptk_len: Supported maxium length of every programming,it decides the max size of programming every time
 */
u32 dual_bank_passive_update_init(u32 fw_crc, u32 fw_size, u16 max_pkt_len, void *priv);

/* @brief:exit the update task
 * @param priv:reserved
 */
u32 dual_bank_passive_update_exit(void *priv);

/* @brief:Judge whether enough space for new fw file
 * @note: it should be called after dual_bank_passive_update_init(...);
 * @param fw_size:fw size of new fw file
 */
u32 dual_bank_update_allow_check(u32 fw_size);


/* @brief:copy the data to temporary buffer and notify task to write non-volatile storage
 * @param data:the pointer to download data
 * @param len:the length to download data
 * @param write_complete_cb:callback for programming done,return 0 if no err occurred
*/
u32 dual_bank_update_write(void *data, u16 len, int (*write_complete_cb)(void *priv));

/* @brief: caculate all the data had flashed,and compare with the cre value intializeed when update init;
 * @crc_init_hdl:if it equals NULL,use internal implementation(CRC16-CCITT Standard);otherwise,use user's customization;
 * @crc_calc_hdl:if it equals NULL,use internal implementation(CRC16-CCITT Standard);otherwise,use user's customization;
 * @verify_result_hdl:when the verification completed,this callback for result notification;
 *                    if crc_res equals 1,crc verification passed,if 0,the verification failed.
*/
u32 dual_bank_update_verify(void (*crc_init_hdl)(void), u32(*crc_calc_hdl)(u32 init_crc, u8 *data, u32 len), int (*verify_result_hdl)(int crc_res));


/* @brief:After the new fw verification succeed,call this api to program the new boot info for new fw
 * @param burn_boot_info_result_hdl:this callback for error notification
 *                                  if err equals 0,the operate to burn boot info succeed,other value means to fail.
 */
u32 dual_bank_update_burn_boot_info(int (*burn_boot_info_result_hdl)(int err));

enum {

    CLEAR_APP_RUNNING_BANK = 0,
    CLEAR_APP_UPDATE_BANK,
};

/* @brief:this api for erasing the boot info of specific bank,it should be called much carefully
 * @param type:it decides which bank's boot info would be erased;
 *             clean the boot info of running bank and call system_reset,system will run the other bank if available;
 */
int flash_update_clr_boot_info(u8 type);

/* @brief:this api for user read flash data to calculate crc
 * @param offset: the offset relative to update area
          read_buf: user data buffer
          read_len: read length
   @returns: Actual read length
 */
u32 dual_bank_update_read_data(u32 offset, u8 *read_buf, u32 read_len);

#endif

