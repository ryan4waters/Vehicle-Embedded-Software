#ifndef FACTORY_SECURITY_H
#define FACTORY_SECURITY_H
#include "Hsm_Types.h"

typedef struct {
    Hsm_U32 deviceId;
    const Hsm_U8 *certificate;
    Hsm_U32 certificateLen;
} FactoryDeviceIdentityType;

Hsm_ReturnType FactorySecurity_ProvisionDevice(
    const FactoryDeviceIdentityType *identity);

Hsm_ReturnType FactorySecurity_RunEndOfLineSecurityTest(void);

Hsm_ReturnType FactorySecurity_ProductionLockPreCheck(void);

#endif
