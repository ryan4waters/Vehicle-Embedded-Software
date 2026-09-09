#include "OBC_PowerLimit.h"

float OBC_PowerLimit_GetCurrentRefA(const CCCP_Status *s)
{
    return s->current_limit_a;
}

float OBC_PowerLimit_GetPowerRefW(const CCCP_Status *s)
{
    return s->power_limit_w;
}
