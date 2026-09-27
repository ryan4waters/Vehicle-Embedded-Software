#ifndef TJA1145_DRV_H
#define TJA1145_DRV_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TJA1145_MODE_SLEEP = 0,
    TJA1145_MODE_STANDBY,
    TJA1145_MODE_NORMAL
} Tja1145_ModeType;

typedef enum {
    TJA1145_WAKE_NONE = 0,
    TJA1145_WAKE_LOCAL,
    TJA1145_WAKE_CAN,
    TJA1145_WAKE_SELECTIVE
} Tja1145_WakeupSourceType;

void Tja1145_Init(void);
void Tja1145_SetNormalMode(void);
void Tja1145_SetStandbyMode(void);
void Tja1145_SetSleepMode(void);
Tja1145_WakeupSourceType Tja1145_GetWakeupSource(void);
uint8_t Tja1145_ReadStatus(void);
void Tja1145_ClearWakeStatus(void);

#endif
