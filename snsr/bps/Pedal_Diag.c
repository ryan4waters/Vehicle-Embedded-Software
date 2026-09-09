#include "Pedal_Diag.h"

void PedalDiag_Init(PedalDiag_t *d)
{
    if (d == 0) return;
    d->total_cycles = 0u;
    d->valid_cycles = 0u;
    d->fault_cycles = 0u;
    d->implausible_cycles = 0u;
    d->timeout_cycles = 0u;
    d->crc_cycles = 0u;
    d->latched_fault = false;
}

void PedalDiag_Update(PedalDiag_t *d, const PedalSignal_t *s)
{
    if ((d == 0) || (s == 0)) return;

    d->total_cycles++;

    if (s->status == PEDAL_OK) {
        d->valid_cycles++;
        return;
    }

    d->fault_cycles++;

    if (s->status == PEDAL_PLAUSIBILITY) d->implausible_cycles++;
    if (s->status == PEDAL_TIMEOUT) d->timeout_cycles++;

    if ((s->diag_bits & (PEDAL_DIAG_S1_CRC | PEDAL_DIAG_S2_CRC)) != 0u) {
        d->crc_cycles++;
    }

    /*
     * Production implementation should apply project-specific debounce and
     * safety reaction thresholds. For an ASIL application, this is not a
     * substitute for the project's safety concept.
     */
    if (d->fault_cycles >= 3u) {
        d->latched_fault = true;
    }
}

bool PedalDiag_IsSafe(const PedalDiag_t *d)
{
    return (d != 0) && !d->latched_fault;
}
