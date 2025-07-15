#ifndef _USER_CFG_ID_H_
#define _USER_CFG_ID_H_

//=================================================================================//
//                            与APP CASE相关配置项[1 ~ 60]                         //
//=================================================================================//

#define 	CFG_EARTCH_ENABLE_ID			1
#define 	CFG_PBG_MODE_INFO				2
#define     CFG_CHARGESTORE_TWS_CHANNEL     3
#define		CFG_SYS_VOL                     4

#define		CFG_MIC_ARRAY_DIFF_CMP_VALUE     5//麦克风阵列算法前补偿值
#define		CFG_MIC_ARRAY_TRIM_EN		     6//麦克风补偿使能
#define     CFG_DMS_MALFUNC_STATE_ID         7//dms故障麦克风检测默认使用哪个mic的参数id
#define		CFG_MIC_TARGET_DIFF_CMP 		 8//目标MIC补偿值



#define     CFG_DHA_FITTING_ID              26  //辅听验配保存参数的id
#define     CFG_REMOTE_DN_00                27

#define    	CFG_IMU_GYRO_OFFEST_ID          28	//空间音频imu陀螺仪偏置
#define    	CFG_IMU_ACC_OFFEST_ID           29	//空间音频imu加速度偏置

#define     VM_LP_TOUCH_KEY0_ALOG_CFG       30
#define     VM_LP_TOUCH_KEY1_ALOG_CFG       31
#define     VM_LP_TOUCH_KEY2_ALOG_CFG       32
#define     VM_LP_TOUCH_KEY3_ALOG_CFG       33
#define     VM_LP_TOUCH_KEY4_ALOG_CFG       34

#define     VM_UPDATE_FLAG                  35
#define 	VM_DMA_RAND					    36
#define 	VM_DMA_RAND					    37
#define 	VM_GMA_ALI_PARA				    38
#define 	CFG_BCS_MAP_WEIGHT				39
#define     CFG_RCSP_ADV_HIGH_LOW_VOL       40
#define     CFG_RCSP_ADV_EQ_MODE_SETTING    41
#define     CFG_RCSP_ADV_EQ_DATA_SETTING    42
#define     ADV_SEQ_RAND                    43
#define     CFG_RCSP_ADV_TIME_STAMP         44
#define     CFG_RCSP_ADV_WORK_SETTING       45
#define     CFG_RCSP_ADV_MIC_SETTING        46
#define     CFG_RCSP_ADV_LED_SETTING        47
#define     CFG_RCSP_ADV_KEY_SETTING        48

#define 	CFG_USB_BREAKPOINT0		        49
#define 	CFG_SD0_BREAKPOINT0		        50
#define 	CFG_SD1_BREAKPOINT0		        51

#define 	CFG_HAVE_MASS_STORAGE           52
#define     CFG_MUSIC_MODE                  53

#define		LP_KEY_EARTCH_TRIM_VALUE	    54

#define     CFG_RCSP_ADV_ANC_VOICE          55
#define     CFG_RCSP_ADV_ANC_VOICE_MODE     56
#define     CFG_RCSP_ADV_ANC_VOICE_KEY      57


#define     CFG_SPK_EQ_SEG_SAVE             58
#define     CFG_SPK_EQ_GLOBAL_GAIN_SAVE     59

#define     CFG_ANC_ADAPTIVE_DATA_ID   		60//保存ANC自适应参数id

#define     CFG_REMOTE_DN_00            63

#endif /* #ifndef _USER_CFG_ID_H_ */
