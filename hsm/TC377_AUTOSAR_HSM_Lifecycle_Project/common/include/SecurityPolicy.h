#ifndef SECURITY_POLICY_H
#define SECURITY_POLICY_H
#include "Hsm_Types.h"

typedef enum {
    SEC_PHASE_FACTORY = 0,
    SEC_PHASE_STARTUP,
    SEC_PHASE_RUNTIME,
    SEC_PHASE_UPDATE,
    SEC_PHASE_AFTER_SALES
} SecurityPhaseType;

typedef enum {
    SEC_OP_BOOT_VERIFY = 0,
    SEC_OP_SECOC,
    SEC_OP_DIAG_UNLOCK,
    SEC_OP_KEY_PROVISION,
    SEC_OP_KEY_ROTATE,
    SEC_OP_FIRMWARE_UPDATE,
    SEC_OP_RECOVERY
} SecurityOperationType;

Hsm_U8 SecurityPolicy_IsAllowed(
    SecurityPhaseType phase,
    SecurityOperationType operation);

void SecurityPolicy_SetPhase(SecurityPhaseType phase);
SecurityPhaseType SecurityPolicy_GetPhase(void);

#endif
