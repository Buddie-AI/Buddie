#ifndef SYS_APP_MSG_H
#define SYS_APP_MSG_H




//app自定义消息发送接口
int app_task_put_usr_msg(int msg, int arg_num, ...);

//app消息获取接口(block参数为0表示内部pend，1直接返回)
void app_task_get_msg(int *msg, int msg_size, int block);

//app按键消息发送接口
int app_task_put_key_msg(int msg, int value);

//app清理按键消息接口
void app_task_clear_key_msg();

#endif

