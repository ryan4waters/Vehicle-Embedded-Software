#ifndef WHEEL_CAPTURE_DMA_H
#define WHEEL_CAPTURE_DMA_H

#include <stdint.h>

void WheelCapture_DMA_Init(void);
void WheelCapture_DMA_Start(void);

/* Called from the DMA/capture adapter when new entries are available. */
void WheelCapture_DMA_Process(void);

#endif
