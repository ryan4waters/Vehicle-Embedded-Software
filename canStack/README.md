# TC377 + TJA1145：AUTOSAR Classic CAN 通信栈与唤醒/休眠参考实现

> 目标：以 Infineon AURIX TC377 + NXP TJA1145 为例，说明 AUTOSAR Classic 下 CAN 通信栈、NM、CanSM、ComM、EcuM、BswM、CanIf、PduR、CanTp、Com、RTE/应用之间如何协作，并给出 KL15 本地唤醒、CAN/NM 总线唤醒、正常通信、异常通信和休眠下电的参考 C 代码。
>
> **重要**：下面代码是“架构/接口级参考实现”，不是某一家 MCAL/BSW 厂商的可直接量产包。TC377 的 MCMCAN、QSPI/SPI、ERU/PORT、SMU、MCAL API，以及 TJA1145 的寄存器配置必须根据实际 AUTOSAR Vendor Package、芯片变体和硬件原理图替换。

---

## 1. 先建立一个最重要的认识：CAN 栈不是一个“CAN驱动”

AUTOSAR Classic 把应用和硬件解耦为：

```text
Application SWC
      |
     RTE
      |
Com / Dcm / CanTp ...
      |
     PduR
      |
CanNm / Nm / CanSM / ComM / BswM
      |
    CanIf
      |
   CanDrv(MCAL)
      |
 TC377 MCMCAN
      |
   CAN TX/RX
      |
 TJA1145
      |
 CANH/CANL
```

AUTOSAR Classic 顶层架构本身就是 Application / RTE / BSW 三层；通信相关功能属于 BSW 的通信服务、ECU abstraction、MCAL 等层次。参考 AUTOSAR 官方 Classic Platform 说明。

---

# 2. TC377 + TJA1145 的硬件边界

典型连接：

```text
                         +----------------------+
                         |       TC377          |
                         |                      |
 SWC <-> RTE <-> COM ... |                      |
                         |   MCMCAN             |
                         |    TX/RX             |
                         |      |               |
                         |      | GPIO/ERU       |
                         |      |               |
                         |    QSPI              |
                         +--+---+---------------+
                            |   |
                     CAN_TX|   |CAN_RX
                            |   |
                      +-----v---v------+
                      |    TJA1145     |
                      |                |
             QSPI --->| SPI            |
       WAKE/ERR <-----| WAKE/INT       |
             KL15 ----| WAKE/local     |
                      |                |
                 INH -+----> ECU电源/使能
                      |                |
                      +----+------+----+
                           |      |
                         CANH    CANL
                           |      |
                           +-- CAN BUS --+
```

TJA1145 支持低功耗 Sleep/Standby、Local Wake、Remote CAN Wake、Selective Wake、SPI 控制和唤醒源识别；因此它适合“ECU 仍挂在蓄电池上，但 MCU/CAN 控制器大部分关闭”的车载节点。

---

# 3. “KL15唤醒”和“NM报文唤醒”最容易混淆的地方

## 3.1 KL15 唤醒

KL15 是本地硬件/车辆电源状态事件。

典型过程：

```text
KL15 ON
  |
  v
TJA1145 WAKE/本地唤醒路径
  |
  v
TC377 Wake-up / EcuM Wakeup Source
  |
  v
EcuM_WakeupValidation
  |
  v
EcuM Startup
  |
  +--> MCU/MCAL初始化
  +--> CanDrv初始化
  +--> CanIf
  +--> CanSM
  +--> ComM
  +--> Nm/CanNm
  +--> Com/PduR/CanTp
  |
  v
CAN_FULL_COMMUNICATION
  |
  v
正常通信
```

这里真正让 MCU 从低功耗状态出来的是“本地唤醒事件”，而不是 CAN 上的一帧 Nm。

---

## 3.2 CAN/NM 总线唤醒

更准确地说，“NM 报文唤醒”通常包含两个层次：

### 层次 A：物理层/收发器唤醒

CAN 总线活动首先被 TJA1145 感知。

TJA1145 可以通过 CAN wake-up pattern / selective wake 机制产生远程唤醒，并提供唤醒源状态。

```text
其他ECU发送CAN/NM相关总线活动
             |
             v
        CANH/CANL
             |
             v
         TJA1145
             |
        Wake detection
             |
             v
       WAKE/INT indication
             |
             v
          TC377
             |
             v
           EcuM
```

所以：

> **MCU 还没有起来之前，AUTOSAR 的 CanNm/ComM/CanSM 并没有“接收 Nm 报文”。**

这是理解 CAN Wake-up 最关键的一点。

### 层次 B：MCU起来之后

MCU 启动通信栈以后，CAN 控制器开始正常接收 CAN Frame。

这时候才出现：

```text
CAN Driver
 -> CanIf
 -> CanNm
 -> Nm
 -> ComM
 -> BswM
```

等 AUTOSAR 层面的 NM 状态机行为。

---

# 4. KL15唤醒 vs CAN/NM唤醒：相同点

二者最终都需要把 ECU 从：

```text
NO_COMMUNICATION / SLEEP
```

拉回：

```text
FULL_COMMUNICATION
```

典型共同路径：

```text
Wake source
   |
   v
EcuM
   |
   v
MCU/MCAL
   |
   v
CanDrv
   |
   v
CanIf
   |
   v
CanSM
   |
   v
ComM
   |
   v
Nm / CanNm
   |
   v
Com / PduR / CanTp
   |
   v
Application
```

---

# 5. KL15唤醒 vs CAN/NM唤醒：差异

| 项目 | KL15唤醒 | CAN/NM总线唤醒 |
|---|---|---|
| 唤醒来源 | 本地电源/IO事件 | CAN总线活动 |
| MCU睡眠期间 | KL15路径可直接产生唤醒 | TJA1145先检测 |
| CAN控制器是否已运行 | 否 | 否 |
| Nm模块是否运行 | 否 | 否 |
| EcuM是否参与 | 是 | 是 |
| TJA1145是否参与 | 通常参与 | 必须参与 |
| MCU起来后 | 初始化CAN栈 | 初始化CAN栈 |
| 后续FULL_COM | 相同 | 相同 |
| 差异主要在哪里 | Wakeup Source | Wakeup Source |

---

# 6. 推荐的状态机

```text
                 +-------------------+
                 |   RUN / FULL COM  |
                 +---------+---------+
                           |
                    shutdown request
                           |
                           v
                 +-------------------+
                 | PREPARE_SLEEP     |
                 +---------+---------+
                           |
                    ComM/CanSM/Nm
                    stop communication
                           |
                           v
                 +-------------------+
                 | NO_COMMUNICATION   |
                 +---------+---------+
                           |
                   TJA1145 sleep
                           |
                           v
                 +-------------------+
                 | ECU LOW POWER     |
                 +-------------------+
                    |             |
               KL15 ON        CAN WAKE
                    |             |
                    +------+------+
                           |
                           v
                       EcuM
                           |
                           v
                       STARTUP
                           |
                           v
                     FULL COM
```

---

# 7. 正常通信：发送路径

例如 Application 要发送：

`VehicleSpeed = 100 km/h`

典型路径：

```text
Application SWC
    |
    | Rte_Write()
    v
RTE
    |
    v
COM
    |
    | I-PDU packing
    v
PduR
    |
    v
CanIf
    |
    v
CanDrv
    |
    v
TC377 MCMCAN
    |
    v
TJA1145
    |
    v
CAN Bus
```

如果使用 AUTOSAR COM：

```text
Signal
   |
   v
COM Signal Packing
   |
   v
I-PDU
   |
   v
PduR
   |
   v
CanIf
```

CanIf 再把逻辑 PDU 转成 CAN Driver 的 HTH/Controller 相关操作。

---

# 8. 正常通信：接收路径

例如收到：

`BMS_Status CAN ID 0x180`

```text
CAN Bus
   |
   v
TJA1145
   |
   v
TC377 MCMCAN
   |
   v
CanDrv ISR
   |
   v
CanIf_RxIndication()
   |
   +-----------> CanNm
   |
   +-----------> PduR
                    |
                    v
                   COM
                    |
                    v
                   RTE
                    |
                    v
             Application SWC
```

这里一定要注意：

> CanIf 是“CAN控制器与上层PDU世界之间的适配层”，并不是所有接收帧都一定经过 COM。

例如：

- NM PDU -> CanIf -> CanNm
- Diagnostic PDU -> CanIf -> PduR -> CanTp/Dcm
- Application CAN signal -> CanIf -> PduR -> Com

---

# 9. NM报文的典型路径

```text
CAN Frame
  |
  v
CanDrv
  |
  v
CanIf
  |
  v
CanNm
  |
  v
Nm
  |
  v
ComM
  |
  v
BswM
```

CanNm 的核心任务包括：

- Network Management state machine
- NM PDU Tx/Rx
- Repeat Message State
- Normal Operation State
- Ready Sleep State
- Bus Sleep State
- NM timeout/repeat handling

因此：

```text
NM报文 != 普通COM报文
```

不要把 Nm PDU 当作普通应用信号处理。

---

# 10. CanSM 的作用

CanSM 不负责解析 CAN Frame。

它负责：

```text
CAN Controller / Transceiver Mode
        |
        v
CAN State Management
```

例如：

```text
NO_COMMUNICATION
       |
       v
FULL_COMMUNICATION
       |
       v
SILENT_COMMUNICATION
       |
       v
FULL_COMMUNICATION
```

典型交互：

```text
ComM
 |
 | Request FULL_COM
 v
CanSM
 |
 +--> CanIf_SetControllerMode()
 |
 +--> CanIf_SetTrcvMode()
 |
 v
FULL COMM
```

---

# 11. ComM 的作用

ComM 是“通信需求管理器”。

例如：

```text
Application需要发送CAN
          |
          v
     ComM_RequestComMode()
          |
          v
       FULL_COM
```

ComM 再协调：

```text
ComM
 |
 +--> CanSM
 |
 +--> Nm
 |
 +--> BswM
```

所以不要在 Application 里面直接：

```c
CanIf_SetControllerMode(CAN_TRCV_NORMAL);
```

量产 AUTOSAR 架构中应该由通信状态管理链完成。

---

# 12. BswM 的作用

BswM 是规则仲裁器。

例如：

```text
IF
    KL15 == ON
    AND
    CANSM == FULL_COMM
    AND
    NM == NETWORK_MODE
THEN
    enable application communication
```

或者：

```text
IF
    ComM == NO_COMMUNICATION
THEN
    switch TJA1145 to sleep
```

---

# 13. EcuM 的作用

EcuM 负责 ECU 的生命周期。

大致：

```text
STARTUP
   |
   v
UP
   |
   v
RUN
   |
   v
GO_SLEEP
   |
   v
SLEEP
   |
   v
WAKEUP
   |
   v
STARTUP
```

EcuM 是“系统电源状态”的核心管理者。

---

# 14. 正常工况完整调用链

## 14.1 启动

```text
Reset
 |
 v
Startup Code
 |
 v
MCAL Init
 |
 v
EcuM
 |
 +--> Port
 +--> Mcu
 +--> Spi
 +--> Can
 |
 v
CanIf
 |
 v
CanSM
 |
 v
ComM
 |
 v
Nm
 |
 v
Com
 |
 v
RTE
 |
 v
Application
```

---

# 15. 正常发送

```text
App
 |
 v
Rte_Write
 |
 v
Com
 |
 v
PduR
 |
 v
CanIf_Transmit
 |
 v
Can_Write
 |
 v
TC377 CAN
 |
 v
TJA1145
 |
 v
BUS
```

---

# 16. 正常接收

```text
BUS
 |
 v
TJA1145
 |
 v
TC377 CAN RX
 |
 v
ISR
 |
 v
Can_MainFunction_Read / Rx ISR
 |
 v
CanIf_RxIndication
 |
 +----> CanNm
 |
 +----> PduR
          |
          +--> Com
          |     |
          |     +--> RTE
          |           |
          |           +--> App
          |
          +--> CanTp
                |
                v
               Dcm
```

---

# 17. 异常工况一：CAN Bus-Off

典型过程：

```text
CAN Controller
     |
     | Bus-Off
     v
CanDrv
     |
     v
CanIf_ControllerBusOff()
     |
     v
CanSM
     |
     v
ComM / BswM
```

CanSM 根据配置执行 Bus-Off Recovery。

常见策略：

```text
FULL_COM
   |
Bus-Off
   |
   v
RECOVERY
   |
   +--> controller stop
   +--> controller init
   +--> controller start
   |
   v
FULL_COM
```

---

# 18. 异常工况二：TJA1145异常

TJA1145 可通过 SPI 状态寄存器提供：

- Wake source
- Error status
- Undervoltage
- Overtemperature
- TXD dominant timeout
- CAN transceiver state

参考架构：

```text
TJA1145
   |
  SPI
   |
   v
Tja1145_Drv
   |
   v
CanTrcv / CanSM / Diag
   |
   +--> DTC
   +--> DEM
   +--> BswM
```

这里建议：

```text
TJA1145 driver
```

只做：

- SPI register access
- mode setting
- wake source read
- low-level diagnostic

不要把业务逻辑塞进 TJA1145 driver。

---

# 19. 异常工况三：NM超时

```text
CanNm
 |
 | NM timeout
 v
Nm
 |
 v
ComM
 |
 v
BswM
 |
 +--> communication state change
 |
 +--> DEM
 |
 +--> application notification
```

具体是否产生 DTC、进入降级状态，要按照项目 AUTOSAR 配置和 OEM 需求确定。

---

# 20. 休眠下电流程

这是实际项目最容易出问题的地方。

推荐顺序：

```text
Application
   |
   | no communication request
   v
ComM
   |
   v
CanSM
   |
   v
CanIf
   |
   +--> stop CAN controller
   |
   v
Nm / CanNm
   |
   v
BswM
   |
   v
EcuM_GO_SLEEP
   |
   v
TJA1145 Sleep / Standby
   |
   v
MCU low power
```

注意：

> “关闭 CAN 栈”不是简单调用一个 Can_DeInit()。

实际应该是：

```text
停止业务通信
 -> 停止/退出NM网络状态
 -> ComM NO_COM
 -> CanSM NO_COM
 -> CanIf controller STOP
 -> CAN transceiver SLEEP/STANDBY
 -> EcuM进入Sleep
```

---

# 21. 为什么 TJA1145 要最后睡？

因为它负责唤醒。

如果：

```text
TJA1145 Sleep
```

但 MCU 也直接彻底关闭唤醒路径，就会出现：

```text
CAN Bus
   |
   X
   |
MCU
```

以后 CAN 无法把 ECU 拉起来。

正确方式是：

```text
MCU睡眠
   |
TJA1145保持可唤醒状态
   |
CAN Bus activity
   |
TJA1145 wake detection
   |
WAKE/INT
   |
TC377
   |
EcuM
```

TJA1145 官方资料明确支持 local wake 和 remote CAN wake，以及 wake source recognition。

---

# 22. 推荐的软件分层

本项目建议：

```text
Application/
    CanDemo_Swc.c

Rte/
    Rte_CanDemo.h

Communication/
    Com
    PduR
    CanTp
    CanNm
    Nm
    ComM
    CanSM
    CanIf

BswM/
    BswM_CanRules.c

EcuM/
    EcuM_CanWake.c

MCAL/
    Can
    Spi
    Port
    Mcu

Trcv/
    Tja1145.c
    Tja1145.h

MCU/
    Tc377_CanHw.c
    Tc377_SpiHw.c
    Tc377_WakeupHw.c
```

---

# 23. 代码设计原则

这套参考代码刻意分成：

```text
AUTOSAR-like service layer
        |
        v
project abstraction
        |
        v
TC377 MCAL adapter
        |
        v
register / vendor MCAL
```

因此未来切到：

```text
TC377
TC4xx
SPC58
RH850
S32K
```

不需要重写：

```text
Com
PduR
CanNm
ComM
CanSM
BswM
EcuM
```

只替换底层 adapter。

---

# 24. 编译/移植注意

实际工程中通常不会自己实现完整的：

- Can
- CanIf
- PduR
- Com
- CanTp
- Nm
- CanNm
- ComM
- CanSM
- EcuM
- BswM

这些模块，而是由 AUTOSAR BSW vendor 提供。

本仓库代码重点展示：

1. 调用关系
2. 状态机
3. Wake-up source
4. TJA1145 driver边界
5. TC377硬件适配边界
6. 异常处理
7. Sleep/Wakeup sequencing

---

# 25. 关键工程结论

### 结论1

KL15 唤醒：

```text
KL15 -> TJA1145/MCU wake -> EcuM -> CAN Stack
```

### 结论2

CAN/NM 唤醒：

```text
CAN Bus -> TJA1145 wake detection -> MCU -> EcuM -> CAN Stack
```

### 结论3

MCU 未启动前：

```text
CanNm / ComM / CanSM / Com
```

都不能真正“接收一帧NM报文”。

唤醒首先发生在：

```text
TJA1145 + MCU Wakeup Hardware
```

层。

### 结论4

MCU起来后，两条路径最终汇合：

```text
EcuM
  |
CanSM
  |
ComM
  |
CanNm/Nm
  |
CanIf
  |
Can
```

### 结论5

休眠不是：

```text
Can_DeInit();
```

而是一个完整的：

```text
Application communication release
 -> ComM NO_COM
 -> CanSM NO_COM
 -> CanIf controller STOP
 -> TJA1145 low power
 -> EcuM sleep
```

状态转换过程。

---

# 26. 官方资料

AUTOSAR Classic Platform：
https://www.autosar.org/standards/classic-platform

NXP TJA1145 datasheet：
https://www.nxp.com/docs/en/data-sheet/TJA1145.pdf

NXP TJA1145 product/reference page：
https://www.nxp.com/products/interfaces/can-transceivers/can-with-flexible-data-rate/high-speed-can-transceiver-with-partial-networking-can-fd-data-rates-up-to-5-mbit-s:TJA1145A

Infineon AURIX TC3xx documentation：
https://documentation.infineon.com/aurixtc3xx/

---

# 27. 最终工程视角

把整个系统压缩成一句话：

```text
TJA1145负责“物理总线和低功耗唤醒”
TC377 CAN负责“CAN控制器”
CanIf负责“CAN硬件与PDU世界适配”
PduR负责“PDU路由”
Com负责“应用Signal/I-PDU”
CanTp负责“分段诊断/传输”
CanNm负责“CAN网络管理”
Nm负责“统一网络管理抽象”
CanSM负责“CAN通信状态”
ComM负责“通信需求”
BswM负责“规则仲裁”
EcuM负责“ECU生命周期和唤醒/休眠”
RTE负责“Application与BSW/SWC之间的接口”
DEM负责“诊断事件”
```

这套关系掌握后，再看 DaVinci Configurator / EB tresos / ISOLAR 生成的 RTE/BSW 配置，就不会只是在“追函数调用”，而是能够从系统状态机角度理解为什么某个 API 在那个时刻被调用。
