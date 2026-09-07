#include "WheelSpeed_AK.h"
#include "WheelCapture.h"

/*
 * AK parser adapter.
 *
 * The exact AK protocol state machine must follow the sensor/OEM
 * specification used by the project. This module deliberately separates:
 *   edge/timestamp acquisition
 * from
 *   AK pulse/protocol interpretation.
 *
 * The common wheel-speed layer consumes normalized pulse information.
 */

typedef struct
{
    uint32_t lastRise;
    uint32_t periodTicks;
    uint32_t riseCount;
    uint32_t fallCount;
} AkState_t;

static AkState_t g_ak[WHEEL_COUNT];

void WheelSpeed_AK_Init(void)
{
    for (uint8_t i = 0U; i < WHEEL_COUNT; ++i)
    {
        g_ak[i].lastRise = 0U;
        g_ak[i].periodTicks = 0U;
        g_ak[i].riseCount = 0U;
        g_ak[i].fallCount = 0U;
    }
}

void WheelSpeed_AK_Process(uint8_t wheel)
{
    WheelCaptureEvent_t e;

    while (WheelCapture_GetEvent(wheel, &e))
    {
        if (e.edge == WHEEL_EDGE_RISING)
        {
            if (g_ak[wheel].lastRise != 0U)
            {
                g_ak[wheel].periodTicks =
                    e.timestamp - g_ak[wheel].lastRise;
            }

            g_ak[wheel].lastRise = e.timestamp;
            g_ak[wheel].riseCount++;
        }
        else
        {
            g_ak[wheel].fallCount++;
        }

        /*
         * TODO: add the exact AK pulse/protocol state machine here:
         * pulse width/classification, status/diagnostic coding,
         * protocol synchronization, invalid sequence detection, etc.
         */
    }
}
