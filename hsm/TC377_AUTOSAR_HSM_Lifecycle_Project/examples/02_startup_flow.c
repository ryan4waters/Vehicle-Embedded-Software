#include "BootManager.h"

void Example_Startup(void)
{
    BootManager_Init();

    /*
     * Normally called from bootloader main loop / OS startup phase.
     */
    for (;;)
    {
        BootManager_MainFunction();
    }
}
