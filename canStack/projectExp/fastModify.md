# FAST MODIFY

## 1. 到底应该在哪一层改 CAN ID、DLC、Signal等等？

一张**工程速查表**。

| 想修改什么           | 推荐修改位置           | 原因                |
| -------------------- | ---------------------- | ------------------- |
| CAN ID               | CanIf / CanDrv配置     | L-PDU属性           |
| Extended/Standard ID | CanIf                  | PDU属性             |
| DLC                  | CanIf PDU配置          | L-PDU属性           |
| Signal StartBit      | COM                    | Signal映射          |
| Signal Length        | COM                    | Signal定义          |
| Signal Endian        | COM                    | Signal定义          |
| Signal Factor/Offset | COM                    | Signal转换          |
| Signal Timeout       | COM                    | Deadline Monitoring |
| Signal Invalid       | COM                    | Signal属性          |
| 整帧周期             | COM                    | I-PDU发送属性       |
| NM报文               | CanNm配置              | NM专用PDU           |
| UDS诊断报文          | CanTp/CanIf/DCM        | 取决于诊断链路      |
| 停止某一帧           | CanIf/COM              | 看想在哪层控制      |
| 停止所有CAN Tx       | CanIf/CanSM            | 不建议直接改COM     |
| 整个ECU只收不发      | CanIf PDU mode / CanSM | 通信模式控制        |
| 只禁止业务报文       | COM                    | 保留NM/诊断         |
| 只禁止NM             | CanNm                  | 不影响普通COM       |
| 只禁止诊断发送       | DCM/CanTp              | 不影响业务通信      |



## 2. 修改 CAN ID：原则上确实找 CanIf

例如：

```text
BMS_Status

原ID：

0x180

改成：

0x280
```

如果这是一个普通 CAN L-PDU：

```text
COM
 │
 │ I-PDU
 ▼
PduR
 │
 ▼
CanIf
 │
 │ CAN ID = 0x180
 ▼
CanDrv
```

那么：CAN ID 属于 **CanIf PDU配置**，而不是 COM Signal。

AUTOSAR CanIf规范也明确把发送/接收 L-PDU 的 **ID、DLC** 作为 CanIf 的配置属性。([AUTOSAR](https://autosar.org/fileadmin/standards/R4.0.3/CP/AUTOSAR_SWS_CANInterface.pdf?utm_source=chatgpt.com))



## 3. 修改 Signal：去 COM

比如：

```text
VehicleSpeed

StartBit = 8
Length   = 16
Factor   = 0.01
Offset   = 0
```

改：

```text
StartBit = 16
```

或者：

```text
Factor = 0.1
```

这属于：

```text
COM Signal / I-PDU mapping
```

而不是 CanIf。

所以可以记成：**CAN ID/DLC管“盒子”，Signal管“盒子里面的数据”。**



## 4. DLC稍微特殊一点

修改某一帧的 DLC 是否去 COM？

严格按照 AUTOSAR 分层：

**普通 CAN L-PDU 的 DLC 更应该从 CanIf/PDU 配置看。**

例如：

```text
CAN ID
0x123

DLC
8
```

它属于 CAN L-PDU 属性。

而 COM负责：

```text
Signal
SignalGroup
I-PDU
Packing
Unpacking
```

所以：

```text
改ID        → CanIf
改DLC       → CanIf
改Signal    → COM
```

这个记法非常实用。



## 5. 工程上非常有用：如何“打桩停发某一帧”？

这个才是真正的工程技巧。

假设：

```text
VehicleStatus
CAN ID = 0x180
周期 = 10ms
```

想临时让 0x180 不发。

不建议直接把 COM 里的周期改成 0。

更推荐增加：

```c
CanDiag_TxBlockSet(0x180, true);
```

然后：

```text
COM
 ↓
PduR
 ↓
CanIf
 ↓
Tx Gate
 ↓
CanDrv
```

增加：

```c
if (CanDiag_IsTxBlocked(PduId))
{
    return E_NOT_OK;
}
```



## 6. 更专业的做法：做一个 Tx Gate

特别建议在项目里留一个：

```text
Can_TestHook
```

例如：

```c
typedef struct
{
    bool globalTxEnable;

    bool pduTxEnable[CAN_PDU_MAX];

} CanTestControlType;
```

然后：

```c
bool CanTest_IsTxAllowed(PduIdType pduId)
{
    if (!CanTestCtrl.globalTxEnable)
    {
        return false;
    }

    return CanTestCtrl.pduTxEnable[pduId];
}
```

CanIf：

```c
Std_ReturnType CanIf_Transmit(
    PduIdType PduId,
    const PduInfoType* PduInfoPtr)
{
    if (!CanTest_IsTxAllowed(PduId))
    {
        return E_NOT_OK;
    }

    return CanDrv_Write(PduId, PduInfoPtr);
}
```

这样调试的时候：

```c
CanTest_SetPduTxEnable(
    CAN_PDU_VEHICLE_STATUS,
    false);
```

就能：只停止 VehicleStatus

而：

```text
NM
UDS
其他CAN报文
```

全部继续正常工作。



## 7. 如果要整个 MCU “只收不发”怎么办？

这个需求非常典型。

例如：要验证 ECU 在总线上只接收，不允许它发送任何业务CAN。

可以做：

```text
CAN RX = ON
CAN TX = OFF
```

概念上：

```text
             CAN
              │
       ┌──────┴──────┐
       ↓             ↓
      RX             TX
      ON             OFF
```

在 AUTOSAR CanIf 层可以使用 PDU mode / Tx mode 控制实现类似效果；CanIf 本身就负责控制 L-PDU 的发送/接收路径。([AUTOSAR](https://autosar.org/fileadmin/standards/R4.0.3/CP/AUTOSAR_SWS_CANInterface.pdf?utm_source=chatgpt.com))

例如项目测试接口：

```c
CanTest_SetGlobalTxEnable(false);
```

结果：

```text
COM        → 可以继续运行
CAN RX     → 正常
CAN TX     → 全部禁止
```

但是有一个非常大的坑： 不建议生产版本永久这么干

因为：

```text
NM
诊断
网络管理
唤醒
状态同步
```

可能都依赖 TX。

所以测试代码最好：

```c
#if CAN_TEST_MODE

CanTest_SetGlobalTxEnable(...);

#endif
```

或者使用：

```text
Development Error / Calibration / Debug Mode
```

控制。



## 8. 但如果只想停“业务CAN”，不能停NM怎么办？

这才是实际项目里更常见的。

例如：

```text
业务：

0x180
0x280
0x380
0x480

NM：

0x500
```

想：

```text
0x180/280/380/480 → 停
0x500             → 正常
```

那么：

```text
不要全局关闭 CanIf TX
```

而是：

```text
COM Tx PDU
    ↓
PDU TX Gate
    ↓
CanIf
```

只 block：

```text
Business PDU
```

保留：

```text
CanNm
CanTp
DCM
```



## 9. 再进一步：要模拟“通信异常”

这个在测试 CAN Fault Diagnostic 时特别好用。

不要每次都拔线。

直接做软件 Fault Injection。

例如：

```c
typedef enum
{
    CAN_FAULT_NONE = 0,

    CAN_FAULT_STOP_PDU,

    CAN_FAULT_STOP_ALL_TX,

    CAN_FAULT_RX_TIMEOUT,

    CAN_FAULT_TX_CONFIRM_TIMEOUT,

    CAN_FAULT_NM_TIMEOUT,

    CAN_FAULT_FORCE_BUS_OFF

} CanFaultInjectType;
```

测试：

```c
CanFaultInject_Set(
    CAN_FAULT_RX_TIMEOUT);
```

模拟：

```text
正常：

CAN Rx
 ↓
CanIf
 ↓
COM
 ↓
Signal更新
```

变成：

```text
CAN Rx
 X
 ↓
COM Deadline Monitoring
 ↓
Timeout
 ↓
DEM
 ↓
DTC
```

这样可以直接验证：

```text
COM Timeout
→ DEM
→ DTC
→ Application Degradation
```

整个链路。



## 10. 非常实用的“CAN调试武器库”

建议项目直接增加一个：CanDebug / CanTest 模块。

里面至少放这些东西。



### ① 全局Tx开关

```c
CanTest_SetGlobalTxEnable(false);
```

实现：整个ECU停止发送



### ② 单PDU Tx开关

```c
CanTest_SetPduTxEnable(
    CAN_PDU_BMS_STATUS,
    false);
```

实现：只停某一帧



### ③ 修改CAN ID

调试版本：

```c
CanTest_SetPduCanId(
    CAN_PDU_BMS_STATUS,
    0x321);
```

用于：临时换ID验证

但注意：**这属于开发/测试能力，不建议直接修改量产CanIf静态配置。**



### ④ 修改周期

```c
CanTest_SetPduPeriod(
    CAN_PDU_BMS_STATUS,
    20);
```

原来：

```text
10ms
```

临时：

```text
20ms
```



### ⑤ 强制Signal

例如：

```c
CanTest_ForceSignal(
    SIG_VEHICLE_SPEED,
    100.0);
```

即使真实车速：

```text
0 km/h
```

CAN发送：

```text
100 km/h
```

非常适合验证：

```text
上游 → COM → CAN → 对端
```



### ⑥ Signal冻结

```c
CanTest_FreezeSignal(
    SIG_VEHICLE_SPEED);
```

例如：

```text
0
10
20
30
40
```

冻结后：

```text
40
40
40
40
40
```

特别适合测试：Signal stale



### ⑦ 模拟Rx Timeout

这是特别推荐做的。

例如：

```c
CanTest_BlockRxPdu(
    CAN_PDU_WHEEL_SPEED);
```

实际上：

```text
CAN总线仍然有：

WheelSpeed
WheelSpeed
WheelSpeed
...
```

但是 ECU：

```text
故意不交给COM
```

最终：

```text
COM Deadline Monitoring
        ↓
Rx Timeout
        ↓
DEM
        ↓
DTC
```

这样不用真的把 CAN 线拔掉。



### ⑧ 模拟Tx Confirmation丢失

更高级一点：

```text
CanIf_Transmit()
      ↓
CAN Controller
      ↓
实际发送成功
      ↓
TxConfirmation
      X
```

故意：

```text
不调用上层 TxConfirmation
```

然后测试：

```text
Tx Confirmation Timeout
```

这对于验证：

```text
CanIf
CanSM
COM
DEM
```

非常有价值。



### ⑨ 模拟Bus-Off

测试版本可以：

```c
CanTest_InjectBusOff();
```

然后模拟：

```text
CAN Controller
      ↓
Bus-Off
      ↓
CanDrv
      ↓
CanIf_ControllerBusOff()
      ↓
CanSM
      ↓
Bus-Off Recovery
      ↓
Restart
```

AUTOSAR CanIf/CanSM本身就定义了 Controller Bus-Off 通知和恢复相关的职责边界。([AUTOSAR](https://autosar.org/fileadmin/standards/R4.0.3/CP/AUTOSAR_SWS_CANInterface.pdf?utm_source=chatgpt.com))



### ⑩ CAN故障定位日志一定要做“时间线”

不要只打印：

```text
CAN ERROR
```

而是：

```text
[123450 ms]
CAN0:
TEC=128
REC=3
State=ERROR_PASSIVE
Error=BIT

[123451 ms]
CanIf:
ControllerBusOff()

[123452 ms]
CanSM:
BUS_OFF

[123552 ms]
CanSM:
RECOVERY_START

[123602 ms]
CAN:
Controller started

[123610 ms]
CanSM:
FULL_COMMUNICATION
```

这就非常有用了。



## 11. 真正适合放在项目里的“CAN故障定位决策树”

```text
                 CAN通信异常
                      │
                      ▼
              CANH/CANL有波形吗？
               /              \
             NO                YES
             │                  │
             ▼                  ▼
        查PHY/供电          TXD/RXD正常吗？
        TJA1145模式         /          \
        STB/EN            NO           YES
                           │             │
                           ▼             ▼
                     查TXD/RXD       CAN Controller
                     短路/断路        错误状态
                     GPIO MUX         │
                                      ▼
                               ┌─────────────┐
                               │ TEC / REC   │
                               │ Error State │
                               │ Bus-Off     │
                               └─────────────┘
                                      │
                ┌─────────────────────┼────────────────────┐
                ▼                     ▼                    ▼
             Bit Error            ACK Error           CRC/Error
                │                     │                    │
                ▼                     ▼                    ▼
          波形/采样点             对端是否存在          BitRate
          TXD/RXD                 终端电阻              SamplePoint
          CANH/CANL               对端是否ACK           EMC
                                     
                                      ▼
                               AUTOSAR继续向上查
                                      │
                         ┌────────────┼────────────┐
                         ▼            ▼            ▼
                       CanIf         COM         CanNm
                         │            │            │
                       Rx/Tx        Timeout       NM
                         │            │            │
                         └────────────┼────────────┘
                                      ▼
                                     DEM
                                      │
                                      ▼
                                     DTC
```



## 12. 把“改CAN东西找哪一层”直接记成这张表

```text
                  CAN
                   │
        ┌──────────┴──────────┐
        │                     │
      PDU属性               Signal属性
        │                     │
   ┌────┼────┐          ┌─────┼──────┐
   │    │    │          │     │      │
  ID   DLC  Direction  Start Length Factor
   │    │               │     │      │
   └────┴──────┐        └─────┴──────┘
               │
             CanIf             COM
```

所以：

> **改ID → CanIf**

> **改DLC → CanIf**

> **改Signal → COM**

> **改周期 → COM**

> **停某业务PDU → COM/CanIf测试Gate**

> **停全部TX → CanIf PDU Mode / CanSM策略**

> **只收不发 → RX ON + TX OFF**

> **只停NM → CanNm**

> **只停诊断 → CanTp/DCM**

> **模拟Timeout → COM/CanIf测试Hook**

> **模拟Bus-Off → CAN Controller/CanDrv测试Hook**



## 13. 一个很重要的工程原则

如果准备把这些东西真正放进 **TC377 + TJA1145 AUTOSAR项目**，反而不建议把所有“奇技淫巧”直接塞进正式 CanIf/COM。

更好的架构是：

```text
                    ┌─────────────────────┐
                    │    Application      │
                    └──────────┬──────────┘
                               │
                    ┌──────────▼──────────┐
                    │       COM           │
                    └──────────┬──────────┘
                               │
                    ┌──────────▼──────────┐
                    │       PduR          │
                    └──────────┬──────────┘
                               │
             ┌─────────────────▼─────────────────┐
             │              CanIf                │
             │                                   │
             │        CAN TEST / DEBUG GATE      │
             │                │                  │
             │       ┌────────▼────────┐         │
             │       │ Tx Enable       │         │
             │       │ Rx Enable       │         │
             │       │ ID Override     │         │
             │       │ DLC Override    │         │
             │       │ PDU Block       │         │
             │       └─────────────────┘         │
             └─────────────────┬─────────────────┘
                               │
                         ┌─────▼─────┐
                         │  CanDrv   │
                         └─────┬─────┘
                               │
                         TC377 MCMCAN
```

**正式AUTOSAR配置负责“产品行为”，CanTest负责“工程调试行为”。**

这样有需求：

- CAN ID临时修改
- DLC修改
- 单帧停发
- 全局只收不发
- Signal强制
- Signal冻结
- Rx Timeout注入
- Tx Confirmation丢失
- Bus-Off注入
- NM Timeout
- CAN Error Counter监控

都不用反复改正式代码。

而且这套东西非常适合和 **TC377 + TJA1145 CAN Diagnostic实例**合并，最终可以形成：

```text
CanDiagnostic
├── Hardware Diagnostic
│   ├── TXD/RXD
│   ├── TJA1145
│   ├── CANH/CANL
│   └── Controller
│
├── Protocol Diagnostic
│   ├── Bit Error
│   ├── ACK Error
│   ├── CRC Error
│   ├── Stuff Error
│   └── Bus-Off
│
├── Communication Diagnostic
│   ├── Rx Timeout
│   ├── Tx Confirmation Timeout
│   └── NM Timeout
│
└── CanTest
    ├── PDU Block
    ├── Global TX Off
    ├── RX/TX Gate
    ├── ID Override
    ├── DLC Override
    ├── Signal Force
    ├── Signal Freeze
    ├── Timeout Injection
    └── Bus-Off Injection
```

这样基本就从一个“CAN诊断代码包”升级成了一个**真正能拿来做日常CAN问题定位的工程调试框架**。