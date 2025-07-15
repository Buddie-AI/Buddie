#include "system/includes.h"

/* --------------------------------------------------------------------------*/
/**
 * @brief A 任务
 *
 * @param p
 */
/* ----------------------------------------------------------------------------*/
static void os_demo_A(void *p)
{
    while (1) {
        printf("os_demo_A runnning");
        os_time_dly(100);
    }
}


/* --------------------------------------------------------------------------*/
/**
 * @brief B 任务
 *
 * @param p
 */
/* ----------------------------------------------------------------------------*/
static void os_demo_B(void *p)
{
    while (1) {
        printf("os_demo_B runnning");
        os_time_dly(100);
    }
}


/* --------------------------------------------------------------------------*/
/**
 * @brief 任务创建实例，禁止在关闭中断或中断中调用此函数
 */
/* ----------------------------------------------------------------------------*/
void os_test(void)
{
    int ret;

    ret = os_task_create(os_demo_A, NULL, 31, 512, 0, "demo_A");
    if (ret != OS_NO_ERR) {
        printf("os_demo_A create err : %d", ret);
    }

    ret = os_task_create(os_demo_B, NULL, 31, 512, 0, "demo_B");
    if (ret != OS_NO_ERR) {
        printf("os_demo_B create err : %d", ret);
    }

    while (1) {
        os_time_dly(1);
    }
}

