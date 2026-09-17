# PORT

这三个 MCU 的思路其实差异很大：

- **TI F29P32**：C2000/C29 架构，GPIO + PinMux + 外设复用，另外有明确的 **XRSn、Boot Mode、JTAG** 等启动相关引脚。
- **SPC58NN**：Power Architecture，核心是 **SIUL2** 管理 Pad，GPIO、复用、输入路径、上下拉、SAFE 状态都由 SIUL2 管理；同时有 **PORST、JTAG、Nexus、WKPU、BAF Boot** 等体系。
- **TC377**：AURIX，Port 是 Pxx.x 形式，除了普通 GPIO/ALT 功能外，有非常重要的 **HWCFG[1:6]、PORST、ESR0/1、EVR/VGATE、DAP/JTAG** 等系统级引脚。

尤其是 **TC377 的 HWCFG**，不能简单按照普通 GPIO 看；而 **F29P32 的 Boot Mode GPIO** 和 **SPC58NN 的 BAF/启动相关引脚**也需要在 PCB 级别处理。

下面按照 OBC/DCDC 软件的实际方式来梳理。

------

## 1. 统一的 Port 分类框架

对于这三个 MCU，建议以后做项目时统一把 MCU 引脚分成下面 **8 类**：

| 类别            | 典型用途                      | 是否需要外部电路             | Boot期间             | App期间             |
| --------------- | ----------------------------- | ---------------------------- | -------------------- | ------------------- |
| ① 普通 GPIO     | 输入/输出、继电器、使能、状态 | 通常不需要                   | 默认状态决定         | 软件配置            |
| ② PWM           | PFC/DCDC/LLC/风扇/泵等        | 通常需要驱动器/栅极电阻等    | **必须防误发波**     | PWM外设接管         |
| ③ 外设复用      | CAN、SPI、UART、LIN、I2C      | 通常需要收发器/上拉等        | 取决于默认状态       | PinMux              |
| ④ ADC/模拟      | 电流、电压、温度              | **通常需要模拟前端**         | 不应随意配置数字输出 | ADC                 |
| ⑤ Boot/启动配置 | Boot Mode、HWCFG、BAF         | **强烈建议外部上下拉/strap** | **极关键**           | 有些可作为 GPIO     |
| ⑥ Reset         | PORST/XRSn/ESR                | **必须按参考设计**           | **极关键**           | 仍然可能参与复位    |
| ⑦ Debug         | JTAG/DAP/Nexus                | 调试接口                     | 极关键               | Debug期间占用       |
| ⑧ Power/Safety  | 电源管理、唤醒、安全、看门狗  | **通常需要外部电路**         | **极关键**           | 与下电/故障密切相关 |

这个分类比单纯的GPIO / PWM / CAN / SPI，更适合汽车 MCU。

因为真正做 PCB 时，最容易出问题的往往不是普通 GPIO，而是：**这个脚在 MCU 还没执行第一条 App 代码之前是什么状态？**

------

## 2. “复用”到底是什么

以一个 MCU Pin 为例：

```text
                 ┌── GPIO
                 │
MCU PAD ─────────┼── PWM
                 │
                 ├── CAN_TX
                 │
                 ├── SPI
                 │
                 └── UART
```

它实际上只有一个物理 Pad。

所以：

```text
GPIO0
EPWM1_A
CAN_TX
SPI_SOUT
UART_TX
```

并不是 5 个物理引脚。

而是：**同一个物理引脚的多个 Alternate Function。**

软件通过 PinMux/Port configuration 决定：

```text
PAD
 ↓
MUX
 ↓
GPIO / PWM / CAN / SPI / UART
```

因此以后看 MCU 数据手册时，建议不要问：

> “这个脚是不是 PWM？”

而应该问：

> **“这个 Pad 支持哪些 ALT Function，以及复位/Boot状态下是什么功能？”**

------

## 3. TI F29P32：Port/GPIO 体系

以 **F29P329SM-Q1** 为例。

它有：

- 1 个 C29 CPU
- 200 MHz
- 105 GPIO
- 18 PWM channels
- 4 CAN-FD
- 5 SPI
- 2 UART
- 4 QEP
- ADC 等

([Texas Instruments](https://www.ti.com/product/F29P329SM-Q1?utm_source=chatgpt.com))

所以它非常适合：OBC PFC / DCDC / 功率控制 DSP 架构。

------

## 4. F29P32 的 GPIO / PWM / 复用关系

可以理解为：

```text
                 F29 GPIO
                    │
       ┌────────────┼────────────┐
       │            │            │
      GPIO         PWM          CAN
       │            │            │
       │           ePWM        CAN-FD
       │
       ├── SPI
       ├── UART
       ├── LIN
       ├── I2C
       ├── ADC
       ├── QEP
       └── 其他外设
```

在 DCDC 项目使用：

```text
GPIO0 → EPWM1_A
GPIO1 → EPWM1_B

GPIO2 → EPWM2_A
GPIO3 → EPWM2_B

GPIO4 → EPWM3_A
GPIO5 → EPWM3_B

GPIO8 → EPWM5_A
GPIO9 → EPWM5_B
```

本质就是：

```text
GPIOx
 ↓
PinMux
 ↓
ePWMx_A/B
 ↓
PWM peripheral
 ↓
Gate Driver
 ↓
MOSFET
```

所以这里的 GPIO0~GPIO9 **并不是普通 GPIO 输出**，而是在 App 初始化后交给 ePWM 外设控制。

------

## 5. F29P32 特别重要：XRSn

F29P32 最重要的系统级引脚之一：**XRSn**

可以把它理解成：**MCU 的外部复位入口/复位状态引脚。**

TI 文档明确说明，XRSn 可以由外部 supervisor 或外部上拉网络参与控制，同时 MCU 内部电源监控也会控制该引脚。([Texas Instruments](https://www.ti.com/lit/ds/symlink/f29h850tu.pdf?ts=1776564953144&utm_source=chatgpt.com))

典型硬件：

```text
                 VDDIO
                   │
                  4.7k
                   │
                   ├──────── XRSn
                   │
              Supervisor
                   │
                   └─────── MCU Reset
```

对于汽车电源控制 MCU：

```text
12V
 ↓
PMIC
 ↓
3.3V
 ↓
MCU
 ↓
XRSn
```

所以 XRSn 属于：**不能当普通 GPIO 设计的系统级引脚。**

------

## 6. F29P32 的 Boot Mode 特别重要

F29P32 使用 Boot ROM。

Boot ROM 在启动过程中会采样 Boot Mode。

目前 F29P32/F29P 系列的数据手册给出了：

```text
Boot mode pin 0 → GPIO84
Boot mode pin 1 → GPIO72
```

并且 Boot Mode Pin 应避免选择 PWM、模拟、USB、JTAG、晶振等敏感功能。([Texas Instruments](https://www.ti.com/lit/ds/symlink/f29h859tu-q1.pdf?utm_source=chatgpt.com))

也就是说：

```text
Power On
   ↓
XRSn释放
   ↓
Boot ROM
   ↓
读取 Boot Mode
   ↓
决定
 ┌───────┬─────────┐
 │       │         │
Flash   UART      Parallel
Boot    Boot       Boot
```

这是一个非常关键的概念：**GPIO72/GPIO84 在 Boot 阶段可能不是 App 所认为的普通 GPIO。**

因此如果把：

```text
GPIO72 → 外部 EN
GPIO84 → 外部 Power Control
```

就必须检查外部电路是否会改变 Boot Mode。

------

## 7. F29P32 的 Boot 时序

TI 给出的启动时序非常值得注意：

```text
Power
  │
  ▼
XRSn Low
  │
  ▼
Boot ROM
  │
  ├── Boot Mode Sampling
  │
  ▼
Peripheral/GPIO configuration
  │
  ▼
User Application
```

在 Boot ROM 阶段，普通 GPIO 的应用级配置还没有完全建立；Boot Mode Pin 会先被采样。([Texas Instruments](https://www.ti.com/lit/ds/symlink/f29h850tu.pdf?ts=1776564953144&utm_source=chatgpt.com))

所以对于 F29P32，建议：

PCB 级：

```text
Boot Pin
   │
  10k
   │
 VDDIO/GND
```

不要依赖：软件内部 Pull-up 来决定 Boot Mode。

因为：**软件还没有运行的时候，软件 Pull-up 根本还没开始配置。**

------

## 8. F29P32 的内部上下拉

F29P32 数据手册说明：

- GPIO Pull-up 默认关闭；
- 可以由软件打开；
- XRSn/TCK/TMS 等特殊引脚有自己的复位状态；
- 未 Bond-out 的 GPIO，Boot ROM 会启用内部 Pull-up。([Texas Instruments](https://www.ti.com/lit/ds/symlink/f29h850tu.pdf?ref_url=https%3A%2F%2Fwww.ti.com%2Fproduct%2Fko-kr%2FF29H850TU&ts=1782249865850&utm_source=chatgpt.com))

因此可以把 F29P32 分成：

```text
Reset
 ├── XRSn → 专用复位
 ├── TCK  → Debug
 ├── TMS  → Debug
 └── GPIO → 默认状态

Boot
 ├── Boot Mode GPIO
 ├── Boot peripheral GPIO
 └── 其他 GPIO

App
 └── 用户 PinMux / GPIO / PWM
```

------

## 9. SPC58NN：SIUL2

SPC58NN 的 Port 核心是：**SIUL2**

ST 的 RM0421 明确说明：SIUL2 控制 MCU Pad configuration、GPIO、外部中断以及触发配置。([STMicroelectronics](https://www.st.com/resource/en/reference_manual/rm0421-.pdf?utm_source=chatgpt.com))

可以理解成：

```text
                  SIUL2
                    │
        ┌───────────┼────────────┐
        │           │            │
       GPIO        ALT          EXTI
        │           │
        │      ┌────┼────┐
        │     CAN  SPI  PWM
        │
        └── Input/Output
```

所以 SPC58NN 做 Port Driver 时，会经常看到：

```text
MSCR
IMCR
GPDO
GPDI
```

这一套。

------

## 10. SPC58NN 的 MSCR / IMCR

这是理解 SPC58NN Port 的关键。

### MSCR

主要理解成：**这个 Pad 自己怎么工作。**

例如：

```text
Output enable
Drive strength
Slew rate
Pull-up / Pull-down
Alternate function
Safe mode behavior
```

------

### IMCR

主要理解成：**某个外设输入信号从哪个 Pad 进来。**

例如：

```text
CAN_RX
    ↑
   IMCR
    ↑
  Pad A / Pad B
```

所以 SPC58NN 很典型的配置是：

```text
PAD
 │
 ├── MSCR
 │     ├── Output
 │     ├── Drive
 │     ├── Pull
 │     └── ALT
 │
 └── IMCR
       └── Input routing
```

------

## 11. SPC58NN 的 GPIO 分类

可以把 SPC58NN 的 Pad 分成：

### A. 普通 GPIO

```text
GPIO input
GPIO output
```

例如：

```text
Relay_EN
Wake_IN
Fault_IN
LED
Power_EN
```

------

### B. 外设输出

```text
CAN_TX
SPI_SOUT
LIN_TX
PWM
Ethernet
```

------

### C. 外设输入

```text
CAN_RX
SPI_SIN
LIN_RX
External Interrupt
Timer Capture
```

------

### D. ADC

```text
Analog Input
```

这些不要简单按照 GPIO 使用。

------

## 12. SPC58NN 的一个非常重要特性：Reset 后大部分 Pad 是 Hi-Z

ST 的硬件设计指南明确说明：

> 为避免 Reset 状态下误激活外部器件，所有 Pad 默认强制为高阻输入，只有 JTAG 等特殊 Pad 除外。([STMicroelectronics](https://www.st.com/resource/en/application_note/an4880-spc58xx-hardware-design-guideline-stmicroelectronics.pdf?utm_source=chatgpt.com))

例如：

```text
Reset

GPIO
 ↓
Hi-Z
```

这对于汽车控制非常重要。

假设：

```text
MCU GPIO
   │
   └── Gate Driver EN
```

如果没有外部上下拉：

```text
MCU Reset
   ↓
GPIO Hi-Z
   ↓
EN Floating
   ↓
Gate Driver 状态不确定
```

因此：

> **任何影响功率器件开关的 GPIO，都不能只依赖 MCU 内部 Pull。**

应该：

```text
MCU GPIO
   │
   ├──── Gate Driver EN
   │
   └──── 10k Pull-down
```

这样：

```text
Reset → EN = 0
App初始化 → EN = 1
```

------

## 13. SPC58NN 的 SAFE Mode 要特别注意

SPC58NN 的 SIUL2 还有：

```text
SAFE MODE
```

某些配置下，进入 SAFE Mode 后：

```text
Output Driver
      ↓
Disabled
```

同时：

```text
Internal weak Pull-up
      ↓
Enabled
```

ST RM0421 对这一行为有明确描述。([STMicroelectronics](https://www.st.com/resource/en/reference_manual/rm0421-.pdf?utm_source=chatgpt.com))

这对于：

```text
OBC
DCDC
PFC
继电器
接触器
Gate Driver
```

非常重要。

因为：

```text
正常运行：
GPIO = 0

SAFE：
GPIO可能不再由正常Output Driver驱动
```

所以安全设计不能只看：

```text
App GPIO = 0
```

而要看：

```text
Reset
Boot
DRUN
SAFE
STANDBY
Shutdown
```

整个生命周期。

------

## 14. SPC58NN 的 PORST

SPC58NN 有：

```text
PORST
ESR0
```

属于专用 Reset 系统。

ST 数据手册明确指出 PORST 是双向 Reset Pad，并建议外部上拉，典型推荐值为 **4.7 kΩ**。([STMicroelectronics](https://www.st.com/resource/en/datasheet/spc58nn84c3.pdf?utm_source=chatgpt.com))

典型：

```text
                 VDD
                  │
                 4.7k
                  │
                  ├──── PORST
                  │
             MCU / Supervisor
```

因此：**PORST 是 PCB 级别必须认真设计的引脚。**

------

## 15. SPC58NN 的 Boot

SPC58NN 和 F29P32 的 Boot 思路不完全一样。

SPC58NN 有：**Boot Assist Flash，BAF**

可以通过异步：

```text
CAN
LIN/UART
```

进行工厂编程/Boot。([STMicroelectronics](https://www.st.com/content/st_com/en/products/automotive-microcontrollers/spc5-32-bit-automotive-mcus/spc5-performance-mcus/spc58-n-line-mcus/spc58nn84c3.html?utm_source=chatgpt.com))

所以在 SPC58NN 项目中需要重点考虑：

```text
PORST
  ↓
Startup
  ↓
BAF / Boot
  ↓
Flash Application
```

尤其要保证：CAN/LIN Boot 所需要的 Pad、收发器和外部电路在启动期间不会被错误拉死。

------

## 16. TC377：Port 架构

TC377 是三者里面最特殊的。

基本 Port 形式：

```text
P00.x
P10.x
P14.x
P15.x
P20.x
P21.x
P32.x
P33.x
...
```

每一个 Pin 可以存在：

```text
GPIO
ALT1
ALT2
ALT3
...
```

比如：

```text
P14.0
P14.1
```

除了普通 Port 功能，还可能承担：

```text
Generic Bootstrap
CAN
ASC
Wake
```

等功能。

------

## 17. TC377 最大的特殊点：HWCFG

这是特别值得单独拿出来的。

TC377 有：

```text
HWCFG[1]
HWCFG[2]
HWCFG[3]
HWCFG[4]
HWCFG[5]
HWCFG[6]
```

对应：

| HWCFG  | Pin   | 主要作用             |
| ------ | ----- | -------------------- |
| HWCFG1 | P14.5 | EVR33 配置           |
| HWCFG2 | P14.2 | EVR Core 配置        |
| HWCFG3 | P14.3 | Boot来源             |
| HWCFG4 | P10.5 | Boot Mode            |
| HWCFG5 | P10.6 | Boot Mode            |
| HWCFG6 | P14.4 | Reset后 Pad 默认状态 |

Infineon 官方启动文档对此有明确说明。([Infineon Documentation](https://documentation.infineon.com/aurixtc3xx/docs/nyb1710229964455?utm_source=chatgpt.com))

------

## 18. TC377 HWCFG1/2：电源相关

这是做汽车电源 MCU 时必须特别关注的。

```text
P14.5 = HWCFG1
P14.2 = HWCFG2
```

主要影响：

```text
EVR33
EVRC
```

相关电源拓扑。

所以：

```text
P14.2
P14.5
```

不是普通 GPIO 的第一优先级。

它们首先属于：**Power Configuration Pin**

------

## 19. TC377 HWCFG3/4/5：Boot

最关键的是：

```text
P14.3 = HWCFG3
P10.5 = HWCFG4
P10.6 = HWCFG5
```

其中 HWCFG3 决定：

```text
Boot configuration
```

可以选择：

```text
HWCFG pins
      或
Flash BMI
```

如果使用 Flash BMI，则 HWCFG4/5 在相应条件下可以不参与 Boot Mode 选择。([Infineon Documentation](https://documentation.infineon.com/aurixtc3xx/docs/nyb1710229964455?utm_source=chatgpt.com))

------

## 20. TC377 Generic Bootstrap

TC377 一个非常有意思的设计是：

```text
P14.0
P14.1
```

可以参与：Generic Bootstrap

并可以检测：

```text
CAN
ASC
```

等 Boot Loader 路径。

Infineon 官方启动文档也明确提到 Generic Bootstrap 可以通过 P14.0/P14.1 进行 CAN/ASC Bootloader 检测。([Infineon Documentation](https://documentation.infineon.com/aurixtc3xx/docs/nyb1710229964455?utm_source=chatgpt.com))

所以：

```text
P14.0/P14.1
```

在调试/量产/Boot 环境非常重要。

------

## 21. TC377 HWCFG6：这是“悬空/上下拉”最典型的例子

```text
P14.4 = HWCFG6
```

它决定：

```text
Reset期间/之后
所有普通 Pad
```

默认是：

```text
Tri-State
```

还是：

```text
Input + Pull-up
```

Infineon 文档明确说明：

> HWCFG6 决定 Port pins 默认是 tri-state，还是 input + pull-up。([Infineon Documentation](https://documentation.infineon.com/aurixtc3xx/docs/nyb1710229964455?utm_source=chatgpt.com))

因此：

```text
HWCFG6 = 0
      ↓
GPIO 默认 Hi-Z

HWCFG6 = 1
      ↓
GPIO 默认 Input + Pull-up
```

这个设计对于汽车 MCU 非常关键。

------

## 22. TC377 HWCFG 并不是一直重新采样

这个特别容易搞错。

Infineon 对 TC377 的说明是：

```text
HWCFG[1,2,3,6]
```

在： **initial supply ramp-up** 时锁存。

它们不会在：

```text
warm PORST
system reset
application reset
```

时重新按照普通意义重新采样。([Infineon Community](https://community.infineon.com/t5/AURIX/TC377-Pins-configuration-confusion/td-p/879208?utm_source=chatgpt.com))

而 HWCFG4/5 的行为和 Boot 配置有关。

这意味着：

```text
Cold Power On
     ↓
HWCFG采样
     ↓
锁存
     ↓
运行
     ↓
Software Reset
     ↓
HWCFG1/2/3/6不会简单重新改变
```

这也是为什么：**TC377 的 HWCFG Pin PCB 上一定要有确定电平。**不能悬空碰运气。

------

## 23. TC377 的 P32.0/P32.1 也要特别注意

做 TC377 项目时如果碰到：

```text
P32.0
P32.1
```

要注意：

```text
VGATE1N
VGATE1P
```

这些 Pin 和 MCU 内部电源/EVR 体系存在关系。

Infineon 对 TC377 的一个实际答复中明确指出，如果外部提供 Core 1.3 V，则可以在满足 HWCFG2 条件后将 P32.0/P32.1 用作普通 GPIO。([Infineon Community](https://community.infineon.com/t5/AURIX/Aurix-TC377TP-Port-P32-0-amp-P32-1-configuration/td-p/1070124?utm_source=chatgpt.com))

所以这种 Pin：**必须先判断芯片电源架构，再决定是否当 GPIO 使用。**

不能只看 Datasheet Pinmux 表。

------

## 24. 三颗 MCU 的“特殊 Pin”对比

建议以后直接放到项目 MCU Design Guide 里的表。

| 功能           | F29P32            | SPC58NN        | TC377           |
| -------------- | ----------------- | -------------- | --------------- |
| 普通 GPIO      | GPIOx             | SIUL2 GPIO     | Pxx.x           |
| PWM            | ePWM              | 复用外设/Timer | CCU6/GTMA/其他  |
| CAN            | CAN-FD            | MCAN/M-TTCAN   | MultiCAN+       |
| SPI            | SPI               | DSPI           | QSPI/SPI        |
| UART           | UART              | LINFlexD       | ASC             |
| ADC            | ADC               | SAR/SD ADC     | VADC            |
| Reset          | **XRSn**          | **PORST**      | **PORST**       |
| Debug          | JTAG              | JTAG/Nexus     | DAP/JTAG        |
| Boot           | Boot Mode GPIO    | BAF            | HWCFG/BMI       |
| Wake           | GPIO/外设机制     | WKPU等         | PMS/Wakeup      |
| 电源配置       | 电源相关专用 Pin  | 电源/SMPS      | **HWCFG1/2**    |
| Reset后Pad配置 | GPIO默认状态      | Hi-Z等         | **HWCFG6决定**  |
| 安全状态       | Safety/Secure机制 | **SAFE**       | Safety/Safe/SMU |

------

## 25. 三颗 MCU 的“必须外部电路”的引脚

必须 PCB 级确定这些不要依赖软件。

#### F29P32

```text
XRSn
Boot Mode Pins
JTAG/DAP
Power Pins
晶振/时钟相关
```

其中 Boot Mode 必须保证上电采样期间稳定。([Texas Instruments](https://www.ti.com/lit/ds/symlink/f29h859tu-q1.pdf?utm_source=chatgpt.com))

------

#### SPC58NN

```text
PORST
Power
JTAG/Nexus
Boot/BAF相关通信
需要保持安全状态的 EN
```

PORST 推荐外部上拉，ST 给出的典型建议为 4.7 kΩ。([STMicroelectronics](https://www.st.com/resource/en/datasheet/spc58nn84c3.pdf?utm_source=chatgpt.com))

------

#### TC377

尤其：

```text
HWCFG1
HWCFG2
HWCFG3
HWCFG4
HWCFG5
HWCFG6

PORST
ESR0
ESR1

P14.0/P14.1
JTAG/DAP
```

其中 HWCFG 必须在规定启动时序内得到确定电平。([Infineon Documentation](https://documentation.infineon.com/aurixtc3xx/docs/nyb1710229964455?utm_source=chatgpt.com))

------

## 26. 哪些 GPIO 最好增加外部上下拉？

这个可以给一个非常实用的设计规则。

### 第一类：功率控制

例如：

```text
PWM_EN
Gate_EN
PFC_EN
DCDC_EN
SR_EN
Relay_EN
Contactor_EN
```

建议：

```text
GPIO
 │
 ├── 外部 Pull-down
 │
 └── Driver
```

目标：

```text
RESET = OFF
BOOT = OFF
APP初始化前 = OFF
APP正常 = ON
FAULT = OFF
```

------

### 第二类：状态输入

例如：

```text
Fault
PG
Power Good
OCP
OVP
OTP
Interlock
Door
Wake
```

如果外部器件可能：

```text
Hi-Z
Open Drain
```

那么 MCU 输入侧必须有确定的：

```text
Pull-up
```

或者：

```text
Pull-down
```

例如：

```text
Fault_OD
   │
   ├──── MCU GPIO
   │
  10k
   │
  3.3V
```

------

### 第三类：Boot Configuration

这个原则更严格：

> **Boot Configuration 不要依赖 MCU 内部 Pull。**

应该：

```text
VDD
 │
10k
 │
BOOT_PIN
 │
MCU
```

或者：

```text
BOOT_PIN
 │
10k
 │
GND
```

因为：

```text
Power ON
 ↓
MCU内部软件还没运行
 ↓
Boot Mode已经需要被确定
```

------

## 27. 推挽/开漏是什么？

这个在汽车 CAN、Fault、Reset、Wake 等场景非常常见。

推挽：

```text
       VDD
        │
       PMOS
        │
OUT ────┤
        │
       NMOS
        │
       GND
```

它可以：

```text
HIGH
LOW
```

主动驱动两个方向。


开漏相当于：

```text
       VDD
        │
     外部Pull-up
        │
        ├──── OUT
        │
       NMOS
        │
       GND
```

MCU：

```text
OUT = 0
```

时：

```text
NMOS ON
↓
LOW
```

MCU：

```text
OUT = 1
```

实际上是：

```text
NMOS OFF
↓
Hi-Z
↓
外部Pull-up
↓
HIGH
```

所以：**Open Drain 本质上只有“主动拉低”，高电平依赖外部上拉。**

------

## 28. 推挽 vs 开漏

| 特性         | Push-Pull      | Open-Drain                |
| ------------ | -------------- | ------------------------- |
| 输出 High    | 主动驱动       | 外部 Pull-up              |
| 输出 Low     | 主动驱动       | 主动拉低                  |
| 需要外部上拉 | 通常不需要     | 通常需要                  |
| 速度         | 高             | 受RC影响                  |
| 多器件共享   | 不适合直接并联 | 很适合                    |
| 常见汽车用途 | PWM/EN/LED     | Fault/Wake/Reset/共享信号 |
| CAN          | 不直接等价     | CAN物理层另有收发器       |

注意：**CAN MCU TX 引脚本身不要简单理解成“CAN Open Drain”。**

CAN 总线的差分开关由：

```text
MCU CAN Controller
        ↓
CAN Transceiver
        ↓
CANH/CANL
```

完成。

------

## 29. Boot 和 App 阶段到底有什么区别？

这是 Port 初始化最核心的问题。

建议把一个 GPIO 的生命周期理解成：

```text
                 MCU Power ON
                      │
                      ▼
               Reset Hardware
                      │
                      ▼
              Boot ROM / BAF
                      │
                      ▼
                Startup Code
                      │
                      ▼
              Port Initialization
                      │
                      ▼
                 Application
                      │
                      ▼
                  Fault/Safe
                      │
                      ▼
                   Shutdown
```

------

## 30. Reset 阶段：软件还没控制 GPIO

例如 DCDC：GPIO_GATE_EN

 App 想：GPIO_GATE_EN = 0

但是：MCU还在Reset

这句话其实没有意义。

因为：**App GPIO 配置代码还没有执行。**

所以：

```text
MCU GPIO
    │
    └── Gate Driver EN
```

如果要求：Reset → EN=0

那么必须靠：外部 Pull-down来保证。

------

## 31. Boot 阶段

Boot ROM 开始执行。

此时：

```text
GPIO
PWM
CAN
SPI
UART
```

还没有完全进入 App 配置。

所以：

```text
PWM_EN
DCDC_EN
PFC_EN
```

这类控制信号最好设计为：

```text
默认 OFF
```

例如：

```text
MCU GPIO
   │
  10k
   │
  GND
```

然后：

```text
App初始化完成
       ↓
确认故障清除
       ↓
确认ADC正常
       ↓
确认PWM配置完成
       ↓
GPIO_EN = 1
```

------

## 32. PWM 是最危险的一类 Port

做：

```text
F29P32
DCDC
100kHz
ePWM
```

这一点尤其重要。

不能简单：

```text
Init GPIO
↓
Mux to PWM
↓
PWM running
```

否则可能出现：

```text
PinMux切换瞬间
       ↓
产生错误电平
       ↓
Gate Driver识别
       ↓
MOS瞬时导通
```

正确思路应该是：

```text
1. PWM Disable
2. 配置 Time Base
3. 配置 Compare
4. 配置 Deadband
5. 配置 Trip Zone
6. 配置 Action Qualifier
7. 设置安全输出状态
8. PinMux
9. Clear Fault
10. Enable PWM
```

特别是：

```text
EPWM1_A
EPWM1_B
EPWM2_A
EPWM2_B
EPWM3_A
EPWM3_B
EPWM5_A
EPWM5_B
```

应该把：**PWM初始化** 和 **Gate Enable** 严格分开。

------

## 33. App 初始化阶段应该怎么做？

推荐以后所有三个 MCU 都按照：

```text
MCU_Init()
   │
   ├── Clock_Init()
   │
   ├── Safety_Init()
   │
   ├── Port_Safe_Init()
   │
   ├── ADC_Init()
   │
   ├── PWM_Init()
   │
   ├── CAN_Init()
   │
   ├── Communication_Init()
   │
   └── Application_Init()
```

其中： Port_Safe_Init() 应该优先。

------

## 34. 建议把 Port 分成 4 个软件初始化阶段

这非常适合现在 OBC 多芯片架构。

### Stage 0：Hardware default

MCU 完全没运行。

靠：

```text
外部 Pull
Hardware strap
Reset circuit
Gate Driver default
```

确保：安全

------

### Stage 1：Boot Port

```text
Boot ROM
Bootloader
```

只保证：

```text
Boot
Debug
Download
Recovery
```

------

### Stage 2：Safe Port

App刚开始：

```text
所有功率输出
      ↓
OFF
```

例如：

```text
PFC_EN = 0
DCDC_EN = 0
PWM_EN = 0
RELAY_EN = 0
```

------

### Stage 3：Application Port

系统确认：

```text
Clock OK
ADC OK
PWM OK
CAN OK
Watchdog OK
Safety OK
```

之后：

```text
DCDC_EN = 1
PFC_EN = 1
PWM_ENABLE = 1
```

------

## 35. 三颗 MCU 最应该关注的“启动/复位/下电”引脚

这是最终重点表。

| MCU     | 极高关注             | 关注原因            |
| ------- | -------------------- | ------------------- |
| F29P32  | **XRSn**             | MCU复位             |
| F29P32  | **GPIO72/GPIO84**    | Boot Mode           |
| F29P32  | JTAG TCK/TMS/TDI/TDO | Debug/启动          |
| F29P32  | Power/Clock          | 上电启动            |
| SPC58NN | **PORST**            | Power-On Reset      |
| SPC58NN | ESR0                 | Reset/Safety        |
| SPC58NN | JTAG/Nexus           | Debug/Boot          |
| SPC58NN | BAF相关 CAN/LIN      | Factory Boot        |
| SPC58NN | WKPU                 | Wake-up             |
| TC377   | **HWCFG1**           | 电源拓扑            |
| TC377   | **HWCFG2**           | Core/EVR            |
| TC377   | **HWCFG3**           | Boot来源            |
| TC377   | **HWCFG4/5**         | Boot Mode           |
| TC377   | **HWCFG6**           | Reset后GPIO默认状态 |
| TC377   | **PORST**            | Power-On Reset      |
| TC377   | **ESR0/ESR1**        | Reset/安全          |
| TC377   | P14.0/P14.1          | Generic Bootstrap   |
| TC377   | P32.0/P32.1          | 电源/EVR相关        |

TC377 的 HWCFG 组合尤其需要在 PCB 设计阶段确定。([Infineon Documentation](https://documentation.infineon.com/aurixtc3xx/docs/nyb1710229964455?utm_source=chatgpt.com))

------

## 36. 从“车载 OBC”角度重新分类

如果不按照 MCU 厂商分类，而按照实际做 OBC 的软件功能分类，反而建议建立下面这张表：

| OBC功能     | F29P32     | SPC58NN       | TC377      |
| ----------- | ---------- | ------------- | ---------- |
| PFC PWM     | ePWM       | Timer/PWM复用 | CCU6/GTM   |
| DCDC PWM    | ePWM       | Timer/PWM     | GTM        |
| SR PWM      | ePWM       | Timer/PWM     | GTM        |
| ADC电压     | ADC        | ADC           | VADC       |
| ADC电流     | ADC        | ADC           | VADC       |
| CAN         | CAN-FD     | MCAN          | MultiCAN   |
| SPI         | SPI        | DSPI          | QSPI/SPI   |
| UART        | UART       | LINFlexD      | ASC        |
| GPIO Enable | GPIO       | SIUL2 GPIO    | Pxx        |
| Fault Input | GPIO/EXTI  | SIUL2/IRQ     | Port/ERU   |
| Wake        | GPIO/系统  | WKPU          | PMS        |
| Reset       | XRSn       | PORST         | PORST      |
| Boot        | GPIO Boot  | BAF           | HWCFG/BMI  |
| Debug       | JTAG       | JTAG/Nexus    | DAP/JTAG   |
| Safety      | C29 Safety | FCCU/Safe     | SMU/Safety |

------

## 37. 最后给一个非常重要的工程判断原则

以后拿到任何一颗 MCU 的 Datasheet，不要先开始配置 GPIO。

先按照这个顺序：

```text
① Power
   ↓
② Reset
   ↓
③ Clock
   ↓
④ Boot
   ↓
⑤ Debug
   ↓
⑥ Safety
   ↓
⑦ Wake-up
   ↓
⑧ GPIO
   ↓
⑨ Peripheral Mux
   ↓
⑩ PWM/ADC/CAN/SPI
```

尤其对于 OBC：

```text
Power
 ↓
Reset
 ↓
Boot
 ↓
Port Safe State
 ↓
ADC
 ↓
PWM
 ↓
Gate Driver Enable
 ↓
Power Stage
```

这个顺序比“先配置 GPIO，再配置外设”重要得多。

------

## 38. 最值得建立的是一张“MCU Pin Matrix”

建议下一步直接把这三个 MCU 做成一张**项目级 Excel/Pin Matrix**，而不是继续停留在概念层。

可以按下面的字段建立：

```text
MCU
│
├── Pin No.
├── Pin Name
├── GPIO
├── ALT0
├── ALT1
├── ALT2
├── ALT3
├── PWM
├── ADC
├── CAN
├── SPI
├── UART/LIN
├── EXTI/Wakeup
├── Boot Function
├── Reset Function
├── Safety Function
├── Default Reset State
├── Default Pull
├── Boot State
├── App State
├── Open Drain/Push Pull
├── External Pull Required
├── External Circuit
├── Power Domain
├── OBC Function
├── PFC/DCDC/LLC/PDU
└── Risk Level
```

这样就可以直接得到三套：

**F29P329SM-Q1 Pin Matrix → SPC58NN Pin Matrix → TC377 Pin Matrix**

然后进一步做成：

```text
MCU
 │
 ├── Power/Reset/Boot
 │
 ├── Debug
 │
 ├── GPIO
 │
 ├── PWM
 │
 ├── ADC
 │
 ├── CAN/LIN
 │
 ├── SPI
 │
 ├── Wakeup
 │
 └── Safety
```

特别是 **F29P32 DCDC PWM、SPC58NN OBC 外设、TC377 AUTOSAR/CAN**，这张 Matrix 可以直接作为以后做 **原理图 → MCAL/底层驱动 → App** 的统一接口定义。

**注意：具体 Pin Number / ALT 编号会随 F29P329SM-Q1、SPC58NN84C3、TC377TP/TX 以及封装变化，不能用“系列级”表直接画 PCB。** F29P329SM-Q1 本身就有 100/144-pin 不同封装选项，而 SPC58NN84C3 是 292-pin BGA；因此下一版最好锁定实际使用的三个具体型号和封装后，再逐 Pin 展开。([Texas Instruments](https://www.ti.com/product/F29P329SM-Q1?utm_source=chatgpt.com))

如果按现在的项目背景，建议下一步直接做成 **“F29P329SM-Q1 + SPC58NN84C3 + TC377TP”三芯片完整 Pin Matrix**，把每一个实际 Pin 的 **GPIO/复用/PWM/ADC/CAN/Boot/Reset/默认上下拉/外部电阻/启动状态/App状态/下电状态** 全部列出来，这会比单纯看三份 Datasheet 实用很多。