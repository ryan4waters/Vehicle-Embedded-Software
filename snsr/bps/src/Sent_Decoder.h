#ifndef SENT_DECODER_H
#define SENT_DECODER_H

#include "Pedal_Types.h"

typedef struct {
    SentConfig_t cfg;
    SentState_e state;
    uint8_t nibble_index;
    uint8_t nibble_buf[8];
    uint16_t last_period_ticks;
    uint32_t frame_counter;
    uint32_t error_counter;
} SentDecoder_t;

void SentDecoder_Init(SentDecoder_t *d, const SentConfig_t *cfg);
bool SentDecoder_DecodeFrame(SentDecoder_t *d,
                             const uint16_t *period_ticks,
                             uint8_t period_count,
                             SentFrame_t *out);

#endif
