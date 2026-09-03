/**
 * @file    main.c
 * @brief   示例主程序：初始化并周期调用 EcuM 状态机
 */

#include "EcuM.h"

int main(void)
{
    /* 硬件初始化（时钟、GPIO 等） */
    // SystemInit();

    /* 模拟上电唤醒：设置一个唤醒源 */
    Ecum_SetWakeupSource(1);

    /* 主循环 */
    while (1)
    {
        Ecum_MainFunction();

        /* 其他应用任务 */
        // App_Task();
    }
}