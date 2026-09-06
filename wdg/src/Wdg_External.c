#include "Wdg_External.h"

void Wdg_External_Init(void)
{
    /*
     * TC377 + TLF35584:
     *
     * 1. QSPI initialization
     * 2. TLF35584 SPI configuration
     * 3. WWD configuration
     * 4. FWD configuration
     * 5. ERR/FSP configuration
     * 6. First watchdog service
     *
     *
     * F29 + SGM820:
     *
     * 1. Configure WDI GPIO
     * 2. Configure nWDO XINT
     * 3. Connect nRESET to XRSn
     */
}

void Wdg_External_Service(void)
{
    /*
     * External watchdog service.
     *
     * TLF35584:
     *   WWD/FWD service
     *
     * SGM820:
     *   WDI falling edge
     */
}