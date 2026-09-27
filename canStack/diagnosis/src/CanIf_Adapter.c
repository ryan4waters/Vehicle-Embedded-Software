#include "CanIf_Adapter.h"
#include "CanSM_Adapter.h"
#include "CanDiag.h"
void CanIf_ControllerBusOff(uint8_t c){CanSM_ControllerBusOff(c);CanDiag_ControllerBusOff();}
void CanIf_RxIndication(uint16_t pdu,const uint8_t *d,uint8_t dlc){(void)d;(void)dlc;CanDiag_RxIndication(pdu);}
void CanIf_TxConfirmation(uint16_t pdu){CanDiag_TxConfirmation(pdu);}
