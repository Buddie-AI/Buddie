#ifndef __BT_EDR_FUN_H__
#define __BT_EDR_FUN_H__

#include "adapter_odev_bt.h"
#include "app_main.h"

/*配置通话时前面丢掉的数据包包数*/
#define ESCO_DUMP_PACKET_ADJUST	      1	/*配置使能*/
#define ESCO_DUMP_PACKET_DEFAULT	  0
#define ESCO_DUMP_PACKET_CALL		  120 /*0~0xFF*/

#if(TCFG_UI_ENABLE && TCFG_SPI_LCD_ENABLE)//点阵屏断开蓝牙连接时可选择不跳回蓝牙模式
#define BACKGROUND_GOBACK             0   //后台链接是否跳回蓝牙 1：跳回
#else
#define BACKGROUND_GOBACK   		  1
#endif

#define TIMEOUT_CONN_TIME             60 //超时断开之后回连的时间s
#define POWERON_AUTO_CONN_TIME        12  //开机去回连的时间

#define PHONE_CALL_FORCE_POWEROFF     0   //通话时候强制关机

#define SBC_FILTER_TIME_MS			  1000	//后台音频过滤时间ms
#define SBC_ZERO_TIME_MS			  500		//静音多长时间认为已经退出
#define NO_SBC_TIME_MS				  200		//无音频时间ms

#define SNIFF_CNT_TIME                5/////<空闲5S之后进入sniff模式
#define SNIFF_MAX_INTERVALSLOT        800
#define SNIFF_MIN_INTERVALSLOT        100
#define SNIFF_ATTEMPT_SLOT            4
#define SNIFF_TIMEOUT_SLOT            1

enum {
    BT_AS_IDEV = 0,
    BT_AS_ODEV,
};

void set_bt_dev_role(u8 role);

struct app_bt_opr {
    u8 init_ok : 1;		// 1-初始化完成
    u8 call_flag : 1;	// 1-由于蓝牙打电话命令切回蓝牙模式
    u8 exit_flag : 1;	// 1-可以退出蓝牙标志
    u8 exiting : 1;	// 1-正在退出蓝牙模式
    u8 wait_exit : 1;	// 1-等退出蓝牙模式
    u8 a2dp_decoder_type: 3;	// 从后台返回时记录解码格式用
    u8 cmd_flag ;	// 1-由于蓝牙命令切回蓝牙模式
    u8 init_start ;	//蓝牙协议栈初始化开始 ,但未初始化不一定已经完成，要判断initok完成的标志
    u8 ignore_discon_tone;	// 1-退出蓝牙模式， 不响应discon提示音
    u8 sbc_packet_valid_cnt;	// 有效sbc包计数
    u8 sbc_packet_valid_cnt_max;// 最大有效sbc包计数
    u8 sbc_packet_lose_cnt;	// sbc丢失的包数
    u8 sbc_packet_step;	// 0-正常；1-退出中；2-后台
    u32 tws_local_back_role : 4;
    u32 tws_local_to_bt_mode : 1;
    u32 a2dp_start_flag : 1;
    u32 bt_back_flag : 4;
    u32 replay_tone_flag : 1;
    u8 esco_dump_packet;
    u8 last_connecting_addr[6];

    u32 sbc_packet_filter_to;	// 过滤超时
    u32 no_sbc_packet_to;	// 无声超时
    u32 init_ok_time;	// 初始化完成时间
    u32 auto_exit_limit_time;	// 自动退出时间限制
    u8 bt_direct_init;
    u8 bt_close_bredr;
    u8 hid_mode;
    u8 force_poweroff;
    u8 call_back_flag;  //BIT(0):hfp_status BIT(1):esco_status

    int timer;
    int tmr_cnt;
    int back_mode_systime;
    int max_tone_timer_hdl;
    int exit_sniff_timer ;

    u32 sco_info;
};
extern struct app_bt_opr app_bt_hdl;
extern BT_USER_PRIV_VAR bt_user_priv_var;

void bt_init();
void bt_close(void);

void sys_auto_shut_down_enable(void);
void sys_auto_shut_down_disable(void);

u8 get_bt_init_status();
void bt_set_led_status(u8 status);
void sys_auto_sniff_controle(u8 enable, u8 *addr);

void bt_wait_phone_connect_control(u8 enable);
int bt_wait_connect_and_phone_connect_switch(void *p);

u8 is_call_now();

//bt status
int bt_connction_status_event_handler(struct bt_event *bt, struct _odev_bt *odev_bt);

//hci event
int bt_hci_event_handler(struct bt_event *bt, struct _odev_bt *odev_bt);

#endif //__BT_EDR_FUN_H__
