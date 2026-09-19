# FSM Base Package V2

面向 TC377 / TI F29P32 / SPC58NN 等 MCU 的通用嵌入式 FSM 基础包。

## 核心架构

- `static const` State Table
- `static const` Transition Table
- 一个 Transition 只保留一个 `condition`
- Transition Table 数组顺序就是优先级
- State 生命周期固定为 `Entry -> Do -> Exit`
- Transition 生命周期固定为 `Exit -> currentState更新 -> Entry`
- 不使用 Event / Guard / TransitionAction / Timeout
- Timer、CAN、GPIO、ADC、BMS、KL15 等均属于应用/服务层
- 不使用动态内存
- FSM Core 不依赖任何 MCU SDK

## Transition

```c
typedef struct {
    FSM_StateId_t currentState;
    FSM_StateId_t targetState;
    FSM_ConditionFunc condition;
} FSM_Transition_t;
```

例如：

```c
static const FSM_Transition_t g_TransitionTable[] =
{
    { STATE_INIT,     STATE_STANDBY,  Cond_InitDone },
    { STATE_STANDBY,  STATE_FAULT,    Cond_Fault },
    { STATE_STANDBY,  STATE_CHARGING, Cond_ChargeRequest },
    { STATE_CHARGING, STATE_FAULT,    Cond_Fault },
};
```

同一 CurrentState 下，第一条返回 `true` 的 Transition 生效。

## Timeout

Timeout 不属于 FSM Core。

```c
static bool Cond_BmsTimeout(void *context)
{
    AppContext_t *ctx = context;
    return Timer_IsExpired(ctx->bmsTimer);
}
```

然后直接配置：

```c
{ STATE_WAIT_BMS, STATE_FAULT, Cond_BmsTimeout }
```

## DoAction

`periodMs` 是调度元数据。FSM Core 不维护定时器。

实际 1ms / 10ms / 100ms 调度由 RTOS、GPT、STM、GTM、DMT、应用 Scheduler 等外部机制负责。

## Transition 生命周期

```text
Current.ExitAction()
        |
        v
currentState = Target
        |
        v
Target.EntryAction()
```

一次 `FSM_Run()` 最多发生一次状态跳转。

## 目录

```text
FSM_Base_Package_V2/
├── include/FSM_Core.h
├── src/FSM_Core.c
├── example/PDU_FSM_Example.h
├── example/PDU_FSM_Example.c
├── example/main.c
├── docs/FSM_Design.md
├── docs/FSM_API.md
├── README.md
└── CMakeLists.txt
```
