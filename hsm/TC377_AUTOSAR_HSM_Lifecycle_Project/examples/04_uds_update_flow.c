#include "UdsUpdate.h"

void Example_UdsUpdate(void)
{
    /*
     * 0x10 02
     */
    (void)UdsUpdate_OnSessionControl(0x02u);

    /*
     * 0x27 security access
     */
    /* (void)UdsUpdate_OnSecurityAccess(...); */

    /*
     * 0x34 request download
     */
    /* (void)UdsUpdate_RequestDownload(&metadata); */

    /*
     * repeated 0x36 transfer data
     */
    /* (void)UdsUpdate_TransferData(address, data, len); */

    /*
     * 0x37 request transfer exit
     */
    (void)UdsUpdate_RequestTransferExit();

    /*
     * 0x31 routine control / commit
     */
    (void)UdsUpdate_RoutineControlCommit();
}
