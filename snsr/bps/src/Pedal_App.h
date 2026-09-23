#ifndef PEDAL_APP_H
#define PEDAL_APP_H

#include "Pedal_Sent.h"
#include "Pedal_Diag.h"

typedef struct {
    PedalSent_t sent;
    PedalDiag_t diag;
} PedalApp_t;

bool PedalApp_Init(PedalApp_t *app,
                   const PedalProfile_t *profile,
                   uint8_t sent_ch1,
                   uint8_t sent_ch2);

void PedalApp_1ms(PedalApp_t *app,
                  uint8_t sent_ch1,
                  uint8_t sent_ch2);

const PedalSignal_t *PedalApp_Get(const PedalApp_t *app);

#endif
