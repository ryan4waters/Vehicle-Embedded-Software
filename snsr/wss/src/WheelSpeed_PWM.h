#ifndef WHEEL_SPEED_PWM_H
#define WHEEL_SPEED_PWM_H

#include <stdint.h>

void WheelSpeed_PWM_Init(void);
void WheelSpeed_PWM_Process(uint8_t wheel);

#endif
