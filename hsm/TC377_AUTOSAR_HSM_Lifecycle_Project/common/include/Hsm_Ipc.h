#ifndef HSM_IPC_H
#define HSM_IPC_H
#include "Hsm_Types.h"

Hsm_ReturnType Hsm_IpcInit(void);
Hsm_ReturnType Hsm_IpcSubmit(
    Hsm_JobType job,
    const Hsm_JobRequestType *req,
    Hsm_JobCallbackType cb,
    Hsm_U32 context);
Hsm_ReturnType Hsm_IpcProcess(void);
void Hsm_Ipc_CompletionIsr(void);

#endif
