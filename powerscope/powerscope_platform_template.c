#include "powerscope_platform.h"

void PowerScope_Platform_Init(void)
{
    /* 初始化时间基准、传输接口 */
}

uint32_t PowerScope_Platform_GetTimestampUs(void)
{
    /* 替换成MCU free-running timer */
    static uint32_t fake_time;
    return fake_time++;
}

void PowerScope_Platform_EnterCritical(void) {}
void PowerScope_Platform_ExitCritical(void) {}

void PowerScope_Platform_Send(const uint8_t *data, uint16_t len)
{
    (void)data;
    (void)len;
    /* 替换成 UART/CAN-FD/Ethernet */
}
