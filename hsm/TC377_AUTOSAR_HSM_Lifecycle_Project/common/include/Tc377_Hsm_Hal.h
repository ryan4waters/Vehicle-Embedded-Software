#ifndef TC377_HSM_HAL_H
#define TC377_HSM_HAL_H
#include "Hsm_Types.h"

void Tc377_HsmHal_Init(void);
Hsm_ReturnType Tc377_HsmHal_Execute(
    Hsm_JobType job,
    Hsm_JobRequestType *req);

#endif
