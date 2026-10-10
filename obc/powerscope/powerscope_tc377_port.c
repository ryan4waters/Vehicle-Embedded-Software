/*
 * TC377适配层骨架
 *
 * 推荐：
 * 1. GTM TOM/ATOM产生PWM同步点
 * 2. GTM -> EVADC硬件触发，在PWM固定相位采样
 * 3. EVADC结果DMA/ISR更新应用变量
 * 4. PowerScope_SampleISR()只做轻量复制
 *
 * 具体寄存器/引脚/IRQ优先级依项目MCAL/iLLD配置。
 */
#include "powerscope_platform.h"

void PowerScope_Platform_Init(void)
{
    /* IfxGtm / IfxEvadc / DMA / UART or CAN-FD */
}

uint32_t PowerScope_Platform_GetTimestampUs(void)
{
    /* 使用STM/GTM free-running counter */
    return 0u;
}

void PowerScope_Platform_EnterCritical(void) {}
void PowerScope_Platform_ExitCritical(void) {}

void PowerScope_Platform_Send(const uint8_t *data, uint16_t len)
{
    (void)data; (void)len;
}
