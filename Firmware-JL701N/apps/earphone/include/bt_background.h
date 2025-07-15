#ifndef BT_BACKGROUND_H
#define BT_BACKGROUND_H


#include "generic/typedef.h"

bool bt_in_background();


void bt_switch_to_foreground(int action, bool exit_curr_app);


int bt_switch_to_background();


void bt_stop_a2dp_slience_detect();


void bt_start_a2dp_slience_detect(int ingore_packet_num);


int bt_background_event_probe_handler(struct bt_event *bt);



















#endif

