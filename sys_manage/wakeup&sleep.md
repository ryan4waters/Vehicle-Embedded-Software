# ECU WakeUp & Sleep

Architecture: Infineon TC377 + TLF35584 + TJA1145

## 1 WakeUp

### 1.1 硬件唤醒阶段

- **唤醒事件检测**
  SBC（System Basis Chip）检测到外部唤醒源（如CAN报文、LIN信号、IGN点火信号等）。
- **INH引脚使能电源**
  SBC的INH引脚输出高电平，使能外部稳压器（LDO/DCDC）给MCU供电。MCU电源上电，POR（上电复位）释放。
- **MCU复位释放**
  MCU内核复位释放，CPU从复位向量地址开始执行代码。

### 1.2 MCU启动代码（Boot ROM & SSW）

TC377内部固化有一段启动软件（Startup Software, SSW），由Boot ROM执行。

- **Boot ROM执行**
  - 检查BMHD（Boot Mode Header），确定启动模式（如从内部Flash启动）。
  - 初始化基本时钟（如PLL）、内存接口。
  - 校验用户代码（可选CRC）。
  - 跳转到用户代码的复位向量（通常为`_START`标签）。
- **用户启动文件（Startup Code）**
  汇编代码完成以下工作：
  - 初始化CPU堆栈指针（`SP`）。
  - 初始化CSA（Context Save Area）区域（TriCore特有）。
  - 设置中断向量表基址。
  - 配置内核看门狗（如有需要）。
  - 初始化全局数据：将.data段从Flash复制到RAM，清零.bss段。
  - 调用C++全局构造函数（若使用C++）。
  - 跳转到`main()`函数。

### 1.3 C运行时初始化与早期初始化

进入`main()`后，通常会进行一些与MCU外设相关的初始化，这些在AUTOSAR中通常由`Mcu`模块完成。

- **MCU底层初始化**
  - 配置系统时钟、PLL，确保内核和外设时钟正确。
  - 配置RAM等待状态、缓存等。
  - 初始化看门狗驱动（如`Wdg_Init`）。
  - 使能必要的中断（或保持关闭，等待OS初始化）。
- **其他基础模块初始化**（可选，视具体集成）
  - `Port_Init`：配置MCU引脚复用及方向。
  - `Dio_Init`：初始化数字输入输出。
  - `Mcu_Init`：初始化MCU驱动（时钟、RAM设置等）。
  - `Gpt_Init`或`Mcu_InitClock`等。

这些调用顺序由集成代码（通常位于`main`或`EcuM`的`Init`函数之前）决定。

### 1.4 进入EcuM主函数

当上述基础初始化完成后，集成代码会调用AUTOSAR ECU状态管理器的初始化函数：

```c
int main(void)
{
    /* 硬件相关初始化（由工具生成） */
    Mcu_Init(...);
    Port_Init(...);
    /* ... */

    /* 调用EcuM初始化 */
    EcuM_Init();   /* 进入ECU状态管理 */

    /* 后续通常不会返回，EcuM会启动OS */
    for(;;);
}
```

- **EcuM_Init的作用**
  - 初始化EcuM模块的内部状态变量、调度表。
  - 设置初始ECU状态（如`ECUM_STATE_STARTUP`）。
  - 初始化唤醒原因管理（若需要）。
  - 启动后续启动流程（如调用`EcuM_StartupTwo`，最终启动OS）。

### 1.5 关于唤醒原因的区分

在EcuM初始化前后，系统可能检查唤醒源，以便决定进入何种运行模式（如正常启动、快速唤醒、睡眠唤醒等）。这通常通过读取SBC的中断寄存器或MCU的唤醒标志实现，但**不影响**从复位到`EcuM_Init`的基本启动流程。

## 2 COM STACK

### 2.1 整体硬件与软件架构

```
                CAN_H / CAN_L
                    │
             ┌──────▼──────┐
             │   TJA1145   │  CAN 收发器，支持局部网络/唤醒
             │  CAN PHY    │
             └──────┬──────┘
                    │ RXD / TXD / SPI / INH
        ┌───────────┼──────────────────────┐
        │           │                      │
   ┌────▼────┐ ┌────▼────┐            ┌────▼────┐
   │ TC377   │ │ TLF35584│            │ 板上电源 │
   │ MCU     │ │  SBC    │◄───────────│/降压电路 │
   │ CAN模块 │ │ 电源管理│   INH/WAKE │         │
   │ SPI主机 │ │ 多轨输出│            │         │
   └────┬────┘ └────┬────┘            └─────────┘
        │           │ 使能/复位/看门狗/SPI
        └───────────┴──────────────► 给 MCU 供电
```

- **TJA1145** 由常电 VBAT*V**B**A**T* 供电，可长期处于低功耗监听模式。
- TJA1145 的 **INH 引脚** 用于唤醒外部电源：检测到总线唤醒后拉高 INH，使能 TLF35584 或板上 DCDC。
- **TC377** 的 CAN 模块通过 **TXD/RXD** 与 TJA1145 连接，通过 **SPI** 配置 TJA1145 的模式/唤醒过滤。
- **TLF35584** 是多轨电源系统基础芯片（SBC），负责给 TC377 提供 1.25V、3.3V、5V 等电源轨，管理复位、窗口看门狗、唤醒源，并通过 SPI 与 MCU 通信。
- TLF35584 可以接受来自 TJA1145 INH、KL15 硬线、CAN 唤醒等信号，从而启动上电时序。


### 2.2 AUTOSAR CAN 通信栈分层与模块职责

| 层       | 模块             | 职责                             |
| :------- | :--------------- | :------------------------------- |
| 应用层   | App SWC          | 收发应用信号，做出逻辑判断       |
| RTE      | RTE              | 连接 SWC 与 BSW，信号映射        |
| 通信服务 | Com              | 信号打包/解包，IPDU 管理         |
| 通信服务 | PduR             | PDU 路由：Com ↔ CanTp/CanIf      |
| 通信服务 | CanNm / ComM     | 网络管理、通信模式仲裁           |
| ECU 抽象 | CanIf            | 抽象 CAN 驱动，提供 PDU 收发接口 |
| ECU 抽象 | CanSM            | 管理 CAN 控制器/收发器模式       |
| MCAL     | CanDrv           | 直接操作 TC377 CAN 硬件寄存器    |
| MCAL     | Spi / Dio / Port | 配置 TJA1145、TLF35584           |
| MCAL     | Mcu / Gpt / Wdg  | 时钟、定时、看门狗               |

CAN 通信栈的主要调用链：

```
App SWC
  │  Rte_Write/Read
  ▼
Com
  │  Com_IPduCallout / Com_RxIndication
  ▼
PduR
  │  PduR_CanIfRxIndication / PduR_CanIfTxConfirmation
  ▼
CanIf
  │  CanIf_RxIndication / CanIf_Transmit
  ▼
CanDrv
  │  Can_Read / Can_Write / 中断
  ▼
TC377 MCMCAN 模块
  │  RXD/TXD
  ▼
TJA1145 → CAN Bus
```


### 2.3 阶段一：唤醒上电时序

#### 2.3.1 硬件唤醒时序

以 **CAN 总线唤醒** 为例：


```
CAN Bus      ──┐   ┌──────────────────────
                │   │  唤醒脉冲/唤醒帧
                └───┘
TJA1145     ──────────────────────────────
                │
                │ 检测到唤醒条件
                ▼
TJA1145 INH ────┘ 拉高（VBAT 供电）
                │
TLF35584     ────┤ 使能电源轨，启动上电时序
                │ 释放 MCU 复位
TC377       ─────┘ 开始启动 BootROM → App 入口
```

若唤醒源是 **KL15 硬线**，则 TLF35584 先被 KL15 唤醒，随后 TJA1145 由 MCU 通过 SPI 从 Sleep 唤醒。

#### 2.3.2 软件上电流程

TC377 复位释放后，执行 AUTOSAR 启动流程：

1. **Bootloader / C 启动代码**
   - 初始化堆栈、C 运行时环境。
2. **EcuM_Init**
   - 检查唤醒源（CAN 唤醒、KL15、复位等）。
   - 初始化 MCU 驱动（时钟、RAM、端口）。
3. **基础 BSW 初始化**
   - `Mcu_InitClock`、`Port_Init`、`Dio_Init`、`Spi_Init`、`Gpt_Init`。
   - 配置 TJA1145 的 SPI 通道，设置其进入 **Normal Mode**。
4. **CanDrv 初始化**
   - `Can_Init` 配置 TC377 MCMCAN 节点：波特率、过滤器、FIFO、中断使能。
   - 设置 CAN 控制器为 **STARTED**。
5. **CanIf / CanSM / CanNm / Com / PduR 初始化**
   - 建立 PDU 路由，使能接收通知、发送确认。
6. **CanSM 状态切换**
   - 从 `CANSM_BSM_S_NOCOM` → `CANSM_BSM_S_FULLCOM`。
   - 此时 CAN 控制器参与总线通信。
7. **ComM / BswM 模式通知**
   - 上报通信就绪，允许 App 发送数据。


### 2.4 阶段二：正常运行态

#### 2.4.1 正常收发时序

#### 接收路径

```
CAN Bus 消息 → TJA1145 物理层 → RXD → TC377 MCMCAN
      │
      ▼
MCMCAN 接收 FIFO 触发中断
      │
      ▼
CanDrv ISR（例如 Can_IsrRxHandler）
      │  读取消息，向上层回调
      ▼
CanIf_RxIndication
      │  根据 HOH/HRH 路由
      ▼
PduR_CanIfRxIndication
      │  路由到 Com 或 CanTp
      ▼
Com_RxIndication
      │  解包信号，调用 Rte 回调
      ▼
App SWC Runnable / RTE Event
```

- 中断上下文：CanDrv ISR 只做必要的数据搬运和状态置位，避免长时间阻塞。
- 根据 AUTOSAR 配置，`CanIf_RxIndication` 可以在 ISR 中调用，也可以由 `CanIf_MainFunction` 在任务中处理。建议采用“中断快速处理 + 任务批处理”方式。

#### 发送路径

```
App SWC 请求发送
      │
      ▼
Rte_Write → Com_SendSignal
      │  将信号打包到 IPDU
      ▼
Com_MainFunctionTx 或 Rte 触发
      │
      ▼
PduR_CanIfTransmit
      │
      ▼
CanIf_Transmit
      │  分配硬件发送对象
      ▼
CanDrv_Write
      │  将数据写入 MCMCAN TX Buffer
      ▼
MCMCAN 发送 → TXD → TJA1145 → CAN Bus
```


发送确认通过中断回调：

```
MCMCAN TX 成功 → CanDrv ISR → CanIf_TxConfirmation → PduR → Com/App
```

#### 2.4.2 周期任务与中断划分

| 执行环境 | 模块                  | 典型周期/触发方式 |
| :------- | :-------------------- | :---------------- |
| 中断     | CanDrv RX/TX/BusOff   | 硬件触发          |
| 周期任务 | CanSM_MainFunction    | 5~20 ms           |
| 周期任务 | CanNm_MainFunction    | 5~100 ms          |
| 周期任务 | Com_MainFunctionRx/Tx | 5~20 ms           |
| 周期任务 | PduR_MainFunction     | 5~20 ms           |
| 周期任务 | BswM_MainFunction     | 10~100 ms         |
| 周期任务 | EcuM_MainFunction     | 10~100 ms         |
| 事件触发 | App SWC Runnable      | 收到信号或周期    |

典型 OS 调度：

- CAN 接收中断优先级最高。
- `Com_MainFunctionRx` 负责信号解包和分发。
- `CanNm_MainFunction` 维护网络管理状态机。
- `CanSM_MainFunction` 监控控制器状态并处理模式切换。
- `BswM_MainFunction` 根据通信状态仲裁电源模式、功能降级。

#### 2.4.3 故障处理：BusOff

当 TC377 CAN 控制器进入 BusOff（发送错误计数器超过 255）：

```
MCMCAN 硬件检测到 BusOff
      │ 触发中断
      ▼
CanDrv ISR 设置 Controller BusOff 标志
      │
      ▼
CanSM_MainFunction 周期检测
      │  CANSM_E_BUS_OFF
      ▼
CanSM 切换到 CANSM_BSM_S_BUSOFF
      │
      ├──► 调用 CanIf 禁止发送/接收
      ├──► 通知 ComM：通信不可用
      ├──► 通知 BswM：进入降级模式
      └──► 尝试恢复：
               Can_SetControllerMode(STOPPED)
               → 等待总线恢复
               → Can_SetControllerMode(STARTED)
               → 若恢复成功回到 FULLCOM
```

若反复 BusOff，BswM 可请求 EcuM 复位或关闭通信，TLF35584 的窗口看门狗也会监控 MCU 是否正常喂狗，异常时触发系统复位。

### 2.5 阶段三：下电休眠时序

下电通常由两种场景触发：

- 总线静默超时，CanNm 进入 Bus Sleep，ComM 释放网络。
- 应用层主动请求下电（例如钥匙下电）。

#### 2.5.1 软件下电流程

text

```
App / BswM 请求下电
      │
      ▼
ComM 请求 NOCOM
      │
      ▼
CanNm 进入 Bus Sleep 状态
      │  停止发送 NM 报文，释放网络
      ▼
CanSM 切换到 CANSM_BSM_S_NOCOM
      │
      ├──► CanIf 断开 PDU 路由
      ├──► CanDrv 停止 CAN 控制器（Can_SetControllerMode(STOPPED)）
      └──► 通过 SPI 配置 TJA1145 进入 Sleep 模式
      │
      ▼
EcuM 执行下电序列
      │
      ├──► 检查唤醒源屏蔽
      ├──► 配置 TLF35584 看门狗停止/进入 Sleep
      ├──► 关闭非必要电源轨
      └──► MCU 执行 WFI / 断电
```

#### 2.5.2 硬件下电时序

```
软件完成 CAN 栈关闭
      │
      ▼
TJA1145 进入 Sleep
      │  INH 拉低
      ▼
TLF35584 检测不到使能，关闭 MCU 电源
      │
      ▼
TC377 掉电/待机
```

此时系统进入低功耗状态：

- **TJA1145** 继续由 VBAT*V**B**A**T* 供电，监控 CAN 总线唤醒条件。
- **TLF35584** 处于 Sleep 或 Standby，保持极低功耗。
- **TC377** 完全断电或处于低功耗待机，等待唤醒。

### 2.6 软硬件配合关键点总结

| 阶段     | 硬件动作                                                   | 软件动作                       | 关键模块                                   |
| :------- | :--------------------------------------------------------- | :----------------------------- | :----------------------------------------- |
| 唤醒上电 | TJA1145 检测唤醒 → INH 拉高 → TLF35584 上电 → MCU 复位释放 | 初始化 CAN 栈，进入 FULLCOM    | EcuM, Mcu, Spi, CanDrv, CanIf, CanSM, ComM |
| 正常运行 | TJA1145 转发 CAN 帧，TC377 MCMCAN 收发                     | 中断接收、周期处理、信号收发   | CanDrv, CanIf, PduR, Com, CanNm, CanSM     |
| 故障     | TC377 进入 BusOff                                          | CanSM 检测，BswM 降级/恢复     | CanSM, BswM, ComM, EcuM                    |
| 下电休眠 | TJA1145 进入 Sleep，INH 拉低，TLF35584 断电                | 停止通信，切换 NOCOM，关闭电源 | ComM, CanNm, CanSM, CanIf, CanDrv, EcuM    |


### 2.7 工程实践建议

1. **TJA1145 SPI 配置要早于 CAN 控制器使能**
   否则 TJA1145 可能处于 Standby/Sleep，无法正常收发。
2. **CAN 接收中断尽量只做数据搬运**
   信号解析放在 `Com_MainFunctionRx` 中，避免中断长时间占用。
3. **BusOff 恢复要有限次重试**
   超过阈值应请求系统复位或降级，避免总线反复进入错误状态。
4. **下电前必须等待 CanNm 进入 Bus Sleep**
   否则可能丢失 NM 报文，导致其他节点不进入休眠。
5. **TLF35584 看门狗要与下电流程配合**
   下电前先停喂狗或配置窗口关闭，避免误复位。
