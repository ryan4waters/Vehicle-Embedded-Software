#ifndef CANSTACK_REF_H
#define CANSTACK_REF_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    CANSTACK_NO_COMM = 0,
    CANSTACK_PREPARE_SLEEP,
    CANSTACK_FULL_COMM,
    CANSTACK_BUSOFF_RECOVERY,
    CANSTACK_WAKEUP
} CanStack_StateType;

typedef enum {
    CAN_WAKE_NONE = 0,
    CAN_WAKE_KL15,
    CAN_WAKE_CAN_BUS,
    CAN_WAKE_NM_SELECTIVE,
    CAN_WAKE_TIMER,
    CAN_WAKE_UNKNOWN
} CanStack_WakeupSourceType;

void CanStack_Init(void);
void CanStack_MainFunction_10ms(void);
void CanStack_RequestFullCommunication(void);
void CanStack_RequestNoCommunication(void);
void CanStack_HandleWakeup(CanStack_WakeupSourceType source);
void CanStack_HandleBusOff(void);
CanStack_StateType CanStack_GetState(void);
CanStack_WakeupSourceType CanStack_GetWakeupSource(void);

#endif
