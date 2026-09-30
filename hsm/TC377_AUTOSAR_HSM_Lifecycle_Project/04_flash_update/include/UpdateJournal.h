#ifndef UPDATE_JOURNAL_H
#define UPDATE_JOURNAL_H
#include "Hsm_Types.h"

typedef enum {
    UPDATE_SLOT_EMPTY = 0,
    UPDATE_SLOT_DOWNLOADING,
    UPDATE_SLOT_VERIFIED,
    UPDATE_SLOT_ACTIVE,
    UPDATE_SLOT_INVALID
} UpdateSlotStateType;

typedef struct {
    Hsm_U32 magic;
    Hsm_U32 version;
    Hsm_U32 imageSize;
    Hsm_U32 imageCrc;
    UpdateSlotStateType state;
} UpdateJournalType;

Hsm_ReturnType UpdateJournal_WriteVerified(
    const UpdateJournalType *j);

Hsm_ReturnType UpdateJournal_SelectBootSlot(
    UpdateSlotStateType *state);

#endif
