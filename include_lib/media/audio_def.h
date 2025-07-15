/*
 *******************************************************************
 *						Audio Common Definitions
 *
 *Note(s):
 *		 (1)Only macro definitions can be defined here.
 *		 (2)Use (1UL << (n)) instead of BIT(n)
 *******************************************************************
 */
#ifndef _AUDIO_DEF_H_
#define _AUDIO_DEF_H_

/*
 *******************************************************************
 *						DAC Definitions
 *******************************************************************
 */

/*
 *******************************************************************
 *						ADC Definitions
 *******************************************************************
 */


/*
 *******************************************************************
 *						Linein(Aux) Definitions
 *******************************************************************
 */


/*
 *******************************************************************
 *						ANC Definitions
 *******************************************************************
 */
//ANC Mode Enable
#define ANC_FF_EN 		 			(1UL << (0))
#define ANC_FB_EN  					(1UL << (1))
#define ANC_HYBRID_EN  			 	(1UL << (2))

//ANC芯片版本定义
#define ANC_VERSION_BR30 			0x01	//AD697N/AC897N
#define ANC_VERSION_BR30C			0x02	//AC699N/AD698N
#define ANC_VERSION_BR36			0x03	//AC700N
#define ANC_VERSION_BR28			0x04	//AC701N/BR40
//多滤波器版本
#define ANC_VERSION_BR28_MULT		0xA4	//AC701N 支持多滤波器

/*
 *******************************************************************
 *						PCM Output Definitions
 *******************************************************************
 */
#define AUDIO_WAY_TYPE_DAC        	(1UL << (0))
#define AUDIO_WAY_TYPE_IIS        	(1UL << (1))
#define AUDIO_WAY_TYPE_FM         	(1UL << (2))
#define AUDIO_WAY_TYPE_HDMI       	(1UL << (3))
#define AUDIO_WAY_TYPE_SPDIF      	(1UL << (4))
#define AUDIO_WAY_TYPE_BT      	 	(1UL << (5))
#define AUDIO_WAY_TYPE_DONGLE		(1UL << (6))

#define AUDIO_WAY_TYPE_MAX			7
#define AUDIO_WAY_TYPE_ALL			0x00ffffff

#endif/*_AUDIO_DEF_H_*/
