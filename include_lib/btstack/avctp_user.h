#ifndef __AVCTP_USER_H__
#define __AVCTP_USER_H__


#include "typedef.h"
#include "btstack_typedef.h"



///***注意：该文件的枚举与库编译密切相关，主要是给用户提供调用所用。用户不能自己在中间添加值。*/
////----user (command) codes----////
typedef enum {
    /*
    使用user_send_cmd_prepare(USER_CMD_TYPE cmd,u16 param_len,u8 *param)发送命令
    //返回0表支持参数个数正确，返回1表不支持，2是参数错误
    要三个参数，没参数说明的命令参数param_len传0，param传NULL
    例子A、USER_CTRL_HFP_CALL_SET_VOLUME命令需要1个参数的使用例子：
    u8 vol = 8;
    user_send_cmd_prepare(USER_CTRL_HFP_CALL_SET_VOLUME,1, &vol);

    例子B、USER_CTRL_DIAL_NUMBER 参数要用数组先存起来，param_len是号码长度，param可传参数数组指针，
    user_val->income_phone_num已经存好号码
    user_send_cmd_prepare(USER_CTRL_DIAL_NUMBER,user_val->phone_num_len,user_val->income_phone_num);

    */

    //链路操作部分
    //回连,使用的是VM的地址，一般按键操作不使用该接口
    USER_CTRL_START_CONNECTION,
    //通过地址去连接，如果知道地址想去连接使用该接口
    USER_CTRL_START_CONNEC_VIA_ADDR,
    //通过指定地址手动回连，该地址是最后一个断开设备的地址
    USER_CTRL_START_CONNEC_VIA_ADDR_MANUALLY,
    //通过地址去连接spp，如果知道地址想去连接使用该接口
    USER_CTRL_START_CONNEC_SPP_VIA_ADDR,

    //断开连接，断开当前所有蓝牙连接
    USER_CTRL_DISCONNECTION_HCI,

    //取消链接
    USER_CTRL_CONNECTION_CANCEL,

    //读取远端名字
    USER_CTRL_READ_REMOTE_NAME,
    //连接或断开SCO或esco,选择这个命令会自动判断要断开还是连接sco
    USER_CTRL_PAUSE_MUSIC,
    //连接或断开SCO或esco,选择这个命令会自动判断要断开还是连接sco
    USER_CTRL_SCO_LINK,
    //连接SCO或esco
    USER_CTRL_CONN_SCO,
    //断开sco或esco
    USER_CTRL_DISCONN_SCO,
    //断开SDP，一般按键操作不使用该接口
    USER_CTRL_DISCONN_SDP_MASTER,

    //关闭蓝牙可发现
    USER_CTRL_WRITE_SCAN_DISABLE,
    //打开蓝牙可发现
    USER_CTRL_WRITE_SCAN_ENABLE,
//   USER_CTRL_WRITE_SCAN_ENABLE_KEY   ,
    //关闭蓝牙可连接
    USER_CTRL_WRITE_CONN_DISABLE,
    //打开蓝牙可连接
    USER_CTRL_WRITE_CONN_ENABLE,
    //  USER_CTRL_WRITE_CONN_ENABLE_KEY     ,
    //控制蓝牙搜索，需要搜索附件设备做功能的连续说明情况在补充完善功能
    USER_CTRL_SEARCH_DEVICE,
    //取消搜索
    USER_CTRL_INQUIRY_CANCEL,
    //取消配对
    USER_CTRL_PAGE_CANCEL,
    ///进入sniff模式，一般按键操作不使用该接口
    USER_CTRL_SNIFF_IN,
    USER_CTRL_SNIFF_EXIT,
    USER_CTRL_ALL_SNIFF_EXIT,

    //hfp链路部分
    //控制打电话音量，注意可能有些手机进度条有变化音量大小没变化，同步要设置样机DAC音量
    /*跟电话音量操作有关的操作最终都执行回调函数call_vol_change*/
    USER_CTRL_HFP_CMD_BEGIN,           /* 接口扩展用来做HFP 连接 */
    USER_CTRL_HFP_CALL_VOLUME_UP,       /*音量加1，手机可以同步显示*/
    USER_CTRL_HFP_CALL_VOLUME_DOWN,      /*音量减1，手机可以同步显示*/
    USER_CTRL_HFP_CALL_SET_VOLUME,   /*设置固定值，手机可以同步显示，需要传1个音量值*/
    USER_CTRL_HFP_CALL_GET_VOLUME,  /*获取音量，默认从call_vol_change返回*/

    //来电接听电话
    USER_CTRL_HFP_CALL_ANSWER,
    //挂断电话
    USER_CTRL_HFP_CALL_HANGUP,
    //回拨上一个打出电话
    USER_CTRL_HFP_CALL_LAST_NO,
    //获取当前通话电话号码
    USER_CTRL_HFP_CALL_CURRENT,
    //通话过程中根据提示输入控制
    /*例子
    char num = '1';
    user_send_cmd_prepare(USER_CTRL_HFP_DTMF_TONES,1,(u8 *)&num);
    */
    //发送打电话时的信号选择DTMF tones ,有一个参数，参数支持{0-9, *, #, A, B, C, D}
    USER_CTRL_HFP_DTMF_TONES,
    //根据电话号码拨号
    /**USER_CTRL_DIAL_NUMBER命令有参数，参数要用数组先存起来，
    param_len是号码长度，param可传参数数组指针*/
    USER_CTRL_DIAL_NUMBER,
    //发送电量  /**要连接上HFP才有用*/
    USER_CTRL_SEND_BATTERY,
    //*控制siri状态*//*可以注册回调函数获取返回值*/
    USER_CTRL_HFP_GET_SIRI_STATUS,
    //*开启siri*/
    USER_CTRL_HFP_GET_SIRI_OPEN,
    //*关闭siri,一般说完话好像自动关闭了,如果要提前终止可调用*/
    USER_CTRL_HFP_GET_SIRI_CLOSE,
    /*获取手机的日期和时间，苹果可以，一般安卓机好像都不行*/
    USER_CTRL_HFP_GET_PHONE_DATE_TIME,
    USER_CTRL_HFP_CMD_SEND_BIA,
    /*获取手机厂商的命令 */
    USER_CTRL_HFP_CMD_GET_MANUFACTURER,
    /*更新当前的电量给手机*/
    USER_CTRL_HFP_CMD_UPDATE_BATTARY,
    //三方通话操作
    //应答
    USER_CTRL_HFP_THREE_WAY_ANSWER1,     //挂断当前去听另一个（未接听或者在保留状态都可以）
    USER_CTRL_HFP_THREE_WAY_ANSWER2,     //保留当前去接听, 或者用于两个通话的切换
    USER_CTRL_HFP_THREE_WAY_ANSWER1X,    //目前没有用
    USER_CTRL_HFP_THREE_WAY_ANSWER2X,    //目前没有用
    USER_CTRL_HFP_THREE_WAY_ANSWER3,   //可以发完USER_CTRL_HFP_THREE_WAY_ANSWER2,又发ANSWER3，自己看看效果
    //拒听
    USER_CTRL_HFP_THREE_WAY_REJECT,           //拒绝后台来电
    USER_CTRL_HFP_DISCONNECT,                   //断开HFP连接
    USER_CTRL_HFP_CMD_END,

    //音乐控制部分
    USER_CTRL_AVCTP_CMD_BEGIN,
    //音乐播放
    USER_CTRL_AVCTP_OPID_PLAY,
    //音乐暂停
    USER_CTRL_AVCTP_OPID_PAUSE,
    //音乐停止
    USER_CTRL_AVCTP_OPID_STOP,
    //音乐下一首
    USER_CTRL_AVCTP_OPID_NEXT,
    //音乐上一首
    USER_CTRL_AVCTP_OPID_PREV,
    //音乐快进
    USER_CTRL_AVCTP_OPID_FORWARD,
    //音乐快退
    USER_CTRL_AVCTP_OPID_REWIND,
    //音乐循环模式
    USER_CTRL_AVCTP_OPID_REPEAT_MODE,
    USER_CTRL_AVCTP_OPID_SHUFFLE_MODE,
    //获取播放歌曲总时间和当前时间接口
    USER_CTRL_AVCTP_OPID_GET_PLAY_TIME,

    //同步音量接口
    USER_CTRL_AVCTP_OPID_SEND_VOL,
//    //AVCTP断开，是音乐控制链路，一般不使用
    USER_CTRL_AVCTP_DISCONNECT,
//    //AVCTP连接，是音乐控制链路，一般不使用
    USER_CTRL_AVCTP_CONN,

    USER_CTRL_AVCTP_CMD_END,

    //高级音频部分
    USER_CTRL_A2DP_CMD_BEGIN,
    //有判断条件的，回连过程连接高级音频，避免手机连也自动发起连接，一般按键操作不使用该接口
    USER_CTRL_AUTO_CONN_A2DP,
    //连接高级音频，回来最后一个断开设备的地址
    USER_CTRL_CONN_A2DP,
    //断开高级音频，只断开高级音频链路，如果有电话还会保留
    USER_CTRL_DISCONN_A2DP,
    //maybe BQB test will use
    USER_CTRL_A2DP_CMD_START					,
    USER_CTRL_A2DP_CMD_CLOSE				,
    USER_CTRL_A2DP_CMD_SUSPEND					,
    USER_CTRL_A2DP_CMD_GET_CONFIGURATION		,
    USER_CTRL_A2DP_CMD_ABORT					,
    USER_CTRL_A2DP_CMD_END,
    //蓝牙关闭
    USER_CTRL_POWER_OFF,
    //蓝牙开启
    USER_CTRL_POWER_ON,
    ///*hid操作定义*/
    USER_CTRL_HID_CMD_BEGIN,
    //按键连接
    USER_CTRL_HID_CONN,
//    //只发一个按键，安卓手机使用
    USER_CTRL_HID_ANDROID,
    //只发一个按键，苹果和部分安卓手机适用
    USER_CTRL_HID_IOS,
//    //发两个拍照按键
    USER_CTRL_HID_BOTH,
    //HID断开
    USER_CTRL_HID_DISCONNECT,
    //Home Key,apply to IOS and Android
    USER_CTRL_HID_HOME				,
    //Return Key,only support Android
    USER_CTRL_HID_RETURN			,
    //LeftArrow Key
    USER_CTRL_HID_LEFTARROW			,
    //RightArrow Key
    USER_CTRL_HID_RIGHTARROW		,
    //Volume Up
    USER_CTRL_HID_VOL_UP			,
    //Volume Down
    USER_CTRL_HID_VOL_DOWN			,

    USER_CTRL_HID_SEND_DATA			,

    USER_CTRL_HID_CMD_END,

    /**有TWS命名的都不用了*/
    /*对箱操作命令*/
    USER_CTRL_TWS_CMD_BEGIN,
    USER_CTRL_SYNC_TRAIN,
    USER_CTRL_SYNC_TRAIN_SCAN,
    USER_CTRL_MONITOR,
    USER_CTRL_TWS_CONNEC_VIA_ADDR,
    USER_CTRL_TWS_COTROL_CDM,
    //清除对箱连接信息
    USER_CTRL_TWS_CLEAR_INFO,
    //断开对箱连接
    USER_CTRL_TWS_DISCONNECTION_HCI,
    //发起对箱连接
    USER_CTRL_TWS_START_CONNECTION,
    USER_CTRL_TWS_SYNC_CDM,
    USER_CTRL_TWS_SYNC_SBC_CDM,
    USER_CTRL_TWS_RESTART_SBC_CDM,
    USER_CTRL_SYNC_TRAIN_CANCEL,
    USER_CTRL_SYNC_TRAIN_SCAN_CANCEL,
    USER_CTRL_TWS_SYNC_CDM_FUN,
    USER_CTRL_TWS_LINEIN_START,
    USER_CTRL_TWS_LINEIN_CLOSE,
    USER_CTRL_TWS_CMD_END,

    ///蓝牙串口发送命令
    USER_CTRL_SPP_CMD_BEGIN,
    /**USER_CTRL_SPP_SEND_DATA命令有参数，参数会先存起来，
    param_len是数据长度，param发送数据指针
    返回0,表示准备成功，会PENDing发完才返回
    3表示上一包数据没发完，*/
    USER_CTRL_SPP_SEND_DATA, //len <= 512
    USER_CTRL_SPP_TRY_SEND_DATA,//
    USER_CTRL_SPP_UPDATA_DATA,
    //serial port profile disconnect command
    USER_CTRL_SPP_DISCONNECT,
    USER_CTRL_SPP_CMD_END,


///pbg发送命令
    USER_CTRL_PBG_CMD_BEGIN,
    USER_CTRL_PBG_SEND_DATA,//len <= 512
    USER_CTRL_PBG_TRY_SEND_DATA,//
    USER_CTRL_PBG_CMD_END,

///adt 发送命令
    USER_CTRL_ADT_CMD_BEGIN,
    USER_CTRL_ADT_CONNECT,
    USER_CTRL_ADT_KEY_MIC_OPEN,
    USER_CTRL_ADT_SEND_DATA,//len <= 512
    USER_CTRL_ADT_TRY_SEND_DATA,//
    USER_CTRL_ADT_CMD_END,

    ///蓝牙电话本功能发送命令
    USER_CTRL_PBAP_CMD_BEGIN,
    //电话本功能读取通话记录的前n条
    USER_CTRL_PBAP_READ_PART,
    //电话本功能读全部记录
    USER_CTRL_PBAP_READ_ALL,
    //电话本功能中断读取记录
    USER_CTRL_PBAP_STOP_READING,

    USER_CTRL_PBAP_CMD_END,


    //蓝牙其他操作
//    //删除最新的一个设备记忆
//    USER_CTRL_DEL_LAST_REMOTE_INFO   ,
//    //删除所有设备记忆
    USER_CTRL_DEL_ALL_REMOTE_INFO,
    USER_CTRL_TEST_KEY,
    USER_CTRL_SEND_USER_INFO,

    USER_CTRL_KEYPRESS,
    USER_CTRL_PAIR,
    USER_CTRL_AFH_CHANNEL,
    USER_CTRL_HALF_SEC_LOOP_CREATE,
    USER_CTRL_HALF_SEC_LOOP_DEL,
    /**音量同步接口，自动选择通过HID还是AVRCP来发送*/
    USER_CTRL_CMD_SYNC_VOL_INC,
    /**音量同步接口，自动选择通过HID还是AVRCP来发送*/
    USER_CTRL_CMD_SYNC_VOL_DEC,
    /*单独HID和普通蓝牙模式的切换接口,音箱SDK才有完整流程*/
    USER_CTRL_CMD_CHANGE_PROFILE_MODE,
    USER_CTRL_CMD_RESERVE_INDEX4,
    USER_CTRL_CMD_RESUME_STACK,
    //获取当前音乐的一些信息
    USER_CTRL_AVCTP_OPID_GET_MUSIC_INFO,

    //MAP功能发送命令
    USER_CTRL_MAP_CMD_BEGIN,
    //MAP读取时间
    USER_CTRL_MAP_READ_TIME,
    //MAP读取未读短信
    USER_CTRL_MAP_READ_INDOX,
    //MAP读取已读短信
    USER_CTRL_MAP_READ_OUTDOX,
    //MAP读取已发读短信
    USER_CTRL_MAP_READ_SENT,
    //MAP读取删除短信
    USER_CTRL_MAP_READ_DELETED,
    //MAP读取草稿箱短信
    USER_CTRL_MAP_READ_DRAFT,
    //MAP停止读取
    USER_CTRL_MAP_STOP_READING,
    USER_CTRL_MAP_CMD_END,
    //不看协议栈的状态去调一次exit，关机的时候可能消息被阻塞住推上来导致SNIFF无法退出，影响后续的关机流程
    USER_CTRL_ALL_SNIFF_EXIT_BY_POWEROFF,

    // 一拖二通话操作
    USER_CTRL_MULTI_HFP_CMD_BEGIN,
    // 设备间通话切换
    USER_CTRL_MULTI_ESCO_SWITCH,
    // 挂断打断设备通话
    USER_CTRL_MULTI_HFP_CALL_HANGUP_BREAK,
    //挂断当前通话，接听来电设备
    USER_CTRL_MULTI_HFP_CALL_HANGUP_BREAK_ACTIVE,
    //前台设备三方通话，挂断当前并接听
    USER_CTRL_MULTI_HFP_THREE_WAY_ANSWER1,
    //前台设备三方通话，保留当前并接听
    USER_CTRL_MULTI_HFP_THREE_WAY_ANSWER2,
    // 被打断设备三方通话，挂断当前接通来电
    USER_CTRL_MULTI_HFP_THREE_WAY_ANSWER1_BREAK,
    // 被打断设备三方通话，挂断当前接通来电并切换到耳机通话
    USER_CTRL_MULTI_HFP_THREE_WAY_ANSWER1_BREAK_SWITCH,
    // 被打断设备三方通话，保留当前接通来电
    USER_CTRL_MULTI_HFP_THREE_WAY_ANSWER2_BREAK,
    // 被打断设备三方通话，保留当前接通来电并切换到耳机通话
    USER_CTRL_MULTI_HFP_THREE_WAY_ANSWER2_BREAK_SWITCH,
    // 被打断设备三方通话，拒绝后台来电
    USER_CTRL_MULTI_HFP_THREE_WAY_ANSWER3_BREAK,
    USER_CTRL_MULTI_HFP_CMD_END,

    USER_CTRL_LAST
} USER_CMD_TYPE;


////----反馈给客户使用的状态----////
typedef enum {
    /*下面是一些即时反馈的状态，无法重复获取的状态*/
    BT_STATUS_POWER_ON   = 1,   /*上电*/
    BT_STATUS_POWER_OFF  = 2,
    BT_STATUS_INIT_OK,          /*初始化完成*/
    BT_STATUS_EXIT_OK,          /*蓝牙退出完成*/
    BT_STATUS_START_CONNECTED,        /*开始连接*/
    BT_STATUS_FIRST_CONNECTED,        /*连接成功*/
    BT_STATUS_SECOND_CONNECTED,        /*连接成功*/
    BT_STATUS_ENCRY_COMPLETE,        /*加密完成*/
    BT_STATUS_FIRST_DISCONNECT,       /*断开连接*/
    BT_STATUS_SECOND_DISCONNECT,        /*断开连接*/
    BT_STATUS_PHONE_INCOME,     /*来电*/
    BT_STATUS_PHONE_NUMBER,     /*来电话号码*/
    BT_STATUS_PHONE_MANUFACTURER,     /*获取手机的厂商*/

    BT_STATUS_PHONE_OUT,        /*打出电话*/
    BT_STATUS_PHONE_ACTIVE,     /*接通电话*/
    BT_STATUS_PHONE_HANGUP,     /*挂断电话*/
    BT_STATUS_BEGIN_AUTO_CON,   /*发起回连*/
    BT_STATUS_MUSIC_SOUND_COME, /*库中加入auto mute判断音乐播放开始*/
    BT_STATUS_MUSIC_SOUND_GO,   /*库中加入auto mute判断音乐播放暂停*/
    BT_STATUS_RESUME,           /*后台有效，手动切回蓝牙*/
    BT_STATUS_RESUME_BTSTACK,   /*后台有效，后台时来电切回蓝牙*/
    BT_STATUS_SUSPEND,          /*蓝牙挂起，退出蓝牙*/
    BT_STATUS_LAST_CALL_TYPE_CHANGE,    /*最后拨打电话的类型，只区分打入和打出两种状态*/

    BT_STATUS_CALL_VOL_CHANGE,     /*通话过程中设置音量会产生这个状态变化*/
    BT_STATUS_SCO_STATUS_CHANGE,    /*当esco/sco连接或者断开时会产生这个状态变化*/
    BT_STATUS_CONNECT_WITHOUT_LINKKEY,   /*判断是首次连接还是配对后的连接，主要依据要不要简易配对或者pin code*/
    BT_STATUS_PHONE_BATTERY_CHANGE,     /*电话电量变化，该状态仅6个等级，0-5*/
    BT_STATUS_RECONNECT_LINKKEY_LOST,     /*回连时发现linkkey丢失了，即手机取消配对了*/
    BT_STATUS_RECONN_OR_CONN,       /*回连成功还是被连接*/
    BT_STATUS_BT_TEST_BOX_CMD,              /*蓝牙收到测试盒消息。1-升级，2-fast test*/
    BT_STATUS_BT_TWS_CONNECT_CMD,
    BT_STATUS_SNIFF_STATE_UPDATE,              /*SNIFF STATE UPDATE*/
    BT_STATUS_TONE_BY_FILE_NAME, /*直接使用文件名播放提示音*/

    BT_STATUS_PHONE_DATE_AND_TIME,   /*获取到手机的时间和日期，注意会有兼容性问题*/
    BT_STATUS_INBAND_RINGTONE,
    BT_STATUS_VOICE_RECOGNITION,
    BT_STATUS_AVRCP_INCOME_OPID,     /*收到远端设备发过来的AVRCP命令*/
    BT_STATUS_HFP_SERVICE_LEVEL_CONNECTION_OK,
    BT_STATUS_CONN_A2DP_CH,
    BT_STATUS_CONN_HFP_CH,
    BT_STATUS_INQUIRY_TIMEOUT,
    /*下面是1个持续的状态，是get_stereo_bt_connect_status获取*/

    /*下面是6个持续的状态，是get_bt_connect_status()获取*/
    BT_STATUS_INITING,          /*正在初始化*/
    BT_STATUS_WAITINT_CONN,     /*等待连接*/
    BT_STATUS_AUTO_CONNECTINT,  /*正在回连*/
    BT_STATUS_CONNECTING,       /*已连接，没有电话和音乐在活动*/
    BT_STATUS_TAKEING_PHONE,    /*正在电话*/
    BT_STATUS_PLAYING_MUSIC,    /*正在音乐*/
    BT_STATUS_A2DP_MEDIA_START,
    BT_STATUS_A2DP_MEDIA_STOP,


    BT_STATUS_BROADCAST_STATE,/*braoadcaset中*/

    BT_STATUS_TRIM_OVER,        /*测试盒TRIM完成*/
} STATUS_FOR_USER;

typedef enum {
    BT_CALL_BATTERY_CHG = 0, //电池电量改变
    BT_CALL_SIGNAL_CHG,      //网络信号改变
    BT_CALL_INCOMING,   //电话打入
    BT_CALL_OUTGOING,   //电话打出
    BT_CALL_ACTIVE,     //接通电话
    BT_CALL_HANGUP,      //电话挂断
    BT_CALL_ALERT,       //远端reach
    BT_CALL_VOL_CHANGED,
} BT_CALL_IND_STA;

typedef enum {
    BT_MUSIC_STATUS_IDLE = 0,
    BT_MUSIC_STATUS_STARTING,
    BT_MUSIC_STATUS_SUSPENDING,
} BT_MUSIC_STATE;  //音乐状态

#define SYS_BT_EVENT_TYPE_CON_STATUS (('C' << 24) | ('O' << 16) | ('N' << 8) | '\0')
#define SYS_BT_EVENT_TYPE_HCI_STATUS (('H' << 24) | ('C' << 16) | ('I' << 8) | '\0')



#define    REMOTE_DEFAULT    0x00
#define    REMOTE_SINK       0x01
#define    REMOTE_SOURCE     0x02


#define    SPP_CH       0x01
#define    HFP_CH       0x02
#define    A2DP_CH      0x04    //media
#define    AVCTP_CH     0x08
#define    HID_CH       0x10
#define    AVDTP_CH     0x20
#define    PBAP_CH      0x40
#define    HFP_AG_CH    0x80
#define    A2DP_SRC_CH  0x2000
#define    MAP_CH       0x8000
struct sniff_ctrl_config_t {
    u16 sniff_max_interval;
    u16 sniff_mix_interval;
    u16 sniff_attemp;
    u16	sniff_timeout;
    u8	sniff_addr[6];
};
extern u32 user_send_cmd_prepare(USER_CMD_TYPE cmd, u16 param_len, u8 *param);
extern int user_send_at_cmd_prepare(u8 *data, u16 len);
/**发射器操作的几个接口*/
extern u32 user_emitter_cmd_prepare(USER_CMD_TYPE cmd, u16 param_len, u8 *param);
u8 get_emitter_connect_status(void);
u16 get_emitter_curr_channel_state();
u8 get_emitter_a2dp_status(void);


/*
u16 get_curr_channel_state();  与  channel  判断区分
主动获取当前链路的连接状态，可以用来判断有哪些链路连接上了
*/
extern u16 get_curr_channel_state();
/*
u8 get_call_status(); 与BT_CALL_IND_STA 枚举的值判断
用于获取当前蓝牙电话的状态
*/
extern u8 get_call_status();
extern void user_cmd_ctrl_init(void *var);

/******当前连接的设备是jl测试盒**********/
extern bool get_remote_test_flag();
extern void set_remote_test_flag(u8 own_remote_test);
extern void bt_fast_test_handle_register(void (*handle)(void));
extern void bt_dut_test_handle_register(void (*handle)(u8));
extern void inquiry_result_handle_register(void (*handle)(char *name, u8 name_len, u8 *addr, u32 dev_class, char rssi));

/*个性化参数设置*/
/*用户调试设置地址，6个byte*/
extern void __set_bt_mac_addr(u8 *addr);

/*用户调试设置name,最长32个字符*/
extern void __set_host_name(const char *name, u8 len);
/*用户调试设置pin code*/
extern void __set_pin_code(const char *code);
/*该接口用于设置上电回连需要依次搜索设备的个数。*/
extern void __set_auto_conn_device_num(u8 num);

/*//回连的超时设置。ms单位。但是对手机发起的连接是没作用的*/
extern void __set_super_timeout_value(u16 time);
/*外部设置支持什么协议*/
extern void bt_cfg_default_init(u8 support);

/*设置电量显示发送更新的周期时间，为0表示关闭电量显示功能*/
extern void __bt_set_update_battery_time(u8 time);
/*给用户设置蓝牙支持连接的个数，主要用于控制控制可发现可连接和回连流程*/
extern void __set_user_ctrl_conn_num(u8 num);
/*提供接口外部设置要保留hfp不要蓝牙通话*/
extern void __set_disable_sco_flag(bool flag);

/*提供接口外部设置简易配对参数*/
extern void __set_simple_pair_param(u8 io_cap, u8 oob_data, u8 mitm);

/*有些自选接口用来实现个性化功能流程，回调函数注册，记得常来看看哟*/
extern void get_battery_value_register(int (*handle)(void));    /*电量发送时获取电量等级的接口注册*/
extern void music_vol_change_handle_register(void (*handle)(int vol), int (*handle2)(void)); /*手机更样机音乐模式的音量同步*/
extern void read_remote_name_handle_register(void (*handle)(u8 status, u8 *addr, u8 *name));  /*获取到名字后的回调函数接口注册函数*/
extern void spp_data_deal_handle_register(void (*handler)(u8 packet_type, u16 channel, u8 *packet, u16 size)); /*支持串口功能的数据处理接口*/
extern void discon_complete_handle_register(void (*handle)(u8 *addr, int reason)); /*断开或者连接上会调用的函数，给客户反馈信息*/

extern void update_bt_current_status(u8 *addr, u8 new_status, u8 conn_status);
extern u8 get_bt_connect_status(void);
extern u8 a2dp_get_status(void);

/*//回连的超时设置。ms单位。但是对手机发起的连接是没作用的*/
extern void __set_super_timeout_value(u16 time);
/*//回连page的超时设置。ms单位*/
extern void __set_page_timeout_value(u16 time);
/*获取的值上限是bt_set_auto_conn_device_num接口配置的值,一般用于做回连策略*/
extern int btstack_get_num_of_remote_device_recorded();
/*上电自动搜索设备的个数*/
extern u8 get_current_poweron_memory_search_index(u8 *temp_mac_addr);
extern void clear_current_poweron_memory_search_index(u8 inc);

extern void __set_user_background_goback(u8 en);

extern bool user_sniff_check_req(u8 sniff_cnt_time);

extern int tws_updata_phone_wait_con_addr(u8 *addr);
extern int tws_updata_internal_addr(u8 *internal_addr_local, u8 *internal_addr_remote);


extern void bt_discon_complete_handle(u8 *addr, int reason);
/*这个接口只用来判断回连或者开可发现可连接的状态*/
extern bool is_1t2_connection(void);
/*用来获取蓝牙连接的设备个数，不包含page状态的计数*/
extern u8 get_total_connect_dev(void);
/*可以通过地址查询HFP的状态*/
extern u8 is_bt_conn_hfp_hangup(u8 *addr);

extern void infor_2_user_handle_register(int (*handle)(u8 *info, u16 len), u8 *buffer_ptr);
/*音乐的ID3信息返回接口注册函数*/
extern void bt_music_info_handle_register(void (*handler)(u8 type, u32 time, u8 *info, u16 len));
/*用户层不需要用了*/
extern void set_bt_vm_interface(u32 vm_index, void *interface);
extern void bredr_stack_init(void);
/*sniff 的计数查询*/
extern bool bt_api_conn_mode_check(u8 enable, u8 *addr);
extern u8 bt_api_enter_sniff_status_check(u16 time_cnt, u8 *addr);
extern void user_cmd_timer_init();
extern void remove_user_cmd_timer();
//get_auto_connect_state有时效性，一般不用。可以用消息BT_STATUS_RECONN_OR_CONN
extern u8 get_auto_connect_state(u8 *addr);
//判断SCO/esco有没有正在使用,两个接口一样的
extern u8 get_esco_coder_busy_flag();
extern bool get_esco_busy_flag();
/*有可能低层刚开始走了连接，但是上层还没有消息，不维护蓝牙的不要随便用*/
extern u8 hci_standard_connect_check(void);
/*设置一个标识给库里面说明正在退出蓝牙*/
extern void set_stack_exiting(u8 exit);
/*根据规则生产BLE的随机地址*/
extern void lib_make_ble_address(u8 *ble_address, u8 *edr_address);
/*查询最后一个VM记录的地址进行回连*/
extern u8 connect_last_device_from_vm();
/*配置协议栈支持HID功能，为了兼容以前的HID独立模式，音箱SDK有使用流程*/
extern void __bt_set_hid_independent_flag(bool flag);
extern int btstack_exit();
/*能量检测之后有一些判断流程要走，非蓝牙开发不用*/
extern int sbc_energy_check(u8 *packet, u16 size);
extern int aac_energy_check(u8 *packet, u16 size);
/**发射器和接收器按键切换的时候要申请和释放资源**/
extern int a2dp_source_init(void *buf, u16 len, int deal_flag);
/**发射器和接收器按键切换的时候要申请和释放资源**/
extern int hfp_ag_buf_init(void *buf, int size, int deal_flag);
/*配置蓝牙协议栈处于发射器流程*/
extern void __set_emitter_enable_flag(u8 flag);
/*用户使用USER_CTRL_INQUIRY_CANCEL就行，下面的用户层不直接使用*/
extern void hci_cancel_inquiry();
/*发射器启动还是暂停数据发送的接口，会发start和suspend命令*/
extern void __emitter_send_media_toggle(u8 *addr, u8 toggle);
/*查询当前有没有a2dp source（发射器的音频发送链路）在连接状态*/
extern u8 is_a2dp_source_dev_null();
/*选择用哪一块VM存储信息，非蓝牙维护人员不用*/
extern u8 get_remote_dev_info_index();
extern u8 check_tws_le_aa(void);
extern void tws_api_set_connect_aa(int);
extern void tws_le_acc_generation_init(void);
extern void tws_api_clear_connect_aa();
extern void clear_sniff_cnt(void);
/**删除VM记录的最后一个设备信息*/
extern u8 delete_last_device_from_vm();

#define BD_CLASS_WEARABLE_HEADSET	0x240404/*ios10.2 display headset icon*/
#define BD_CLASS_HANDS_FREE			0x240408/*ios10.2 display bluetooth icon*/
#define BD_CLASS_MICROPHONE			0x240410
#define BD_CLASS_LOUDSPEAKER		0x240414
#define BD_CLASS_HEADPHONES			0x240418
#define BD_CLASS_CAR_AUDIO			0x240420
#define BD_CLASS_HIFI_AUDIO			0x240428
#define BD_CLASS_PHONEBOOK			0x340404
#define BD_CLASS_PAN_DEV            0X020118

#define BD_CLASS_SMART_PHONE        0x5A020C//0x40020C

#define BD_CLASS_MOUSE              0x002580
#define BD_CLASS_KEYBOARD           0x002540
#define BD_CLASS_KEYBOARD_MOUSE     0x0025C0
#define BD_CLASS_REMOTE_CONTROL     0x00254C

#define BD_CLASS_TRANSFER_HEALTH    0x10091C

//LDAC采样率
#define LDAC_SAMPLING_FREQ_44100    0x20
#define LDAC_SAMPLING_FREQ_48000    0x10
#define LDAC_SAMPLING_FREQ_88200    0x08
#define LDAC_SAMPLING_FREQ_96000    0x04
// 配置LDAC支持的采样率
extern void __set_a2dp_ldac_sampling_freq(u8 a2dp_ldac_sampling_freq);

/*修改什么的类型，会影响到手机显示的图标*/
extern void __change_hci_class_type(u32 class);
/*配置通话使用16k的msbc还是8k的cvsd*/
extern void __set_support_msbc_flag(bool flag);
/*配置协议栈使用支持AAC的信息*/
extern void __set_support_aac_flag(bool flag);

/*设置1拖2时电话是否抢断标识*/
extern void __set_hfp_switch(u8 switch_en);
/*
 *设置1拖2时电话是否恢复标识
 *通话结束的时候，如果还有手机在通话，自动切到蓝牙端
 */
extern void __set_hfp_restore(u8 restore_en);
/*当前设备被打断时是否自动暂停*/
extern void __set_auto_pause_flag(u8 flag);
/*当前设备被打断时是否自动暂停*/
extern void __set_auto_pause_flag(u8 flag);
/*高级音频设置标志是否允许后者打断前者*/
extern void __set_music_break_in_flag(u8 flag);
/*高级音频打断检测数据包阈值设置*/
extern void __set_a2dp_sound_detect_counter(u8 sound_come, u8 sound_go);
/*pan的控制接口和发数接口,
  addr指定就按指定的查找，NULL就默认正在使用那个
  cmd 下面定义的宏用户可以使用
  param 传参数需要的值或者data包的长度
  data 传的是要发数据的包指针
 */
#define USER_PAN_CMD_SEND_DATA  0xff
int user_pan_send_cmd(u8 *addr, u32 cmd, u32 param, u8 *data);

enum {
    BD_ESCO_IDLE = 0,		/*当前没有设备通话中*/
    BD_ESCO_BUSY_CURRENT,	/*当前地址对应的设备通话中*/
    BD_ESCO_BUSY_OTHER,	 	/*通话中的设备非当前地址*/
};

enum {
    BD_GET_SECOND_ACTIVE = 0,   /*获取当前设备的三方通话状态*/
    BD_GET_SECOND_BREAK,        /*获取后台设备的三方通话状态*/
};

extern u8 check_esco_state_via_addr(u8 *addr);
/*判断是否主动回连*/
extern u8 get_auto_connect_state(u8 *addr);

typedef struct __hid_sdp_info {
    u16 vid_private;
    u16 pid_private;
    u16 ver_private;

    u8  sub_class;
    u8  country_code;
    bool virtual_cable;
    bool reconnect_initiate;
    bool sdp_disable;
    bool battery_power;
    bool remote_wake;
    bool normally_connectable;
    bool boot_device;
    u16 version;
    u16 parser_version;
    u16 profile_version;
    u16 supervision_timeout;
    u16 language;
    u16 bt_string_offset;
    u16 descriptor_len;
    u8 *descriptor;
    char *service_name;
    char *service_description;
    char *provide_name;
    void (*sdp_request_respone_callback)(u8 type);
    u8 *extra_buf;
    u8 extra_len;
} hid_sdp_info_t;


typedef struct {
    u16 chl_id;
    u16 data_len;
    u8  *data_ptr;
} hid_s_param_t;

extern u16 sdp_create_diy_device_ID_service(u8 *buffer, u16 buffer_size);
extern u16 sdp_create_diy_hid_service(u8 *buffer, u16 buffer_size, const u8 *hid_descriptor, u16 hid_descriptor_size);
u8 get_remote_vol_sync(void);
void set_start_search_spp_device(u8 spp);


u8 restore_remote_device_info_opt(bd_addr_t *mac_addr, u8 conn_device_num, u8 id);
u8 restore_remote_device_info_profile(bd_addr_t *mac_addr, u8 device_num, u8 id, u8 profile);
/*remote dev type*/
/*0:unknow,1-android,2:apple_inc,0x03-xiaomi*/
enum {
    REMOTE_DEV_UNKNOWN  = 0,
    REMOTE_DEV_ANDROID		,
    REMOTE_DEV_IOS			,
    REMOTE_DEV_XIAOMI   	,
};
u8 remote_dev_company_ioctrl(bd_addr_t dev_addr, u8 op_flag, u8 value);
u8 hci_standard_link_check(void);

//获取当前是否有conn处于sniff状态
bool bt_api_get_sniff_state();



#endif
