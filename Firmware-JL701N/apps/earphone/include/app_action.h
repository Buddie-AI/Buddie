#ifndef APP_ACTION_H
#define APP_ACTION_H


#define ACTION_EARPHONE_MAIN    0x0001
#define ACTION_A2DP_START       0x0002
#define ACTION_ESCO_START       0x0003
#define ACTION_BY_KEY_MODE      0x0004

#define ACTION_IDLE_MAIN    	0x0010
#define ACTION_IDLE_POWER_OFF   0x0011

#define ACTION_MUSIC_MAIN       0x0020
#define ACTION_MUSIC_TWS_RX     0x0021

#define ACTION_PC_MAIN          0x0030

#define ACTION_AUX_MAIN         0x0040

#define APP_NAME_BT										"earphone"
#define APP_NAME_IDLE									"idle"
#define APP_NAME_PC										"pc"
#define APP_NAME_MUSIC									"music"
#define APP_NAME_AUX									"aux"

extern void task_switch(const char *name, int action);
#define task_switch_to_bt()     task_switch(APP_NAME_BT, ACTION_EARPHONE_MAIN)


#endif

