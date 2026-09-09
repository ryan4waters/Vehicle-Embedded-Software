#ifndef PEDAL_DIAG_H
#define PEDAL_DIAG_H

#include "Pedal_Types.h"

typedef struct {
    uint32_t total_cycles;
    uint32_t valid_cycles;
    uint32_t fault_cycles;
    uint32_t implausible_cycles;
    uint32_t timeout_cycles;
    uint32_t crc_cycles;
    bool latched_fault;
} PedalDiag_t;

void PedalDiag_Init(PedalDiag_t *d);
void PedalDiag_Update(PedalDiag_t *d, const PedalSignal_t *s);
bool PedalDiag_IsSafe(const PedalDiag_t *d);

#endif
