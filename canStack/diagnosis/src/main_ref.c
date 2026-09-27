#include "CanDiag.h"
#include "CanSM_Adapter.h"
#include "Dem_Adapter.h"
#include "Tja1145_Drv.h"
#include "Tc377_CanHw.h"
int main(void){Tc377_Can_Init();Tja1145_Init();CanDiag_Init();Tc377_Can_Start();for(;;){CanSM_MainFunction_10ms();CanDiag_MainFunction_10ms();Dem_MainFunction_10ms();}return 0;}
