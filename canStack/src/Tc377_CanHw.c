#include "Tc377_CanHw.h"
#include "CanStack_Ref.h"

/*
 * This is an MCAL adapter boundary.
 * A production project should call the vendor Can MCAL rather than
 * directly touching MCMCAN registers from the communication service.
 */

void Tc377_Can_Init(void)
{
    /* Can_Init(&Can_Config); */
}

void Tc377_Can_SetControllerMode(bool run)
{
    if (run) {
        /* Can_SetControllerMode(CAN_CONTROLLER, CAN_CS_STARTED); */
    } else {
        /* Can_SetControllerMode(CAN_CONTROLLER, CAN_CS_STOPPED); */
    }
}

void Tc377_Can_Tx(uint32_t canId, const uint8_t *data, uint8_t dlc)
{
    (void)canId;
    (void)data;
    (void)dlc;
    /* Can_Write(Hth, &PduInfo); */
}

void Tc377_Can_RxIndication(uint32_t canId, const uint8_t *data, uint8_t dlc)
{
    /* This models CanIf_RxIndication() upward path. */
    (void)canId;
    (void)data;
    (void)dlc;
}

void Tc377_Can_BusOffIsr(void)
{
    /* Real path:
     * MCMCAN ISR -> CanDrv -> CanIf_ControllerBusOff -> CanSM.
     */
    CanStack_HandleBusOff();
}
