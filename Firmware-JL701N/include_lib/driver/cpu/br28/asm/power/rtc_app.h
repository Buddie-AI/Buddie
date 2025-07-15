#ifndef __RTC_APP_H__
#define __RTC_APP_H__

enum {
    R3_WKUP_SRC_ALM = 0,
    R3_WKUP_SRC_256HZ = 4,
    R3_WKUP_SRC_64HZ,
    R3_WKUP_SRC_2HZ,
    R3_WKUP_SRC_1HZ,
};


/************************R3_ALM_CON*****************************/
#define ALM_ALMOUT(a)				P33_CON_SET(R3_ALM_CON, 7, 1, a)

#define ALM_CLK_SEL(a)				P33_CON_SET(R3_ALM_CON, 2, 3, a)

#define ALM_ALMEN(a)				P33_CON_SET(R3_ALM_CON, 0, 1, a)

//Macro for CLK_SEL
enum {
    CLK_SEL_32K = 1,
    CLK_SEL_12M,
    CLK_SEL_24M,
    CLK_SEL_LRC,
};

/************************R3_RTC_CON0*****************************/
#define RTC_ALM_RDEN(a)				P33_CON_SET(R3_RTC_CON0, 5, 1, a)

#define RTC_RTC_RDEN(a)				P33_CON_SET(R3_RTC_CON0, 4, 1, a)

#define RTC_ALM_WREN(a)				P33_CON_SET(R3_RTC_CON0, 1, 1, a)

#define RTC_RTC_WREN(a)				P33_CON_SET(R3_RTC_CON0, 0, 1, a)

/************************R3_OSL_CON*****************************/
#define OSL_X32XS(a)				P33_CON_SET(R3_OSL_CON, 4, 2, a)

#define OSL_X32TS(a)				P33_CON_SET(R3_OSL_CON, 2, 1, a)

#define OSL_X32OS(a)				P33_CON_SET(R3_OSL_CON, 1, 1, a)

#define OSL_X32ES(a)				P33_CON_SET(R3_OSL_CON, 0, 1, a)

/************************R3_TIME_CPND*****************************/
#define TIME_256HZ_CPND(a)			P33_CON_SET(R3_TIME_CPND, 0, 1, a)

#define TIME_64HZ_CPND(a)			P33_CON_SET(R3_TIME_CPND, 1, 1, a)

#define TIME_2HZ_CPND(a)			P33_CON_SET(R3_TIME_CPND, 2, 1, a)

#define TIME_1HZ_CPND(a)			P33_CON_SET(R3_TIME_CPND, 3, 1, a)

#endif
