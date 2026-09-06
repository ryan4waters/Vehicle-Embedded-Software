#include "WheelCapture_eMIOS.h"

/*
 * SPC58-specific adapter.
 *
 * Configure in the actual project:
 * 1. SIUL2/IOMUX: wheel input -> eMIOS channel.
 * 2. eMIOS channel in input-capture/SAIC or equivalent mode.
 * 3. Free-running counter, common time base.
 * 4. Capture rising/falling edges as required.
 * 5. Route capture event to eDMA through the device's DMA request mechanism.
 *
 * Do NOT copy register names from this template blindly: the exact
 * SPC58NN derivative and MCAL/RTD version determine the register/API names.
 */

void WheelCapture_eMIOS_Init(void)
{
    /* TODO: configure SIUL2 + eMIOS + DMA request source. */
}

void WheelCapture_eMIOS_Start(void)
{
    /* TODO: enable timer/capture channels and DMA. */
}
