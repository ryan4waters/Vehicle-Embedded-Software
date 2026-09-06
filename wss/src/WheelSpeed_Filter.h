#ifndef WHEEL_SPEED_FILTER_H
#define WHEEL_SPEED_FILTER_H

#include <stdbool.h>

typedef struct
{
    float speedKph;
    float accelerationKphPerS;

    float p00;
    float p01;
    float p10;
    float p11;

    bool initialized;
} WheelKalman_t;

void WheelKalman_Init(WheelKalman_t *kf, float initialSpeedKph);
float WheelKalman_Update(WheelKalman_t *kf,
                         float measurementKph,
                         float dt,
                         float measurementNoise);

#endif
