#include <assert.h>
#include <math.h>
#include "CCCP.h"

int main(void)
{
    CCCP_Init();

    for (int i = 0; i < 100; ++i)
        CCCP_1msTask();

    for (int i = 0; i < 10; ++i)
        CCCP_10msTask();

    const CCCP_Status *s = CCCP_GetStatus();

    assert(s != 0);
    assert(fabs(s->cp.pwm.frequency_hz - 1000.0f) < 0.1f);
    assert(fabs(s->cp.pwm.duty - 0.5f) < 0.001f);
    assert(fabs(s->cp.evse_current_a - 30.0f) < 0.01f);

    return 0;
}
