#include "WheelSpeed_PWM.h"
#include "WheelCapture.h"

typedef struct
{
    uint32_t lastRise;
    uint32_t lastFall;

    uint32_t periodTicks;
    uint32_t highTicks;

    uint32_t riseCount;
    uint32_t fallCount;
} PwmState_t;

static PwmState_t g_pwm[WHEEL_COUNT];

void WheelSpeed_PWM_Init(void)
{
    for (uint8_t i = 0U; i < WHEEL_COUNT; ++i)
    {
        g_pwm[i].lastRise = 0U;
        g_pwm[i].lastFall = 0U;
        g_pwm[i].periodTicks = 0U;
        g_pwm[i].highTicks = 0U;
        g_pwm[i].riseCount = 0U;
        g_pwm[i].fallCount = 0U;
    }
}

void WheelSpeed_PWM_Process(uint8_t wheel)
{
    WheelCaptureEvent_t e;

    while (WheelCapture_GetEvent(wheel, &e))
    {
        if (e.edge == WHEEL_EDGE_RISING)
        {
            if (g_pwm[wheel].lastRise != 0U)
            {
                g_pwm[wheel].periodTicks =
                    e.timestamp - g_pwm[wheel].lastRise;
            }

            g_pwm[wheel].lastRise = e.timestamp;
            g_pwm[wheel].riseCount++;
        }
        else
        {
            if (g_pwm[wheel].lastRise != 0U)
            {
                g_pwm[wheel].highTicks =
                    e.timestamp - g_pwm[wheel].lastRise;
            }

            g_pwm[wheel].lastFall = e.timestamp;
            g_pwm[wheel].fallCount++;
        }
    }
}
