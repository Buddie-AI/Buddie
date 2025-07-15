#include "app_config.h"
#include "system/includes.h"
#include "audio_config.h"
#include "debug.h"
#include "asm/spi.h"
#include "lis2de12_driver.h"
#include "hw_fft.h"
#include "timer.h"
#include "spp_online_db.h"
#include "lis2de12.h"
#include "jiffies.h"


#define hello_task_name "bone_task"
#define LOG_TAG             "[bone_task]"

static u8 fifo_data[6];             // 用于存储 fifo 数据

/************************ SPP send*********************/
char amplitude_str[12];
char vad_is_activate_str[12];
u16 len = 0;



/************************* 骨传导fft相关配置 ********************** */
#define BUFFER_SIZE 128
#define FREQ_N 64

static bool bone_log_flag = false;
static int buffer1[BUFFER_SIZE] = {0};
static int buffer2[BUFFER_SIZE] = {0};
static volatile bool buffer1_full = false;
static volatile bool buffer2_full = false;
static u16 bone_timer_id = (u16) - 1; // 定时器 id
static char* log_string; // 用于打印日志

// 信号量或标志位
static u16 index1 = 0;
static u16 index2 = 0;
static unsigned int fft_config_bone;
static unsigned int ifft_config_bone;
static int motion_data[BUFFER_SIZE]; // FFT 输入缓冲区
static int motion_freq[FREQ_N]; // FFT 输出频率数组

static bool vad_flag = false;   // 语音识别标志位


// vad調參
u8 vad_is_activate = 0;
static int vad_threshold_lis2de12 = 1000;
u32 vad_last_jiffies_lis2de12= 0;
u16 vad_timer_cnt_lis2de12= 0;
static bool vad_false_condition = false;
static int people_in_noise_freq_1 = 6;
static int people_in_noise_freq_2 = 7;
static int human_voice_freq_1 = 11;
static int human_voice_freq_2 = 12;
int max1 = -2147483648, max2 = -2147483648, max3 = -2147483648;
int idx1 = -1, idx2 = -1, idx3 = -1;





// 对 spi.h 中提供的固定 spi1 结构体进行配置 
const struct spi_platform_data spi1_p_data = {
    .port = {TCFG_HW_SPI1_PORT_CLK, TCFG_HW_SPI1_PORT_DO, TCFG_HW_SPI1_PORT_DI, (u8)-1, (u8)-1},
    .mode = TCFG_HW_SPI1_MODE,
    .role = TCFG_HW_SPI1_ROLE,
    .clk = TCFG_HW_SPI1_BAUD,
};


// 自定义 bone spi 句柄
const static spi_handle_t bone_spi = {
    .spi_hdl = SPI1,
    .spi_cs = IO_PORTG_07,
};

const static spi_handle_t* bone_spi_info = &bone_spi;


/*******************     CS 线配置    *********************/ 
// only support 4-wire mode
#define spi_cs_init() \
    do { \
        gpio_write(bone_spi_info->spi_cs, 1); \
        gpio_set_direction(bone_spi_info->spi_cs, 0); \
        gpio_set_die(bone_spi_info->spi_cs, 1); \
    } while (0)
#define spi_cs_uninit() \
    do { \
        gpio_set_die(bone_spi_info->spi_cs, 0); \
        gpio_set_direction(bone_spi_info->spi_cs, 1); \
        gpio_set_pull_up(bone_spi_info->spi_cs, 0); \
        gpio_set_pull_down(bone_spi_info->spi_cs, 0); \
    } while (0)

#define spi_cs_h()                  gpio_write(bone_spi_info->spi_cs, 1)
#define spi_cs_l()                  gpio_write(bone_spi_info->spi_cs, 0)

#define spi_read_byte()             spi_recv_byte(bone_spi_info->spi_hdl, NULL)
#define spi_write_byte(x)           spi_send_byte(bone_spi_info->spi_hdl, x)
#define spi_dma_read(x, y)          spi_dma_recv(bone_spi_info->spi_hdl, x, y)
#define spi_dma_write(x, y)         spi_dma_send(bone_spi_info->spi_hdl, x, y)
#define spi_set_width(x)            spi_set_bit_mode(bone_spi_info->spi_hdl, x)
#define spi_init()              spi_open(bone_spi_info->spi_hdl)
#define spi_closed()            spi_close(bone_spi_info->spi_hdl)
#define spi_suspend()           hw_spi_suspend(bone_spi_info->spi_hdl)
#define spi_resume()            hw_spi_resume(bone_spi_info->spi_hdl)




/**************************  bone_device **************************** */
#define WATERMARK_LEVEL 15   // 设置水印级别
static u8 samples = 0x00;     // 用于存储 fifo 中的数据个数



status_t lis2de12_dev_init(void* handle)
{
    u8_t CTRL_REG1 = 0x9c;  // 10011100 使用5376Hz，使用z轴
    u8_t test_value = 0x00;


    if (LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG1, &CTRL_REG1, 1) != MEMS_SUCCESS)
    {
        log_info("LIS2DE12_CTRL_REG1 write failed\n");
        return MEMS_ERROR;
    }
    if (bone_log_flag)
    {
        LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG1, &test_value, 1);
        log_info("LIS2DE12_CTRL_REG1: %d\n", test_value);
        test_value = 0x00;
    }

    // 设置量程
    if (LIS2DE12_SetFullScale(handle, LIS2DE12_FULLSCALE_2) != MEMS_SUCCESS)
    {
        log_info("LIS2DE12_SetFullScale failed\n");
        return MEMS_ERROR;
    }
    if (bone_log_flag)
    {
        LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG4, &test_value, 1);
        log_info("量程0 LIS2DE12_CTRL_REG4: %d\n", test_value);
        test_value = 0x00;
    }

    // 启用 FIFO 并设置为流模式
    if (LIS2DE12_FIFOModeEnable(handle, LIS2DE12_FIFO_STREAM_MODE) != MEMS_SUCCESS)
    {
        log_info("LIS2DE12_FIFOModeEnable failed\n");
        return MEMS_ERROR;
    }
    test_value = 0x40;
    LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG5, &test_value, 1);
    if (bone_log_flag)
    {
        LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG5, &test_value, 1);
        log_info("fifo中断64 LIS2DE12_CTRL_REG5: %d\n", test_value);
        test_value = 0x00;
    }

    // 设置水印级别
    if (LIS2DE12_SetWaterMark(handle, WATERMARK_LEVEL) != MEMS_SUCCESS)
    {
        log_info("LIS2DE12_SetWaterMark failed\n");
        return MEMS_ERROR;
    }
    if (bone_log_flag)
    {
        LIS2DE12_ACC_ReadReg(handle, LIS2DE12_FIFO_CTRL_REG, &test_value, 1);
        log_info("水印级别143 LIS2DE12_FIFO_CTRL_REG: %d\n", test_value);
        test_value = 0x00;
    }

    // 启动水印中断
    if (LIS2DE12_SetInt1Pin(handle, 0x04) != MEMS_SUCCESS)
    {
        log_info("LIS2DE12_SetTriggerInt failed\n");
        return MEMS_ERROR;
    }
    if (bone_log_flag)
    {
        LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG3, &test_value, 1);
        log_info("水印中断4 LIS2DE12_CTRL_REG3: %d\n", test_value);
        test_value = 0x00;
    }

    // 启动中断触发时钟位置
    if (LIS2DE12_SetInt2Pin(handle, LIS2DE12_INT_ACTIVE_HIGH) != MEMS_SUCCESS)
    {
        log_info("LIS2DE12_SetInt2Pin failed\n");
        return MEMS_ERROR;
    }
    if (bone_log_flag)
    {
        LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG6, &test_value, 1);
        log_info("中断触发时钟位置0 LIS2DE12_CTRL_REG6: %d\n", test_value);
        log_info("骨传导配置完成\n");
    }
    
    return MEMS_SUCCESS;
}





/*************************** vad ************************************/
void vad_detect(void)
{
    int amplitude = 0;

    // 噪声频段
    for (int i = 1; i <= 5; i ++)
    {
        amplitude -= 1.2 * motion_freq[i];
    }


    // 语音频段
    for (int i = 10; i <= 20; i++) {
        amplitude += motion_freq[i];
        if (motion_freq[i] > motion_freq[human_voice_freq_1]) {
            human_voice_freq_2 = human_voice_freq_1;
            human_voice_freq_1 = i;
        } else if (i != human_voice_freq_1 && motion_freq[i] > motion_freq[human_voice_freq_2]) {
            human_voice_freq_2 = i;
        }
    }
    amplitude += motion_freq[human_voice_freq_1];
    amplitude += motion_freq[human_voice_freq_2];


    for (int i = 6; i < 10; i ++)
    {
        amplitude += motion_freq[i];
    }
    for (int i = 21; i <= 30; i ++)
    {
        amplitude += motion_freq[i];
    }


    // 基准频段
    for (int i = 40; i <= 60; i ++)
    {
        amplitude -= motion_freq[i];
    }

    // sprintf(amplitude_str, "%d", amplitude);
    // strcat(amplitude_str, "\n");
    // u16 len = strlen(amplitude_str);
    // online_spp_send_data((u8 *)amplitude_str, len);
    // return;

    if (amplitude > vad_threshold_lis2de12 && amplitude <= 10000)
    {
        vad_timer_cnt_lis2de12++;
    }else    // 連續兩個達到閾值，防止誤觸發
    {
        vad_timer_cnt_lis2de12 = 0;
    }

    if (vad_timer_cnt_lis2de12 > 1)
    {
        vad_is_activate = 1;
    }else
    {
        vad_is_activate = 0;
    }
    // log_info("vad:%d", vad_is_activate);
    // sprintf(vad_is_activate_str, "%d", vad_is_activate);
    // strcat(vad_is_activate_str, "\n");
    // u16 len_1 = strlen(vad_is_activate_str);
    // online_spp_send_data(vad_is_activate_str, len_1);
}




/**************************** bone_fft ******************************/
void perform_fft(int *data)
{
    // uint8_t hex_data[BUFFER_SIZE * 4 + 6];  // 每个整数占4字节，加上开头和结尾
    // int idx = 0;

    // // 开头的 00 00 00
    // hex_data[idx++] = 0x00;
    // hex_data[idx++] = 0x00;
    // hex_data[idx++] = 0x00;

    // // 将 buffer1 中的每个整数以16进制格式发送
    // for (int i = 0; i < BUFFER_SIZE; i++) {
    //     // 将每个整数的4字节分别提取并转成16进制
    //     hex_data[idx++] = (data[i] >> 24) & 0xFF;  // 最高字节
    //     hex_data[idx++] = (data[i] >> 16) & 0xFF;  // 次高字节
    //     hex_data[idx++] = (data[i] >> 8) & 0xFF;   // 次低字节
    //     hex_data[idx++] = data[i] & 0xFF;          // 最低字节
    // }

    // // 结尾的 FF FF FF
    // hex_data[idx++] = 0xFF;
    // hex_data[idx++] = 0xFF;
    // hex_data[idx++] = 0xFF;

    // // 发送数据
    // online_spp_send_data(hex_data, idx);

    // return;


    hw_fft_run(fft_config_bone, data, motion_data);
    // 提取频率分量
    for (int i = 0; i < FREQ_N; i++)
    {
        motion_freq[i] = abs(motion_data[i]);
    }



    // 使用spp传输频段，分析数据
    // uint8_t hex_data[FREQ_N * 4 + 6];  // 每个整数占4字节，加上开头和结尾
    // int idx = 0;
    // // 开头的 00 00 00
    // hex_data[idx++] = 0x00;
    // hex_data[idx++] = 0x00;
    // hex_data[idx++] = 0x00;
    // // 将 buffer1 中的每个整数以16进制格式发送
    // for (int i = 0; i < FREQ_N; i++) {
    //     // 将每个整数的4字节分别提取并转成16进制
    //     hex_data[idx++] = (motion_freq[i] >> 24) & 0xFF;  // 最高字节
    //     hex_data[idx++] = (motion_freq[i] >> 16) & 0xFF;  // 次高字节
    //     hex_data[idx++] = (motion_freq[i] >> 8) & 0xFF;   // 次低字节
    //     hex_data[idx++] = motion_freq[i] & 0xFF;          // 最低字节
    // }
    // // 结尾的 FF FF FF
    // hex_data[idx++] = 0xFF;
    // hex_data[idx++] = 0xFF;
    // hex_data[idx++] = 0xFF;
    // // 发送数据
    // online_spp_send_data(hex_data, idx);
    // return;





    // 判断是否为语音
    vad_detect();

}




void bone_task_timer_cb()
{
    LIS2DE12_ACC_ReadReg(bone_spi_info, LIS2DE12_FIFO_SRC_REG, &samples, 1);
    samples &= 0x1f;
    // log_info("samples: %d\n", samples);
    for (int i = samples; i > 0; i --)
    {
        if (LIS2DE12_ACC_ReadReg(bone_spi_info, 0x28, fifo_data, 6) != MEMS_SUCCESS)
        {
            log_info("LIS2DE12_GetFifoData failed\n");
            return;
        }

        if (!buffer1_full)
        {
            buffer1[index1] = fifo_data[5];
            if (buffer1[index1] >= 128)
            {
                buffer1[index1] = buffer1[index1] - 256;
            }
            buffer1[index1] = (int)(buffer1[index1] * 15.69);
            index1++;
        }else if (!buffer2_full)
        {
            buffer2[index2] = fifo_data[5];
            if (buffer2[index2] >= 128)
            {
                buffer2[index2] = buffer2[index2] - 256;
            }
            buffer2[index2] = (int)(buffer2[index2] * 15.6);
            index2++;
        }

        if (index1 == BUFFER_SIZE)
        {
            buffer1_full = true;           
            index1 = 0;
        }else if (index2 == BUFFER_SIZE)
        {
            buffer2_full = true;           
            index2 = 0;
        }
    }
}


void vad_task()
{
    while(1)
    {
        if (buffer1_full)
        {
            perform_fft(buffer1);
            buffer1_full = false;
        }else if (buffer2_full)
        {
            perform_fft(buffer2);
            buffer2_full = false;
        }
        os_time_dly(1); // 10 ms检测一次
    }
}







// 使用timer中定时器任务
void bone_task_init()
{

    // 使用spi初始化骨传导
    spi_cs_init();
    spi_init();


    // 初始化骨传导传感器
    if (lis2de12_dev_init(bone_spi_info) != MEMS_SUCCESS)
    {
        log_info("lis2de12_dev_init failed\n");
        return;
    }
    fft_config_bone = hw_fft_config(128, 7, 0, 0, 1);
    // ifft_config_bone = hw_fft_config(128, 7, 1, 1, 1);

    task_create(vad_task, NULL, "vad_task");
    bone_timer_id = usr_timer_add(NULL, bone_task_timer_cb, 3, 0);
    // u16 bone_timer_id = sys_timer_add(NULL, bone_task_timer_cb, 2); 
}



void bone_task_uninit()
{
    // 關閉傳感器運轉
    u8_t CTRL_REG1 = 0x98;  // 10011000 ,關閉所有軸
    u8_t test_value = 0x00;
    if (LIS2DE12_ACC_WriteReg(bone_spi_info, LIS2DE12_CTRL_REG1, &CTRL_REG1, 1) != MEMS_SUCCESS)
    {
        log_info("關閉過程LIS2DE12_CTRL_REG1 write failed\n");
        return;
    }
    if (bone_log_flag)
    {
        LIS2DE12_ACC_ReadReg(bone_spi_info, LIS2DE12_CTRL_REG1, &test_value, 1);
        log_info("LIS2DE12_CTRL_REG1: %d\n", test_value);
        test_value = 0x00;
    }

    // 關閉任務
    usr_timer_del(bone_timer_id);
    task_kill("vad_task");

    // 關閉spi
    spi_cs_uninit();
    spi_closed();

    if (bone_log_flag)
    {
        log_info("已經關閉骨傳導");
    }

}






