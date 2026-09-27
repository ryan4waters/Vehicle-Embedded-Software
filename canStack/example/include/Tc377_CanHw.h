#ifndef TC377_CAN_HW_H
#define TC377_CAN_HW_H

#include <stdbool.h>
#include <stdint.h>

void Tc377_Can_Init(void);
void Tc377_Can_SetControllerMode(bool run);
void Tc377_Can_Tx(uint32_t canId, const uint8_t *data, uint8_t dlc);
void Tc377_Can_RxIndication(uint32_t canId, const uint8_t *data, uint8_t dlc);
void Tc377_Can_BusOffIsr(void);

#endif
