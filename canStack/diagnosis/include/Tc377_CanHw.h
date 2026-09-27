#ifndef TC377_CAN_HW_H
#define TC377_CAN_HW_H
#include <stdint.h>
#include <stdbool.h>
void Tc377_Can_Init(void); void Tc377_Can_Start(void); void Tc377_Can_Stop(void); void Tc377_Can_GetErrorState(uint16_t *tec,uint16_t *rec,bool *passive); void Tc377_Can_BusOffIsr(void);
#endif
