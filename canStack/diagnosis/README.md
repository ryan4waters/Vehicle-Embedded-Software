# CAN DIAGNOSIS

把 **CAN 通信故障诊断**理解成 4 层问题：

```text
① CAN控制器发现异常
        ↓
② CAN Driver / CanIf 获取硬件状态
        ↓
③ CanSM / ComM 判断“通信状态是否异常”
        ↓
④ DEM / DCM / Application 形成最终诊断故障
```

而在 **TC377 + TJA1145 + AUTOSAR Classic** 中，还要额外区分：

```text
TC377 MCMCAN控制器故障
        +
TJA1145收发器故障
        +
CAN总线物理故障
        +
CAN通信超时/丢帧
        +
NM网络管理异常
        +
诊断协议层异常
```

这几个东西虽然最终都可能表现成“CAN通信故障”，但**底层检测原理完全不同**。



# 一、先建立完整的 CAN 故障诊断模型

推荐在工程里把 CAN 故障划成下面 6 类。

| 故障类型                       | 谁发现     | 典型故障                                      |
| ------------------------------ | ---------- | --------------------------------------------- |
| CAN Controller Error           | MCMCAN     | Error Active、Error Passive、Bus-Off          |
| CAN Bit Error                  | MCMCAN硬件 | Bit Error、Stuff Error、CRC Error、Form Error |
| CAN Physical/Transceiver       | TJA1145    | 收发器异常、欠压、过温、TXD Dominant          |
| CAN Communication Timeout      | COM        | 某报文规定时间没收到                          |
| NM Communication Fault         | CanNm/Nm   | NM超时、网络状态异常                          |
| Diagnostic Communication Fault | CanTp/Dcm  | UDS传输异常                                   |

整个关系可以画成：

```text
                 CAN故障
                    │
       ┌────────────┼─────────────┐
       │            │             │
       ▼            ▼             ▼
 Controller      Transceiver    Communication
   Error            Error          Timeout
       │            │             │
       ▼            ▼             ▼
     CanDrv       CanTrcv        COM
       │            │             │
       └───────┬────┴─────────────┘
               ▼
             CanIf
               │
       ┌───────┼─────────┐
       ▼       ▼         ▼
     CanSM    ComM      DEM
       │       │
       ▼       ▼
     BswM     Nm
```



# 二、最底层：CAN 为什么能够“发现错误”？

CAN 最厉害的地方之一就是：**发送节点并不是简单地“把数据发出去就完事”，而是在发送过程中不断监控总线。**

CAN 使用：

```text
CRC
ACK
Bit Monitoring
Bit Stuffing
Frame Format
Error Flag
Error Counter
```

等机制进行错误检测。



# 三、CAN 控制器内部的两个错误计数器

CAN 节点内部最重要的两个东西：

```text
TEC = Transmit Error Counter
REC = Receive Error Counter
```

即：

```text
TEC：发送错误计数器
REC：接收错误计数器
```

它们决定 CAN 节点处于：

```text
Error Active
      ↓
Error Passive
      ↓
Bus-Off
```



# 四、Error Active

正常状态：

```text
TEC < 128
REC < 128
```

此时：Error Active

如果发现错误，可以发送：Active Error Flag

也就是主动错误标志。



# 五、Error Passive

当：

```text
TEC >= 128 或 REC >= 128
```

CAN 节点进入：

```text
Error Passive
```

这个时候说明：这个节点已经明显比正常状态更容易出现通信错误。但是它还没有完全退出 CAN 网络。



# 六、Bus-Off

最严重的是：

```text
TEC >= 256
```

进入：

```text
BUS-OFF
```

这意味着：

```text
Node
 │
 X
 │
CAN BUS
```

节点停止主动参与 CAN 通信。

所以：**Bus-Off 不是“CAN报文没收到”，而是 CAN 控制器认为自身已经严重异常，需要退出总线。**



# 七、CAN控制器为什么会增加错误计数？

例如：

```text
Bit Error
Stuff Error
CRC Error
Form Error
ACK Error
```

都可能造成错误计数。



# 八、Bit Error

例如 ECU 发：

```text
1
```

但是它在总线上读取回来：

```text
0
```

那么：

```text
TX = 1
RX = 0
```

CAN 控制器发现：

```text
TX != BUS
```

于是：

```text
Bit Error
```

伪代码：

```c
if (tx_bit != sampled_bus_bit)
{
    CAN_Error.BitError = TRUE;

    CAN_TEC++;
}
```

当然实际 CAN 控制器内部远比这个复杂，因为 ACK、仲裁、错误标志等阶段有特殊规则。



# 九、ACK Error

这是实际调试中非常常见的一类。

假设：

```text
ECU A
  |
  | TX
  v
CAN BUS
```

但是整个总线上没有其他正常节点。

发送节点发完数据后，在 ACK Slot 检查：ACK = recessive

说明：没人确认收到

于是：ACK Error

可能出现：TEC增加

如果持续发送：

```text
ACK Error
ACK Error
ACK Error
...
```

最终可能：

```text
TEC -> 256
      ↓
   BUS-OFF
```

所以在实验室里看到：“CAN发送不了，最后 Bus-Off”

首先一定要考虑：CAN总线上是不是根本没有其他节点？

以及：

```text
终端电阻
CANH/CANL
收发器
波特率
采样点
```



# 十、CRC Error

CAN Frame 中包含 CRC。

简化：

```text
Sender
  |
  | Data
  | CRC
  v
Receiver
```

Receiver 自己重新计算：

```c
crc_calculated = CRC_Calculate(frame_data);

if (crc_calculated != frame_crc)
{
    CRC_Error = TRUE;
}
```

常见原因：

```text
EMI
信号完整性差
CANH/CANL干扰
波特率问题
采样点问题
```



# 十一、Stuff Error

CAN 有 Bit Stuffing 机制。

例如连续出现多个相同 bit：

```text
111111
```

不能直接发送。

需要插入：

```text
1111101
```

接收端如果发现违反 Stuff Rule：

```text
Stuff Error
```



# 十二、Form Error

CAN Frame 有固定格式字段。

例如：

```text
CRC Delimiter
ACK Delimiter
EOF
```

这些字段必须是特定的 recessive bit。

如果收到不符合规范的值：

```text
Form Error
```



# 十三、这些错误最后怎么进入 AUTOSAR？

底层大概是：

```text
MCMCAN
   │
   │ Error Status
   ▼
CanDrv
   │
   ├── Error Passive
   ├── Bus-Off
   └── Controller Error
   │
   ▼
CanIf
   │
   ▼
CanSM
```

特别是 Bus-Off：

```text
MCMCAN
   │
   │ Bus-Off interrupt
   ▼
CAN ISR
   │
   ▼
Can Driver
   │
   ▼
CanIf_ControllerBusOff()
   │
   ▼
CanSM
```



# 十四、Bus-Off 是怎么处理的？

这里特别容易误解。

不是：

```c
if (bus_off)
{
    Can_Init();
}
```

这么简单。

AUTOSAR 下更接近：

```text
FULL_COM
   │
   │ Bus-Off
   ▼
CANSM_BUS_OFF
   │
   ▼
Recovery
   │
   ├── Controller STOP
   │
   ├── Controller INIT
   │
   ├── Controller START
   │
   ▼
Communication Recovery
```



# 十五、CanSM 的 Bus-Off 伪代码

可以把它理解成：

```c
void CanSM_MainFunction(void)
{
    switch (CanSM_State)
    {
        case CANSM_FULL_COMMUNICATION:

            if (CanSM_BusOffDetected)
            {
                CanSM_State = CANSM_BUS_OFF_RECOVERY;

                CanIf_SetControllerMode(
                    CAN_CONTROLLER,
                    CAN_CS_STOPPED
                );

                RecoveryTimer = 0;
            }

            break;


        case CANSM_BUS_OFF_RECOVERY:

            RecoveryTimer++;

            if (RecoveryTimer >= RECOVERY_TIME)
            {
                CanIf_SetControllerMode(
                    CAN_CONTROLLER,
                    CAN_CS_STARTED
                );

                CanSM_BusOffDetected = FALSE;

                CanSM_State =
                    CANSM_FULL_COMMUNICATION;
            }

            break;
    }
}
```

实际 AUTOSAR BSW 中会复杂很多，但理解逻辑就是这个。



# 十六、但是 Bus-Off 恢复以后，故障算不算消失？

这里要区分两个概念：

### 通信状态恢复

```text
Bus-Off
 ↓
Recovery
 ↓
CAN正常
```

和：

### 诊断故障恢复

```text
DEM Event
 ↓
Failed
 ↓
Passed
 ↓
Confirmed / Healing
```

这两个不是一回事。



# 十七、DEM 才是“故障记忆”的核心

例如：CAN Bus-Off

可以产生一个 DEM Event：CAN_BUSOFF_EVENT

检测到：

```c
Dem_SetEventStatus(
    DEM_EVENT_CAN_BUSOFF,
    DEM_EVENT_STATUS_FAILED
);
```

恢复：

```c
Dem_SetEventStatus(
    DEM_EVENT_CAN_BUSOFF,
    DEM_EVENT_STATUS_PASSED
);
```

DEM 再根据：

```text
Debounce
Healing
Confirmation
Aging
Operation Cycle
```

决定最终 DTC 状态。



# 十八、因此 CAN 故障处理最好分成两条链

非常重要：

```text
               CAN故障
                  │
          ┌───────┴────────┐
          │                │
          ▼                ▼
   Communication Path   Diagnostic Path
          │                │
          ▼                ▼
        CanSM             DEM
          │                │
          ▼                ▼
     恢复通信             记录DTC
```

例如 Bus-Off：

```text
Bus-Off
  │
  ├──────────────> CanSM
  │                  │
  │                  ▼
  │               Recovery
  │
  └──────────────> DEM
                     │
                     ▼
                  DTC记录
```



# 十九、第二类非常重要的故障：CAN通信超时

这和 Bus-Off 完全不同。

假设：

```text
BMS -> OBC
CAN ID = 0x180
周期 = 10ms
```

要求：

```text
10ms ± tolerance
```

结果：

```text
0ms     收到
10ms    收到
20ms    收到
30ms    没收到
40ms    没收到
50ms    没收到
```

这时候：CAN Controller 可能完全正常。

甚至：CAN Bus 也完全正常。

只是：**期望的某个报文没有按要求到达。**

这属于：Communication Timeout



# 二十、Timeout 通常在哪里检测？

AUTOSAR COM。

例如配置：

```text
RxTimeout = 100ms
```

COM 每次收到 PDU：

```c
RxTimer = 0;
```

没有收到：

```c
RxTimer += MainPeriod;
```

达到：

```c
if (RxTimer >= 100ms)
```

产生：

```text
Timeout
```



# 二十一、COM Timeout 伪代码

```c
void Com_RxIndication(uint16_t pduId)
{
    ComRx[pduId].Timer = 0;
    ComRx[pduId].Received = TRUE;
}

void Com_MainFunction(void)
{
    for (uint16_t i = 0;
         i < COM_RX_PDU_NUM;
         i++)
    {
        if (ComRx[i].Received == TRUE)
        {
            ComRx[i].Received = FALSE;
            continue;
        }

        ComRx[i].Timer += COM_MAIN_PERIOD_MS;

        if (ComRx[i].Timer >=
            ComRx[i].Timeout)
        {
            ComRx[i].Timeout = TRUE;

            Dem_SetEventStatus(
                ComRx[i].DemEvent,
                DEM_EVENT_STATUS_FAILED
            );

            ComRx[i].Timer = 0;
        }
    }
}
```

实际 COM 中通常还会涉及：

```text
Deadline Monitoring
Rx Deadline Monitoring
Invalidation
Substitute Value
Timeout Notification
```



# 二十二、Timeout 和 Bus-Off 怎么区分？

这个非常重要。

### 情况 A

```text
整个CAN网络都通信不了
```

可能：

```text
Bus-Off
Transceiver failure
CANH/CANL短路
CAN控制器异常
```

### 情况 B

```text
只有0x180没有
其他CAN报文都正常
```

更应该考虑：

```text
Sender ECU异常
CAN ID配置错误
PDU routing问题
COM配置错误
报文周期异常
```

因此：Bus-Off 是**节点级通信状态故障**。

而：Rx Timeout 更像是**某一个通信对象级故障**。



# 二十三、第三类：TJA1145 收发器故障

这一层经常被软件工程师漏掉。

结构是：

```text
TC377
  │
  │ CAN TX/RX
  ▼
TJA1145
  │
  ▼
CAN BUS
```

所以 CAN 控制器正常：

```text
MCMCAN = OK
```

不代表：

```text
TJA1145 = OK
```

例如：

```text
TJA1145 undervoltage
TJA1145 overtemperature
TXD dominant timeout
CAN transceiver state abnormal
```

这些需要通过：SPI 读取 TJA1145 状态。



# 二十四、TJA1145 故障诊断伪代码

建议做成独立的 Transceiver Diagnostic：

```c
typedef struct
{
    bool UnderVoltage;
    bool OverTemperature;
    bool TxDominantTimeout;
    bool WakeError;
    bool TransceiverError;

} Tja1145_DiagType;
```

周期检测：

```c
void Tja1145_Diagnostic_10ms(void)
{
    uint8_t status;

    status = Tja1145_ReadStatus();

    if (status & TJA_UV_BIT)
    {
        Dem_SetEventStatus(
            DEM_EVENT_TJA_UV,
            DEM_EVENT_STATUS_FAILED
        );
    }
    else
    {
        Dem_SetEventStatus(
            DEM_EVENT_TJA_UV,
            DEM_EVENT_STATUS_PASSED
        );
    }


    if (status & TJA_OT_BIT)
    {
        Dem_SetEventStatus(
            DEM_EVENT_TJA_OT,
            DEM_EVENT_STATUS_FAILED
        );
    }


    if (status & TJA_TXD_DOMINANT_TIMEOUT)
    {
        Dem_SetEventStatus(
            DEM_EVENT_TJA_TXD_DOM,
            DEM_EVENT_STATUS_FAILED
        );
    }
}
```

实际寄存器 bit 要按照使用的 TJA1145/TJA1145A 具体 datasheet 版本配置。



# 二十五、第四类：CAN NM 故障

这个和普通 CAN Timeout 又不一样。

例如：

```text
CAN NM
ID = 0x500
周期 = 100ms
```

如果 NM 长时间没有：

```text
Rx NM
```

可能影响：

```text
Nm State
   ↓
ComM
   ↓
Communication Mode
```

典型状态：

```text
Repeat Message
       ↓
Normal Operation
       ↓
Ready Sleep
       ↓
Bus Sleep
```

所以：**NM异常**可能不是一个普通的CAN ID Timeout，而是**网络管理状态发生变化。**



# 二十六、NM 和 COM Timeout 的本质区别

```text
COM Timeout
    ↓
“我需要的业务报文没有收到”
```

而：

```text
CanNm异常
    ↓
“这个网络的管理状态发生异常”
```

例如：

```text
BMS Status
```

超时：

```text
COM层
```

而：

```text
NM PDU
```

超时/状态变化：

```text
CanNm/Nm/ComM
```



# 二十七、第五类：CAN Physical Fault

例如：

```text
CANH short to GND
CANL short to GND
CANH short to VBAT
CANH-CANL short
Open circuit
终端电阻异常
```

这类问题 CAN 控制器不一定能告诉你：

```text
“CANH短地了”
```

它通常只告诉你：

```text
大量 Bit Error
ACK Error
Error Passive
Bus-Off
```

因此：CAN Controller Error往往是**物理层问题的结果，而不是物理层故障本身。**



# 二十八、例如 CANH 对地短路

可能出现：

```text
CANH/CANL
   ↓
Signal distortion
   ↓
Bit Error
   ↓
TEC/REC增加
   ↓
Error Passive
   ↓
Bus-Off
```

最终软件看到的：

```text
Bus-Off
```

但真正根因：

```text
CANH Short to GND
```

所以诊断工程一定要区分：Fault symptom 和 Fault root cause



# 二十九、推荐的 CAN 故障诊断架构

如果是 TC377 + TJA1145，建议直接设计成：

```text
                    CAN Diagnosis
                         │
       ┌─────────────────┼──────────────────┐
       │                 │                  │
       ▼                 ▼                  ▼
 Controller          Transceiver        Communication
 Diagnosis            Diagnosis          Diagnosis
       │                 │                  │
       ▼                 ▼                  ▼
 BusOff/Error        TJA1145 Status       Rx Timeout
 Error Counter       UV/OT/TXD           NM Timeout
 Controller State    Wake Status          PDU Error
       │                 │                  │
       └────────────┬────┴──────────────────┘
                    ▼
                   DEM
                    │
                    ▼
                   DTC
                    │
                    ▼
                  DCM/UDS
```



# 三十、定义一个统一的 CAN Diagnosis 状态结构

例如：

```c
typedef struct
{
    /* Controller */
    bool ControllerError;
    bool ErrorPassive;
    bool BusOff;

    uint8_t Tec;
    uint8_t Rec;

    /* Transceiver */
    bool TrcvError;
    bool TrcvUnderVoltage;
    bool TrcvOverTemperature;
    bool TxDominantTimeout;

    /* Communication */
    bool RxTimeout;
    bool TxConfirmationTimeout;

    /* NM */
    bool NmTimeout;
    bool NmStateError;

} CanDiag_StatusType;
```



# 三十一、周期诊断框架

```c
void CanDiag_MainFunction_10ms(void)
{
    CanDiag_ReadControllerStatus();

    CanDiag_CheckBusOff();

    CanDiag_CheckErrorPassive();

    CanDiag_CheckTransceiver();

    CanDiag_CheckRxTimeout();

    CanDiag_CheckTxConfirmation();

    CanDiag_CheckNm();

    CanDiag_UpdateDem();
}
```

但是这里还有一个很重要的设计原则：**不要让 CanDiag 自己重复实现 COM、CanSM、CanNm、DEM 已经具备的机制。**

它更适合作为诊断聚合/项目诊断策略层，而不是再造一个 AUTOSAR CAN Stack。



# 三十二、建议最终形成的职责边界

| 模块           | 应该负责什么                             |
| -------------- | ---------------------------------------- |
| MCMCAN         | Bit/CRC/ACK/Stuff/Form、TEC/REC、Bus-Off |
| CanDrv         | 获取控制器状态、ISR                      |
| CanIf          | Controller/PDU适配                       |
| TJA1145 Driver | Transceiver状态、SPI、Wake               |
| CanSM          | CAN通信状态、Bus-Off恢复                 |
| ComM           | 通信需求                                 |
| CanNm          | NM状态机                                 |
| COM            | PDU/Signal Timeout                       |
| PduR           | PDU路由                                  |
| DEM            | 故障事件记忆                             |
| DCM            | UDS诊断服务                              |
| BswM           | 根据状态执行规则                         |
| Application    | 根据诊断状态进行功能降级                 |

这个边界非常重要。



# 三十三、一个完整 Bus-Off + DEM 的伪代码

把整个链路串起来：

```c
/* =====================================================
 * TC377 MCMCAN ISR
 * ===================================================== */

void CAN0_BusOff_ISR(void)
{
    CanDrv_BusOffDetected();
}


/* =====================================================
 * CAN Driver
 * ===================================================== */

void CanDrv_BusOffDetected(void)
{
    CanIf_ControllerBusOff(CAN_CONTROLLER_0);
}


/* =====================================================
 * CanIf
 * ===================================================== */

void CanIf_ControllerBusOff(uint8_t Controller)
{
    CanSM_ControllerBusOff(Controller);
}


/* =====================================================
 * CanSM
 * ===================================================== */

void CanSM_ControllerBusOff(uint8_t Controller)
{
    CanSM[Controller].BusOff = TRUE;

    Dem_SetEventStatus(
        DEM_EVENT_CAN_BUSOFF,
        DEM_EVENT_STATUS_FAILED
    );

    CanSM[Controller].State =
        CANSM_BUS_OFF_RECOVERY;
}


/* =====================================================
 * CanSM MainFunction
 * ===================================================== */

void CanSM_MainFunction(void)
{
    if (CanSM.State == CANSM_BUS_OFF_RECOVERY)
    {
        if (CanSM_RecoveryTimerExpired())
        {
            CanIf_SetControllerMode(
                CAN_CONTROLLER_0,
                CAN_CS_STARTED
            );

            CanSM.State =
                CANSM_FULL_COMMUNICATION;
        }
    }
}
```

然后：

```text
Bus-Off
 ↓
CanSM Recovery
 ↓
CAN重新工作
```

但 DEM：

```text
DEM_EVENT_CAN_BUSOFF
```

是否马上变成 `PASSED`，取决于诊断策略和 DEM 配置，而不是“CAN重新启动”这一动作自动决定。



# 三十四、再看一个 Rx Timeout

```c
void Com_RxIndication(uint16_t PduId)
{
    ComRx[PduId].Timer = 0;
}


void Com_MainFunction_10ms(void)
{
    for (uint16_t i = 0;
         i < COM_RX_NUM;
         i++)
    {
        ComRx[i].Timer += 10;

        if (ComRx[i].Timer >=
            ComRx[i].Timeout)
        {
            ComRx[i].TimeoutDetected = TRUE;

            Dem_SetEventStatus(
                ComRx[i].DemEvent,
                DEM_EVENT_STATUS_FAILED
            );

            App_CanSignalTimeout(
                ComRx[i].SignalGroup
            );
        }
    }
}
```

应用层最终可能：

```c
if (BmsStatusTimeout)
{
    OBC_EnablePower = FALSE;
    OBC_SetDegradeMode();
}
```

这就是：

```text
CAN通信故障
      ↓
诊断
      ↓
功能安全/功能策略
      ↓
功能降级
```



# 三十五、整个 CAN 故障链

```text
                         CAN BUS
                            │
                    ┌───────┴───────┐
                    │               │
                 TJA1145          物理线路
                    │
              ┌─────┴─────┐
              │           │
          Transceiver   Wake
           Diagnosis
              │
              ▼
           TC377
              │
          MCMCAN HW
              │
      ┌───────┼────────┐
      │       │        │
   CRC/ACK   TEC/REC  BusOff
      │       │        │
      └───────┼────────┘
              ▼
           CanDrv
              │
              ▼
           CanIf
              │
      ┌───────┼───────────────┐
      │       │               │
      ▼       ▼               ▼
    CanSM    CanNm           PduR
      │       │               │
      │       ▼          ┌────┴────┐
      │      Nm          │         │
      │       │          ▼         ▼
      │      ComM       COM       CanTp
      │       │          │         │
      │       ▼          │         ▼
      │      BswM         │       Dcm
      │                  │
      └─────────┬────────┘
                ▼
               DEM
                │
                ▼
               DTC
                │
                ▼
               DCM
                │
                ▼
              UDS 0x19
```

实践 TC377 + TJA1145 ，最应该记住的就是：

**1. Bus-Off ≠ CAN报文Timeout**

```text
Bus-Off       → CAN控制器/总线级故障
Rx Timeout    → 某个PDU/Signal级故障
```

**2. TJA1145故障 ≠ MCMCAN故障**

```text
TJA1145 → SPI状态/收发器诊断
MCMCAN  → TEC/REC/Error/Bus-Off
```

**3. CAN错误计数器是底层故障演化的核心**

```text
Error
 ↓
TEC/REC
 ↓
Error Active
 ↓
Error Passive
 ↓
Bus-Off
```

**4. AUTOSAR里 Bus-Off 的核心路径是**

```text
MCMCAN
 → CanDrv
 → CanIf
 → CanSM
 → Recovery
```

**5. CAN报文Timeout的核心路径是**

```text
CAN
 → CanIf
 → PduR
 → COM
 → Deadline Monitoring
 → DEM
```

**6. NM故障的核心路径是**

```text
CAN
 → CanIf
 → CanNm
 → Nm
 → ComM
 → BswM
```

**7. 最终 DTC 的产生通常不是 CanSM 自己“存故障”，而是通过 DEM 管理故障事件。**