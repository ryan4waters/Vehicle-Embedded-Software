#ifndef SECOC_RUNTIME_H
#define SECOC_RUNTIME_H
#include "Hsm_Types.h"

typedef struct {
    Hsm_U32 canId;
    Hsm_U8 dlc;
    Hsm_U8 data[8];
} SecCanFrameType;

Hsm_ReturnType SecOC_RuntimeTx(
    const SecCanFrameType *frame,
    Hsm_U32 freshness,
    Hsm_U8 *mac,
    Hsm_U32 *macLen);

Hsm_ReturnType SecOC_RuntimeRxVerify(
    const SecCanFrameType *frame,
    Hsm_U32 freshness,
    const Hsm_U8 *mac,
    Hsm_U32 macLen);

#endif
