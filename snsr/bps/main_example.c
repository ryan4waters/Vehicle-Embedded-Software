#include "Pedal_App.h"

/*
 * Example application integration.
 * Replace raw range values with the selected HELLA sensor's interface data.
 */
static PedalProfile_t g_profile = {
    .raw_min = 0u,
    .raw_max = 4095u,
    .raw_zero = 200u,
    .raw_full = 3800u,
    .max_plausibility_delta_pct = 10u,
    .timeout_ms = 20u,
    .debounce_ms = 5u
};

static PedalApp_t g_pedal;

void App_PedalInit(void)
{
    /*
     * SENT channel IDs are project-defined.
     * Example: 0 = pedal channel A, 1 = pedal channel B.
     */
    (void)PedalApp_Init(&g_pedal, &g_profile, 0u, 1u);
}

/* Call every 1 ms from an OS task or timer ISR. */
void App_PedalTask1ms(void)
{
    PedalApp_1ms(&g_pedal, 0u, 1u);

    const PedalSignal_t *s = PedalApp_Get(&g_pedal);

    if ((s != 0) && (s->status == PEDAL_OK)) {
        /* Feed s->demand_pct to the upper vehicle-control layer. */
    } else {
        /* Safe reaction: inhibit/limit torque according to vehicle safety concept. */
    }
}
