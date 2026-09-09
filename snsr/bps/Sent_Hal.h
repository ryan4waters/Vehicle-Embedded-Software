#ifndef SENT_HAL_H
#define SENT_HAL_H

#include "Pedal_Types.h"
#include <stdbool.h>

typedef struct {
    bool (*Init)(uint8_t channel);
    bool (*ReadFrame)(uint8_t channel, SentFrame_t *frame);
    uint32_t (*GetTimestampMs)(void);
} SentHalApi_t;

const SentHalApi_t *SentHal_GetApi(void);

#endif
