#include "Pedal_App.h"

bool PedalApp_Init(PedalApp_t *app,
                   const PedalProfile_t *profile,
                   uint8_t sent_ch1,
                   uint8_t sent_ch2)
{
    if ((app == 0) || (profile == 0)) return false;

    PedalDiag_Init(&app->diag);
    return PedalSent_Init(&app->sent, profile, sent_ch1, sent_ch2);
}

void PedalApp_1ms(PedalApp_t *app,
                  uint8_t sent_ch1,
                  uint8_t sent_ch2)
{
    if (app == 0) return;

    PedalSent_Main1ms(&app->sent, sent_ch1, sent_ch2);
    PedalDiag_Update(&app->diag, PedalSent_GetSignal(&app->sent));
}

const PedalSignal_t *PedalApp_Get(const PedalApp_t *app)
{
    if (app == 0) return 0;
    return PedalSent_GetSignal(&app->sent);
}
