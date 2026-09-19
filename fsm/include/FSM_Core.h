#ifndef FSM_CORE_H
#define FSM_CORE_H
#include <stdint.h>
#include <stdbool.h>

typedef uint16_t FSM_StateId_t;
#define FSM_STATE_INVALID ((FSM_StateId_t)0xFFFFU)

typedef void (*FSM_ActionFunc)(void *context);
typedef bool (*FSM_ConditionFunc)(void *context);

typedef struct {
    FSM_ActionFunc action;
    uint32_t periodMs;
} FSM_DoAction_t;

typedef struct {
    FSM_StateId_t id;
    FSM_ActionFunc entryAction;
    FSM_ActionFunc exitAction;
    const FSM_DoAction_t *doActions;
    uint8_t doActionCount;
} FSM_State_t;

typedef struct {
    FSM_StateId_t currentState;
    FSM_StateId_t targetState;
    FSM_ConditionFunc condition;
} FSM_Transition_t;

typedef struct {
    const FSM_State_t *stateTable;
    uint16_t stateCount;
    const FSM_Transition_t *transitionTable;
    uint16_t transitionCount;
    const FSM_State_t *currentState;
    void *context;
} FSM_t;

typedef enum {
    FSM_STATUS_OK = 0,
    FSM_STATUS_NULL_PTR,
    FSM_STATUS_NO_STATES,
    FSM_STATUS_NO_TRANSITIONS,
    FSM_STATUS_INVALID_INITIAL_STATE
} FSM_Status_t;

FSM_Status_t FSM_Init(FSM_t *fsm,
                      const FSM_State_t *stateTable, uint16_t stateCount,
                      const FSM_Transition_t *transitionTable, uint16_t transitionCount,
                      void *context);

FSM_Status_t FSM_SetInitialState(FSM_t *fsm, FSM_StateId_t initialState);
void FSM_Run(FSM_t *fsm);
void FSM_RunDoActions(FSM_t *fsm);
FSM_StateId_t FSM_GetCurrentStateId(const FSM_t *fsm);
const FSM_State_t *FSM_GetCurrentState(const FSM_t *fsm);
const FSM_State_t *FSM_FindState(const FSM_t *fsm, FSM_StateId_t stateId);

#endif
