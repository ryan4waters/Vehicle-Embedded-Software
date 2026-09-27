# RXD/TDX FAULT

建立一条：**示波器现象 → MCU RXD/TXD → CAN Controller 错误状态 → CanDrv → CanIf → COM/CanNm → DEM**的故障定位链。

## 1. CAN 硬件故障问题定位指南

结论：**RXD/TXD 对 GND/VCC 的短路，很多情况下不是“直接诊断成 RXD 短路/TXD 短路”，而是最终表现为 Bit Error、ACK Error、Bus-Off、报文收发异常。**

也就是说：**CAN 控制器通常知道“通信不对”，但不知道“PCB上哪根线短路了”。**

真正定位到 RXD/TXD 短路，通常需要：

- CAN Controller 状态
- TXD/RXD 电平
- CANH/CANL 波形
- TJA1145 状态/故障寄存器
- 外部物理测量

联合判断。

## 2. CAN链路

TC377 + TJA1145 可以简化成：

```text
TC377
┌──────────────────────┐
│                      │
│ CAN Controller       │
│                      │
│       TXD ────────────────> TJA1145 TXD
│                      │
│       RXD <──────────────── TJA1145 RXD
│                      │
└──────────────────────┘
                         │
                         │
                  ┌──────▼──────┐
                  │   TJA1145   │
                  │ CAN Trans.  │
                  └──────┬──────┘
                         │
                    CANH │ CANL
                         │
                    CAN Bus
```

数据方向：

```text
发送：

TC377 CAN Controller
        │
        │ TXD
        ▼
     TJA1145
        │
        │ CANH/CANL
        ▼
      CAN Bus
```

接收：

```text
CAN Bus
   │
   │ CANH/CANL
   ▼
 TJA1145
   │
   │ RXD
   ▼
TC377 CAN Controller
```

所以故障可以分成四个区域：

```text
① MCU内部
② MCU ↔ Transceiver
③ Transceiver内部
④ CANH/CANL总线
```

这四类故障的表现完全不同。



## 3. TXD/RXD 故障矩阵

这是实际调试时最值得打印出来的一张表。

| 故障          | MCU看到什么                  | CAN总线表现           | Controller可能表现            | 常见最终故障        |
| ------------- | ---------------------------- | --------------------- | ----------------------------- | ------------------- |
| TXD-GND短路   | TXD始终0                     | 持续Dominant          | Bit Error / Bus-Off           | TXD stuck dominant  |
| TXD-VCC短路   | TXD始终1                     | ECU无法主动发Dominant | Bit Error / ACK Error         | TXD stuck recessive |
| TXD断路       | 通常被Transceiver内部上拉成1 | ECU基本不主动发送     | TX异常/无发送                 | TXD open            |
| RXD-GND短路   | MCU始终看到0                 | 总线可能正常          | 接收异常、发送时出现Bit Error | RXD stuck dominant  |
| RXD-VCC短路   | MCU始终看到1                 | 总线可能正常          | 接收不到Dominant              | RXD stuck recessive |
| RXD断路       | RXD可能浮空/固定电平         | 总线可能正常          | 接收随机/完全异常             | RXD open            |
| CANH-GND      | 总线电气异常                 | CAN波形异常           | Bit/ACK/Error Passive/Bus-Off | CANH short GND      |
| CANL-GND      | 可能形成Dominant相关异常     | 总线可能持续Dominant  | Error/Bus-Off                 | CANL short GND      |
| CANH-VBAT     | 总线电气异常                 | CAN波形异常           | Error/Bus-Off                 | CANH short VBAT     |
| CANL-VBAT     | 总线电气异常                 | CAN波形异常           | Error/Bus-Off                 | CANL short VBAT     |
| CANH-CANL短路 | 差分电压消失                 | 无正常通信            | Error/Bus-Off                 | CANH/L short        |

但是这里面有几个非常“刁钻”的点。



## 4. TXD-GND 短路是最好理解的

假设：

```text
TC377 TXD
   │
   ├────── X
   │      │
   │     GND
```

TXD 是：

```text
TXD = 0 → Transceiver发送Dominant
TXD = 1 → Transceiver发送Recessive
```

如果 TXD 被短到 GND：

```text
TXD = 0
```

于是 TJA1145 一直尝试：

```text
CANH ↑
CANL ↓
```

也就是持续 Dominant。

**这会发生什么？**

整个 CAN 总线：

```text
Dominant
Dominant
Dominant
Dominant
Dominant
...
```

其他 ECU：

```text
发送Recessive
      ↓
实际总线Dominant
      ↓
检测到Bit Error
```

于是整个网络都可能出现：

```text
Bit Error
↓
Error Counter增加
↓
Error Passive
↓
Bus-Off
```

现代 CAN Transceiver 通常会有：**TXD Dominant Timeout**

防止一个 TXD 被拉低之后永久占用 CAN 总线。TI 对这一类故障也明确说明，TXD 持续 dominant 会阻塞总线，而 TXD dominant timeout 用于解除这种情况。([TI E2E](https://e2e.ti.com/support/interface-group/interface/f/interface-forum/179437/can-transceiver-fault-question?utm_source=chatgpt.com))

所以：

```text
TXD-GND
   ↓
TXD stuck dominant
   ↓
TJA1145 TXD dominant timeout
   ↓
Transceiver停止持续驱动
   ↓
CAN网络恢复
```

这时候就可以把：**TXD-GND短路**和**软件一直发送0 / CAN控制器卡死在发送**结合起来分析。



## 5. TXD-VCC 短路反而比较“隐蔽”

假设：

```text
TXD = 1
```

那么 TJA1145 永远收到：

```text
Recessive
```

也就是说：ECU无法主动发送 Dominant。

例如 MCU 想发送：

```text
10110010
```

实际上 TXD：

```text
11111111
```

TJA1145只能一直发送 Recessive。

这时候 CAN Controller 会看到什么？

CAN Controller 自己认为：

```text
我要发送：

0
```

但是 RXD反馈：

```text
1
```

于是：

```text
TX bit ≠ Bus bit
```

就可能形成：**Bit Error**

但有一个特别重要的例外：

**Arbitration 阶段**

CAN本身允许：

```text
我发送Recessive
总线是Dominant
```

因为这可能意味着：

> 我输掉了仲裁。

所以不能简单看到：

```text
TXD = 1
RXD = 0
```

就立即认为 Bit Error。

要结合：

- 当前是否在 Arbitration Field
- 当前发送的是 Data/CRC/Ack
- Controller 的 error status
- TEC/REC

一起看。



## 6. RXD-GND短路是非常有意思的故障

这个时候：

```text
TJA1145正常工作

CAN Bus
   ↓
TJA1145
   ↓
RXD = 0
   ↓
TC377
```

但：CAN总线实际上可能完全正常

例如：

```text
CANH/CANL

101101010100101
```

但是 MCU 永远看到：

```text
000000000000000
```

那么发送时会怎样？

假设 TC377发送：

```text
TXD：

1 0 1 1 0 0
```

正常情况下：

```text
RXD：

1 0 1 1 0 0
```

现在 RXD 被拉到 GND：

```text
RXD：

0 0 0 0 0 0
```

于是：

```text
Controller
  ↓
发送Dominant
  ↓
TXD = 0
  ↓
Bus = 0
  ↓
RXD = 0
```

这个阶段反而可能没问题。

但是：

```text
Controller发送Recessive
TXD = 1
Bus = 1
RXD应该 = 1

实际RXD = 0
```

于是：TX ≠ RX

Controller会开始认为：**Bus bit 与发送预期不一致**

最终可能出现：

```text
Bit Error
→ Error Counter
→ Error Passive
→ Bus-Off
```

所以非常关键：**RXD-GND并不一定导致“CAN总线本身异常”，但会让本 ECU 的 CAN Controller 对总线产生错误判断。**



## 7. RXD-VCC短路

反过来：

```text
RXD = 1
```

MCU永远认为：

```text
Bus = Recessive
```

即使真实 CAN：

```text
CAN Bus = Dominant
```

MCU也看不到。

于是：

```text
CAN Bus：

000101100

MCU RXD：

111111111
```

这时候：

- ECU无法正常接收
- CAN Controller的发送回读也可能异常
- ACK也可能失败
- 发送报文可能出现 ACK Error
- 最终可能 Error Passive / Bus-Off



## 8. RXD断路怎么办？

这个问题非常容易被忽略。

假设：

```text
TJA1145 RXD ───── X ───── TC377 RXD
```

RXD开路。

这时候不能简单说：一定检测成 RXD Open。

因为最终 MCU RXD 的状态取决于：

- TJA1145 RXD输出结构
- MCU GPIO输入结构
- MCU内部Pull-Up/Pull-Down
- PCB外部上下拉
- 输入漏电流

有些 CAN Transceiver 的 TXD 输入本身具有内部上拉；例如 TI 对 TCAN1042 的说明中明确提到 TXD 开路会被内部上拉解释为 High。([TI E2E](https://e2e.ti.com/support/interface-group/interface/f/interface-forum/946807/tcan1042-q1-how-does-the-txd-pin-work?utm_source=chatgpt.com))

但是 **RXD 是输出端**，它的开路行为不能照搬 TXD。

所以：RXD open 更准确的工程判断方式是：

```text
CAN Bus正常
+
TJA1145 RXD pin正常
+
MCU RXD pin固定不变/浮动
```

然后通过：

```text
示波器同时看：

CANH
CANL
TJA1145_RXD
TC377_RXD
```

判断。



## 9. CANH/CANL故障和RXD/TXD故障要区分

这是现场定位最容易搞混的地方。

例如：

```text
CANH → GND
```

不一定意味着：

```text
TJA1145报一个“CANH短地故障”
```

很多 CAN Transceiver 并没有这么精细的诊断。

最终看到的可能只是：

```text
CAN Controller

Bit Error
Error Passive
Bus-Off
```

甚至不同短路方向产生的表现都不一样。

TI 的一个实际案例就很典型：不同的 CANH/CANL 对电源/地短路组合，并不一定都会触发同一个 FAULT 指示；具体取决于故障是否形成持续 dominant 等条件。([TI E2E](https://e2e.ti.com/support/interface-group/interface/f/interface-forum/1057575/tcan337-fault-pin-state-not-changing?utm_source=chatgpt.com))

所以：**不要把 CAN Controller 的 Bit Error 直接等价成“CANH短路”。**



## 10. 现场使用“四层定位法”

以后碰到 CAN 故障，不要一上来就改软件。

直接按照：

```text
Layer 1：总线
Layer 2：Transceiver
Layer 3：MCU CAN Controller
Layer 4：AUTOSAR
```

来查。



### Layer 1：先看 CANH/CANL

示波器：

```text
CH1 = CANH
CH2 = CANL
```

检查：

```text
Idle
Dominant
Recessive
差分幅度
边沿
振铃
终端电阻
共模电压
```

重点：

```text
CANH ≈ CANL
```

→ 差分信号没出来。

```text
CANH持续高 / CANL持续低
```

→ 重点查 Dominant。

```text
CANH/CANL都有波形，但是大量错误
```

→ 查：

- Bit Rate
- Sample Point
- Termination
- 时钟
- 拓扑
- 波形质量



### Layer 2：直接看 TXD/RXD

这个技巧非常重要。

示波器四通道：

```text
CH1 CANH
CH2 CANL
CH3 TXD
CH4 RXD
```

形成：

```text
          TJA1145
             │
TXD ─────────┤
             │
CANH ────────┤
CANL ────────┤
             │
RXD ─────────┘
```

然后就能快速判断：

#### Case A

```text
TXD变化
RXD变化
CANH/CANL变化
```

→ PHY基本正常。



#### Case B

```text
TXD变化
RXD不变化
CANH/CANL正常
```

重点查：

```text
RXD PCB
RXD pin mux
MCU GPIO
TJA1145 RXD
```



#### Case C

```text
TXD一直0
```

重点查：

```text
MCU GPIO
CAN Controller
TXD PCB
短GND
软件发送状态
```



#### Case D

```text
TXD一直1
```

重点查：

```text
TXD短VCC
TX功能没打开
CAN Controller没有发送
CAN Controller处于Bus-Off
CanIf Tx PDU OFF
CanSM没有进入Full Communication
```



#### Case E

```text
TXD正常
CANH/CANL不动
```

重点查：

```text
TJA1145 mode
STB/EN
VCC
VIO
Transceiver fault
TXD input
```



### Layer 3：看 CAN Controller

TC377这里建议至少实时记录：

```c
typedef struct
{
    uint32_t tec;
    uint32_t rec;

    bool errorActive;
    bool errorPassive;
    bool busOff;

    uint32_t bitErrorCnt;
    uint32_t stuffErrorCnt;
    uint32_t crcErrorCnt;
    uint32_t formErrorCnt;
    uint32_t ackErrorCnt;

} CanControllerDiagType;
```

然后做成：

```text
CAN Controller Dashboard
```

例如：

```text
TEC          : 128
REC          : 0

Error State  : ERROR_PASSIVE
Bus-Off      : NO

Bit Error    : 25
ACK Error    : 8
CRC Error    : 0
Stuff Error  : 0
Form Error   : 0
```

这比单纯一个：

```text
CAN communication fault = TRUE
```

有价值太多。



### Layer 4：AUTOSAR定位

最终形成：

```text
TC377 MCMCAN
       ↓
CanDrv
       ↓
CanIf
       ↓
 ┌─────┼────────────┐
 ↓     ↓            ↓
COM   CanNm        CanTp
 ↓     ↓            ↓
RTE    Nm          PduR
       ↓
     ComM
       ↓
     CanSM
       ↓
      DEM
       ↓
      DTC
       ↓
      DCM
       ↓
     UDS 0x19
```

AUTOSAR CanIf配置本身就包含 L-PDU 的 ID、DLC、接收过滤、上下层PDU映射等信息。([AUTOSAR](https://autosar.org/fileadmin/standards/R4.0.3/CP/AUTOSAR_SWS_CANInterface.pdf?utm_source=chatgpt.com))

所以现场不要只看：

```text
COM timeout
```

而应该向下追：

```text
COM timeout
↓
CanIf有没有收到？
↓
CanDrv有没有收到？
↓
CAN Controller有没有收到？
↓
RXD有没有变化？
↓
CANH/CANL有没有变化？
```