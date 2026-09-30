#include "UpdateJournal.h"

Hsm_ReturnType UpdateJournal_WriteVerified(
    const UpdateJournalType *j)
{
    if (!j) return HSM_E_PARAM;
    /*
     * Store journal atomically in protected NVM.
     * Add power-loss safe double-copy / sequence counter.
     */
    return HSM_E_NOT_READY;
}

Hsm_ReturnType UpdateJournal_SelectBootSlot(
    UpdateSlotStateType *state)
{
    if (!state) return HSM_E_PARAM;
    *state = UPDATE_SLOT_EMPTY;
    return HSM_E_NOT_READY;
}
