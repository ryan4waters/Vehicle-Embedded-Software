#include "CanSM_Adapter.h"
#include "Tc377_CanHw.h"
static uint32_t t; static uint8_t recovery;
void CanSM_ControllerBusOff(uint8_t c){(void)c;recovery=1;t=0;Tc377_Can_Stop();}
void CanSM_MainFunction_10ms(void){if(recovery){t+=10;if(t>=100){Tc377_Can_Start();recovery=0;}}}
