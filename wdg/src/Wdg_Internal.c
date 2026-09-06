#include "Wdg_Internal.h"

void Wdg_Internal_Init(void)
{
    /*
     * Configure MCU internal WDG.
     *
     * TC377:
     *   CPU WDG / Safety WDG
     *
     * F29:
     *   Internal WWD
     *
     * Exact register/API configuration should be
     * implemented in the MCU-specific layer.
     */
}

void Wdg_Internal_Service(void)
{
    /*
     * Internal WDG service.
     *
     * Do not service this watchdog from an
     * independent timer ISR.
     *
     * Service only after application supervision
     * has passed.
     */
}