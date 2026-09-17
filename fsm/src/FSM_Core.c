#include "FSM_Core.h"

static FSM_State_t *FSM_FindState(FSM_t *fsm, FSM_StateId_t id)
{
    uint16_t i;
    for (i = 0U; i < fsm->state_count; ++i)
    {
        if (fsm->states[i].id == id)
        {
            return &fsm->states[i];
        }
    }
    return (FSM_State_t *)0;
}

static bool FSM_EventMatch(FSM_EventId_t expected, FSM_EventId_t actual)
{
    return (expected == FSM_EVENT_NONE) || (expected == actual);
}

static bool FSM_GuardPass(FSM_GuardFunc guard, void *context)
{
    return (guard == (FSM_GuardFunc)0) || guard(context);
}

static void FSM_CallAction(FSM_ActionFunc action, void *context)
{
    if (action != (FSM_ActionFunc)0)
    {
        action(context);
    }
}

FSM_Status_t FSM_Init(FSM_t *fsm, void *context)
{
    if (fsm == (FSM_t *)0)
    {
        return FSM_ERR_NULL;
    }

    *fsm = (FSM_t){0};
    fsm->context = context;
    fsm->current_state = (const FSM_State_t *)0;
    fsm->pending_event = FSM_EVENT_NONE;
    return FSM_OK;
}

FSM_Status_t FSM_RegisterState(FSM_t *fsm,
                               FSM_StateId_t id,
                               FSM_ActionFunc entry_action,
                               FSM_ActionFunc exit_action,
                               uint32_t timeout_ms)
{
    FSM_State_t *state;

    if (fsm == (FSM_t *)0)
    {
        return FSM_ERR_NULL;
    }

    if (FSM_FindState(fsm, id) != (FSM_State_t *)0)
    {
        return FSM_ERR_DUPLICATE;
    }

    if (fsm->state_count >= FSM_MAX_STATES)
    {
        return FSM_ERR_FULL;
    }

    state = &fsm->states[fsm->state_count++];
    *state = (FSM_State_t){0};
    state->id = id;
    state->entry_action = entry_action;
    state->exit_action = exit_action;
    state->timeout_ms = timeout_ms;

    return FSM_OK;
}

FSM_Status_t FSM_RegisterDoAction(FSM_t *fsm,
                                  FSM_StateId_t state_id,
                                  FSM_ActionFunc action,
                                  uint32_t period_ms,
                                  FSM_ActionMode_t mode)
{
    FSM_State_t *state;

    if ((fsm == (FSM_t *)0) || (action == (FSM_ActionFunc)0))
    {
        return FSM_ERR_NULL;
    }

    state = FSM_FindState(fsm, state_id);
    if (state == (FSM_State_t *)0)
    {
        return FSM_ERR_NOT_FOUND;
    }

    if (state->do_action_count >= FSM_MAX_ACTIONS_PER_STATE)
    {
        return FSM_ERR_FULL;
    }

    state->do_actions[state->do_action_count++] =
        (FSM_CyclicAction_t)
        {
            .action = action,
            .period_ms = period_ms,
            .elapsed_ms = 0U,
            .mode = mode,
            .enabled = true
        };

    return FSM_OK;
}

FSM_Status_t FSM_RegisterTransition(FSM_t *fsm,
                                    FSM_StateId_t source_state,
                                    FSM_StateId_t target_state,
                                    FSM_EventId_t event_id,
                                    FSM_GuardFunc guard,
                                    FSM_TransitionActionFunc action,
                                    FSM_Priority_t priority)
{
    FSM_State_t *source;
    FSM_State_t *target;
    FSM_Transition_t *transition;

    if (fsm == (FSM_t *)0)
    {
        return FSM_ERR_NULL;
    }

    source = FSM_FindState(fsm, source_state);
    target = FSM_FindState(fsm, target_state);

    if ((source == (FSM_State_t *)0) ||
        (target == (FSM_State_t *)0))
    {
        return FSM_ERR_NOT_FOUND;
    }

    if (fsm->transition_pool_count >= FSM_MAX_TRANSITIONS)
    {
        return FSM_ERR_FULL;
    }

    transition = &fsm->transition_pool[fsm->transition_pool_count++];
    *transition = (FSM_Transition_t)
    {
        .target_state = target_state,
        .event_id = event_id,
        .guard = guard,
        .action = action,
        .priority = priority,
        .enabled = true
    };

    if (source->transitions == (FSM_Transition_t *)0)
    {
        source->transitions = transition;
    }
    source->transition_count++;

    return FSM_OK;
}

FSM_Status_t FSM_SetInitialState(FSM_t *fsm, FSM_StateId_t state_id)
{
    FSM_State_t *state;

    if (fsm == (FSM_t *)0)
    {
        return FSM_ERR_NULL;
    }

    state = FSM_FindState(fsm, state_id);
    if (state == (FSM_State_t *)0)
    {
        return FSM_ERR_NOT_FOUND;
    }

    fsm->current_state = state;
    state->state_elapsed_ms = 0U;
    state->enter_count++;
    fsm->initialized = true;

    FSM_CallAction(state->entry_action, fsm->context);

    return FSM_OK;
}

FSM_Status_t FSM_PostEvent(FSM_t *fsm, FSM_EventId_t event_id)
{
    if (fsm == (FSM_t *)0)
    {
        return FSM_ERR_NULL;
    }

    fsm->pending_event = event_id;
    return FSM_OK;
}

void FSM_Tick(FSM_t *fsm, uint32_t elapsed_ms)
{
    uint8_t i;
    FSM_State_t *state;

    if ((fsm == (FSM_t *)0) ||
        (!fsm->initialized) ||
        (fsm->current_state == (const FSM_State_t *)0))
    {
        return;
    }

    fsm->tick_ms += elapsed_ms;
    state = (FSM_State_t *)fsm->current_state;
    state->state_elapsed_ms += elapsed_ms;

    for (i = 0U; i < state->do_action_count; ++i)
    {
        FSM_CyclicAction_t *item = &state->do_actions[i];

        if (!item->enabled)
        {
            continue;
        }

        if (item->mode == FSM_ACTION_ONCE)
        {
            FSM_CallAction(item->action, fsm->context);
            item->enabled = false;
            continue;
        }

        if (item->period_ms == 0U)
        {
            FSM_CallAction(item->action, fsm->context);
            continue;
        }

        item->elapsed_ms += elapsed_ms;
        if (item->elapsed_ms >= item->period_ms)
        {
            item->elapsed_ms %= item->period_ms;
            FSM_CallAction(item->action, fsm->context);
        }
    }
}

void FSM_Run(FSM_t *fsm)
{
    FSM_State_t *state;
    FSM_Transition_t *best;
    uint16_t i;
    FSM_Priority_t best_priority = 0xFFU;

    if ((fsm == (FSM_t *)0) ||
        (!fsm->initialized) ||
        (fsm->current_state == (const FSM_State_t *)0) ||
        fsm->transition_locked)
    {
        return;
    }

    state = (FSM_State_t *)fsm->current_state;
    best = (FSM_Transition_t *)0;

    /*
     * Transition selection is data-driven:
     * no state-specific switch-case is used.
     */
    for (i = 0U; i < state->transition_count; ++i)
    {
        FSM_Transition_t *t = &state->transitions[i];

        if (!t->enabled)
        {
            continue;
        }

        if (!FSM_EventMatch(t->event_id, fsm->pending_event))
        {
            continue;
        }

        if (!FSM_GuardPass(t->guard, fsm->context))
        {
            continue;
        }

        if ((best == (FSM_Transition_t *)0) ||
            (t->priority < best_priority))
        {
            best = t;
            best_priority = t->priority;
        }
    }

    if (best != (FSM_Transition_t *)0)
    {
        FSM_State_t *target = FSM_FindState(fsm, best->target_state);

        if (target != (FSM_State_t *)0)
        {
            fsm->transition_locked = true;

            FSM_CallAction(state->exit_action, fsm->context);

            if (best->action != (FSM_TransitionActionFunc)0)
            {
                best->action(fsm->context, state->id, target->id);
            }

            fsm->current_state = target;
            target->state_elapsed_ms = 0U;
            target->enter_count++;
            state->transition_count_runtime++;

            FSM_CallAction(target->entry_action, fsm->context);

            fsm->pending_event = FSM_EVENT_NONE;
            fsm->transition_locked = false;
        }
    }
}

const FSM_State_t *FSM_GetCurrentState(const FSM_t *fsm)
{
    return (fsm != (const FSM_t *)0) ? fsm->current_state : (const FSM_State_t *)0;
}

FSM_StateId_t FSM_GetCurrentStateId(const FSM_t *fsm)
{
    const FSM_State_t *state = FSM_GetCurrentState(fsm);
    return (state != (const FSM_State_t *)0) ? state->id : FSM_STATE_INVALID;
}

uint32_t FSM_GetStateElapsedMs(const FSM_t *fsm)
{
    const FSM_State_t *state = FSM_GetCurrentState(fsm);
    return (state != (const FSM_State_t *)0) ? state->state_elapsed_ms : 0U;
}
