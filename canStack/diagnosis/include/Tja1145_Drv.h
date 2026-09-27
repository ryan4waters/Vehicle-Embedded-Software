#ifndef TJA1145_DRV_H
#define TJA1145_DRV_H
#include <stdbool.h>
typedef struct { bool undervoltage; bool overtemperature; bool txdDominantTimeout; bool canWake; } Tja1145_StatusType;
void Tja1145_Init(void); bool Tja1145_ReadDiagnostic(Tja1145_StatusType *s); void Tja1145_SetNormalMode(void); void Tja1145_SetStandbyMode(void); void Tja1145_SetSleepMode(void);
#endif
