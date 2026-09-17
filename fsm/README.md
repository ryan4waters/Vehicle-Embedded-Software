# FSM Base Package

面向嵌入式/车载软件的注册式、函数指针驱动有限状态机基础组件。

## 核心特性
- 状态、迁移、Guard、Entry/Exit/DoAction 均可注册
- 当前状态通过指针保存，不使用 switch-case 轮询状态
- Transition 支持优先级
- 支持 Event + Guard
- 支持周期 DoAction（1ms/10ms/100ms 等）
- 支持状态超时
- 支持状态进入次数、运行时间、迁移计数等诊断信息
- 支持多个 FSM 实例
- Core 不依赖 MCU；可直接用于 TC377/F29P32/SPC58NN 上层应用
- 时间基准通过 FSM_Tick() 注入，不绑定具体定时器

## 推荐运行方式
1. 初始化应用 context
2. FSM_Init()
3. RegisterState()
4. RegisterTransition()
5. SetInitialState()
6. 周期任务中调用 FSM_Run()
7. 中断/底层事件中可调用 FSM_PostEvent()

## 目录
- include/FSM_Core.h       公共接口
- include/FSM_Config.h     编译期配置
- src/FSM_Core.c           核心实现
- example/PDU_FSM_Example.h
- example/PDU_FSM_Example.c 示例：PDU 风格状态机
- example/main.c            伪任务入口

## 注意
注册接口默认适合初始化阶段使用；建议初始化完成后不再修改状态/迁移表。
代码避免在“根据具体状态选择行为”的主体逻辑中使用 switch-case/if-else，行为由函数指针和注册表驱动。
