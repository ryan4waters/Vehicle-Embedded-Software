#include "Dem_Adapter.h"
static uint8_t s[16];
void Dem_SetEventStatus(uint16_t id,Dem_StatusType st){if(id<16)s[id]=(uint8_t)st;}
uint8_t Dem_GetEventStatus(uint16_t id){return id<16?s[id]:0;}
void Dem_MainFunction_10ms(void){/* real DEM: debounce, operation cycle, healing, aging, DTC status */}
