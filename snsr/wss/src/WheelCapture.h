#ifndef WHEEL_CAPTURE_H
#define WHEEL_CAPTURE_H

#include <stdint.h>
#include <stdbool.h>
#include "WheelSpeed_Cfg.h"

typedef enum
{
    WHEEL_EDGE_RISING = 0U,
    WHEEL_EDGE_FALLING = 1U
} WheelEdgeType_t;

typedef struct
{
    uint32_t timestamp;
    WheelEdgeType_t edge;
} WheelCaptureEvent_t;

void WheelCapture_Init(void);
bool WheelCapture_GetEvent(uint8_t wheel, WheelCaptureEvent_t *event);

/* Hardware/MCAL adapter hooks. */
void WheelCapture_HwInit(void);
void WheelCapture_HwStart(void);

#endif
