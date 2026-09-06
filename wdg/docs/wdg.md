# WDG

- **方案 A：TC377 + TLF35584**
- **方案 B：TI F29P32x + SGM820**

先说明一个非常重要的区别：
**TLF35584 本身可以形成真正意义上的 Window Watchdog + Functional Watchdog 双监控；SGM820 本质上是普通超时型外部 Watchdog，并不是严格意义上的 Window Watchdog。**

所以 F29P32x + SGM820 更准确的架构是：
**F29 内部 Window Watchdog + SGM820 外部 Functional Watchdog**

而 TC377 + TLF35584 可以进一步做到：
**TC377 内部 WDG + TLF35584 外部 Window WDG + TLF35584 外部 Functional WDG。**

以这两套方案来理解汽车 MCU 的“双重看门狗”。

# 一、建立整个 WDG 安全架构

```
                    ┌──────────────────────────────┐
                    │          Application         │
                    │ PDU / DCDC / CAN / PWM ...   │
                    └──────────────┬───────────────┘
                                   │
                         Alive / Timing supervision
                                   │
                    ┌──────────────▼───────────────┐
                    │       Internal Watchdog      │
                    │                              │
                    │ CPU WDG / Window WDG / NMI   │
                    └──────────────┬───────────────┘
                                   │
                         Fault / Timeout
                                   │
                    ┌──────────────▼───────────────┐
                    │ SMU / ESM / Safety Manager   │
                    │                              │
                    │ Alarm                        │
                    │ NMI                          │
                    │ Reset                        │
                    │ Error Pin                    │
                    └───────┬─────────────┬────────┘
                            │             │
                         Internal      External
                         reaction      reaction
                            │             │
                            │             ▼
                            │      External Watchdog
                            │             │
                            │             │
                            ▼             ▼
                       CPU Reset       MCU RESET
                                          │
                                          ▼
                                    Safe State / Restart
```

真正做 ASIL 软件时，关键不是“有两个 WDG”。

而是：**两个 WDG 必须尽可能避免共因失效。**

也就是说：

```
软件死循环
    ↓
内部 WDG
    ↓
SMU/ESM
    ↓
Reset

同时

软件死循环
    ↓
外部 WDG
    ↓
独立硬件
    ↓
Reset
```

两个监控链路不能都依赖同一个软件任务、同一个 Timer、同一个中断。


# 二、方案 A：TC377 + TLF35584

![TLF35584+TC3xx](.\TLF35584+TC3xx.jpg)

![TC3xxFusa](.\TC3xxFusa.jpg)

![WWDtriggering](.\WWDtriggering.jpg)

![TLF35584+TC264](.\TLF35584+TC264.jpg)

Infineon 官方对 TC3xx + TLF35584 的推荐安全连接实际上已经非常接近这个架构：SPI、FSP/ERR、ESR1/INT、WDI、PORST/ROT 等均用于 MCU 与 PMIC 的安全交互。


# 三、TC377 + TLF35584 硬件连接

最核心的连接关系：

```
                  TC377
        ┌───────────────────────┐
        │                       │
        │       TriCore CPU     │
        │                       │
        │  CPU WDG              │
        │      │                │
        │      ▼                │
        │     SMU               │
        │      │                │
        │      │ FSP            │
        │      ├───────────────────────┐
        │      │                       │
        │      │                       ▼
        │      │                  TLF35584
        │      │               ┌──────────────┐
        │      └──────────────►│ ERR          │
        │                      │              │
        │ WDI GPIO ───────────►│ WDI          │
        │                      │              │
        │ ESR1 ◄───────────────│ INT          │
        │                      │              │
        │ PORST ◄──────────────│ ROT          │
        │                      │              │
        │ QSPI ───────────────►│ SPI          │
        │                      │              │
        └──────────────────────┤              │
                               │ SS1 / SS2    │
                               └──────────────┘
```

官方推荐连接包括：

| TC377     | TLF35584 | 作用              |
| --------- | -------- | -----------------|
| QSPI_MISO | SDO      | SPI读             |
| QSPI_MOSI | SDI      | SPI写             |
| QSPI_CS   | CSN      | SPI片选           |
| QSPI_CLK  | SCL      | SPI时钟           |
| SMU FSP   | ERR      | MCU故障报告       |
| GPIO/ESR1 | INT      | PMIC故障/服务请求  |
| GPIO      | WDI      | 外部Window WD     |
| PORST     | ROT      | PMIC复位MCU       |
| GPIO      | SS1      | Safe State        |
| GPIO      | SS2      | Safe State        |

这些连接是 Infineon 官方 TC3xx/TLF35584 安全集成方案的一部分。


# 四、这里最重要的是 FSP

很多人第一次接触 TC377 + TLF35584 时，会认为：

```
TC377 WDG
      ↓
TLF35584 WDI
      ↓
TLF35584 RESET
```

实际上不完整。

还有另外一条非常重要的：

```
TC377内部故障
      ↓
SMU
      ↓
FSP
      ↓
TLF35584 ERR
      ↓
TLF35584安全状态
```

也就是说：**FSP 不是喂狗信号。**

它是：**MCU → PMIC 的故障报告通道。**


# 五、TC377 内部到底有哪些 WDG？

TC3xx 有：

```
CPU0 WDG
CPU1 WDG
CPU2 WDG
CPU3 WDG
...
Safety WDG
```

每个 CPU 有自己的 WDG，同时还有一个面向整个 MCU 的 Safety WDG。

例如 CPU0：

```
CPU0
 │
 ├── Application
 │
 ├── Interrupt
 │
 ├── Trap
 │
 └── CPU0 WDG
```

Safety WDG：

```
              ┌──── CPU0
              ├──── CPU1
              ├──── CPU2
Safety WDG ───┼──── Shared resource
              ├──── SCU
              └──── Safety software
```

所以它们解决的问题不完全一样。


# 六、TC377 WDG 最容易被误解的地方：ENDINIT

TC3xx 的 WDG 不只是“程序不喂狗就复位”。

它还有一个非常重要的功能：**保护关键寄存器。**

例如：

```
CPU
 ↓
Clear ENDINIT
 ↓
修改受保护寄存器
 ↓
Set ENDINIT
```

如果：

```
Clear ENDINIT
 ↓
程序跑飞
 ↓
没有重新 Set ENDINIT
```

WDG 超时。

于是：

```
WDG
 ↓
SMU Alarm
 ↓
NMI / Reset / FSP
```

官方文档明确指出，ENDINIT 打开后有时间限制，如果没有及时重新置回，会产生 WDG alarm。


# 七、所以 TC377 内部 WDG 是怎么工作的？

可以理解成：

```
                 CPU0
                  │
                  │
          ┌───────▼────────┐
          │ CPU0 WDG       │
          └───────┬────────┘
                  │
           Timeout / Error
                  │
                  ▼
                SMU
                  │
       ┌──────────┼───────────┐
       │          │           │
      ISR         NMI       RESET
       │          │           │
       └──────────┴───────────┘
                  │
                  ▼
                 FSP
                  │
                  ▼
              TLF35584
```

SMU 可以把不同 Alarm 配置成 ISR、NMI、CPU Reset、System Reset、Emergency Stop 或 FSP 等不同反应。


# 八、SMU 才是 TC377 WDG 的核心

建议把 TC377 的安全链路记成：

```
Safety Mechanism
      ↓
     Alarm
      ↓
     SMU
      ↓
 Reaction Configuration
      ↓
 ┌────┼─────┬──────┐
 NMI  Reset  FSP    ES
```

例如：

```
CPU WDG Timeout
       ↓
SMU Alarm Gx.Ay
       ↓
      NMI
       ↓
NMI Handler
       ↓
记录故障
       ↓
触发 Reset
```

或者：

```
CPU WDG Timeout
       ↓
SMU
       ↓
FSP
       ↓
TLF35584
       ↓
ROT
       ↓
TC377 PORST
```

这就是**内部 + 外部双重监控**。


# 九、为什么需要 NMI？

因为普通 ISR 可能被软件关闭：

```
disableInterrupts();
```

甚至：

```
CPU进入死循环
```

此时普通中断可能无法正常执行。

NMI：

```
Normal ISR
    ↓
可能被屏蔽

NMI
    ↓
更高优先级
    ↓
用于严重安全故障
```

所以一个典型设计是：

```
WDG Timeout
      ↓
     SMU
      ↓
     NMI
      ↓
NMI Handler
      ↓
记录原因
      ↓
触发安全复位
```


# 十、TLF35584 又是什么？

TLF35584 是一个独立的安全电源/监控器。

它内部自己有：

```
Voltage Monitor
     +
Functional WD
     +
Window WD
     +
Safety Monitor
     +
Reset Generator
     +
SPI
     +
ERR/FSP
```

因此：

```
TC377
  ↑
  │
软件可能已经死掉
  │
  ↓
TLF35584
  │
独立硬件定时
  │
  ↓
Reset / Safe State
```

这就是为什么汽车 MCU 不应该只依赖内部 WDG。


# 十一、TLF35584 的 Functional WD 和 Window WD

## Functional Watchdog

可以理解：

```
必须在规定时间内喂狗
```

例如：

```
|------------------- 15ms ------------------|

                Feed
                  ↑
                 OK
```

如果：

```
|------------------- 15ms ------------------|

                                                    Feed
                                                      ↑
                                                  TOO LATE
```

触发 Fault。


# 十二、Window Watchdog

Window WD 更严格。

它要求：

```
不能太早
也不能太晚
```

例如：

```
        Closed Window
|----------------------|

                         Open Window
                         |-----------|
                            Feed
                             ↑
                             OK
```

太早：

```
Feed
 ↓
Closed Window
 ↓
Fault
```

太晚：

```
Open Window
 ↓
Timeout
 ↓
Fault
```

TLF35584 的 WWD 可以通过 WDI 或 SPI 进行服务。Infineon 的应用笔记给出的示例就是 Closed Window 5 ms + Open Window 10 ms，同时 FWD 15 ms。


# 十三、TC377 + TLF35584 推荐架构

```
             Application
                  │
                  ▼
          ┌──────────────┐
          │ Alive Monitor│
          └───────┬──────┘
                  │
                  ▼
             CPU WDG
                  │
                  ▼
                 SMU
              ┌───┴───┐
              │       │
             NMI     FSP
              │       │
              │       ▼
              │   TLF35584
              │       │
              │    WWD/FWD
              │       │
              │       ▼
              │     ROT
              │       │
              ▼       ▼
          NMI Handler RESET
```


# 十四、TC377 软件喂狗应该怎么设计？

**千万不要这样：**

```
void MainLoop(void)
{
    while(1)
    {
        AppTask();
        FeedWatchdog();
    }
}
```

因为如果：

```
AppTask()
 ↓
死循环
```

那么：

```
FeedWatchdog()
```

永远不会执行。

这当然能检测“死循环”，但是检测不到很多**局部功能已经死掉**的情况。


# 十五、更好的方法：Alive Counter

例如：

```
volatile uint32 g_DcdcAlive;
volatile uint32 g_CanAlive;
volatile uint32 g_PduAlive;
```

任务执行：

```
void Dcdc_Task(void)
{
    Dcdc_Control();

    g_DcdcAlive++;
}
```

CAN：

```
void Can_Task(void)
{
    Can_Process();

    g_CanAlive++;
}
```

然后 Watchdog Supervisor：

```
void Wdg_Supervisor(void)
{
    if(g_DcdcAlive == g_DcdcAliveLast)
    {
        WdgFault |= WDG_FAULT_DCDC_NOT_ALIVE;
        return;
    }

    if(g_CanAlive == g_CanAliveLast)
    {
        WdgFault |= WDG_FAULT_CAN_NOT_ALIVE;
        return;
    }

    g_DcdcAliveLast = g_DcdcAlive;
    g_CanAliveLast  = g_CanAlive;

    Wdg_Service();
}
```

这样才是真正的：

```
Application
      ↓
Functional Monitoring
      ↓
Watchdog Service
```

而不是：

```
Timer Interrupt
      ↓
Feed WD
```


# 十六、但是还有一个问题：Timer ISR 自己死了怎么办？

这就是 Window Watchdog 的价值。

例如：

```
Timer ISR
    ↓
Feed WDG
```

如果 Timer：

```
异常频繁进入
```

那么：

```
Feed
Feed
Feed
Feed
Feed
```

可能变成：

```
过早喂狗
```

Window WD 就可以检测。

因此：**Window WD 不仅检测“没执行”，还能检测“执行太快”。**

这就是它比普通 Functional WD 更强的地方。


# 十七、TC377 外部 WWD 推荐用专用 OS Task

例如：

```
1ms task
     │
     ▼
Safety Supervisor
     │
     ├── DCDC alive
     ├── PDU alive
     ├── CAN alive
     ├── Timing check
     ├── Stack check
     └── WDG decision
             │
             ▼
       TLF35584 WDI
```

不要让每一个普通任务都直接喂外狗。


# 十八、TC377 TLF35584 初始化

初始化顺序非常重要。

推荐：

```
Power On
   ↓
TC377 Start
   ↓
CPU WDG/Safety WDG
   ↓
Clock
   ↓
QSPI
   ↓
TLF35584 SPI
   ↓
Read TLF status
   ↓
Configure WWD
   ↓
Configure FWD
   ↓
Configure ERR/FSP
   ↓
First WWD service
   ↓
First FWD service
   ↓
NORMAL state
```

Infineon 官方给出的基本初始化队列也是读取 WWD/FWD 状态、使能 FSP、配置 WWD/FWD、第一次服务，然后进入 NORMAL。


# 十九、TC377 代码框架

下面代码是**工程结构级代码**。具体寄存器值应该根据 TC377 型号、iLLD 版本和 TLF35584 variant 配置。

### `Wdg_Tc377.h`

```
#ifndef WDG_TC377_H
#define WDG_TC377_H

#include "Ifx_Types.h"

typedef enum
{
    WDG_OK = 0,
    WDG_ERR_TIMEOUT,
    WDG_ERR_ALIVE,
    WDG_ERR_TLF,
    WDG_ERR_INTERNAL
} Wdg_StatusType;

typedef struct
{
    uint32 dcdcAlive;
    uint32 pduAlive;
    uint32 canAlive;
    uint32 appAlive;

    uint32 lastDcdcAlive;
    uint32 lastPduAlive;
    uint32 lastCanAlive;
    uint32 lastAppAlive;

    uint32 fault;
} Wdg_ContextType;

void Wdg_Init(void);
void Wdg_MainFunction(void);

void Wdg_NotifyDcdc(void);
void Wdg_NotifyPdu(void);
void Wdg_NotifyCan(void);
void Wdg_NotifyApp(void);

void Wdg_ServiceInternal(void);
void Wdg_ServiceExternal(void);

void Wdg_SmuInit(void);

#endif
```


# 二十、TC377 主控制代码

```
#include "Wdg_Tc377.h"
#include "IfxScuWdt.h"
#include "IfxSmu.h"

static Wdg_ContextType g_wdg;

void Wdg_Init(void)
{
    g_wdg.dcdcAlive = 0;
    g_wdg.pduAlive  = 0;
    g_wdg.canAlive  = 0;
    g_wdg.appAlive  = 0;

    g_wdg.lastDcdcAlive = 0;
    g_wdg.lastPduAlive  = 0;
    g_wdg.lastCanAlive  = 0;
    g_wdg.lastAppAlive  = 0;

    g_wdg.fault = 0;

    Wdg_SmuInit();
}
```


# 二十一、任务 Alive

```
void Wdg_NotifyDcdc(void)
{
    g_wdg.dcdcAlive++;
}

void Wdg_NotifyPdu(void)
{
    g_wdg.pduAlive++;
}

void Wdg_NotifyCan(void)
{
    g_wdg.canAlive++;
}

void Wdg_NotifyApp(void)
{
    g_wdg.appAlive++;
}
```


# 二十二、Supervisor

```
void Wdg_MainFunction(void)
{
    uint32 healthy = 1U;

    if(g_wdg.dcdcAlive == g_wdg.lastDcdcAlive)
    {
        g_wdg.fault |= 0x01U;
        healthy = 0U;
    }

    if(g_wdg.pduAlive == g_wdg.lastPduAlive)
    {
        g_wdg.fault |= 0x02U;
        healthy = 0U;
    }

    if(g_wdg.canAlive == g_wdg.lastCanAlive)
    {
        g_wdg.fault |= 0x04U;
        healthy = 0U;
    }

    if(g_wdg.appAlive == g_wdg.lastAppAlive)
    {
        g_wdg.fault |= 0x08U;
        healthy = 0U;
    }

    g_wdg.lastDcdcAlive = g_wdg.dcdcAlive;
    g_wdg.lastPduAlive  = g_wdg.pduAlive;
    g_wdg.lastCanAlive  = g_wdg.canAlive;
    g_wdg.lastAppAlive  = g_wdg.appAlive;

    if(healthy)
    {
        Wdg_ServiceInternal();
        Wdg_ServiceExternal();
    }
}
```

这个设计很重要：

```
Application正常
     ↓
Supervisor检测
     ↓
Healthy
     ↓
Internal WD
     +
External WD
```

只要应用异常：

```
Healthy = 0
     ↓
不喂狗
     ↓
Internal WD timeout
     +
External WD timeout
```


# 二十三、SMU 配置

这里需要特别注意：

SMU 是安全配置，必须使用 Safety ENDINIT。

例如：

```
void Wdg_SmuInit(void)
{
    uint16 password;

    password =
        IfxScuWdt_getSafetyWatchdogPasswordInline();

    IfxScuWdt_clearSafetyEndinit(password);

    /*
     * 1. Configure SMU alarm reaction
     *
     * WDG alarm
     *     -> NMI
     *     -> FSP
     *     -> Reset
     */

    /*
     * Example only:
     *
     * SMU_AGxCFy = ...
     */

    IfxScuWdt_setSafetyEndinit(password);
}
```

实际项目建议使用 iLLD/MCAL 提供的 SMU API，而不是直接硬编码 `SMU_AGxCFy`。

Infineon 官方示例也采用 Safety ENDINIT 保护 SMU 配置，并将 FSP 配置到 TC3xx 的错误输出引脚。


# 二十四、TC377 NMI

NMI Handler 的原则：**不要在 NMI 中做复杂业务。**

例如：

```
void Wdg_NmiHandler(void)
{
    uint32 smuAlarm;

    smuAlarm = Read_Smu_Alarm();

    g_wdg.fault |= smuAlarm;

    /*
     * Stop PWM
     */
    Disable_All_Pwm();

    /*
     * Force safe state
     */
    Enter_Safe_State();

    /*
     * Do NOT:
     * - printf
     * - CAN blocking transmit
     * - long loop
     * - wait for semaphore
     */

    Request_System_Reset();
}
```


# 二十五、为什么 NMI 不能做太多事情？

因为：

```
NMI
 ↓
CPU可能已经异常
```

如果在 NMI 里面：

```
Can_Send();
```

然后 CAN 卡住：

```
NMI
 ↓
CAN
 ↓
死循环
```

反而失去了 WDG 的意义。

正确方式：

```
NMI
 ↓
记录最小故障信息
 ↓
关闭危险输出
 ↓
Reset
```


# 二十六、FSP 是另一条安全通道

正常：

```
TC377
FSP = Healthy pattern
       ↓
TLF35584
       ↓
正常
```

故障：

```
TC377
  ↓
SMU Alarm
  ↓
FSP abnormal
  ↓
TLF35584 ERR
  ↓
Safety reaction
```

所以即使：

```
TC377 CPU
 ↓
完全死掉
```

只要 TLF35584 检测到 MCU 的安全通信/故障信号异常，就可以采取独立安全动作。

Infineon 将 FSP 定义为 MCU 内部故障向外部安全控制器报告的接口；TLF35584 是典型外部安全控制器。


# 二十七、现在看第二套：F29P32x + SGM820

这个方案的思路非常不一样。

F29P32x 本身包含：

```
CPU1
CPU2
CPU3
ESM
WWD
NMI Watchdog
LSL
ERAD
External INT
```

TI 的 F29P32x 数据手册明确把 ESM、NMI Watchdog、WWD 和多个 CPU 放在 CPU 系统中。


# 二十八、F29 + SGM820 硬件拓扑

推荐：

```
                    F29P32x
             ┌───────────────────┐
             │                   │
             │ CPU1              │
             │  │                │
             │  ▼                │
             │ Internal WWD      │
             │  │                │
             │  ▼                │
             │ ESM               │
             │  │                │
             │  └──────► NMI     │
             │                   │
             │ GPIO ─────────────┼─────────┐
             │                   │         │
             │ XINT ◄────────────┼────┐    │
             │                   │    │    │
             │ XRSn ◄────────────┼────┼────┤
             └───────────────────┘    │    │
                                      │    ▼
                                  nWDO │ SGM820
                                      │  ┌────────┐
                                      └──│ WDI    │
                                         │        │
                              VCC ───────│ VCC    │
                                         │        │
                              GND ───────│ GND    │
                                         │        │
                             nRESET ◄────│ nRESET │
                                         │        │
                             nWDO ───────│ nWDO   │
                                         └────────┘
```

SGM820 的关键引脚：

| Pin    | 功能                 |
| ------ | -------------------- |
| VCC    | 电源                 |
| CWD    | Watchdog timeout配置 |
| nMR    | 手动复位             |
| GND    | 地                   |
| SET    | WD使能/模式          |
| WDI    | 喂狗输入             |
| nWDO   | Watchdog输出         |
| nRESET | MCU复位              |

这些定义来自 SGM820 数据手册。


# 二十九、SGM820 是什么类型的狗？

这个地方非常重要。

SGM820：

```
WDI
 ↓
Timeout Counter
 ↓
nWDO
 ↓
nRESET
```

它要求 WDI 在规定 timeout 内产生下降沿。

例如：

```
|------------- tWD -------------|

             ↓
            WDI
             ↓
            OK
```

超过：

```
|------------- tWD -------------|

                                    ↓
                                   WDI
                                    ↓
                                   FAIL
```

SGM820 官方说明 WDI 是下降沿触发，必须在 tWD 内出现脉冲，否则 nWDO 进入低阻状态。

所以它是：**Functional Timeout Watchdog**

而不是：**True Window Watchdog**


# 三十、那么 F29 + SGM820 怎么实现“双重监控”？

可以：

```
              Application
                   │
                   ▼
              Supervisor
                /     \
               /       \
              ▼         ▼
        F29 Internal   GPIO
          WWD           │
              │         ▼
              ▼       SGM820
             ESM        │
              │         │
             NMI        │
              │         │
              ▼         ▼
            Reset ◄──── nRESET
```

这里两个 WDG：

```
内部：
F29 WWD

外部：
SGM820 Functional WD
```

形成：

```
                 software fault
                       │
              ┌────────┴────────┐
              │                 │
              ▼                 ▼
          F29 WWD            SGM820
              │                 │
              ▼                 ▼
             ESM              nWDO
              │                 │
             NMI                │
              │                 │
              └──────┬──────────┘
                     ▼
                    XRSn
```


# 三十一、F29 的 ESM 是什么？

F29 里面没有沿用传统 C2000 的 SYS_ERR 架构。

TI 明确说明：

> F29 的 NMI 由 ESM 处理，传统 SYS_ERR interrupt 已经移除。

所以：

```
F29 Error
    ↓
ESM
    ↓
NMI / Interrupt / Error Pin
```

这是从老 C2000 迁移到 F29 时非常容易踩坑的地方。


# 三十二、F29 ESM 架构

可以理解成：

```
CPU1
 └── ESM CPU1

CPU2
 └── ESM CPU2

CPU3
 └── ESM CPU3

System
 └── System ESM
```

ESM 可以管理大量 Error Event。

TI SDK 文档指出，ESM 支持 CPU instance、System ESM、Safety Aggregator，并支持大量 error events，以及 High Priority Watchdog、NMI、Error Pin 等机制。


# 三十三、F29 的 NMI Watchdog

这是非常关键的一层：

```
Error Event
    ↓
ESM
    ↓
NMI
    ↓
NMI ISR
    ↓
Clear Error
    ↓
EOI
```

如果：

```
NMI ISR
 ↓
没有清除 ESM RAW
```

则：

```
NMI Watchdog
 ↓
Timeout
 ↓
XRSn / CPU Reset
```

TI 官方 Error Handling Guide 明确说明了这一机制。


# 三十四、这个机制非常像：

```
第一层：
Error
 ↓
NMI

第二层：
NMI Handler
 ↓
NMIWD

第三层：
NMI Handler也死了
 ↓
Reset
```

这其实非常漂亮。


# 三十五、所以 F29 可以形成三级保护

```
Application fault
       ↓
Internal WWD
       ↓
ESM
       ↓
NMI
       ↓
NMI Handler
       ↓
NMI Watchdog
       ↓
XRSn
```

同时：

```
Application fault
       ↓
SGM820
       ↓
nRESET
       ↓
XRSn
```

两个方向同时存在。


# 三十六、XINT 在这里干什么？

**XINT 不是 WDG。**

它是：External Interrupt。

例如：

```
SGM820 nWDO
       │
       ▼
F29 GPIO
       │
       ▼
XINT
       │
       ▼
CPU ISR
```

这样可以让 CPU 在外部 WDG 即将/已经发生故障时记录信息。

但真正的 Reset：

```
SGM820 nRESET
      ↓
F29 XRSn
```

不要经过软件。

所以：

```
nWDO → XINT
```

用于：diagnosis / logging

而：

```
nRESET → XRSn
```

用于：hardware reset


# 三十七、千万不要这样设计

错误：

```
SGM820
 nWDO
   ↓
 XINT
   ↓
CPU ISR
   ↓
software reset
```

这相当于：

```
外部狗
 ↓
软件
 ↓
软件复位
```

失去了独立外部硬件监控的意义。

正确：

```
                 ┌── XINT → diagnosis
SGM820 nWDO ─────┤
                 │
                 └── nRESET → XRSn
```


# 三十八、F29 WWD 配置思路

这里建议：

```
F29 Internal WWD
      ↓
ESM
      ↓
High Priority
      ↓
NMI
      ↓
NMI ISR
```

如果 NMI Handler 正常：

```
clear error
EOI
log
```

如果 NMI Handler 本身死了：

```
NMIWD
 ↓
XRSn
```

TI 的 F29 SDK 已经提供 SysCtl WWD self-test 支持。


# 三十九、F29 代码结构

推荐工程：

```
wdg/
├── Wdg.h
├── Wdg.c
├── Wdg_F29.h
├── Wdg_F29.c
├── Wdg_Sgm820.h
├── Wdg_Sgm820.c
├── Wdg_Esm.c
└── Wdg_Test.c
```


# 四十、F29 WDG Header

```
#ifndef WDG_F29_H
#define WDG_F29_H

#include <stdint.h>

typedef enum
{
    WDG_F29_OK = 0,
    WDG_F29_ERR_ALIVE,
    WDG_F29_ERR_INTERNAL,
    WDG_F29_ERR_EXTERNAL
} WdgF29_Status;

typedef struct
{
    volatile uint32_t appAlive;
    volatile uint32_t dcdcAlive;
    volatile uint32_t canAlive;

    uint32_t lastAppAlive;
    uint32_t lastDcdcAlive;
    uint32_t lastCanAlive;

    volatile uint32_t fault;
} WdgF29_Context;

void WdgF29_Init(void);
void WdgF29_MainFunction(void);

void WdgF29_AppAlive(void);
void WdgF29_DcdcAlive(void);
void WdgF29_CanAlive(void);

void WdgF29_ServiceExternal(void);

void WdgF29_NmiHandler(void);
void WdgF29_ExtWdgHandler(void);

#endif
```


# 四十一、F29 Supervisor

```
#include "Wdg_F29.h"

static WdgF29_Context g_wdg;

void WdgF29_MainFunction(void)
{
    uint32_t healthy = 1U;

    if(g_wdg.appAlive == g_wdg.lastAppAlive)
    {
        g_wdg.fault |= 0x01U;
        healthy = 0U;
    }

    if(g_wdg.dcdcAlive == g_wdg.lastDcdcAlive)
    {
        g_wdg.fault |= 0x02U;
        healthy = 0U;
    }

    if(g_wdg.canAlive == g_wdg.lastCanAlive)
    {
        g_wdg.fault |= 0x04U;
        healthy = 0U;
    }

    g_wdg.lastAppAlive  = g_wdg.appAlive;
    g_wdg.lastDcdcAlive = g_wdg.dcdcAlive;
    g_wdg.lastCanAlive  = g_wdg.canAlive;

    if(healthy)
    {
        WdgF29_ServiceExternal();
    }
}
```


# 四十二、为什么这里没有直接写内部 WDG Feed？

因为真正项目中：

```
F29 Internal WWD
```

建议由 SDK/SysConfig 配置，并把服务动作封装成独立接口。

例如：

```
static void WdgF29_ServiceInternal(void)
{
    /*
     * TI F29 SDK / SysCtl WWD service
     *
     * Exact API depends on SDK release
     */
}
```

不要把具体寄存器地址硬编码在应用层。


# 四十三、SGM820 喂狗

SGM820 WDI 是下降沿触发。

所以：

```
void WdgF29_ServiceExternal(void)
{
    GPIO_write(WDG_GPIO, 0U);

    __asm(" NOP");
    __asm(" NOP");

    GPIO_write(WDG_GPIO, 1U);
}
```

真正项目要保证：

```
pulse > datasheet minimum
```

SGM820 数据手册给出的 WDI/nMR 最小脉冲宽度为 50 ns。


# 四十四、不要直接用软件 delay 喂狗

错误：

```
delay_us(100);
FeedDog();
```

因为：

```
CPU Clock变化
Interrupt latency
Compiler optimization
Flash wait state
```

都可能影响。

推荐：

```
CPU Timer / ePWM / deterministic ISR
                  ↓
             Supervisor
                  ↓
               WDI
```

也就是说：**喂狗时间基准应该来自可靠、确定性的硬件时间基准。**


# 四十五、F29 XINT

例如：

```
void WdgF29_ExtWdgHandler(void)
{
    /*
     * SGM820 nWDO active
     */

    g_wdg.fault |= WDG_FAULT_EXTERNAL;

    /*
     * Capture diagnostic information
     */
    Save_ResetContext();

    /*
     * DO NOT depend on software reset here.
     * nRESET remains hardware connected to XRSn.
     */
}
```

这里 XINT 的目的主要是：

```
nWDO
 ↓
XINT
 ↓
记录：
  - PC
  - SP
  - task ID
  - fault flags
  - reset reason
```


# 四十六、F29 NMI Handler

```
__interrupt void WdgF29_NmiHandler(void)
{
    uint32_t status;

    status = ESM_GetRawStatus();

    Save_EsmStatus(status);

    Disable_All_Pwm();

    /*
     * Clear ESM raw status
     */
    ESM_ClearRawStatus(status);

    /*
     * EOI is mandatory
     */
    ESM_WriteEOI();

    /*
     * Do not perform blocking operations here.
     */
}
```

这里特别注意：**F29 的 ESM RAW flag 清掉之后还要正确执行 EOI。**

否则 ESM 输出仍可能保持 asserted，从而继续触发 NMI。TI 官方 FAQ 对这一点有明确说明。


# 四十七、F29 NMI Watchdog

这是一个非常漂亮的安全机制：

```
                 ESM Error
                     │
                     ▼
                    NMI
                     │
                     ▼
              NMI Handler
              ┌──────┴──────┐
              │             │
           正常执行       卡死
              │             │
              ▼             ▼
          Clear+EOI       NMIWD
                            │
                            ▼
                           XRSn
```

所以不能把 NMI Handler 写成：

```
while(1)
{
}
```

因为 NMIWD 会最终把 MCU 拉回来。


# 四十八、两套方案放在一起比较

| 功能            | TC377                 | TLF35584     | F29P32x       | SGM820 |
| --------------- | --------------------- | ------------ | ------------- | ------ |
| 内部 WDG        | ✅                     | —            | ✅             | —      |
| Window WD       | 内部安全机制/时间监控 | ✅            | ✅ WWD         | ❌      |
| Functional WD   | CPU/Safety WDG        | ✅            | WWD可配置     | ✅      |
| NMI             | SMU                   | INT/安全反应 | ESM           | —      |
| NMI WD          | —/SMU Recovery机制    | —            | ✅             | —      |
| 外部独立硬件    | —                     | ✅            | —             | ✅      |
| Error reporting | SMU/FSP               | ERR          | ESM/Error Pin | nWDO   |
| MCU Reset       | SCU                   | ROT          | XRSn          | nRESET |
| SPI             | QSPI                  | SPI          | —             | —      |
| Safe State      | SMU/ES                | SS1/SS2      | GPIO/ESM      | —      |


# 四十九、两套方案最大的架构区别

### TC377 + TLF35584

可以形成：

```
            TC377
              │
          CPU WDG
              │
             SMU
          ┌───┴────┐
          │        │
         NMI      FSP
          │        │
          │        ▼
          │    TLF35584
          │      │  │
          │     WWD FWD
          │      │  │
          │      └──┤
          │         ▼
          └──────► RESET
```

这是非常完整的安全架构。


### F29 + SGM820

```
              F29
               │
            Internal
              WWD
               │
              ESM
               │
              NMI
               │
            NMIWD
               │
               ▼
              XRSn

       independent path

              │
             GPIO
              │
            SGM820
              │
       ┌──────┴──────┐
      nWDO          nRESET
       │               │
      XINT            XRSn
```

也是很强的双重监控，但是： **SGM820 不是 Window Watchdog。**


# 五十、真正量产时建议 WDG 分层

如果是做汽车电源/OBC/DCDC ECU，建议最终软件结构设计成：

```
                    WDG Architecture
                           │
              ┌────────────┴────────────┐
              │                         │
        Internal Monitor          External Monitor
              │                         │
      ┌───────┼────────┐              │
      │       │        │              │
   CPU WDG  ESM/NMI  Alive        External WDG
      │       │        │              │
      └───────┴────────┘              │
              │                       │
              ▼                       ▼
           Decision              Hardware
              │                       │
              └──────────┬────────────┘
                         ▼
                       RESET
```


# 五十一、Application Alive 应该监控什么？

对于这种 OBC/DCDC 软件，不会只监控：

```
MainLoop Alive
```

而应该至少：

```
PDU
 ├── Scheduler
 ├── CAN
 ├── State Machine
 └── Diagnostic

DCDC
 ├── Fast Control
 ├── Voltage Loop
 ├── Current Loop
 ├── PWM
 └── Protection

Safety
 ├── ADC
 ├── Temperature
 ├── Fault Manager
 └── WDG Supervisor
```

例如：

```
Wdg_NotifyDcdc();
Wdg_NotifyPdu();
Wdg_NotifyCan();
Wdg_NotifySafety();
```


# 五十二、还要增加 Timing Monitor

例如 DCDC 控制任务要求：

```
Ts = 100 us
```

实际：

```
100 us
100 us
100 us
350 us   ← fault
100 us
```

仅 Alive Counter 可能检测不到。

所以需要：

```
Alive Monitor
+
Deadline Monitor
```

例如：

```
if(controlTime > DCDC_MAX_EXECUTION_TIME)
{
    Wdg_Fault |= WDG_FAULT_DEADLINE;
}
```


# 五十三、最终喂狗逻辑应该是

```
                  WDG Supervisor
                        │
          ┌─────────────┼──────────────┐
          │             │              │
        Alive         Deadline       Fault
          │             │              │
          └─────────────┼──────────────┘
                        │
                     Healthy?
                     /      \
                   YES       NO
                    │         │
                    ▼         ▼
                 Feed WD   Stop Feed
                    │         │
                    ▼         ▼
             External WD   Internal WD
                              │
                              ▼
                             NMI
                              │
                              ▼
                            RESET
```

这才是比较完整的汽车安全软件设计。


# 五十四、调试阶段最重要的测试矩阵

不要只测试：不喂狗 → 能不能复位。

至少做下面这些：

| 测试              | 内狗          | 外狗         | 预期            |
| ----------------- | ------------- | ------------ | --------------- |
| 正常运行          | OK            | OK           | 正常            |
| Application死循环 | Timeout       | Timeout      | Reset           |
| Supervisor死循环  | Timeout       | Timeout      | Reset           |
| CPU中断关闭       | Timeout       | Timeout      | Reset           |
| Timer停止         | Timeout       | Timeout      | Reset           |
| WDG提前喂         | Window fault  | —            | Reset           |
| WDG晚喂           | Timeout       | Timeout      | Reset           |
| NMI Handler死循环 | NMIWD/SMU     | 外狗         | Reset           |
| 外狗WDI断线       | OK            | Timeout      | Reset           |
| 内狗故意停止      | Timeout       | OK           | Reset           |
| FSP异常           | SMU           | TLF reaction | Safe state      |
| MCU时钟异常       | Clock monitor | External     | Reset           |
| SPI异常           | —             | TLF fault    | Safety reaction |
| XINT异常          | —             | nRESET仍有效 | Reset           |


# 五十五、最值得做的 10 个 Fault Injection

### 1. 主循环死循环

```
while(1)
{
}
```

预期：

```
Internal WD
+
External WD
```


### 2. Supervisor 死循环

```
while(1)
{
}
```

这个特别重要。

如果：

```
Application正常
Supervisor死掉
```

外狗必须最终复位。


### 3. WDG 提前喂

TC377 + TLF35584：

```
Close Window
 ↓
Feed
 ↓
Fault
```

验证 Window WD。


### 4. WDG 不喂

```
Feed disabled
```

验证 Timeout。


### 5. NMI Handler 死循环

```
void NMI_Handler(void)
{
    while(1);
}
```

验证：

```
NMI
 ↓
NMIWD / recovery
 ↓
RESET
```


### 6. WDI GPIO 卡死

例如：

```
GPIO_Write(WDI, 1);
```

永远不产生下降沿。

SGM820：

```
Timeout
 ↓
nWDO
 ↓
nRESET
```


### 7. 外部 WDG 断线

直接断：

```
F29 GPIO
     X
SGM820 WDI
```

验证外狗独立性。


### 8. 内狗故障

故意不 service：

```
Internal WWD
 ↓
ESM
 ↓
NMI
```

而 SGM820 继续正常。


### 9. ESM Flag 不清

这是 F29 很重要的测试：

```
NMI
 ↓
不 Clear RAW
```

应该最终：

```
NMIWD
 ↓
XRSn
```


### 10. EOI 不执行

```
ESM_ClearRawStatus();
```

但是：

```
ESM_WriteEOI();
```

不执行。

验证是否再次进入 NMI。


# 五十六、调试时一定要保存 Reset Cause

否则现场出现：

```
ECU重启
```

根本不知道：

```
POR？
WDG？
ESM？
External WD？
Software reset？
Voltage fault？
```

因此启动第一时间：

```
ResetReason_t reason;

reason = ReadResetReason();

SaveResetReason(reason);
```

然后：

```
ResetReason
    │
    ├── Internal WDG
    ├── External WDG
    ├── ESM NMIWD
    ├── Power Reset
    ├── Software Reset
    └── External Reset
```

F29 的 ESM/EAM 状态在 XRSn/CPU reset 后不会像普通易失状态一样简单消失，相关错误状态可用于 reset 后诊断；TI 官方也特别强调了这一点。


# 五十七、调试时建议加一个 RAM Fault Record

例如：

```
typedef struct
{
    uint32_t magic;

    uint32_t resetCause;

    uint32_t wdgFault;

    uint32_t esmStatus;

    uint32_t smuStatus;

    uint32_t tlfStatus;

    uint32_t taskAlive;

    uint32_t cpuPc;

    uint32_t cpuSp;

} WdgFaultRecord;
```

然后：

```
volatile WdgFaultRecord g_faultRecord;
```

NMI 中只做：

```
g_faultRecord.magic = 0x57444746;
g_faultRecord.resetCause = ReadResetCause();
g_faultRecord.smuStatus = ReadSMU();
```


# 五十八、特别提醒：不要在 NMI 里做 Flash 写

错误：

```
NMI()
{
    Flash_Write(...);
}
```

因为 Flash 写可能：

```
几十/几百 us
甚至更长
```

而且可能依赖：

```
interrupt
clock
bus
```

正确做法：

```
NMI
 ↓
RAM记录
 ↓
Reset
 ↓
Startup
 ↓
检测 RAM记录
 ↓
Flash保存
```


# 五十九、可以把整个安全链路记成一句话

### TC377

**WDG发现时间异常 → SMU统一收口 → NMI负责软件诊断 → FSP负责对外报告 → TLF35584负责独立安全反应。**

### F29

**WWD发现时间异常 → ESM统一收口 → NMI负责软件诊断 → NMIWD防止NMI Handler失控 → SGM820提供独立外部超时复位。**


# 六十、最终推荐的软件目录

车载项目中，建议最终不要把这些代码全部塞进一个 `wdg.c`。

建议：

```
Safety/
│
├── Wdg/
│   ├── Wdg.h
│   ├── Wdg.c
│   │
│   ├── Wdg_Internal.h
│   ├── Wdg_Internal.c
│   │
│   ├── Wdg_External.h
│   ├── Wdg_External.c
│   │
│   ├── Wdg_Supervisor.h
│   ├── Wdg_Supervisor.c
│   │
│   ├── Wdg_FaultRecord.h
│   └── Wdg_FaultRecord.c
│
├── SMU/
│   ├── Smu.h
│   └── Smu.c
│
├── ESM/
│   ├── Esm.h
│   └── Esm.c
│
└── SafetyTest/
    ├── Wdg_Test.h
    └── Wdg_Test.c
```


# 六十一、完整代码框架

下面是一套可以作为工程骨架直接落地的版本。

## `Wdg.h`

```c
#ifndef WDG_H
#define WDG_H

#include <stdint.h>

typedef enum
{
    WDG_STATE_INIT = 0,
    WDG_STATE_RUNNING,
    WDG_STATE_FAULT
} WdgState;

typedef enum
{
    WDG_FAULT_NONE       = 0x00000000UL,
    WDG_FAULT_APP        = 0x00000001UL,
    WDG_FAULT_DCDC       = 0x00000002UL,
    WDG_FAULT_CAN        = 0x00000004UL,
    WDG_FAULT_SAFETY     = 0x00000008UL,
    WDG_FAULT_DEADLINE   = 0x00000010UL,
    WDG_FAULT_INTERNAL   = 0x00000020UL,
    WDG_FAULT_EXTERNAL   = 0x00000040UL,
    WDG_FAULT_ESM        = 0x00000080UL,
    WDG_FAULT_SMU        = 0x00000100UL
} WdgFault;

typedef struct
{
    volatile uint32_t appAlive;
    volatile uint32_t dcdcAlive;
    volatile uint32_t canAlive;
    volatile uint32_t safetyAlive;

    uint32_t lastAppAlive;
    uint32_t lastDcdcAlive;
    uint32_t lastCanAlive;
    uint32_t lastSafetyAlive;

    volatile uint32_t fault;
    volatile WdgState state;

} WdgContext;

void Wdg_Init(void);
void Wdg_MainFunction(void);

void Wdg_NotifyApp(void);
void Wdg_NotifyDcdc(void);
void Wdg_NotifyCan(void);
void Wdg_NotifySafety(void);

uint32_t Wdg_IsHealthy(void);

#endif
```


## `Wdg.c`

```c
#include "Wdg.h"
#include "Wdg_Internal.h"
#include "Wdg_External.h"

static WdgContext g_wdg;

void Wdg_Init(void)
{
    g_wdg.appAlive    = 0U;
    g_wdg.dcdcAlive   = 0U;
    g_wdg.canAlive    = 0U;
    g_wdg.safetyAlive = 0U;

    g_wdg.lastAppAlive    = 0U;
    g_wdg.lastDcdcAlive   = 0U;
    g_wdg.lastCanAlive    = 0U;
    g_wdg.lastSafetyAlive = 0U;

    g_wdg.fault = WDG_FAULT_NONE;
    g_wdg.state = WDG_STATE_INIT;

    Wdg_Internal_Init();
    Wdg_External_Init();

    g_wdg.state = WDG_STATE_RUNNING;
}

void Wdg_MainFunction(void)
{
    uint32_t healthy = 1U;

    if(g_wdg.appAlive == g_wdg.lastAppAlive)
    {
        g_wdg.fault |= WDG_FAULT_APP;
        healthy = 0U;
    }

    if(g_wdg.dcdcAlive == g_wdg.lastDcdcAlive)
    {
        g_wdg.fault |= WDG_FAULT_DCDC;
        healthy = 0U;
    }

    if(g_wdg.canAlive == g_wdg.lastCanAlive)
    {
        g_wdg.fault |= WDG_FAULT_CAN;
        healthy = 0U;
    }

    if(g_wdg.safetyAlive == g_wdg.lastSafetyAlive)
    {
        g_wdg.fault |= WDG_FAULT_SAFETY;
        healthy = 0U;
    }

    g_wdg.lastAppAlive =
        g_wdg.appAlive;

    g_wdg.lastDcdcAlive =
        g_wdg.dcdcAlive;

    g_wdg.lastCanAlive =
        g_wdg.canAlive;

    g_wdg.lastSafetyAlive =
        g_wdg.safetyAlive;

    if(healthy)
    {
        Wdg_Internal_Service();
        Wdg_External_Service();
    }
}

void Wdg_NotifyApp(void)
{
    g_wdg.appAlive++;
}

void Wdg_NotifyDcdc(void)
{
    g_wdg.dcdcAlive++;
}

void Wdg_NotifyCan(void)
{
    g_wdg.canAlive++;
}

void Wdg_NotifySafety(void)
{
    g_wdg.safetyAlive++;
}

uint32_t Wdg_IsHealthy(void)
{
    return (g_wdg.fault == WDG_FAULT_NONE);
}
```


## `Wdg_Internal.h`

```c
#ifndef WDG_INTERNAL_H
#define WDG_INTERNAL_H

void Wdg_Internal_Init(void);
void Wdg_Internal_Service(void);

#endif
```


## `Wdg_Internal.c`

```c
#include "Wdg_Internal.h"

void Wdg_Internal_Init(void)
{
    /*
     * Configure MCU internal WDG.
     *
     * TC377:
     *   CPU WDG / Safety WDG
     *
     * F29:
     *   Internal WWD
     *
     * Exact register/API configuration should be
     * implemented in the MCU-specific layer.
     */
}

void Wdg_Internal_Service(void)
{
    /*
     * Internal WDG service.
     *
     * Do not service this watchdog from an
     * independent timer ISR.
     *
     * Service only after application supervision
     * has passed.
     */
}
```


## `Wdg_External.h`

```c
#ifndef WDG_EXTERNAL_H
#define WDG_EXTERNAL_H

void Wdg_External_Init(void);
void Wdg_External_Service(void);

#endif
```


## `Wdg_External.c`

```c
#include "Wdg_External.h"

void Wdg_External_Init(void)
{
    /*
     * TC377 + TLF35584:
     *
     * 1. QSPI initialization
     * 2. TLF35584 SPI configuration
     * 3. WWD configuration
     * 4. FWD configuration
     * 5. ERR/FSP configuration
     * 6. First watchdog service
     *
     *
     * F29 + SGM820:
     *
     * 1. Configure WDI GPIO
     * 2. Configure nWDO XINT
     * 3. Connect nRESET to XRSn
     */
}

void Wdg_External_Service(void)
{
    /*
     * External watchdog service.
     *
     * TLF35584:
     *   WWD/FWD service
     *
     * SGM820:
     *   WDI falling edge
     */
}
```


# 六十二、特别强调一个问题

上面这套“完整代码文件”是：**软件架构完整 + 接口完整 + 故障处理完整的工程模板**

不是可以直接在具体板子上编译通过的最终量产代码。

原因是没有：

### TC377

- TC377 的具体 Variant
- iLLD 版本
- TASKING 还是 HighTec
- QSPI 使用哪个 Channel
- TLF35584 具体 Variant
- TLF35584 的 WWDCFG/FWDCFG 参数
- Safety concept 中 SMU Alarm Mapping
- FSP 使用 FSP0 还是 FSP1

### F29

- F29P32x 的具体型号
- CPU1/CPU2/CPU3 如何使用
- TI C2000Ware/F29 SDK 版本
- SysConfig 配置
- WWD 的目标窗口
- 哪个 GPIO 连接 SGM820 WDI
- 哪个 GPIO/XINT 连接 nWDO
- SGM820A 还是 SGM820B
- CWD 电容值

这些参数会直接决定寄存器和 Driver API。

尤其是 **TLF35584 的 WWD/FWD SPI command 和 F29 ESM/WWD 寄存器**，不建议为了“看起来完整”而硬写一堆未经项目 Variant 验证的 magic number。


# 六十三、下一步真正值得做的

在**TC377 + TLF35584 原理图**和**F29P32x + SGM820 原理图**基础上，进一步落成方案。


下面这种是**真正可以拿去做软件设计评审/安全设计评审**的结构：

```
                 ┌─────────────────────┐
                 │       TC377         │
                 │                     │
Application ────►│ CPU0                │
                 │   │                 │
                 │   ▼                 │
                 │ CPU WDG             │
                 │   │                 │
                 │   ▼                 │
                 │  SMU                │
                 │  │││                │
                 │  ││└── NMI ─────┐   │
                 │  │└── FSP ───┐  │   │
                 │  └── Reset   │  │   │
                 └──────────────┼──┼───┘
                                │  │
                        ┌───────▼──▼──────┐
                        │    TLF35584     │
                        │                 │
                        │ ERR ◄── FSP     │
                        │ WDI ◄── GPIO    │
                        │ SPI ◄── QSPI    │
                        │ INT ──► ESR1    │
                        │ ROT ──► PORST   │
                        │                 │
                        │ FWD + WWD       │
                        └─────────────────┘
```

以及：

```
                 ┌──────────────────────┐
                 │       F29P32x         │
                 │                      │
Application ────►│ CPU1                 │
                 │  │                   │
                 │  ▼                   │
                 │ Internal WWD         │
                 │  │                   │
                 │  ▼                   │
                 │ ESM                  │
                 │  │                   │
                 │ NMI                  │
                 │  │                   │
                 │ NMIWD                │
                 │  │                   │
                 │ XRSn ◄──────────┐    │
                 │                  │    │
                 │ XINT ◄──────┐    │    │
                 └─────────────┼────┼────┘
                               │    │
                         ┌─────▼────▼─────┐
                         │    SGM820      │
                         │                │
                         │ WDI ◄── GPIO   │
                         │ nWDO ──► XINT  │
                         │ nRESET ► XRSn  │
                         └────────────────┘
```

然后可以继续把**“CPU内核执行流”**再往下拆一层：

```
Application
     ↓
Scheduler
     ↓
Safety Supervisor
     ↓
Alive / Deadline
     ↓
WDG Service
     ↓
WDG HW
     ↓
ESM / SMU
     ↓
NMI Vector
     ↓
NMI Context Save
     ↓
Fault Record
     ↓
PWM Safe State
     ↓
EOI / FSP
     ↓
Reset
     ↓
Boot
     ↓
Reset Cause
     ↓
Fault Record Recovery
```

这部分才是把 **“WDG驱动”提升成真正汽车功能安全软件**的关键。

另外，TI 的 F29 文档目前明确强调 ESM/NMIWD 的处理方式，而 Infineon 对 TC3xx + TLF35584 也明确给出了 FSP、WWD/FWD、ESR1 和 reset 的安全集成路径；因此上面的两套架构并不是简单类比，而是分别贴近两个厂商推荐的安全机制。