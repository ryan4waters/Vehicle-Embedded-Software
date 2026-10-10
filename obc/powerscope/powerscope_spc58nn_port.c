/*
 * SPC58NN适配层骨架
 *
 * 推荐：
 * ADC/eMIOS/Timer产生固定采样节拍
 * DMA/ISR更新应用变量
 * DSPI/FlexCAN/Ethernet上传
 */
#include "powerscope_platform.h"

void PowerScope_Platform_Init(void)
{
    /* SPC5Studio / MCAL / SDK init */
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
