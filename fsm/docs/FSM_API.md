# FSM API

```c
FSM_Status_t FSM_Init(...);
FSM_Status_t FSM_SetInitialState(...);
void FSM_Run(FSM_t *fsm);
void FSM_RunDoActions(FSM_t *fsm);
FSM_StateId_t FSM_GetCurrentStateId(const FSM_t *fsm);
const FSM_State_t *FSM_GetCurrentState(const FSM_t *fsm);
const FSM_State_t *FSM_FindState(const FSM_t *fsm, FSM_StateId_t stateId);
```

核心没有 Register API。

配置通过 `static const` 表完成。
