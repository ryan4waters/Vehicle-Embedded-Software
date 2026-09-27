#ifndef CANIF_ADAPTER_H
#define CANIF_ADAPTER_H
#include <stdint.h>
void CanIf_ControllerBusOff(uint8_t controller); void CanIf_RxIndication(uint16_t pdu,const uint8_t *data,uint8_t dlc); void CanIf_TxConfirmation(uint16_t pdu);
#endif
