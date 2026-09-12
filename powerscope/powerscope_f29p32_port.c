/*
 * TI F29P32x适配层骨架
 *
 * 推荐：
 * ePWM SOCA/SOCB -> ADC
 * ADC EOC -> DMA/ISR
 * 使用CPU Timer或PWM time-base生成timestamp
 * 上传可选 SCI / CAN-FD / Ethernet
 */
#include "powerscope_platform.h"

void PowerScope_Platform_Init(void)
{
    /* DriverLib / SysConfig generated init */
}

uint32_t PowerScope_Platform_GetTimestampUs(void)
{
    return 0u;
}

void PowerScope_Platform_EnterCritical(void) {}
void PowerScope_Platform_ExitCritical(void) {}

void PowerScope_Platform_Send(const uint8_t *data, uint16_t len)
{
    (void)data; (void)len;
}
