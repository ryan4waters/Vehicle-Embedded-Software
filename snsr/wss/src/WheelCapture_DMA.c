#include "WheelCapture_DMA.h"
#include "WheelCapture.h"

/*
 * The DMA should transfer capture register values to RAM with minimal CPU
 * intervention. The adapter converts each raw capture entry into:
 * timestamp + edge polarity, then calls WheelCapture_PushEvent().
 */

void WheelCapture_DMA_Init(void)
{
    /* TODO:
     * - configure DMA source = capture register
     * - destination = circular RAM buffer
     * - configure transfer width
     * - configure request source
     * - configure circular/linked-list mode as required
     */
}

void WheelCapture_DMA_Start(void)
{
    /* TODO: enable DMA channel(s). */
}

void WheelCapture_DMA_Process(void)
{
    /* TODO: consume DMA write pointer and push events into WheelCapture. */
}
