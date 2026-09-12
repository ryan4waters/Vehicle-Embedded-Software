#ifndef POWERSCOPE_PLATFORM_H
#define POWERSCOPE_PLATFORM_H
#include <stdint.h>

void PowerScope_Platform_Init(void);
uint32_t PowerScope_Platform_GetTimestampUs(void);
void PowerScope_Platform_EnterCritical(void);
void PowerScope_Platform_ExitCritical(void);

/* 传输层由UART/CAN-FD/Ethernet等实现 */
void PowerScope_Platform_Send(const uint8_t *data, uint16_t len);

#endif
