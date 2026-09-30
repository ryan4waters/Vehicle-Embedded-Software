#ifndef UDS_UPDATE_H
#define UDS_UPDATE_H
#include "Hsm_Types.h"

typedef enum {
    UDS_UPD_IDLE = 0,
    UDS_UPD_AUTH,
    UDS_UPD_ERASE,
    UDS_UPD_DOWNLOAD,
    UDS_UPD_HASH,
    UDS_UPD_SIGNATURE,
    UDS_UPD_VERSION,
    UDS_UPD_COMMIT,
    UDS_UPD_ERROR
} UdsUpdateStateType;

typedef struct {
    Hsm_U32 targetAddress;
    Hsm_U32 imageSize;
    Hsm_U32 version;
    Hsm_U8 expectedHash[32];
    Hsm_U8 signature[64];
} UdsImageMetadataType;

Hsm_ReturnType UdsUpdate_OnSessionControl(Hsm_U8 session);
Hsm_ReturnType UdsUpdate_OnSecurityAccess(
    const Hsm_U8 *request, Hsm_U32 len);
Hsm_ReturnType UdsUpdate_RequestDownload(
    const UdsImageMetadataType *meta);
Hsm_ReturnType UdsUpdate_TransferData(
    Hsm_U32 address, const Hsm_U8 *data, Hsm_U32 len);
Hsm_ReturnType UdsUpdate_RequestTransferExit(void);
Hsm_ReturnType UdsUpdate_RoutineControlCommit(void);

#endif
