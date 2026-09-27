#include "Tc377_CanHw.h"
#include "CanDiag.h"
#include "CanIf_Adapter.h"
void Tc377_Can_Init(void){/* Can_Init(&Can_Config) */}
void Tc377_Can_Start(void){/* Can_SetControllerMode(...STARTED) */}
void Tc377_Can_Stop(void){/* Can_SetControllerMode(...STOPPED) */}
void Tc377_Can_GetErrorState(uint16_t *tec,uint16_t *rec,bool *passive){if(tec)*tec=0;if(rec)*rec=0;if(passive)*passive=false;/* replace by MCAL */}
void Tc377_Can_BusOffIsr(void){CanIf_ControllerBusOff(0);}
