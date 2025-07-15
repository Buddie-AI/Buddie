#ifndef IOCTL_INF_H
#define IOCTL_INF_H




#define IOCTL_SET_IRQ_NUM 				1
#define IOCTL_SET_PRIORITY 				2
#define IOCTL_SET_DATA_WIDTH 			3
#define IOCTL_SET_SPEED 				4
#define IOCTL_SET_DETECT_MODE 			5
#define IOCTL_SET_DETECT_FUNC 			6
#define IOCTL_SET_DETECT_TIME_INTERVAL  7
#define IOCTL_SET_PORT 					8
#define IOCTL_SET_PORT_FUNC 			9
#define IOCTL_SET_CS_PORT_FUNC 		 	10
#define IOCTL_SET_READ_MODE 			11
#define IOCTL_SET_WRITE_MODE 		    12
#define IOCTL_SET_WRITE_PROTECT 	    13
#define IOCTL_SET_START_BIT 			14
#define IOCTL_SET_STOP_BIT 				15
#define IOCTL_FLUSH 					16
#define IOCTL_REGISTER_IRQ_HANDLER      17
#define IOCTL_UNREGISTER_IRQ_HANDLER    18
#define IOCTL_GET_SYS_TIME              19
#define IOCTL_SET_SYS_TIME              20
#define IOCTL_GET_ALARM                 21
#define IOCTL_SET_ALARM                 22
#define IOCTL_SET_CAP_LOWSPEED_CARD	    23
#define IOCTL_SET_VDD50_EN              30
#define IOCTL_GET_WEEKDAY             	32
#define IOCTL_CLR_READ_MODE             33
#define IOCTL_SET_READ_CRC              34
#define IOCTL_GET_READ_CRC              35
#define IOCTL_GET_VOLUME                36
#define IOCTL_SET_VOLUME                37
#define IOCTL_SET_ALARM_ENABLE          38
#define IOCTL_CMD_RESUME                39
#define IOCTL_CMD_SUSPEND               40
#define IOCTL_SET_BASE_ADDR             41
#define IOCTL_SET_ASYNC_MODE            42
#define IOCTL_GET_SPEED 				43
#define IOCTL_SET_ACTIVE_STATUS			44
#define IOCTL_POWER_RESUME              45
#define IOCTL_POWER_SUSPEND             46


#define IOCTL_GET_ID 					100
#define IOCTL_GET_SECTOR_SIZE			101
#define IOCTL_GET_BLOCK_SIZE			102
#define IOCTL_GET_CAPACITY 				103
#define IOCTL_GET_WIDTH 				104
#define IOCTL_GET_HEIGHT				105
#define IOCTL_GET_BLOCK_NUMBER          106
#define IOCTL_CHECK_WRITE_PROTECT       107
#define IOCTL_GET_STATUS                108
#define IOCTL_GET_TYPE                  109
#define IOCTL_GET_MAX_LUN               110
#define IOCTL_GET_CUR_LUN               111
#define IOCTL_SET_CUR_LUN               112
#define IOCTL_SET_FORCE_RESET           113
#define IOCTL_SET_CAPACITY 				114


#define IOCTL_ERASE_SECTOR 				200
#define IOCTL_ERASE_BLOCK 				201
#define IOCTL_ERASE_CHIP 				202
#define IOCTL_SET_ENC_END               203
#define IOCTL_ERASE_PAGE                204
#define IOCTL_SDMMC_ERASE               205


#define IOCTL_SET_DATA_CALLBACK         301

#define IOCTL_GET_PART_INFO             320

struct ioctl_irq_handler {
    void *priv;
    void *handler;
};



#endif

