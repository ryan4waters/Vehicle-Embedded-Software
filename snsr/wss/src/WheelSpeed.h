#ifndef WHEEL_SPEED_H
#define WHEEL_SPEED_H

#include <stdint.h>
#include <stdbool.h>
#include "WheelSpeed_Filter.h"

typedef enum
{
    WHEEL_SENSOR_AK = 0U,
    WHEEL_SENSOR_PWM = 1U
} WheelSensorType_t;

typedef enum
{
    WHEEL_MEAS_INVALID = 0U,
    WHEEL_MEAS_M,
    WHEEL_MEAS_T
} WheelMeasurementSource_t;

typedef enum
{
    WHEEL_STATUS_INIT = 0U,
    WHEEL_STATUS_VALID,
    WHEEL_STATUS_TIMEOUT,
    WHEEL_STATUS_IMPLAUSIBLE,
    WHEEL_STATUS_CAPTURE_OVERFLOW
} WheelStatus_t;

typedef struct
{
    WheelSensorType_t sensorType;

    float pulsePerRev;
    float circumferenceM;

    float rawSpeedKph;
    float filteredSpeedKph;
    float rpm;
    float frequencyHz;
    float duty;

    uint32_t periodTicks;
    uint32_t highTicks;

    uint32_t risingEdgeCount;
    uint32_t fallingEdgeCount;
    uint32_t pulseCount;

    uint32_t lastEdgeTimestamp;
    uint32_t measurementTimestamp;

    WheelMeasurementSource_t source;
    WheelStatus_t status;

    bool valid;
    bool timeout;
    bool stuckHigh;
    bool stuckLow;
    bool implausible;

    WheelKalman_t kalman;
} WheelSpeedData_t;

void WheelSpeed_Init(void);
void WheelSpeed_MainFunction(void);
const WheelSpeedData_t *WheelSpeed_Get(uint8_t wheel);

#endif
