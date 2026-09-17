#ifndef FSM_CORE_H
#define FSM_CORE_H

#include <stdint.h>
#include <stdbool.h>
#include "FSM_Config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t FSM_StateId_t;
typedef uint16_t FSM_EventId_t;
typedef uint8_t  FSM_Priority_t;

#define FSM_STATE_INVALID   ((FSM_StateId_t)0xFFFFU)
#define FSM_EVENT_NONE      ((FSM_EventId_t)0xFFFFU)

typedef void (*FSM_ActionFunc)(void *context);
typedef bool (*FSM_GuardFunc)(void *context);
typedef void (*FSM_TransitionActionFunc)(void *context,
                                         FSM_StateId_t source,
                                         FSM_StateId_t target);

typedef enum
{
    FSM_ACTION_PERIODIC = 0U,
    FSM_ACTION_ONCE
} FSM_ActionMode_t;

typedef struct
{
    FSM_ActionFunc action;
    uint32_t period_ms;
    uint32_t elapsed_ms;
    FSM_ActionMode_t mode;
    bool enabled;
} FSM_CyclicAction_t;

struct FSM_State;

typedef struct
{
    FSM_StateId_t target_state;
    FSM_EventId_t event_id;
    FSM_GuardFunc guard;
    FSM_TransitionActionFunc action;
    FSM_Priority_t priority;
    bool enabled;
} FSM_Transition_t;

typedef struct FSM_State
{
    FSM_StateId_t id;
    FSM_ActionFunc entry_action;
    FSM_ActionFunc exit_action;

    FSM_CyclicAction_t do_actions[FSM_MAX_ACTIONS_PER_STATE];
    uint8_t do_action_count;

    FSM_Transition_t *transitions;
    uint16_t transition_count;

    uint32_t timeout_ms;
    uint32_t state_elapsed_ms;

    uint32_t enter_count;
    uint32_t transition_count_runtime;
} FSM_State_t;

typedef struct
{
    FSM_State_t states[FSM_MAX_STATES];
    uint16_t state_count;

    FSM_Transition_t transition_pool[FSM_MAX_TRANSITIONS];
    uint16_t transition_pool_count;

    const FSM_State_t *current_state;
    void *context;

    FSM_EventId_t pending_event;
    bool initialized;
    bool transition_locked;

    uint32_t tick_ms;
} FSM_t;

typedef enum
{
    FSM_OK = 0,
    FSM_ERR_NULL,
    FSM_ERR_FULL,
    FSM_ERR_DUPLICATE,
    FSM_ERR_NOT_FOUND,
    FSM_ERR_INVALID
} FSM_Status_t;

FSM_Status_t FSM_Init(FSM_t *fsm, void *context);

FSM_Status_t FSM_RegisterState(FSM_t *fsm,
                               FSM_StateId_t id,
                               FSM_ActionFunc entry_action,
                               FSM_ActionFunc exit_action,
                               uint32_t timeout_ms);

FSM_Status_t FSM_RegisterDoAction(FSM_t *fsm,
                                  FSM_StateId_t state_id,
                                  FSM_ActionFunc action,
                                  uint32_t period_ms,
                                  FSM_ActionMode_t mode);

FSM_Status_t FSM_RegisterTransition(FSM_t *fsm,
                                    FSM_StateId_t source_state,
                                    FSM_StateId_t target_state,
                                    FSM_EventId_t event_id,
                                    FSM_GuardFunc guard,
                                    FSM_TransitionActionFunc action,
                                    FSM_Priority_t priority);

FSM_Status_t FSM_SetInitialState(FSM_t *fsm, FSM_StateId_t state_id);

FSM_Status_t FSM_PostEvent(FSM_t *fsm, FSM_EventId_t event_id);

void FSM_Tick(FSM_t *fsm, uint32_t elapsed_ms);
void FSM_Run(FSM_t *fsm);

const FSM_State_t *FSM_GetCurrentState(const FSM_t *fsm);
FSM_StateId_t FSM_GetCurrentStateId(const FSM_t *fsm);
uint32_t FSM_GetStateElapsedMs(const FSM_t *fsm);

#ifdef __cplusplus
}
#endif

#endif
