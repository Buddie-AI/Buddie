#ifndef _RDEC_H_
#define _RDEC_H_

#define DEVICE_EVENT_FROM_RDEC		(('R' << 24) | ('D' << 16) | ('E' << 8) | '\0')

enum rdec_index {
    RDEC0,
    RDEC1,
    RDEC2,
};

struct rdec_device {
    enum rdec_index index;
    u8 sin_port0; 	//采样信号端口0
    u8 sin_port1; 	//采样信号端口1
    u8 key_value0; 	//键值1
    u8 key_value1; 	//键值2
};

struct rdec_platform_data {
    u8 enable;
    u8 num; 	//rdec数量
    const struct rdec_device *rdec;
};

#define RDEC_PLATFORM_DATA_BEGIN(data) \
		static const struct rdec_platform_data data = {

#define RDEC_PLATFORM_DATA_END() \
};

/*********************** rdec 初始化 ******************************/
int rdec_init(const struct rdec_platform_data *user_data);


#endif

