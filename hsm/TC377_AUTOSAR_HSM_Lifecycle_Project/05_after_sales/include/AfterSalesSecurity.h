#ifndef AFTER_SALES_SECURITY_H
#define AFTER_SALES_SECURITY_H
#include "Hsm_Types.h"

Hsm_ReturnType AfterSales_AuthenticateTester(
    const Hsm_U8 *request, Hsm_U32 len);

Hsm_ReturnType AfterSales_StartSecureUpdate(void);

Hsm_ReturnType AfterSales_RecoverToFactorySafeState(void);

#endif
