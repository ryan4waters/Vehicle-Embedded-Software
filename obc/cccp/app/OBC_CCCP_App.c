#include "CCCP.h"
#include "OBC_PDU.h"

void OBC_CCCP_AppInit(void)
{
    CCCP_Init();
    OBC_PDU_Init();
}

void OBC_CCCP_App_1ms(void)
{
    CCCP_1msTask();
}

void OBC_CCCP_App_10ms(void)
{
    CCCP_10msTask();
    OBC_PDU_10msTask(CCCP_GetStatus());
}
