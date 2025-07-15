
# make 编译并下载
# make VERBOSE=1 显示编译详细过程
# make clean 清除编译临时文件
#
# 注意： Linux 下编译方式：
#     1. 从 http://pkgman.jieliapp.com/doc/all 处找到下载链接
#     2. 下载后，解压到 /opt/jieli 目录下，保证
#       /opt/jieli/common/bin/clang 存在（注意目录层次）
#     3. 确认 ulimit -n 的结果足够大（建议大于8096），否则链接可能会因为打开文件太多而失败
#       可以通过 ulimit -n 8096 来设置一个较大的值
#

# 工具路径设置
ifeq ($(OS), Windows_NT)
# Windows 下工具链位置
TOOL_DIR := C:/JL/pi32/bin
CC    := clang.exe
CXX   := clang.exe
LD    := pi32v2-lto-wrapper.exe
AR    := llvm-ar.exe
MKDIR := mkdir_win -p
RM    := rm -rf

SYS_LIB_DIR := C:/JL/pi32/pi32v2-lib/r3-large
SYS_INC_DIR := C:/JL/pi32/pi32v2-include
EXT_CFLAGS  := # Windows 下不需要 -D__SHELL__
export PATH:=$(TOOL_DIR);$(PATH)

## 后处理脚本
FIXBAT          := tools\utils\fixbat.exe # 用于处理 utf8->gbk 编码问题
POST_SCRIPT     := cpu/br28/tools/download.bat
RUN_POST_SCRIPT := cpu\br28\tools\download.bat
else
# Linux 下工具链位置
TOOL_DIR := /opt/jieli/pi32v2/bin
CC    := clang
CXX   := clang
LD    := lto-wrapper
AR    := lto-ar
MKDIR := mkdir -p
RM    := rm -rf
export OBJDUMP := $(TOOL_DIR)/objdump
export OBJCOPY := $(TOOL_DIR)/objcopy
export OBJSIZEDUMP := $(TOOL_DIR)/objsizedump

SYS_LIB_DIR := $(TOOL_DIR)/../lib/r3-large
SYS_INC_DIR := $(TOOL_DIR)/../include
EXT_CFLAGS  := -D__SHELL__ # Linux 下需要这个保证正确处理 download.c
export PATH:=$(TOOL_DIR):$(PATH)

## 后处理脚本
FIXBAT          := touch # Linux下不需要处理 bat 编码问题
POST_SCRIPT     := cpu/br28/tools/download.sh
RUN_POST_SCRIPT := bash $(POST_SCRIPT)
endif

CC  := $(TOOL_DIR)/$(CC)
CXX := $(TOOL_DIR)/$(CXX)
LD  := $(TOOL_DIR)/$(LD)
AR  := $(TOOL_DIR)/$(AR)
# 输出文件设置
OUT_ELF   := cpu/br28/tools/sdk.elf
OBJ_FILE  := $(OUT_ELF).objs.txt
# 编译路径设置
BUILD_DIR := objs

# 编译参数设置
CFLAGS := \
	-target pi32v2 \
	-mcpu=r3 \
	-integrated-as \
	-flto \
	-Wuninitialized \
	-Wno-invalid-noreturn \
	-fno-common \
	-integrated-as \
	-Oz \
	-g \
	-flto \
	-fallow-pointer-null \
	-fprefer-gnu-section \
	-Wno-shift-negative-value \
	-Wundef \
	-Wframe-larger-than=256 \
	-Wincompatible-pointer-types \
	-Wreturn-type \
	-Wimplicit-function-declaration \
	-mllvm -pi32v2-large-program=true \
	-fms-extensions \
	-fdiscrete-bitfield-abi \
	-w \


# C++额外的编译参数
CXXFLAGS :=


# 宏定义
DEFINES := \
	-DSUPPORT_MS_EXTENSIONS \
	-DCONFIG_RELEASE_ENABLE \
	-DCONFIG_CPU_BR28 \
	-DCONFIG_USB_SUPPORT_MRX_TX \
	-DCONFIG_PRINT_IN_MASK \
	-DCONFIG_NEW_BREDR_ENABLE \
	-DCONFIG_NEW_MODEM_ENABLE \
	-DCONFIG_NEW_TWS_FORWARD_ENABLE \
	-DCONFIG_UCOS_ENABLE \
	-DCONFIG_EQ_SUPPORT_ASYNC \
	-DEQ_CORE_V1 \
	-DCONFIG_AAC_CODEC_FFT_USE_MUTEX \
	-DCONFIG_DNS_ENABLE \
	-DCONFIG_DMS_MALLOC \
	-DCONFIG_MMU_ENABLE \
	-DCONFIG_SBC_CODEC_HW \
	-DCONFIG_MSBC_CODEC_HW \
	-DCONFIG_AEC_M=128 \
	-DCONFIG_SUPPORT_WIFI_DETECT \
	-DCONFIG_AUDIO_ONCHIP \
	-DCONFIG_MEDIA_NEW_ENABLE \
	-DCONFIG_SUPPORT_EX_TWS_ADJUST \
	-D__GCC_PI32V2__ \
	-DCONFIG_MAC_ADDR_GET_USED_SFC \
	-DTCFG_APP_BT_EN=1 \
	-DEVENT_HANDLER_NUM_CONFIG=2 \
	-DEVENT_TOUCH_ENABLE_CONFIG=0 \
	-DEVENT_POOL_SIZE_CONFIG=256 \
	-DCONFIG_EVENT_KEY_MAP_ENABLE=0 \
	-DTIMER_POOL_NUM_CONFIG=15 \
	-DAPP_ASYNC_POOL_NUM_CONFIG=0 \
	-DUSE_SDFILE_NEW=1 \
	-DSDFILE_STORAGE=0 \
	-DVFS_FILE_POOL_NUM_CONFIG=1 \
	-DFS_VERSION=0x020001 \
	-DFATFS_VERSION=0x020101 \
	-DSDFILE_VERSION=0x020000 \
	-DVFS_ENABLE=1 \
	-DVM_MAX_PAGE_ALIGN_SIZE_CONFIG=16*1024 \
	-DVM_MAX_SECTOR_ALIGN_SIZE_CONFIG=16*1024 \
	-DVM_ITEM_MAX_NUM=256 \
	-DCONFIG_TWS_ENABLE \
	-DCONFIG_EARPHONE_CASE_ENABLE \
	-DCONFIG_OS_AFFINITY_ENABLE \
	-DCONFIG_NEW_CFG_TOOL_ENABLE \
	-DCONFIG_LITE_AEC_ENABLE=0 \
	-DAUDIO_REC_LITE \
	-DAUDIO_DEC_LITE \
	-DAUDIO_REC_POOL_NUM=1 \
	-DAUDIO_DEC_POOL_NUM=3 \
	-DCONFIG_BTCTRLER_TASK_DEL_ENABLE \
	-DCONFIG_LINK_DISTURB_SCAN_ENABLE=0 \
	-DCONFIG_UPDATA_ENABLE \
	-DCONFIG_OTA_UPDATA_ENABLE \
	-DCONFIG_ITEM_FORMAT_VM \


DEFINES += $(EXT_CFLAGS) # 额外的一些定义

# 头文件搜索路径
INCLUDES := \
	-Iinclude_lib \
	-Iinclude_lib/driver \
	-Iinclude_lib/driver/device \
	-Iinclude_lib/driver/cpu/br28 \
	-Iinclude_lib/system \
	-Iinclude_lib/system/generic \
	-Iinclude_lib/system/device \
	-Iinclude_lib/system/fs \
	-Iinclude_lib/system/ui \
	-Iinclude_lib/btctrler \
	-Iinclude_lib/btctrler/port/br28 \
	-Iinclude_lib/update \
	-Iinclude_lib/agreement \
	-Iinclude_lib/btstack/third_party/common \
	-Iinclude_lib/btstack/third_party/rcsp \
	-Iinclude_lib/media/media_new \
	-Iinclude_lib/media/media_new/media \
	-Iinclude_lib/media/media_new/media/cpu/br28 \
	-Iinclude_lib/media/media_new/media/cpu/br28/asm \
	-Iapps/common \
	-Iapps/earphone/include \
	-Iapps/earphone/include/wireless_mic \
	-Iapps/common/icsd/anc \
	-Iapps/common/icsd/adt \
	-Iapps/common/power_manage \
	-Iapps/common/device \
	-Iapps/common/device/ntc \
	-Iapps/common/device/imu_sensor \
	-Iapps/common/audio \
	-Iapps/common/music \
	-Iapps/common/include \
	-Iapps/common/config/include \
	-Iapps/common/dev_manager \
	-Iapps/common/third_party_profile/common \
	-Iapps/common/third_party_profile/interface \
	-Iapps/common/third_party_profile/jieli \
	-Iapps/common/third_party_profile/jieli/trans_data_demo \
	-Iapps/common/third_party_profile/jieli/online_db \
	-Iapps/common/third_party_profile/jieli/JL_rcsp \
	-Iapps/common/third_party_profile/jieli/JL_rcsp/adv_app_setting \
	-Iapps/common/third_party_profile/jieli/JL_rcsp/bt_trans_data \
	-Iapps/common/third_party_profile/jieli/JL_rcsp/adv_rcsp_protocol \
	-Iapps/common/third_party_profile/jieli/JL_rcsp/rcsp_updata \
	-Iapps/earphone/board/br28 \
	-Icpu/br28 \
	-Icpu/br28/audio_dec \
	-Icpu/br28/spatial_effect \
	-Iapps/common/third_party_profile/Tecent_LL/include \
	-Iapps/common/third_party_profile/Tecent_LL/tecent_ll_demo \
	-Iapps/common/cJSON \
	-Iinclude_lib/media \
	-Iinclude_lib/btstack \
	-Iinclude_lib/driver/cpu/br28/asm/power \
	-Iapps/common/third_party_profile/tuya_protocol \
	-Iapps/common/third_party_profile/tuya_protocol/app/demo \
	-Iapps/common/third_party_profile/tuya_protocol/app/product_test \
	-Iapps/common/third_party_profile/tuya_protocol/app/uart_common \
	-Iapps/common/third_party_profile/tuya_protocol/extern_components/mbedtls \
	-Iapps/common/third_party_profile/tuya_protocol/port \
	-Iapps/common/third_party_profile/tuya_protocol/sdk/include \
	-Iapps/common/third_party_profile/tuya_protocol/sdk/lib \
	-Iapps/earphone/tuya \
	-Iapps/common/device/usb \
	-Iapps/common/device/usb/device \
	-Iapps/common/device/usb/host \
	-Iapps/earphone/wireless_mic/bt \
	-Iapps/earphone/wireless_mic/idev \
	-Iapps/earphone/wireless_mic/idev/idev_bt \
	-Iapps/earphone/wireless_mic/idev/idev_bt/include \
	-Iapps/earphone/wireless_mic/idev/idev_mic \
	-Iapps/earphone/wireless_mic/odev \
	-Iapps/earphone/wireless_mic/odev/odev_bt \
	-Iapps/earphone/wireless_mic/odev/odev_bt/odev_edr \
	-Iapps/earphone/wireless_mic/odev/odev_bt/odev_ble \
	-Iapps/earphone/wireless_mic/odev/odev_bt/include \
	-Iapps/earphone/wireless_mic/odev/odev_dac \
	-Iapps/earphone/wireless_mic/process \
	-Iapps/earphone/wireless_mic/audio \
	-Iapps/earphone/wireless_mic/audio/wireless \
	-Iapps/common/ezxml \
	-Icpu/br28/audio_dec/audio_dec_file.h \
	-Iinclude_lib/system/math/cpu/br28 \
	-Iinclude_lib/media/aispeech/asr/include \
	-Iinclude_lib/media/aispeech/enc/include \
	-Icpu/br28/audio_hearing \
	-Iinclude_lib/media/cvp \
	-I$(SYS_INC_DIR) \


# 需要编译的 .c 文件
c_SRC_FILES := \
	apps/common/audio/amplitude_statistic.c \
	apps/common/audio/audio_dvol.c \
	apps/common/audio/audio_export_demo.c \
	apps/common/audio/audio_noise_gate.c \
	apps/common/audio/audio_ns.c \
	apps/common/audio/audio_plc.c \
	apps/common/audio/audio_utils.c \
	apps/common/audio/decode/audio_key_tone.c \
	apps/common/audio/decode/decode.c \
	apps/common/audio/demo/audio_demo.c \
	apps/common/audio/sine_make.c \
	apps/common/audio/uartPcmSender.c \
	apps/common/audio/wm8978/iic.c \
	apps/common/audio/wm8978/wm8978.c \
	apps/common/bt_common/bt_test_api.c \
	apps/common/cJSON/cJSON.c \
	apps/common/config/app_config.c \
	apps/common/config/bt_profile_config.c \
	apps/common/config/ci_transport_uart.c \
	apps/common/config/new_cfg_tool.c \
	apps/common/debug/debug.c \
	apps/common/debug/debug_lite.c \
	apps/common/dev_manager/dev_manager.c \
	apps/common/dev_manager/dev_reg.c \
	apps/common/dev_manager/dev_update.c \
	apps/common/device/gSensor/SC7A20.c \
	apps/common/device/gSensor/STK8321.c \
	apps/common/device/gSensor/da230.c \
	apps/common/device/gSensor/lis2de12.c \
	apps/common/device/gSensor/lis2de12_driver.c \
	apps/common/device/gSensor/gSensor_manage.c \
	apps/common/device/gSensor/mpu6050.c \
	apps/common/device/gx8002_npu/gx8002_enc/gx8002_enc.c \
	apps/common/device/gx8002_npu/gx8002_npu.c \
	apps/common/device/gx8002_npu/gx8002_npu_event_deal.c \
	apps/common/device/gx8002_npu/gx8002_upgrade/app_upgrade/gx_uart_upgrade_app.c \
	apps/common/device/gx8002_npu/gx8002_upgrade/app_upgrade/gx_uart_upgrade_tws.c \
	apps/common/device/gx8002_npu/gx8002_upgrade/gx_uart_upgrade.c \
	apps/common/device/gx8002_npu/gx8002_upgrade/gx_uart_upgrade_porting.c \
	apps/common/device/gx8002_npu/gx8002_upgrade/sdfile_upgrade/gx_uart_upgrade_sdfile.c \
	apps/common/device/gx8002_npu/gx8002_upgrade/spp_upgrade/gx_fifo.c \
	apps/common/device/gx8002_npu/gx8002_upgrade/spp_upgrade/gx_uart_upgrade_spp.c \
	apps/common/device/imu_sensor/icm_42670p/icm_42670p.c \
	apps/common/device/imu_sensor/icm_42670p/inv_imu_apex.c \
	apps/common/device/imu_sensor/icm_42670p/inv_imu_driver.c \
	apps/common/device/imu_sensor/icm_42670p/inv_imu_transport.c \
	apps/common/device/imu_sensor/imuSensor_manage.c \
	apps/common/device/imu_sensor/lsm6dsl/lsm6dsl.c \
	apps/common/device/imu_sensor/mpu6887/mpu6887p.c \
	apps/common/device/imu_sensor/mpu9250/mpu9250.c \
	apps/common/device/imu_sensor/qmi8658/qmi8658c.c \
	apps/common/device/imu_sensor/sh3001/sh3001.c \
	apps/common/device/in_ear_detect/in_ear_detect.c \
	apps/common/device/in_ear_detect/in_ear_manage.c \
	apps/common/device/ir_sensor/ir_manage.c \
	apps/common/device/ir_sensor/jsa1221.c \
	apps/common/device/key/adkey.c \
	apps/common/device/key/adkey_rtcvdd.c \
	apps/common/device/key/ctmu_touch_key.c \
	apps/common/device/key/iokey.c \
	apps/common/device/key/irkey.c \
	apps/common/device/key/key_driver.c \
	apps/common/device/key/touch_key.c \
	apps/common/device/key/uart_key.c \
	apps/common/device/norflash/norflash_sfc.c \
	apps/common/device/ntc/ntc_det.c \
	apps/common/device/usb/device/cdc.c \
	apps/common/device/usb/device/descriptor.c \
	apps/common/device/usb/device/hid.c \
	apps/common/device/usb/device/msd.c \
	apps/common/device/usb/device/msd_upgrade.c \
	apps/common/device/usb/device/task_pc.c \
	apps/common/device/usb/device/uac1.c \
	apps/common/device/usb/device/uac_stream.c \
	apps/common/device/usb/device/usb_device.c \
	apps/common/device/usb/device/user_setup.c \
	apps/common/device/usb/host/adb.c \
	apps/common/device/usb/host/aoa.c \
	apps/common/device/usb/host/audio.c \
	apps/common/device/usb/host/audio_demo.c \
	apps/common/device/usb/host/hid.c \
	apps/common/device/usb/host/usb_bulk_transfer.c \
	apps/common/device/usb/host/usb_ctrl_transfer.c \
	apps/common/device/usb/host/usb_host.c \
	apps/common/device/usb/host/usb_storage.c \
	apps/common/device/usb/usb_config.c \
	apps/common/device/usb/usb_host_config.c \
	apps/common/ezxml/ezxml.c \
	apps/common/ezxml/ezxml_example.c \
	apps/common/fat_nor/cfg_private.c \
	apps/common/fat_nor/virfat_flash.c \
	apps/common/file_operate/file_bs_deal.c \
	apps/common/file_operate/file_manager.c \
	apps/common/icsd/adt/icsd_adt.c \
	apps/common/icsd/adt/icsd_adt_app.c \
	apps/common/icsd/anc/icsd_anc_app.c \
	apps/common/icsd/anc/icsd_anc_board.c \
	apps/common/icsd/anc/icsd_anc_data.c \
	apps/common/jl_kws/jl_kws_algo.c \
	apps/common/jl_kws/jl_kws_audio.c \
	apps/common/jl_kws/jl_kws_event.c \
	apps/common/jl_kws/jl_kws_main.c \
	apps/common/music/breakpoint.c \
	apps/common/music/music_decrypt.c \
	apps/common/music/music_id3.c \
	apps/common/music/music_player.c \
	apps/common/temp_trim/dtemp_pll_trim.c \
	apps/common/test/fs_test.c \
	apps/common/test/os_test.c \
	apps/common/third_party_profile/Tecent_LL/tecent_ll_demo/ll_demo.c \
	apps/common/third_party_profile/Tecent_LL/tecent_ll_demo/ll_task.c \
	apps/common/third_party_profile/Tecent_LL/tecent_protocol/ble_qiot_import.c \
	apps/common/third_party_profile/Tecent_LL/tecent_protocol/ble_qiot_llsync_data.c \
	apps/common/third_party_profile/Tecent_LL/tecent_protocol/ble_qiot_llsync_device.c \
	apps/common/third_party_profile/Tecent_LL/tecent_protocol/ble_qiot_llsync_event.c \
	apps/common/third_party_profile/Tecent_LL/tecent_protocol/ble_qiot_llsync_ota.c \
	apps/common/third_party_profile/Tecent_LL/tecent_protocol/ble_qiot_service.c \
	apps/common/third_party_profile/Tecent_LL/tecent_protocol/ble_qiot_template.c \
	apps/common/third_party_profile/Tecent_LL/tecent_protocol/ble_qiot_utils_base64.c \
	apps/common/third_party_profile/Tecent_LL/tecent_protocol/ble_qiot_utils_crc.c \
	apps/common/third_party_profile/Tecent_LL/tecent_protocol/ble_qiot_utils_hmac.c \
	apps/common/third_party_profile/Tecent_LL/tecent_protocol/ble_qiot_utils_log.c \
	apps/common/third_party_profile/Tecent_LL/tecent_protocol/ble_qiot_utils_md5.c \
	apps/common/third_party_profile/Tecent_LL/tecent_protocol/ble_qiot_utils_sha1.c \
	apps/common/third_party_profile/common/3th_profile_api.c \
	apps/common/third_party_profile/common/custom_cfg.c \
	apps/common/third_party_profile/common/mic_rec.c \
	apps/common/third_party_profile/interface/app_protocol_api.c \
	apps/common/third_party_profile/interface/app_protocol_common.c \
	apps/common/third_party_profile/interface/app_protocol_dma.c \
	apps/common/third_party_profile/interface/app_protocol_gfps.c \
	apps/common/third_party_profile/interface/app_protocol_gma.c \
	apps/common/third_party_profile/interface/app_protocol_mma.c \
	apps/common/third_party_profile/interface/app_protocol_ota.c \
	apps/common/third_party_profile/interface/app_protocol_tme.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_app_setting/adv_adaptive_noise_reduction.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_app_setting/adv_anc_voice.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_app_setting/adv_anc_voice_key.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_app_setting/adv_bt_name_setting.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_app_setting/adv_eq_setting.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_app_setting/adv_hearing_aid_setting.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_app_setting/adv_high_low_vol_setting.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_app_setting/adv_key_setting.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_app_setting/adv_led_setting.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_app_setting/adv_mic_setting.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_app_setting/adv_music_info_setting.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_app_setting/adv_time_stamp_setting.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_app_setting/adv_work_setting.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_rcsp_protocol/rcsp_adv_bluetooth.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_rcsp_protocol/rcsp_adv_customer_user.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_rcsp_protocol/rcsp_adv_opt.c \
	apps/common/third_party_profile/jieli/JL_rcsp/adv_rcsp_protocol/rcsp_adv_tws_sync.c \
	apps/common/third_party_profile/jieli/JL_rcsp/bt_trans_data/le_rcsp_adv_module.c \
	apps/common/third_party_profile/jieli/JL_rcsp/bt_trans_data/rcsp_adv_spp_user.c \
	apps/common/third_party_profile/jieli/JL_rcsp/rcsp_updata/rcsp_adv_user_update.c \
	apps/common/third_party_profile/jieli/JL_rcsp/rcsp_updata/rcsp_ch_loader_download.c \
	apps/common/third_party_profile/jieli/JL_rcsp/rcsp_updata/rcsp_user_update.c \
	apps/common/third_party_profile/jieli/le_hogp.c \
	apps/common/third_party_profile/jieli/online_db/online_db_deal.c \
	apps/common/third_party_profile/jieli/online_db/spp_online_db.c \
	apps/common/third_party_profile/jieli/trans_data_demo/le_trans_data.c \
	apps/common/third_party_profile/jieli/trans_data_demo/spp_trans_data.c \
	apps/common/third_party_profile/jieli/wireless_mic/le_wireless_mic_client.c \
	apps/common/third_party_profile/jieli/wireless_mic/le_wireless_mic_server.c \
	apps/common/third_party_profile/tuya_protocol/app/demo/tuya_ble_app_demo.c \
	apps/common/third_party_profile/tuya_protocol/app/demo/tuya_ota.c \
	apps/common/third_party_profile/tuya_protocol/app/product_test/tuya_ble_app_production_test.c \
	apps/common/third_party_profile/tuya_protocol/app/uart_common/tuya_ble_app_uart_common_handler.c \
	apps/common/third_party_profile/tuya_protocol/extern_components/mbedtls/aes.c \
	apps/common/third_party_profile/tuya_protocol/extern_components/mbedtls/ccm.c \
	apps/common/third_party_profile/tuya_protocol/extern_components/mbedtls/hmac.c \
	apps/common/third_party_profile/tuya_protocol/extern_components/mbedtls/md5.c \
	apps/common/third_party_profile/tuya_protocol/extern_components/mbedtls/sha1.c \
	apps/common/third_party_profile/tuya_protocol/extern_components/mbedtls/sha256.c \
	apps/common/third_party_profile/tuya_protocol/port/tuya_ble_port.c \
	apps/common/third_party_profile/tuya_protocol/port/tuya_ble_port_JL.c \
	apps/common/third_party_profile/tuya_protocol/port/tuya_ble_port_peripheral.c \
	apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_api.c \
	apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_bulk_data.c \
	apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_data_handler.c \
	apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_event.c \
	apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_event_handler.c \
	apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_event_handler_weak.c \
	apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_feature_weather.c \
	apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_gatt_send_queue.c \
	apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_heap.c \
	apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_main.c \
	apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_mem.c \
	apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_mutli_tsf_protocol.c \
	apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_queue.c \
	apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_storage.c \
	apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_unix_time.c \
	apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_utils.c \
	apps/common/ui/lcd_simple/lcd_simple_api.c \
	apps/common/ui/lcd_simple/ui.c \
	apps/common/ui/lcd_simple/ui_mainmenu.c \
	apps/common/update/testbox_update.c \
	apps/common/update/update.c \
	apps/common/update/update_tws.c \
	apps/common/update/update_tws_new.c \
	apps/earphone/aec/br28/audio_aec.c \
	apps/earphone/aec/br28/audio_aec_demo.c \
	apps/earphone/aec/br28/audio_aec_dms.c \
	apps/earphone/aec/br28/audio_aec_online.c \
	apps/earphone/aec/br28/audio_cvp_3mic.c \
	apps/earphone/aec/br28/audio_cvp_ais_3mic.c \
	apps/earphone/app_ancbox.c \
	apps/earphone/app_anctool.c \
	apps/earphone/app_main.c \
	apps/earphone/app_protocol_deal.c \
	apps/earphone/app_task_switch.c \
	apps/earphone/app_testbox.c \
	apps/earphone/audio_enc_mpt_cvp_ctr.c \
	apps/earphone/audio_enc_mpt_self.c \
	apps/earphone/ble_adv.c \
	apps/earphone/board/br28/board_jl7016g_hybrid.c \
	apps/earphone/board/br28/board_jl7018f_demo.c \
	apps/earphone/board/br28/board_jl701n_anc.c \
	apps/earphone/board/br28/board_jl701n_btemitter.c \
	apps/earphone/board/br28/board_jl701n_demo.c \
	apps/earphone/bt_auto_test.c \
	apps/earphone/bt_background.c \
	apps/earphone/bt_ble.c \
	apps/earphone/bt_ble_hid.c \
	apps/earphone/bt_emitter.c \
	apps/earphone/bt_tws.c \
	apps/earphone/default_event_handler.c \
	apps/earphone/earphone.c \
	apps/earphone/eartch_event_deal.c \
	apps/earphone/font/fontinit.c \
	apps/earphone/idle.c \
	apps/earphone/key_event_deal.c \
	apps/earphone/kws_voice_event_deal.c \
	apps/earphone/linein/linein.c \
	apps/earphone/ll_sync_demo/ll_sync_demo.c \
	apps/earphone/log_config/app_config.c \
	apps/earphone/log_config/lib_btctrler_config.c \
	apps/earphone/log_config/lib_btstack_config.c \
	apps/earphone/log_config/lib_driver_config.c \
	apps/earphone/log_config/lib_media_config.c \
	apps/earphone/log_config/lib_system_config.c \
	apps/earphone/log_config/lib_update_config.c \
	apps/earphone/music/sd_music.c \
	apps/earphone/pbg_demo.c \
	apps/earphone/pc/pc.c \
	apps/earphone/power_manage/app_charge.c \
	apps/earphone/power_manage/app_chargestore.c \
	apps/earphone/power_manage/app_power_manage.c \
	apps/earphone/power_manage/app_umidigi_chargestore.c \
	apps/earphone/rcsp/jl_phone_app.c \
	apps/earphone/rcsp/rcsp_adv.c \
	apps/earphone/tone_table.c \
	apps/earphone/trans_data_demo/trans_data_demo.c \
	apps/earphone/tuya/tuya_app.c \
	apps/earphone/tuya/tuya_demo.c \
	apps/earphone/ui/lcd_simple/my_demo.c \
	apps/earphone/ui_manage.c \
	apps/earphone/user_cfg.c \
	apps/earphone/version.c \
	apps/earphone/vol_sync.c \
	apps/earphone/wireless_mic/app_main_wireless_mic.c \
	apps/earphone/wireless_mic/audio/adapter_adc.c \
	apps/earphone/wireless_mic/audio/adapter_media.c \
	apps/earphone/wireless_mic/audio/wireless/adapter_wireless_dec.c \
	apps/earphone/wireless_mic/audio/wireless/adapter_wireless_enc.c \
	apps/earphone/wireless_mic/bt/bt_edr_fun.c \
	apps/earphone/wireless_mic/bt/bt_status_event.c \
	apps/earphone/wireless_mic/idev/adapter_idev.c \
	apps/earphone/wireless_mic/idev/idev_bt/adapter_idev_bt.c \
	apps/earphone/wireless_mic/idev/idev_bt/idev_ble/adapter_idev_ble.c \
	apps/earphone/wireless_mic/idev/idev_mic/adapter_idev_mic.c \
	apps/earphone/wireless_mic/odev/adapter_odev.c \
	apps/earphone/wireless_mic/odev/odev_bt/adapter_odev_bt.c \
	apps/earphone/wireless_mic/odev/odev_bt/odev_ble/adapter_odev_ble.c \
	apps/earphone/wireless_mic/odev/odev_bt/odev_ble/odev_ble_wireless.c \
	apps/earphone/wireless_mic/odev/odev_bt/odev_edr/adapter_odev_edr.c \
	apps/earphone/wireless_mic/odev/odev_dac/adapter_odev_dac.c \
	apps/earphone/wireless_mic/process/adapter_process.c \
	cpu/br28/adc_api.c \
	cpu/br28/aec_tool.c \
	cpu/br28/app_audio.c \
	cpu/br28/audio_anc.c \
	cpu/br28/audio_anc_fade_ctr.c \
	cpu/br28/audio_anc_mult_scene.c \
	cpu/br28/audio_capture.c \
	cpu/br28/audio_codec_clock.c \
	cpu/br28/audio_common/audio_mic_codec.c \
	cpu/br28/audio_common/audio_dec_codec.c \
	cpu/br28/audio_cvp_dut.c \
	cpu/br28/audio_cvp_sync.c \
	cpu/br28/audio_dec.c \
	cpu/br28/audio_dec/audio_dec_file.c \
	cpu/br28/audio_dec/audio_dec_iis.c \
	cpu/br28/audio_dec/audio_dec_pc.c \
	cpu/br28/audio_dec/audio_dec_pcm.c \
	cpu/br28/audio_dec/audio_usb_mic.c \
	cpu/br28/audio_dec_eff.c \
	cpu/br28/audio_demo/audio_adc_demo.c \
	cpu/br28/audio_demo/audio_dac_demo.c \
	cpu/br28/audio_demo/audio_fft_demo.c \
	cpu/br28/audio_demo/audio_matrix_demo.c \
	cpu/br28/audio_demo/audio_wind_detect_demo.c \
	cpu/br28/audio_effect_develop.c \
	cpu/br28/audio_enc.c \
	cpu/br28/audio_general.c \
	cpu/br28/audio_hearing/audio_hearing_aid.c \
	cpu/br28/audio_hearing/audio_hearing_aid_lp.c \
	cpu/br28/audio_hearing/audio_sidetone.c \
	cpu/br28/audio_link.c \
	cpu/br28/audio_sync.c \
	cpu/br28/charge.c \
	cpu/br28/chargestore.c \
	cpu/br28/clock_manager.c \
	cpu/br28/eq_config.c \
	cpu/br28/fft_and_pca.c \
	cpu/br28/hw_fft.c \
	cpu/br28/icsd_anc_user.c \
	cpu/br28/iic_hw.c \
	cpu/br28/iic_soft.c \
	cpu/br28/irflt.c \
	cpu/br28/lp_touch_key.c \
	cpu/br28/lp_touch_key_alog.c \
	cpu/br28/lp_touch_key_tool.c \
	cpu/br28/lua_port_api.c \
	cpu/br28/mcpwm.c \
	cpu/br28/mic_dut_process.c \
	cpu/br28/overlay_code.c \
	cpu/br28/pdm_link.c \
	cpu/br28/plcnt.c \
	cpu/br28/power/power_app.c \
	cpu/br28/power/power_check.c \
	cpu/br28/power/power_port.c \
	cpu/br28/power/power_trim.c \
	cpu/br28/private_iis.c \
	cpu/br28/pwm_led.c \
	cpu/br28/rdec.c \
	cpu/br28/setup.c \
	cpu/br28/smart_voice/aispeech_asr.c \
	cpu/br28/smart_voice/jl_kws_platform.c \
	cpu/br28/smart_voice/kws_event.c \
	cpu/br28/smart_voice/nn_vad.c \
	cpu/br28/smart_voice/smart_voice_config.c \
	cpu/br28/smart_voice/smart_voice_core.c \
	cpu/br28/smart_voice/user_asr.c \
	cpu/br28/smart_voice/vad_clock_trim.c \
	cpu/br28/smart_voice/vad_mic.c \
	cpu/br28/smart_voice/voice_mic_data.c \
	cpu/br28/sound_device.c \
	cpu/br28/spi.c \
	cpu/br28/tone_player.c \
	cpu/br28/tws_audio.c \
	cpu/br28/uart_dev.c \
	cpu/br28/umidigi_chargestore.c \


# 需要编译的 .S 文件
S_SRC_FILES := \
	apps/earphone/sdk_version.z.S \


# 需要编译的 .s 文件
s_SRC_FILES :=


# 需要编译的 .cpp 文件
cpp_SRC_FILES :=


# 需要编译的 .cc 文件
cc_SRC_FILES :=


# 需要编译的 .cxx 文件
cxx_SRC_FILES :=


# 链接参数
LFLAGS := \
	--plugin-opt=-pi32v2-always-use-itblock=false \
	--plugin-opt=-enable-ipra=true \
	--plugin-opt=-pi32v2-merge-max-offset=4096 \
	--plugin-opt=-pi32v2-enable-simd=true \
	--plugin-opt=mcpu=r3 \
	--plugin-opt=-global-merge-on-const \
	--plugin-opt=-inline-threshold=5 \
	--plugin-opt=-inline-max-allocated-size=32 \
	--plugin-opt=-inline-normal-into-special-section=true \
	--plugin-opt=-dont-used-symbol-list=malloc,free,sprintf,printf,puts,putchar \
	--plugin-opt=save-temps \
	--plugin-opt=-pi32v2-enable-rep-memop \
	--plugin-opt=-warn-stack-size=256 \
	--sort-common \
	--plugin-opt=-used-symbol-file=cpu/br28/sdk_used_list.used \
	--plugin-opt=-pi32v2-large-program=true \
	--gc-sections \
	--start-group \
	cpu/br28/liba/cpu.a \
	cpu/br28/liba/system.a \
	cpu/br28/liba/btstack.a \
	cpu/br28/liba/rcsp_stack.a \
	cpu/br28/liba/tme_stack.a \
	cpu/br28/liba/google_fps.a \
	cpu/br28/liba/btctrler.a \
	cpu/br28/liba/aec.a \
	cpu/br28/liba/lib_icsd_anc.a \
	cpu/br28/liba/media.a \
	cpu/br28/liba/libepmotion.a \
	cpu/br28/liba/libAptFilt_pi32v2_OnChip.a \
	cpu/br28/liba/libEchoSuppress_pi32v2_OnChip.a \
	cpu/br28/liba/libNoiseSuppress_pi32v2_OnChip.a \
	cpu/br28/liba/libSplittingFilter_pi32v2_OnChip.a \
	cpu/br28/liba/libDelayEstimate_pi32v2_OnChip.a \
	cpu/br28/liba/libDualMicSystem_pi32v2_OnChip.a \
	cpu/br28/liba/libDualMicSystem_flexible_pi32v2_OnChip.a \
	cpu/br28/liba/libAdaptiveEchoSuppress_pi32v2_OnChip.a \
	cpu/br28/liba/libOpcore_maskrom_pi32v2_OnChip.a \
	cpu/br28/liba/lib_esco_repair.a \
	cpu/br28/liba/noisegate.a \
	cpu/br28/liba/sbc_eng_lib.a \
	cpu/br28/liba/mp3_dec_lib.a \
	cpu/br28/liba/mp3_decstream_lib.a \
	cpu/br28/liba/mp3tsy_dec_lib.a \
	cpu/br28/liba/wav_dec_lib.a \
	cpu/br28/liba/flac_dec_lib.a \
	cpu/br28/liba/m4a_dec_lib.a \
	cpu/br28/liba/ape_dec_lib.a \
	cpu/br28/liba/wma_dec_lib.a \
	cpu/br28/liba/amr_dec_lib.a \
	cpu/br28/liba/dts_dec_lib.a \
	cpu/br28/liba/agreement.a \
	cpu/br28/liba/media_app.a \
	cpu/br28/liba/lc3_codec_lib.a \
	cpu/br28/liba/opus_dec_lib.a \
	cpu/br28/liba/speex_enc_lib.a \
	cpu/br28/liba/ldac_dec_lib.a \
	cpu/br28/liba/opus_enc_lib.a \
	cpu/br28/liba/bt_hash_enc.a \
	cpu/br28/liba/libjlsp.a \
	cpu/br28/liba/libjlsp_kws.a \
	cpu/br28/liba/lfaudio_plc_lib.a \
	cpu/br28/liba/drc.a \
	cpu/br28/liba/crossover_coff_old.a \
	cpu/br28/liba/lib_point360_td.a \
	cpu/br28/liba/lib_resample_cal.a \
	cpu/br28/liba/lib_resample_fast_cal.a \
	cpu/br28/liba/SpatialAudio.a \
	cpu/br28/liba/SensorCalib.a \
	cpu/br28/liba/lib_pitchshifter.a \
	cpu/br28/liba/VirtualBass.a \
	cpu/br28/liba/lib_sur_cal.a \
	cpu/br28/liba/limiter_noiseGate.a \
	cpu/br28/liba/howling.a \
	cpu/br28/liba/lib_howlings_phf.a \
	cpu/br28/liba/libllns.a \
	cpu/br28/liba/libkwscommon.a \
	apps/common/third_party_profile/tuya_protocol/sdk/lib/libtuya_lib.a \
	cpu/br28/liba/lib_icsd_adt.a \
	cpu/br28/liba/lib_diafx.a \
	cpu/br28/liba/libFFT_pi32v2_OnChip.a \
	cpu/br28/liba/wtg_dec_lib.a \
	cpu/br28/liba/bfilterfun_lib.a \
	cpu/br28/liba/aac_dec_lib.a \
	cpu/br28/liba/crypto_toolbox_Osize.a \
	cpu/br28/liba/lib_dns.a \
	cpu/br28/liba/update.a \
	cpu/br28/liba/compressor.a \
	cpu/br28/liba/limiter.a \
	--end-group \
	-Tcpu/br28/sdk.ld \
	-M=cpu/br28/tools/sdk.map \
	--plugin-opt=mcpu=r3 \
	--plugin-opt=-mattr=+fprev1 \


LIBPATHS := \
	-L$(SYS_LIB_DIR) \


LIBS := \
	$(SYS_LIB_DIR)/libm.a \
	$(SYS_LIB_DIR)/libc.a \
	$(SYS_LIB_DIR)/libm.a \
	$(SYS_LIB_DIR)/libcompiler-rt.a \



c_OBJS    := $(c_SRC_FILES:%.c=%.c.o)
S_OBJS    := $(S_SRC_FILES:%.S=%.S.o)
s_OBJS    := $(s_SRC_FILES:%.s=%.s.o)
cpp_OBJS  := $(cpp_SRC_FILES:%.cpp=%.cpp.o)
cxx_OBJS  := $(cxx_SRC_FILES:%.cxx=%.cxx.o)
cc_OBJS   := $(cc_SRC_FILES:%.cc=%.cc.o)

OBJS      := $(c_OBJS) $(S_OBJS) $(s_OBJS) $(cpp_OBJS) $(cxx_OBJS) $(cc_OBJS)
DEP_FILES := $(OBJS:%.o=%.d)


OBJS      := $(addprefix $(BUILD_DIR)/, $(OBJS))
DEP_FILES := $(addprefix $(BUILD_DIR)/, $(DEP_FILES))


VERBOSE ?= 0
ifeq ($(VERBOSE), 1)
QUITE :=
else
QUITE := @
endif

# 一些旧的 make 不支持 file 函数，需要 make 的时候指定 LINK_AT=0 make
LINK_AT ?= 1

# 表示下面的不是一个文件的名字，无论是否存在 all, clean, pre_build 这样的文件
# 还是要执行命令
# see: https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean pre_build

# 不要使用 make 预设置的规则
# see: https://www.gnu.org/software/make/manual/html_node/Suffix-Rules.html
.SUFFIXES:

all: pre_build $(OUT_ELF)
	$(info +POST-BUILD)
	$(QUITE) $(RUN_POST_SCRIPT) sdk

pre_build:
	$(info +PRE-BUILD)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P cpu/br28/sdk_used_list.c -o cpu/br28/sdk_used_list.used
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P cpu/br28/sdk_ld.c -o cpu/br28/sdk.ld
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P cpu/br28/tools/download.c -o $(POST_SCRIPT)
	$(QUITE) $(FIXBAT) $(POST_SCRIPT)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P cpu/br28/tools/isd_config_rule.c -o cpu/br28/tools/isd_config.ini

clean:
	$(QUITE) $(RM) $(OUT_ELF)
	$(QUITE) $(RM) $(BUILD_DIR)



ifeq ($(LINK_AT), 1)
$(OUT_ELF): $(OBJS)
	$(info +LINK $@)
	$(shell $(MKDIR) $(@D))
	$(file >$(OBJ_FILE), $(OBJS))
	$(QUITE) $(LD) -o $(OUT_ELF) @$(OBJ_FILE) $(LFLAGS) $(LIBPATHS) $(LIBS)
else
$(OUT_ELF): $(OBJS)
	$(info +LINK $@)
	$(shell $(MKDIR) $(@D))
	$(QUITE) $(LD) -o $(OUT_ELF) $(OBJS) $(LFLAGS) $(LIBPATHS) $(LIBS)
endif


$(BUILD_DIR)/%.c.o : %.c
	$(info +CC $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.S.o : %.S
	$(info +AS $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.s.o : %.s
	$(info +AS $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.cpp.o : %.cpp
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.cxx.o : %.cxx
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.cc.o : %.cc
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

-include $(DEP_FILES)
