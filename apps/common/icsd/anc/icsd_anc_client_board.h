
/*********************************************************************

 						 ANC 耳道自适应配置文件

*********************************************************************/

#include "icsd_anc.h"

#define DEVE_BOARD							0x00	//开发板
#define HT03_HYBRID_6F						0x01 	//CUSTOM1_CFG
#define HT03_HYBRID_6G						0x02 	//CUSTOM2_CFG
#define A896_HYBRID_6G						0x03 	//CUSTOM3_CFG
#define HT05_6G               		        0x04 	//CUSTOM4_CFG
#define ANC05_HYBRID						0x05 	//CUSTOM5_CFG
#define G02_HYBRID_6G						0x06 	//CUSTOM6_CFG
#define AC098_HYBRID_6G						0x07 	//CUSTOM7_CFG
#define G96_HYBRID							0x08	//CUSTOM8_CFG
#define H3_HYBRID							0x09	//CUSTOM9_CFG
#define P90_HYBRID							0x0a	//CUSTOM10_CFG
#define JH4006_HYBRID						0x0d	//CUSTOM10_CFG

#define D28_HYBRID                          0x80
#define H3_TOZO_HYBIRD                      0x81

//自适应板级配置，适配不同样机
//#define ADAPTIVE_CLIENT_BOARD	 HT03_HYBRID_6G
//#define ADAPTIVE_CLIENT_BOARD	 G96_HYBRID
//#define ADAPTIVE_CLIENT_BOARD	 P90_HYBRID
//#define ADAPTIVE_CLIENT_BOARD	 G02_HYBRID_6G
//#define ADAPTIVE_CLIENT_BOARD	 JH4006_HYBRID
#define ADAPTIVE_CLIENT_BOARD	 D28_HYBRID
//#define ADAPTIVE_CLIENT_BOARD	 H3_TOZO_HYBIRD
//#define ADAPTIVE_CLIENT_BOARD	 H3_HYBIRD

#if ADAPTIVE_CLIENT_BOARD == HT03_HYBRID_6G
#define ICSD_ANC_MODE     			TWS_TONE_BYPASS_MODE
#define ANC_ADAPTIVE_FF_ORDER				8				/*ANC自适应FF滤波器阶数, 原厂指定*/
#define ANC_ADAPTIVE_FB_ORDER				5				/*ANC自适应FB滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_CMP_ORDER				8				/*ANC自适应CMP滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_RECORD_FF_ORDER		8				/*ANC自适应耳道记忆FF滤波器阶数，原厂指定*/
#define ANC_FB_TRAIN_OFF            	    0
#define ANC_FB_TRAIN_NEW                    0
#define ANC_EARPHONE_CHECK_EN       		1
#define ANC_ADAPTIVE_CMP_EN         		1 				//自适应CMP补偿
#define ANC_ADAPTIVE_TONE_DELAY				700

#elif ADAPTIVE_CLIENT_BOARD == G02_HYBRID_6G
#define ICSD_ANC_MODE     			TWS_TONE_BYPASS_MODE
#define ANC_ADAPTIVE_FF_ORDER				8				/*ANC自适应FF滤波器阶数, 原厂指定*/
#define ANC_ADAPTIVE_FB_ORDER				6				/*ANC自适应FB滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_CMP_ORDER				4				/*ANC自适应CMP滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_RECORD_FF_ORDER		8				/*ANC自适应耳道记忆FF滤波器阶数，原厂指定*/
#define ANC_FB_TRAIN_OFF            	    0
#define ANC_FB_TRAIN_NEW                    0
#define ANC_EARPHONE_CHECK_EN       		0
#define ANC_ADAPTIVE_CMP_EN         		0 				//自适应CMP补偿
#define ANC_ADAPTIVE_TONE_DELAY				700

#elif ADAPTIVE_CLIENT_BOARD == ANC05_HYBRID
#define ICSD_ANC_MODE     			HEADSET_TONES_MODE //HEADSET_TONE_BYPASS_MODE
#define ANC_ADAPTIVE_FF_ORDER				8				/*ANC自适应FF滤波器阶数, 原厂指定*/
#define ANC_ADAPTIVE_FB_ORDER				5				/*ANC自适应FB滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_CMP_ORDER				8				/*ANC自适应CMP滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_RECORD_FF_ORDER		8				/*ANC自适应耳道记忆FF滤波器阶数，原厂指定*/
#define ANC_FB_TRAIN_OFF            	    0
#define ANC_FB_TRAIN_NEW                    0
#define ANC_EARPHONE_CHECK_EN       		0
#define ANC_ADAPTIVE_CMP_EN         		1 				//自适应CMP补偿
#define ANC_ADAPTIVE_TONE_DELAY 			700

#elif ADAPTIVE_CLIENT_BOARD == H3_HYBRID
#define ICSD_ANC_MODE     			HEADSET_TONES_MODE //HEADSET_TONE_BYPASS_MODE
#define ANC_ADAPTIVE_FF_ORDER				8				/*ANC自适应FF滤波器阶数, 原厂指定*/
#define ANC_ADAPTIVE_FB_ORDER				6				/*ANC自适应FB滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_CMP_ORDER				8				/*ANC自适应CMP滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_RECORD_FF_ORDER		8				/*ANC自适应耳道记忆FF滤波器阶数，原厂指定*/
#define ANC_FB_TRAIN_OFF            	    0
#define ANC_FB_TRAIN_NEW                    0
#define ANC_EARPHONE_CHECK_EN       	    1
#define ANC_ADAPTIVE_CMP_EN         		1 				//自适应CMP补偿
#define ANC_ADAPTIVE_TONE_DELAY 			700

#elif ADAPTIVE_CLIENT_BOARD == H3_TOZO_HYBIRD
#define ICSD_ANC_MODE     			HEADSET_TONES_MODE //HEADSET_TONE_BYPASS_MODE
#define ANC_ADAPTIVE_FF_ORDER				8				/*ANC自适应FF滤波器阶数, 原厂指定*/
#define ANC_ADAPTIVE_FB_ORDER				6				/*ANC自适应FB滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_CMP_ORDER				8				/*ANC自适应CMP滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_RECORD_FF_ORDER		8				/*ANC自适应耳道记忆FF滤波器阶数，原厂指定*/
#define ANC_FB_TRAIN_OFF            	    0
#define ANC_FB_TRAIN_NEW                    0
#define ANC_EARPHONE_CHECK_EN       	    1
#define ANC_ADAPTIVE_CMP_EN         		1 				//自适应CMP补偿
#define ANC_ADAPTIVE_TONE_DELAY 			700

#elif ADAPTIVE_CLIENT_BOARD == G96_HYBRID
#define ICSD_ANC_MODE     			TWS_TONE_BYPASS_MODE //HEADSET_TONE_BYPASS_MODE
#define ANC_ADAPTIVE_FF_ORDER				8				/*ANC自适应FF滤波器阶数, 原厂指定*/
#define ANC_ADAPTIVE_FB_ORDER				5				/*ANC自适应FB滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_CMP_ORDER				8				/*ANC自适应CMP滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_RECORD_FF_ORDER		8				/*ANC自适应耳道记忆FF滤波器阶数，原厂指定*/
#define ANC_FB_TRAIN_OFF            	    0
#define ANC_FB_TRAIN_NEW                    1
#define ANC_EARPHONE_CHECK_EN       	    1
#define ANC_ADAPTIVE_CMP_EN         		0 				//自适应CMP补偿
#define ANC_ADAPTIVE_TONE_DELAY 			700

#elif ADAPTIVE_CLIENT_BOARD == P90_HYBRID
//#define ICSD_ANC_MODE     			TWS_TONE_BYPASS_MODE
#define ICSD_ANC_MODE     			TWS_TONE_MODE
#define ANC_ADAPTIVE_FF_ORDER				8				/*ANC自适应FF滤波器阶数, 原厂指定*/
#define ANC_ADAPTIVE_FB_ORDER				5				/*ANC自适应FB滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_CMP_ORDER				8				/*ANC自适应CMP滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_RECORD_FF_ORDER		8				/*ANC自适应耳道记忆FF滤波器阶数，原厂指定*/
#define ANC_FB_TRAIN_OFF            	    1
#define ANC_FB_TRAIN_NEW                    0
#define ANC_EARPHONE_CHECK_EN       	    0
#define ANC_ADAPTIVE_CMP_EN         		1 				//自适应CMP补偿
#define ANC_ADAPTIVE_TONE_DELAY  			1600//700

#elif ADAPTIVE_CLIENT_BOARD == D28_HYBRID
#define ICSD_ANC_MODE     			TWS_TONE_MODE
#define ANC_ADAPTIVE_FF_ORDER				10				/*ANC自适应FF滤波器阶数, 原厂指定*/
#define ANC_ADAPTIVE_FB_ORDER				5				/*ANC自适应FB滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_CMP_ORDER				8				/*ANC自适应CMP滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_RECORD_FF_ORDER		10				/*ANC自适应耳道记忆FF滤波器阶数，原厂指定*/
#define ANC_FB_TRAIN_OFF            	    1
#define ANC_FB_TRAIN_NEW                    0
#define ANC_EARPHONE_CHECK_EN       	    0
#define ANC_ADAPTIVE_CMP_EN         		1 				//自适应CMP补偿
#define ANC_ADAPTIVE_TONE_DELAY  			1600//700

#elif ADAPTIVE_CLIENT_BOARD == JH4006_HYBRID
#define ICSD_ANC_MODE     			HEADSET_TONES_MODE //HEADSET_TONE_BYPASS_MODE
#define ANC_ADAPTIVE_FF_ORDER				8				/*ANC自适应FF滤波器阶数, 原厂指定*/
#define ANC_ADAPTIVE_FB_ORDER				6				/*ANC自适应FB滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_CMP_ORDER				8				/*ANC自适应CMP滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_RECORD_FF_ORDER		8				/*ANC自适应耳道记忆FF滤波器阶数，原厂指定*/
#define ANC_FB_TRAIN_OFF            	    0
#define ANC_FB_TRAIN_NEW                    0
#define ANC_EARPHONE_CHECK_EN       	    0
#define ANC_ADAPTIVE_CMP_EN         		0 				//自适应CMP补偿
#define ANC_ADAPTIVE_TONE_DELAY 			700

#else
#define ICSD_ANC_MODE     			TWS_TONE_BYPASS_MODE
#define ANC_ADAPTIVE_FF_ORDER				8				/*ANC自适应FF滤波器阶数, 原厂指定*/
#define ANC_ADAPTIVE_FB_ORDER				5				/*ANC自适应FB滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_CMP_ORDER				4				/*ANC自适应CMP滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_RECORD_FF_ORDER		8				/*ANC自适应耳道记忆FF滤波器阶数，原厂指定*/
#define ANC_FB_TRAIN_OFF            	    1
#define ANC_FB_TRAIN_NEW                    0
#define ANC_EARPHONE_CHECK_EN       		0
#define ANC_ADAPTIVE_CMP_EN         		0 				//自适应CMP补偿
#define ANC_ADAPTIVE_TONE_DELAY 			700
#endif/*ADAPTIVE_CLIENT_BOARD*/


