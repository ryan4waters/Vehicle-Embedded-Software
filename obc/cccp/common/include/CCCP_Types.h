#ifndef CCCP_TYPES_H
#define CCCP_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    CCCP_CP_UNKNOWN = 0,
    CCCP_CP_A_12V,
    CCCP_CP_B_9V,
    CCCP_CP_C_6V,
    CCCP_CP_FAULT_0V,
    CCCP_CP_NEG_12V
} CCCP_CpState;

typedef enum
{
    CCCP_STATE_OFF = 0,
    CCCP_STATE_WAIT_PLUG,
    CCCP_STATE_PLUGGED,
    CCCP_STATE_WAIT_BMS,
    CCCP_STATE_PRECHARGE,
    CCCP_STATE_CHARGING,
    CCCP_STATE_STOPPING,
    CCCP_STATE_FAULT
} CCCP_State;

typedef struct
{
    uint32_t period_ticks;
    uint32_t high_ticks;
    uint32_t timer_hz;
    bool valid;
} CCCP_PlatformPwmCapture;

typedef struct
{
    uint32_t period_ticks;
    uint32_t high_ticks;
    uint32_t timer_hz;
    float period_us;
    float high_us;
    float frequency_hz;
    float duty;
    bool valid;
} CCCP_PwmMeasure;

typedef struct
{
    float voltage_v;
    CCCP_CpState state;
    CCCP_PwmMeasure pwm;
    float evse_current_a;
    bool pwm_valid;
    bool digital_comm_required;
    bool valid;
} CCCP_CpInfo;

typedef struct
{
    float resistance_ohm;
    float cable_current_a;
    bool valid;
} CCCP_CcInfo;

typedef struct
{
    CCCP_CpInfo cp;
    CCCP_CcInfo cc;

    float bms_current_a;
    float bms_power_w;
    float thermal_current_a;
    float oem_current_a;
    float obc_current_a;
    float oem_power_w;
    float obc_power_w;

    float current_limit_a;
    float power_limit_w;

    bool plug_present;
    bool vehicle_ready;
    bool charge_ready;
    bool charge_allowed;
    bool fault;
} CCCP_Status;

#endif
