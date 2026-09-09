#include "CCCP.h"
#include "CCCP_Cfg.h"
#include "CCCP_Platform.h"

void CCCP_MeasureCp(CCCP_CpInfo *cp)
{
    CCCP_PlatformPwmCapture raw = CCCP_Platform_GetCpPwmCapture();

    cp->voltage_v = CCCP_Platform_ReadCpVoltage();
    cp->pwm.period_ticks = raw.period_ticks;
    cp->pwm.high_ticks = raw.high_ticks;
    cp->pwm.timer_hz = raw.timer_hz;
    cp->pwm.valid = raw.valid;
    cp->pwm.frequency_hz = 0.0f;
    cp->pwm.duty = 0.0f;
    cp->pwm.period_us = 0.0f;
    cp->pwm.high_us = 0.0f;
    cp->pwm_valid = false;

    if (raw.valid && raw.period_ticks > 0U && raw.timer_hz > 0U)
    {
        cp->pwm.period_us =
            ((float)raw.period_ticks * 1000000.0f) /
            (float)raw.timer_hz;

        cp->pwm.high_us =
            ((float)raw.high_ticks * 1000000.0f) /
            (float)raw.timer_hz;

        cp->pwm.frequency_hz =
            (float)raw.timer_hz / (float)raw.period_ticks;

        cp->pwm.duty =
            (float)raw.high_ticks / (float)raw.period_ticks;

        cp->pwm_valid =
            (cp->pwm.frequency_hz >= CCCP_CP_FREQ_MIN_HZ) &&
            (cp->pwm.frequency_hz <= CCCP_CP_FREQ_MAX_HZ) &&
            (cp->pwm.duty >= CCCP_CP_PWM_MIN_VALID) &&
            (cp->pwm.duty <= CCCP_CP_PWM_MAX_VALID);
    }

    cp->valid = true;
}

void CCCP_MeasureCc(CCCP_CcInfo *cc)
{
    cc->resistance_ohm = CCCP_Platform_ReadCcResistance();
    cc->valid = (cc->resistance_ohm > 0.0f);
}
