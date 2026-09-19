# FSM Design

## 1. State

```c
typedef struct {
    FSM_StateId_t id;
    FSM_ActionFunc entryAction;
    FSM_ActionFunc exitAction;
    const FSM_DoAction_t *doActions;
    uint8_t doActionCount;
} FSM_State_t;
```

Entry：进入状态执行一次。

Do：状态保持期间执行。

Exit：离开状态执行一次。

## 2. Transition

```c
typedef struct {
    FSM_StateId_t currentState;
    FSM_StateId_t targetState;
    FSM_ConditionFunc condition;
} FSM_Transition_t;
```

Transition 不包含 Event、Guard、Action、Priority、Timeout。

## 3. Polling

```text
FSM_Run()
  |
  +-- current DoAction
  |
  +-- transition[0]
  |      |
  |      +-- condition true -> transition
  |
  +-- transition[1]
  |
  +-- ...
```

首个满足条件的 Transition 生效。

## 4. 应用层职责

应用层负责：

- State ID
- State Table
- Transition Table
- Condition
- EntryAction
- ExitAction
- DoAction
- Timer
- CAN/GPIO/ADC/BMS 等输入

FSM Core 只负责通用生命周期执行。

## 5. 推荐分层

```text
Application
    |
Service / HAL
    |
FSM Core
    |
MCU Driver
```

FSM Core 不包含 TC377、F29P32、SPC58NN 专用头文件。
