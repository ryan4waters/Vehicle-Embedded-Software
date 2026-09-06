#ifndef WDG_H
#define WDG_H

#include <stdint.h>

typedef enum
{
    WDG_STATE_INIT = 0,
    WDG_STATE_RUNNING,
    WDG_STATE_FAULT
} WdgState;

typedef enum
{
    WDG_FAULT_NONE       = 0x00000000UL,
    WDG_FAULT_APP        = 0x00000001UL,
    WDG_FAULT_DCDC       = 0x00000002UL,
    WDG_FAULT_CAN        = 0x00000004UL,
    WDG_FAULT_SAFETY     = 0x00000008UL,
    WDG_FAULT_DEADLINE   = 0x00000010UL,
    WDG_FAULT_INTERNAL   = 0x00000020UL,
    WDG_FAULT_EXTERNAL   = 0x00000040UL,
    WDG_FAULT_ESM        = 0x00000080UL,
    WDG_FAULT_SMU        = 0x00000100UL
} WdgFault;

typedef struct
{
    volatile uint32_t appAlive;
    volatile uint32_t dcdcAlive;
    volatile uint32_t canAlive;
    volatile uint32_t safetyAlive;

    uint32_t lastAppAlive;
    uint32_t lastDcdcAlive;
    uint32_t lastCanAlive;
    uint32_t lastSafetyAlive;

    volatile uint32_t fault;
    volatile WdgState state;

} WdgContext;

void Wdg_Init(void);
void Wdg_MainFunction(void);

void Wdg_NotifyApp(void);
void Wdg_NotifyDcdc(void);
void Wdg_NotifyCan(void);
void Wdg_NotifySafety(void);

uint32_t Wdg_IsHealthy(void);

#endif