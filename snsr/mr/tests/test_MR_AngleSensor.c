#include <stdio.h>
#include <math.h>
#include "../include/MR_AngleSensor.h"

int main(void)
{
    MR_AngleSensor_t ctx;
    MR_Output_t out;
    MR_Calib_t c = {0,0,1,1,0,0,1};
    MR_Limits_t lim = {-2,2,-2,2,0.5,1.5,30000,5};

    MR_Init(&ctx, &c, 0.001f);

    for (int i = 0; i < 360; ++i)
    {
        float a = (float)i * 3.14159265359f / 180.0f;
        MR_Process(&ctx, sinf(a), cosf(a), &out, &lim);
        printf("%d, %.3f, %.3f, %.3f\n",
               i, sinf(a), cosf(a), out.angle_deg);
    }

    return 0;
}
