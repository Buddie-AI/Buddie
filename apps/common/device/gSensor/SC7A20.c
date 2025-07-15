#include "gSensor/SC7A20.h"
#include "gSensor/gSensor_manage.h"
#include "app_config.h"

#include "gSensor/SC7A20.h"
#include "math.h"


#if TCFG_SC7A20_EN

u8  volatile  sl_click_timer_en = 0;

#define INT_IO IO_PORTC_02
#define SL_Sensor_Algo_Release_Enable 1


u8 Click_time = 0X00;
u8 Click_add_flag = 0X00;


u32 SL_MEMS_i2cRead(u8 addr, u8 reg, u8 len, u8 *buf)
{
    return _gravity_sensor_get_ndata(addr, reg, buf, len);
}

u8  SL_MEMS_i2cWrite(u8 addr, u8 reg, u8 data)
{
    gravity_sensor_command(addr, reg, data);
    return 0;
}

char SC7A20_Check()
{
    u8 reg_value = 0;
    SL_MEMS_i2cRead(SC7A20_R_ADDR, SC7A20_WHO_AM_I, 1, &reg_value);
    if (reg_value == 0x11) {
        return 0x01;
    } else {
        return 0x00;
    }
}

u8 SC7A20_Config(void)
{
    u8 Check_Flag = 0;
    u8 write_num = 0, i = 0;

    u8  ODR = 0x6f;                      //400HZ/6f 200HZ
    u8  HP  = 0x0c;                      //开启高通滤波
    u8  click_int = 0x04;//0x82                //将Click中断映射到INT1 OVERRUN
    u8  range     = 0x90;                //4g量程
    u8  fifo_en   = 0x40;                //使能fifo
    u8  fifo_mode = 0x9c;  //0x9d              //Stream模式，0-29(WTM30个)
    u8  click_mode = 0x15;               //单击3轴触发

    Check_Flag = SC7A20_Check();
    if (Check_Flag == 1) {
        SL_MEMS_i2cWrite(SC7A20_W_ADDR, SC7A20_CTRL_REG1, ODR);
        SL_MEMS_i2cWrite(SC7A20_W_ADDR, SC7A20_CTRL_REG2, HP);
        SL_MEMS_i2cWrite(SC7A20_W_ADDR, SC7A20_CTRL_REG3, click_int);// click int1
        SL_MEMS_i2cWrite(SC7A20_W_ADDR, SC7A20_CTRL_REG4, range);
        SL_MEMS_i2cWrite(SC7A20_W_ADDR, SC7A20_CTRL_REG5, fifo_en);
        SL_MEMS_i2cWrite(SC7A20_W_ADDR, SC7A20_FIFO_CTRL_REG, fifo_mode);
        /*
        SL_MEMS_i2cWrite(SC7A20_W_ADDR, SC7A20_CLICK_CFG, click_mode);//单Z轴
        SL_MEMS_i2cWrite(SC7A20_W_ADDR, SC7A20_CLICK_THS, SC7A20_CLICK_TH);//62.6mg(4g)*10
        SL_MEMS_i2cWrite(SC7A20_W_ADDR, SC7A20_TIME_LIMIT, SC7A20_CLICK_WINDOWS);
        SL_MEMS_i2cWrite(SC7A20_W_ADDR, SC7A20_TIME_LATENCY, SC7A20_CLICK_LATENCY);
        */
        SL_MEMS_i2cWrite(SC7A20_W_ADDR, SC7A20_CTRL_REG6, SC7A20_INT_LEVEL);
        printf("check ok\n");
        return 0;
    } else {
        printf("check fail\n");
        return -1;
    }
}
u8 SC7A20_disable(void)
{
    u8 Check_Flag = SC7A20_Check();
    if (Check_Flag == 1) {
        u8  ODR = 0x00;
        SL_MEMS_i2cWrite(SC7A20_W_ADDR, SC7A20_CTRL_REG1, ODR);
        printf("check ok\n");
        return 0;
    } else {
        printf("check fail\n");
        return -1;
    }

}

#if SL_Sensor_Algo_Release_Enable==0
extern signed short         SL_DEBUG_DATA[10][128];
extern u8        SL_DEBUG_DATA_LEN;
#endif

u8 sl_pp_num = 0;

u8 SL_Get_CLICK_PP_CNT(u8 fun_flag)
{
    if (fun_flag == 0) {
        sl_pp_num = 0;
    }
    return sl_pp_num;
}

unsigned int SL_Click_Sqrt(unsigned int sqrt_data)
{
    unsigned int SL_SQRT_LOW, SL_SQRT_UP, SL_SQRT_MID;
    u8 sl_sqrt_num = 0;

    SL_SQRT_LOW = 0;
    SL_SQRT_UP = sqrt_data;
    SL_SQRT_MID = (SL_SQRT_UP + SL_SQRT_LOW) / 2;
    while (sl_sqrt_num < 200) {
        if (SL_SQRT_MID * SL_SQRT_MID > sqrt_data) {
            SL_SQRT_UP = SL_SQRT_MID;
        } else {
            SL_SQRT_LOW = SL_SQRT_MID;
        }
        if (SL_SQRT_UP - SL_SQRT_LOW == 1) {
            if (SL_SQRT_UP * SL_SQRT_UP - sqrt_data > sqrt_data - SL_SQRT_LOW * SL_SQRT_LOW) {
                return SL_SQRT_LOW;
            } else {
                return SL_SQRT_UP;
            }
        }
        SL_SQRT_MID = (SL_SQRT_UP + SL_SQRT_LOW) / 2;
        sl_sqrt_num++;
    }
    return 0;
}

char SC7A20_Click_Read(int TH1, int TH2)
{
    u8 i = 0, j = 0, k = 0;
    u8 click_num = 0;
    u8 fifo_len;
    unsigned int  sc7a20_data = 0;
    unsigned int  fifo_data_xyz[32];
    u8 click_result = 0x00;
    unsigned int  click_sum = 0;
    u8 data1[6];
    signed char   data[6];

    g_printf("INT_HAPPEN!\r\n");

    SL_MEMS_i2cRead(SC7A20_R_ADDR, SC7A20_SRC_REG, 1, &fifo_len);
    if ((fifo_len & 0x40) == 0x40) {
        fifo_len = 32;
    } else {
        fifo_len = fifo_len & 0x1f;
    }

    for (i = 0; i < fifo_len; i++) {
        SL_MEMS_i2cRead(SC7A20_R_ADDR, 0xA8, 6, &data1[0]);
        data[1] = (signed char)data1[1];
        data[3] = (signed char)data1[3];
        data[5] = (signed char)data1[5];
        sc7a20_data = (data[1]) * (data[1]) + (data[3]) * (data[3]) + (data[5]) * (data[5]);
        sc7a20_data = SL_Click_Sqrt(sc7a20_data);
        fifo_data_xyz[i] = sc7a20_data;

#if SL_Sensor_Algo_Release_Enable==0
        SL_DEBUG_DATA[0][i] = data[1];
        SL_DEBUG_DATA[1][i] = data[3];
        SL_DEBUG_DATA[2][i] = data[5];
        SL_DEBUG_DATA[3][i] = fifo_data_xyz[i];
        SL_DEBUG_DATA_LEN = fifo_len;
#endif
    }

    k = 0;
    for (i = 1; i < fifo_len - 1; i++) {
        if ((fifo_data_xyz[i + 1] > TH1) && (fifo_data_xyz[i - 1] < 30)) {
            g_printf("in_th\r\n");
            if (click_num == 0) {
                click_sum = 0; //first peak
                for (j = 0; j < i - 1; j++) {
                    if (fifo_data_xyz[j] > fifo_data_xyz[j + 1]) {
                        click_sum += fifo_data_xyz[j] - fifo_data_xyz[j + 1];
                    } else {
                        click_sum += fifo_data_xyz[j + 1] - fifo_data_xyz[j];
                    }
                }
#if SL_Sensor_Algo_Release_Enable==0
                g_printf("click_sum:%d!\r\n", click_sum);
#endif
                if (click_sum > TH2) {
                    sl_pp_num++;
                    break;
                }
                k = i;
            } else {
                k = i; //sencond peak
            }
        }

        if (k != 0) {
            if (fifo_data_xyz[i - 1] - fifo_data_xyz[i + 1] > TH1 - 10) {
                if (i - k < 5) {
                    click_num = 1;
                    break;
                }
            }
        }
    }
    if (click_num == 1) {
        click_result = 1;
    } else {
        click_result = 0;
    }

    return click_result;
}

//GPIO?????,??INT2
//Service function in Int Function
//u8  sl_click_timer_en     =0;
u8  sl_click_status       = 0;
unsigned short click_timer_cnt       = 0;
unsigned short click_timer_total_cnt = 0;
u8  click_click_final_cnt = 0;

char  SC7A20_Click_Alog(void)
{
    u8 click_status = 0;

    SL_MEMS_i2cWrite(SC7A20_W_ADDR, SC7A20_CTRL_REG3, 0x00);
    click_status = SC7A20_Click_Read(SC7A20_TH1, SC7A20_TH2); //40,50

    g_printf("click_status = 0x%x\n", click_status);
    if (click_status == 1) {
        if (sl_click_timer_en == 0) {
            //set click timer flag
            sl_click_timer_en    = 1;
            //clear click timer cnt value
            click_timer_cnt      = 0;
            click_timer_total_cnt = 0;
            click_click_final_cnt = 0;
        }
        sl_click_status = 1;
    }
    SL_MEMS_i2cWrite(SC7A20_W_ADDR, SC7A20_CTRL_REG3, 0x82);
    return click_status;
}

//This fuction is execute in 50ms timer when the timer is opened
char SC7A20_click_status(void)
{
    u8 click_e_cnt = 0;
    if (sl_click_timer_en == 1) {
        click_timer_cnt++;
        if ((click_timer_cnt < click_pp_num) && (sl_click_status == 1)) {
            g_printf("status:%d\r\n", sl_click_status);
            g_printf("fin_num:%d\r\n", click_click_final_cnt);
            sl_click_status = 0;
            click_timer_total_cnt = click_timer_total_cnt + click_timer_cnt;
            click_timer_cnt = 0;
            click_click_final_cnt++;
        }

        click_e_cnt = SL_Get_CLICK_PP_CNT(1);

        if ((((click_timer_cnt >= click_pp_num) || (click_timer_total_cnt >= click_max_num)) && (click_e_cnt < 1)) ||
            ((click_timer_cnt >= click_pp_num) && (click_e_cnt > 0)))
//		if((click_timer_cnt>=click_pp_num)||(click_timer_total_cnt>=click_max_num))
        {
            //clear click timer flag
            sl_click_timer_en = 0;
            //clear click timer cnt value
            click_timer_cnt      = 0;
            click_timer_total_cnt = 0;
            g_printf("fin_num:%d\r\n", click_click_final_cnt);
            if (click_e_cnt > 0) {
                click_e_cnt = SL_Get_CLICK_PP_CNT(0);
                return 0;
            } else {
                return click_click_final_cnt;
            }
        }
    }
    return 0;
}

void SC7A20_int_io_detect(u8 int_io_status)
{
    static u8 int_io_status_old;
    /* u8 int_io_status = 0; */
    /* int_io_status = gpio_read(INT_IO); */
    if ((int_io_status) && (int_io_status_old == 0)) {
        log_e("sc7a20_int_io_detect\n");
        SC7A20_Click_Alog();
    } else {
    }
    int_io_status_old = int_io_status;
}

void SC7A20_read_data(axis_info_t *sl_accel)
{
    u8 data[6];
    u8 fifo_len = 0;
    SL_MEMS_i2cRead(SC7A20_R_ADDR, SC7A20_SRC_REG, 1, &fifo_len);
    if ((fifo_len & 0x40) == 0x40) {
        fifo_len = 32;
    } else {
        fifo_len = fifo_len & 0x1f;
    }
    for (u8 i = 0; i < fifo_len; i++) {
        SL_MEMS_i2cRead(SC7A20_R_ADDR, 0xA8, 6, data);
        sl_accel[i].x = (short)((data[1] << 8) | data[0]);
        //   sl_accel[i].x = sl_accel[i].x >> 4;
        sl_accel[i].y = (short)((data[3] << 8) | data[2]);
        // sl_accel[i].y = sl_accel[i].y >> 4;
        sl_accel[i].z = (short)((data[5] << 8) | data[4]);
        // sl_accel[i].z = sl_accel[i].z >> 4;
        //   printf("group:%2d,sl_accel_x:%5d,  sl_accel_y:%5d,  sl_accel_z:%5d\n", i, sl_accel[i].x, sl_accel[i].y, sl_accel[i].z);
    }
}
void SC7A20_ctl(u8 cmd, void *arg)
{
    char res;
    switch (cmd) {
    case GSENSOR_DISABLE:
        res = SC7A20_disable();
        memcpy(arg, &res, 1);
        break;
    case GSENSOR_RESET_INT:
        res = SC7A20_Config();
        memcpy(arg, &res, 1);
        break;
    case GSENSOR_RESUME_INT:
        break;
    case GSENSOR_INT_DET:
        SC7A20_int_io_detect(*(u8 *)arg);
        break;
    case READ_GSENSOR_DATA:
        SC7A20_read_data((axis_info_t *)arg);
        break;
    case SEARCH_SENSOR:
        res = SC7A20_Check();
        memcpy(arg, &res, 1);
        break;
    default:

        break;
    }
}


static u8 sc7a20_idle_query(void)
{
    return !sl_click_timer_en;
}

REGISTER_GRAVITY_SENSOR(gSensor) = {
    .logo = "sc7a20",
    .gravity_sensor_init  = SC7A20_Config,
    .gravity_sensor_check = SC7A20_click_status,
    .gravity_sensor_ctl   = SC7A20_ctl,
};

REGISTER_LP_TARGET(sc7a20_lp_target) = {
    .name = "sc7a20",
    .is_idle = sc7a20_idle_query,
};


#endif

