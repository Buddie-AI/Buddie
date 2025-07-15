#ifndef __POWER_RESET_H__
#define __POWER_RESET_H__

/*
 *复位原因包括两种
 1.系统复位源: p33 p11 主系统
 2.自定义复位源：断言、异常等
 */
enum RST_REASON {
    /*主系统*/
    MSYS_P11_RST,						//P11复位
    MSYS_DVDD_POR_RST,					//DVDD上电
    MSYS_SOFT_RST,						//主系统软件复位
    MSYS_P2M_RST,						//低功耗唤醒复位(softoff advance && deepsleep)
    MSYS_POWER_RETURN,					//主系统未被复位

    /*P11*/
    P11_PVDD_POR_RST,					//pvdd上电
    P11_IVS_RST,						//低功耗唤醒复位(softoff legacy)
    P11_P33_RST,						//p33复位
    P11_WDT_RST,						//看门狗复位
    P11_SOFT_RST,						//软件复位
    P11_MSYS_RST,						//主系统复位P11
    P11_POWER_RETURN,					//P11系统未被复位

    /*P33*/
    P33_VDDIO_POR_RST,					//vddio上电复位(电池/vpwr供电)
    P33_VDDIO_LVD_RST,					//vddio低压复位、上电复位(电池/vpwr供电)
    P33_VCM_RST,						//vcm高电平短接复位
    P33_PPINR_RST,						//数字io输入长按复位
    P33_P11_RST,						//p11系统复位p33，rset_mask=0
    P33_SOFT_RST,						//p33软件复位，一般软件复位指此系统复位源，所有系统会直接复位。
    P33_PPINR1_RST,						//模拟io输入长按复位，包括charge_full、vatch、ldoint、vabt_det
    P33_POWER_RETURN,					//p33系统未被复位。

    //SUB
    P33_EXCEPTION_SOFT_RST,	    		//异常软件复位
    P33_ASSERT_SOFT_RST,				//断言软件复位
    P33_XOSC_RESUME_ERR_RST,			//快速起振恢复失败软件复位
    P33_LPTMR_TIMEOUT_RST,				//LPTMR软件复位
};

void power_reset_close();

void reset_source_dump(void);

void p33_soft_reset(void);

void set_reset_source_value(enum RST_REASON index);

u32 get_reset_source_value(void);

u8 is_reset_source(enum RST_REASON index);

int cpu_reset_by_soft();

#endif


