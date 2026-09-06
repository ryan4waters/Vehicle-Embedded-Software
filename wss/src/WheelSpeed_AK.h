#ifndef WHEEL_SPEED_AK_H
#define WHEEL_SPEED_AK_H

#include <stdint.h>

void WheelSpeed_AK_Init(void);
void WheelSpeed_AK_Process(uint8_t wheel);

#endif
