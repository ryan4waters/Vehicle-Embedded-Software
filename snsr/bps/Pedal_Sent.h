#ifndef PEDAL_SENT_H
#define PEDAL_SENT_H

#include "Pedal_Types.h"
#include "Sent_Hal.h"

typedef struct {
    SentHalApi_t hal;
    PedalProfile_t profile;
    PedalSignal_t signal;
    uint32_t last_frame_ms[2];
    uint16_t last_raw[2];
    uint16_t diag_debounce[32];
    bool initialized;
} PedalSent_t;

bool PedalSent_Init(PedalSent_t *ctx,
                    const PedalProfile_t *profile,
                    uint8_t sent_ch1,
                    uint8_t sent_ch2);

void PedalSent_Main1ms(PedalSent_t *ctx,
                       uint8_t sent_ch1,
                       uint8_t sent_ch2);

const PedalSignal_t *PedalSent_GetSignal(const PedalSent_t *ctx);

#endif
