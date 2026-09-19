#include "FSM_Core.h"

static void FSM_CallAction(FSM_ActionFunc action, void *context)
{
    if (action != NULL) { action(context); }
}

const FSM_State_t *FSM_FindState(const FSM_t *fsm, FSM_StateId_t stateId)
{
    uint16_t i;
    if (fsm == NULL) { return NULL; }
    for (i = 0U; i < fsm->stateCount; ++i) {
        if (fsm->stateTable[i].id == stateId) {
            return &fsm->stateTable[i];
        }
    }
    return NULL;
}

FSM_Status_t FSM_Init(FSM_t *fsm,
                      const FSM_State_t *stateTable, uint16_t stateCount,
                      const FSM_Transition_t *transitionTable, uint16_t transitionCount,
                      void *context)
{
    if (fsm == NULL) { return FSM_STATUS_NULL_PTR; }
    if ((stateTable == NULL) || (stateCount == 0U)) { return FSM_STATUS_NO_STATES; }
    if ((transitionTable == NULL) || (transitionCount == 0U)) { return FSM_STATUS_NO_TRANSITIONS; }

    fsm->stateTable = stateTable;
    fsm->stateCount = stateCount;
    fsm->transitionTable = transitionTable;
    fsm->transitionCount = transitionCount;
    fsm->currentState = NULL;
    fsm->context = context;
    return FSM_STATUS_OK;
}

FSM_Status_t FSM_SetInitialState(FSM_t *fsm, FSM_StateId_t initialState)
{
    const FSM_State_t *state;
    if (fsm == NULL) { return FSM_STATUS_NULL_PTR; }

    state = FSM_FindState(fsm, initialState);
    if (state == NULL) { return FSM_STATUS_INVALID_INITIAL_STATE; }

    fsm->currentState = state;
    FSM_CallAction(state->entryAction, fsm->context);
    return FSM_STATUS_OK;
}

void FSM_RunDoActions(FSM_t *fsm)
{
    uint8_t i;
    const FSM_State_t *state;

    if ((fsm == NULL) || (fsm->currentState == NULL)) { return; }
    state = fsm->currentState;

    for (i = 0U; i < state->doActionCount; ++i) {
        if (state->doActions[i].action != NULL) {
            state->doActions[i].action(fsm->context);
        }
    }
}

static void FSM_DoTransition(FSM_t *fsm, const FSM_Transition_t *transition)
{
    const FSM_State_t *currentState;
    const FSM_State_t *targetState;

    currentState = fsm->currentState;
    targetState = FSM_FindState(fsm, transition->targetState);

    if ((currentState == NULL) || (targetState == NULL)) { return; }

    FSM_CallAction(currentState->exitAction, fsm->context);
    fsm->currentState = targetState;
    FSM_CallAction(targetState->entryAction, fsm->context);
}

void FSM_Run(FSM_t *fsm)
{
    uint16_t i;
    const FSM_Transition_t *transition;

    if ((fsm == NULL) || (fsm->currentState == NULL)) { return; }

    FSM_RunDoActions(fsm);

    /* Array order is transition priority; first true condition wins. */
    for (i = 0U; i < fsm->transitionCount; ++i) {
        transition = &fsm->transitionTable[i];

        if (transition->currentState != fsm->currentState->id) { continue; }
        if (transition->condition == NULL) { continue; }

        if (transition->condition(fsm->context)) {
            FSM_DoTransition(fsm, transition);
            break; /* At most one transition per Run(). */
        }
    }
}

FSM_StateId_t FSM_GetCurrentStateId(const FSM_t *fsm)
{
    if ((fsm == NULL) || (fsm->currentState == NULL)) {
        return FSM_STATE_INVALID;
    }
    return fsm->currentState->id;
}

const FSM_State_t *FSM_GetCurrentState(const FSM_t *fsm)
{
    return (fsm == NULL) ? NULL : fsm->currentState;
}
