#  CCCP

**OBC 的 CP/CCCP 从“国标信号”一路贯穿到“整车充电状态机、唤醒、PDU、功率限流和最终代码”**。如果按量产 OBC 软件的思路来做，建议不要把 CCCP 只理解成“测一个 CP 电压、算一个 PWM”，而是建立下面这条完整链路：

> **GB/T 标准 → OEM 电气需求 → CP/CC 硬件 → MCU/TIM/ADC → CP 状态识别 → PWM 解码 → 插枪/拔枪 → 唤醒 → PDU 状态机 → BMS/CAN → OBC 功率限流 → 充电停止**

下面以**国标交流 OBC，典型单相/三相 AC 充电**为主线讲。需要特别区分：GB/T 27930-2023 是**非车载传导式充电机与 EV 的数字通信协议**，主要用于 DC 充电场景；OBC 的交流充电 CP/CC 主体应从 GB/T 18487.1 和 GB/T 20234.2 等体系理解。GB/T 18487.1-2023 已于 2024-04-01 实施，GB/T 20234.2-2015 目前仍为现行标准。([Open Standard SAMR](https://openstd.samr.gov.cn/bzgk/std/newGbInfo?hcno=AEF144D5381E9FDBD265AFA5A87595A3&utm_source=chatgpt.com))

------

## 一、先建立一个最重要的认识：CCCP 到底是什么

在 OBC 项目里，经常把：

- **CP：Control Pilot**
- **CC：Connection Confirm**

统称为 **CC/CP、CCCP 或充电连接确认/控制导引系统**。

它实际上完成了四件事：

```text
                 充电枪
                    │
          ┌─────────┴─────────┐
          │                   │
         CP                  CC
          │                   │
          ▼                   ▼
    充电设备能力        插枪/电缆容量确认
    +充电状态             +机械连接确认
          │                   │
          └─────────┬─────────┘
                    ▼
               OBC MCU
                    │
       ┌────────────┼─────────────┐
       ▼            ▼             ▼
    唤醒系统      PDU状态机      功率限制
       │            │             │
       └────────────┼─────────────┘
                    ▼
               OBC开始充电
```

所以：

**CP 不是单纯的“PWM 输入”。**

它同时承担：

1. 插枪检测
2. 车辆是否准备充电
3. 充电设备最大允许电流
4. 数字通信模式识别
5. 充电过程中连接状态监控
6. 充电停止条件判断

而 CC 更偏向：

1. 插枪确认
2. 电缆额定载流能力确认
3. 连接器状态判断

------

## 二、国标 → OBC 软件需求，应该怎么拆

建议在项目里建立这样一张需求追踪表：

| 层级     | 内容                   | 最终落在哪里 |
| -------- | ---------------------- | ------------ |
| 国家标准 | GB/T 18487.1           | CCCP需求     |
| 接口标准 | GB/T 20234.2           | CP/CC接口    |
| OEM规范  | CP电压阈值、滤波时间等 | CCCP配置     |
| 系统需求 | 插枪后多久唤醒         | Wakeup       |
| PDU需求  | CP状态对应哪个状态机   | PDU          |
| 功率需求 | 最大充电电流           | OBC功率限流  |
| 诊断需求 | CP异常、CC异常         | DTC          |
| 软件需求 | ADC/TIM采样周期        | Driver       |
| 代码     | `CP_Process()`         | CCCP模块     |

这是实际开发中特别重要的一点：

> **不要直接从国标写 C 代码。**

中间必须有一层：

```text
GB/T
 ↓
OEM System Requirement
 ↓
Software Requirement
 ↓
Software Design
 ↓
Driver
 ↓
Application
 ↓
Test Case
```

否则后面 OEM 改一个 CP 电压阈值，整个代码都容易失控。

------

## 三、CP 的物理原理

典型 CP 可以简化成：

```text
        OBC / EVSE
             │
        ±12V / PWM
             │
             │ CP
             │
             ├───────────────┐
             │               │
             │              R1
             │               │
             │               ├──── PE
             │               │
             │              S2
             │               │
             │              R2
             │               │
             └───────────────┘
```

车辆侧通过电阻网络和开关 S2 改变 CP 对 PE 的等效负载。

于是：

```text
未连接
CP ≈ +12V

连接车辆
CP ≈ +9V

车辆准备充电
CP ≈ +6V

异常
CP ≈ 0V

负半周
CP ≈ -12V
```

GB/T 18487.1-2023 的 AC CP 状态图明确包含：

```text
State 1     +12V       未连接
State 2     +9V        已连接
State 3     +6V        车辆准备充电
State 0      0V        异常
State 4     -12V       异常/特殊状态
```

同时还存在 PWM 状态，例如：

```text
State 1'    +12V PWM
State 2'    +9V PWM
State 3'    +6V PWM
```

标准给出了这些状态之间的转换关系。([GB Standards](https://member.gbstandards.org/rjsoa/GB_order/GB_files/ID9163-2024-2-7/GBT 18487.1-2023 English Version.pdf?utm_source=chatgpt.com))

------

## 四、CP 的 PWM 到底表达什么？

这是 CCCP 最核心的地方。

假设充电设备输出：

```text
CP = ±12V

PWM = 1 kHz
```

PWM duty：

```text
D = Ton / T
```

例如：

```text
T = 1 ms

Ton = 600 us

D = 60%
```

对于 CP PWM，GB/T 18487.1-2023 给出的主要映射包括：

```text
10% ≤ D ≤ 85%

Imax = D × 100 × 0.6
```

所以：

```text
D = 10%
Imax = 6A

D = 20%
Imax = 12A

D = 50%
Imax = 30A

D = 60%
Imax = 36A

D = 85%
Imax = 51A
```

而高占空比区域：

```text
85% < D ≤ 90%

Imax = (D×100 - 64) × 2.5
```

并受标准规定的最大电流限制。5% PWM 则表示需要数字通信，不能简单理解成“5%就是3A”。([Scribd](https://www.scribd.com/document/766330125/GB-T-18487-1-2023-电动汽车传导充电系统-第1部分-通用要求?utm_source=chatgpt.com))

------

## 五、所以 OBC 软件真正需要得到的不是 Duty，而是 CP Capability

例如 MCU 测出来：

```text
CP Frequency = 999.7 Hz

CP Duty = 49.8%

CP High Level = 9.05 V

CP Low Level = -11.9 V
```

软件应该转换成：

```text
CP State = VEHICLE_READY

CP PWM = VALID

EVSE_MAX_CURRENT ≈ 29.88A
```

然后进入：

```text
PDU
 ↓
允许充电
 ↓
功率计算
 ↓
电流限值
```

------

## 六、一个完整 OBC CCCP 硬件框图

可以把硬件理解成：

```text
                  AC INPUT
                     │
              ┌──────┴──────┐
              │             │
             L/N           PE
              │             │
              │             │
          ┌───▼───┐         │
          │ Relay │         │
          └───┬───┘         │
              │             │
              │             │
       ┌──────▼─────────────▼──────┐
       │          OBC               │
       │                            │
       │  PFC → LLC → DC/DC        │
       │                            │
       │  MCU                       │
       │   │                        │
       │   ├── ADC                  │
       │   │    ↑                   │
       │   │   CP_SENSE             │
       │   │                        │
       │   ├── TIM                  │
       │   │    ↑                   │
       │   │   CP_PWM               │
       │   │                        │
       │   ├── GPIO                 │
       │   │    ↑                   │
       │   │   CC_SENSE             │
       │   │                        │
       │   ├── CAN                  │
       │   │                        │
       │   └── PDU                  │
       └────────────────────────────┘
```

实际项目中 CP 前面通常还有：

```text
CP
 │
 ├─ TVS / ESD
 │
 ├─ RC滤波
 │
 ├─ 分压
 │
 ├─ 钳位
 │
 └─ ADC
```

同时可能有一路专门进入：

```text
Comparator
   ↓
GPIO / TIM Capture
```

这就产生两种典型方案。

------

## 七、方案一：ADC 测 CP 电压

例如：

```text
CP
 │
 R1
 │────── ADC
 R2
 │
GND
```

如果：

```text
R1 = 30k
R2 = 10k
```

那么：

$V_{ADC}=V_{CP}\frac{R_2}{R_1+R_2}$

即：

$V_{CP}=V_{ADC}\frac{R_1+R_2}{R_2}$

所以：

```text
ADC = 3.0V

CP = 3 × 4
   = 12V
```

这样 ADC 可以判断：

```text
+12V
+9V
+6V
0V
```

但是这里有一个非常关键的问题：

> **ADC 不适合单独承担 CP PWM 精确频率/Duty 测量。**

因为 CP 是 1 kHz 方波。

更合理：

```text
ADC
 ↓
CP Voltage Level

TIM Input Capture
 ↓
Frequency
Duty
```

------

## 八、方案二：TIM Input Capture 测 CP PWM

这是更推荐的量产软件架构。

假设：

```text
MCU Timer Clock = 100 MHz
Prescaler = 99
```

得到：

```text
TIM Counter = 1 MHz
```

那么：

```text
1 count = 1 us
```

CP：

```text
Frequency ≈ 1kHz

Period ≈ 1000us
```

假设 TIM 捕获：

```text
Rising Edge = 1000
Falling Edge = 1600
Next Rising = 2000
```

那么：

```text
HighTime = 1600 - 1000
         = 600 us

Period = 2000 - 1000
       = 1000 us
```

因此：

$D=\frac{600}{1000}=60\%$

频率：

$f=\frac{1}{1000\mu s}=1000Hz$

然后：

$I_{EVSE}=60\times0.6=36A$

------

## 九、TIM Capture 的软件逻辑

建议：

```text
TIM Capture ISR
       │
       ▼
记录 Rising
       │
       ▼
记录 Falling
       │
       ▼
记录 Next Rising
       │
       ▼
Period
HighTime
Duty
Frequency
       │
       ▼
CP Decode
```

代码核心可以写成：

```c
typedef struct
{
    uint32_t rise_prev;
    uint32_t rise_curr;
    uint32_t fall_curr;

    uint32_t period_cnt;
    uint32_t high_cnt;

    float duty;
    float frequency;

    bool valid;
} CP_PwmMeasure_t;
```

计算：

```c
period_cnt =
    rise_curr - rise_prev;

high_cnt =
    fall_curr - rise_prev;

duty =
    (float)high_cnt /
    (float)period_cnt;

frequency =
    TIM_FREQ_HZ /
    (float)period_cnt;
```

------

## 十、实际量产不能直接这么算

因为：

```text
CP边沿
    ↓
ADC噪声
EMI
开关电源噪声
继电器动作
振铃
    ↓
可能产生假边沿
```

所以建议：

```text
TIM Capture
     ↓
Range Check
     ↓
Frequency Check
     ↓
Duty Check
     ↓
连续N次有效
     ↓
滤波
     ↓
状态机
```

例如：

```c
#define CP_FREQ_MIN_HZ       900.0f
#define CP_FREQ_MAX_HZ      1100.0f

#define CP_DUTY_MIN          0.03f
#define CP_DUTY_MAX          0.97f

#define CP_VALID_COUNT          3U
```

------

## 十一、CP 电压和 PWM Duty 必须“联合判断”

这是很多初学者容易犯的错误。

不能：

```c
if (duty > 0.5)
{
    charging = true;
}
```

应该：

```text
CP Voltage
    +
CP Frequency
    +
CP Duty
    +
CC State
    +
PDU State
    +
HVIL
    +
AC Voltage
    +
Fault
```

共同决定：

```text
CanCharge
```

也就是：

$CanCharge= CP\_Valid \land CC\_Valid \land HVIL\_OK \land AC\_Valid \land NoCriticalFault \land BMS\_Allow$

------

## 十二、CC 又是怎么工作的？

可以简化：

```text
CC
 │
 ├── Cable resistor
 │
 └── ADC
```

软件得到：

```text
CC ADC
 ↓
Rcc
 ↓
Cable Capacity
```

例如概念上：

```text
Rcc ≈ 某一电阻
       ↓
对应某种线缆额定能力
```

最终形成：

```c
I_cc_limit = DecodeCableCurrent(cc_resistance);
```

然后与 CP 最大电流取最小值：

```c
I_cp_limit = DecodeCpCurrent(cp_duty);

I_connect_limit =
    MIN(I_cp_limit, I_cc_limit);
```

这一步非常重要。

------

## 十三、真正的 OBC 充电电流限制不是 CP 一个值

最终：

$I_{charge,max} = \min ( I_{CP}, I_{CC}, I_{OBC}, I_{BMS}, I_{Grid}, I_{Thermal}, I_{OEM} )$

例如：

```text
CP允许       32A
CC允许       32A
OBC额定      30A
BMS允许      25A
温度降额     22A
OEM限制      24A
```

最终：

```text
Icharge_max = 22A
```

这才是量产 OBC 的真正限流链路。

------

## 十四、CCCP 如何影响唤醒？

典型流程：

```text
Vehicle OFF
    │
    ▼
MCU低功耗
    │
    │ CP/CC事件
    ▼
Wakeup
    │
    ▼
初始化 ADC/TIM
    │
    ▼
CP/CC检测
    │
    ▼
确认插枪
    │
    ▼
PDU进入CHARGE_PREPARE
```

例如：

```text
CP ≈ 12V
CC disconnected
```

那么：

```text
No Plug
```

如果：

```text
CP ≈ 9V
CC valid
```

则：

```text
Plug Inserted
```

唤醒链可以设计成：

```text
CC GPIO interrupt
       │
       ▼
Wakeup Request
       │
       ▼
PDU
       │
       ▼
Enable OBC Low Voltage
       │
       ▼
CCCP_Init()
       │
       ▼
CP/CC valid?
```

------

## 十五、但要特别注意：Wakeup 不等于允许充电

这是架构设计的关键。

应该：

```text
CC/CP Event
      ↓
Wakeup
      ↓
System Init
      ↓
CCCP Validation
      ↓
PDU Charging State
      ↓
BMS Communication
      ↓
Power Stage Enable
```

而不是：

```text
CC
 ↓
Wakeup
 ↓
直接PWM发波
```

后者安全风险很大。

------

## 十六、PDU 状态机建议这样设计

建议 OBC 项目至少：

```text
PDU_OFF
   │
   ▼
PDU_WAKEUP
   │
   ▼
PDU_INIT
   │
   ▼
PDU_WAIT_CP
   │
   ▼
PDU_PLUGGED
   │
   ▼
PDU_WAIT_BMS
   │
   ▼
PDU_PRECHARGE
   │
   ▼
PDU_CHARGING
   │
   ├──────► PDU_DERATING
   │
   ▼
PDU_STOPPING
   │
   ▼
PDU_FINISH
   │
   ▼
PDU_SLEEP
```

------

## 十七、CP 对 PDU 的影响

可以做成：

```text
                 CP
                 │
        ┌────────┴────────┐
        ▼                 ▼
   Voltage State       PWM Decode
        │                 │
        ▼                 ▼
  Plug State         EVSE Current
        │                 │
        └────────┬────────┘
                 ▼
             CCCP State
                 │
                 ▼
             PDU State
```

例如：

### CP = 12V

```text
UNCONNECTED
```

PDU：

```text
WAIT_PLUG
```

### CP = 9V

```text
CONNECTED
```

PDU：

```text
PLUGGED
```

### CP = 6V + PWM

```text
VEHICLE_READY
```

PDU：

```text
CAN_START
PRECHARGE
```

### CP 突然变回 9V

```text
Vehicle no longer ready
```

PDU：

```text
STOP_CHARGING
```

### CP 消失

```text
Plug removed
```

PDU：

```text
IMMEDIATE_STOP
```

------

## 十八、CP PWM 又如何影响 OBC 功率？

假设：

```text
CP = 50%
```

得到：

$I_{CP}=50\times0.6=30A$

假设：

```text
AC = 220V
PF = 0.99
η = 0.95
```

那么输入功率上限：

$P_{AC}\approx220\times30\times0.99$

约：

```text
6.53 kW
```

如果 OBC 额定：

```text
6.6kW
```

那么：

```text
P_limit
=
min(
6.53kW,
6.6kW,
BMS_limit,
thermal_limit
)
```

然后 PFC/OBC 控制器再根据这个功率限值产生：

```text
Iac_ref
```

------

## 十九、CP 对 PFC 的影响

完整链：

```text
CP Duty
   ↓
EVSE Max Current
   ↓
OBC Current Limit
   ↓
Power Limit
   ↓
PFC Iref
   ↓
AC Input Current
```

例如：

```c
P_limit = MIN(
    cp_power_limit,
    bms_power_limit,
    thermal_power_limit,
    oem_power_limit
);
```

然后：

```c
Iac_rms_ref =
    P_limit / Vac_rms;
```

PFC 再进一步进行：

```text
Vac
 ↓
Voltage Loop
 ↓
Power / Current Reference
 ↓
Current Loop
 ↓
PWM
```

所以 CP 最终是会“间接影响功率级 PWM”的。

------

## 二十、推荐的软件架构

建议 OBC 项目把 CCCP 独立成：

```text
CCCP
│
├── CCCP_Drv
│   ├── ADC
│   ├── TIM
│   ├── GPIO
│   └── Wakeup
│
├── CCCP_Measure
│   ├── CP Voltage
│   ├── CP PWM
│   ├── CC Resistance
│   └── Filtering
│
├── CCCP_Decode
│   ├── CP State
│   ├── CP Current
│   └── CC Capacity
│
├── CCCP_State
│   ├── Plug
│   ├── Ready
│   ├── Charging
│   └── Abnormal
│
└── CCCP_Manager
    ├── Wakeup
    ├── PDU
    ├── Power Limit
    └── Fault
```

------

## 二十一、推荐的 C 文件结构

建议最终至少拆成：

```text
CCCP_Cfg.h
CCCP.h
CCCP.c

CCCP_Drv.h
CCCP_Drv.c

CCCP_Measure.h
CCCP_Measure.c

CCCP_Decode.h
CCCP_Decode.c

CCCP_State.h
CCCP_State.c

OBC_PDU.h
OBC_PDU.c
```

其中：

```text
CCCP_Drv
```

负责 MCU。

而：

```text
CCCP_Decode
CCCP_State
```

完全不要依赖具体 MCU。

这样以后：

```text
TC377
   ↓
TI C2000
   ↓
NXP
```

只换 Driver。

------

## 二十二、核心数据结构

```c
typedef enum
{
    CP_STATE_UNKNOWN = 0,
    CP_STATE_A_12V,
    CP_STATE_B_9V,
    CP_STATE_C_6V,
    CP_STATE_FAULT_0V,
    CP_STATE_NEG_12V
} CP_State_t;
```

PWM：

```c
typedef struct
{
    uint32_t period_cnt;
    uint32_t high_cnt;

    float frequency_hz;
    float duty;

    bool valid;
} CP_Pwm_t;
```

CP：

```c
typedef struct
{
    float voltage_v;
    CP_State_t state;

    CP_Pwm_t pwm;

    float evse_current_a;

    bool valid;
} CP_Info_t;
```

CC：

```c
typedef struct
{
    float resistance_ohm;
    float cable_current_a;
    bool valid;
} CC_Info_t;
```

最终：

```c
typedef struct
{
    CP_Info_t cp;
    CC_Info_t cc;

    float current_limit_a;
    float power_limit_w;

    bool plug_present;
    bool charge_ready;
    bool charge_allowed;
} CCCP_Status_t;
```

------

## 二十三、CP 电压状态判断

实际项目不要：

```c
if (voltage == 9.0f)
```

而应该：

```c
static CP_State_t CP_DecodeVoltage(float voltage)
{
    if ((voltage > 10.5f) &&
        (voltage < 13.5f))
    {
        return CP_STATE_A_12V;
    }

    if ((voltage > 7.5f) &&
        (voltage < 10.5f))
    {
        return CP_STATE_B_9V;
    }

    if ((voltage > 4.5f) &&
        (voltage < 7.5f))
    {
        return CP_STATE_C_6V;
    }

    if ((voltage > -1.5f) &&
        (voltage < 1.5f))
    {
        return CP_STATE_FAULT_0V;
    }

    return CP_STATE_UNKNOWN;
}
```

**这里的阈值只是软件架构示例，不应该直接作为 OEM 最终标定值。**

量产项目应从：

```text
GB/T
+
OEM
+
硬件误差
+
ADC误差
+
温漂
+
滤波
```

一起计算。

------

## 二十四、PWM Decode

```c
static float CP_DecodeCurrent(float duty)
{
    float duty_percent;
    float current;

    duty_percent = duty * 100.0f;

    if ((duty_percent >= 10.0f) &&
        (duty_percent <= 85.0f))
    {
        current = duty_percent * 0.6f;
    }
    else if ((duty_percent > 85.0f) &&
             (duty_percent <= 90.0f))
    {
        current = (duty_percent - 64.0f) * 2.5f;

        if (current > 63.0f)
        {
            current = 63.0f;
        }
    }
    else
    {
        current = 0.0f;
    }

    return current;
}
```

这里对应 GB/T 18487.1-2023 的 PWM 电流映射。([GB Standards](https://member.gbstandards.org/rjsoa/GB_order/GB_files/ID9163-2024-2-7/GBT 18487.1-2023 English Version.pdf?utm_source=chatgpt.com))

------

## 二十五、但是 5% PWM 必须特殊处理

例如：

```c
if (duty_percent >= 4.5f &&
    duty_percent <= 5.5f)
{
    cp_need_digital_comm = true;
}
```

此时不能：

```c
Imax = 5 × 0.6
     = 3A
```

而应该理解成：

```text
需要数字通信
```

也就是说：

```text
5% CP
 ↓
CCCP
 ↓
Digital Communication Required
 ↓
PDU
 ↓
CAN/其他充电通信
 ↓
允许能量传输
```

------

## 二十六、完整限流代码

这是项目里非常值得单独抽出来的模块：

```c
float CCCP_CalcCurrentLimit(
    float cp_current,
    float cc_current,
    float obc_current,
    float bms_current,
    float thermal_current,
    float oem_current)
{
    float limit = cp_current;

    if (cc_current < limit)
        limit = cc_current;

    if (obc_current < limit)
        limit = obc_current;

    if (bms_current < limit)
        limit = bms_current;

    if (thermal_current < limit)
        limit = thermal_current;

    if (oem_current < limit)
        limit = oem_current;

    return limit;
}
```

最后：

```c
OBC_CurrentLimit_A =
    CCCP_CalcCurrentLimit(
        cp_current,
        cc_current,
        obc_rated_current,
        bms_current,
        thermal_current,
        oem_current);
```

------

## 二十七、再把它接到 PFC

```c
void OBC_PowerLimitUpdate(void)
{
    float p_limit;

    p_limit =
        OBC_CurrentLimit_A *
        GridVoltage_Rms_V *
        GridPowerFactor;

    if (p_limit > OBC_RATED_POWER_W)
    {
        p_limit = OBC_RATED_POWER_W;
    }

    OBC_PowerLimit_W = p_limit;
}
```

然后：

```c
void PFC_Control(void)
{
    float iac_ref;

    iac_ref =
        OBC_PowerLimit_W /
        GridVoltage_Rms_V;

    PFC_SetCurrentReference(iac_ref);
}
```

实际量产控制当然会比这个复杂：

```text
P_limit
 ↓
PFC voltage loop
 ↓
feedforward
 ↓
current reference
 ↓
current PI
 ↓
PWM
```

这里只是为了把 **CCCP → PFC** 的软件关系打通。

------

## 二十八、CCCP Manager

最后把所有东西串起来：

```c
void CCCP_MainFunction_10ms(void)
{
    CCCP_ReadSignals();

    CCCP_Filter();

    CCCP_DecodeCP();

    CCCP_DecodeCC();

    CCCP_UpdateState();

    CCCP_UpdateCurrentLimit();

    CCCP_UpdatePowerLimit();

    CCCP_UpdateWakeup();

    CCCP_UpdatePDURequest();

    CCCP_UpdateFault();
}
```

------

## 二十九、PDU 的核心逻辑

例如：

```c
void PDU_ChargeStateMachine(void)
{
    switch (pdu_state)
    {
        case PDU_WAIT_PLUG:

            if (CCCP_IsPlugged())
            {
                pdu_state = PDU_PLUGGED;
            }

            break;


        case PDU_PLUGGED:

            if (!CCCP_IsPlugged())
            {
                pdu_state = PDU_WAIT_PLUG;
            }
            else if (CCCP_IsVehicleReady())
            {
                pdu_state = PDU_WAIT_BMS;
            }

            break;


        case PDU_WAIT_BMS:

            if (!CCCP_IsPlugged())
            {
                pdu_state = PDU_WAIT_PLUG;
            }
            else if (BMS_IsReady())
            {
                pdu_state = PDU_PRECHARGE;
            }

            break;


        case PDU_PRECHARGE:

            if (Precharge_OK())
            {
                pdu_state = PDU_CHARGING;
            }

            break;


        case PDU_CHARGING:

            if (!CCCP_IsVehicleReady())
            {
                pdu_state = PDU_STOPPING;
            }

            if (CriticalFault())
            {
                pdu_state = PDU_STOPPING;
            }

            break;


        case PDU_STOPPING:

            OBC_StopPowerStage();

            if (PowerStageOff())
            {
                pdu_state = PDU_WAIT_PLUG;
            }

            break;


        default:
            pdu_state = PDU_WAIT_PLUG;
            break;
    }
}
```

------

## 三十、这里真正形成一个完整闭环

最终的软件关系其实是：

```text
              ┌───────────────┐
              │     CP/CC     │
              └───────┬───────┘
                      │
          ┌───────────┴───────────┐
          ▼                       ▼
      CP State                CP PWM
          │                       │
          │                 EVSE Current
          │                       │
          └───────────┬───────────┘
                      ▼
                  CCCP Manager
                      │
        ┌─────────────┼─────────────┐
        ▼             ▼             ▼
     Wakeup          PDU        Power Limit
        │             │             │
        │             ▼             ▼
        │          Charging       PFC
        │                           │
        │                           ▼
        │                          LLC
        │                           │
        └───────────────────────────┘
```

------

## 三十一、建议把整个 OBC CCCP 分成 6 个软件周期

一个比较合理的架构：

| Task           | 周期      | 内容       |
| -------------- | --------- | ---------- |
| CP Capture ISR | 边沿      | TIM捕获    |
| CCCP Fast      | 1 ms      | CP PWM/ADC |
| CCCP State     | 10 ms     | 状态机     |
| PDU            | 10 ms     | 充电状态   |
| Power Limit    | 10~20 ms  | 功率限值   |
| Diagnosis      | 10~100 ms | 故障诊断   |

其中：

```text
TIM Capture
```

不要放到：

```text
10ms Task
```

里面算边沿。

而应该：

```text
TIM HW
 ↓
ISR
 ↓
Raw Capture
 ↓
1ms Task
 ↓
Decode
```

------

## 三十二、整个项目最终可以形成这张“软件地图”

```text
                    GB/T 18487.1
                          │
                          ▼
                    OEM CCCP Spec
                          │
              ┌───────────┴───────────┐
              │                       │
             CP                      CC
              │                       │
       ┌──────┴──────┐         ┌──────┴──────┐
       │             │         │             │
      ADC           TIM       ADC           GPIO
       │             │         │             │
       └──────┬──────┘         └──────┬──────┘
              │                       │
              ▼                       ▼
        CP Voltage/Duty          Cable Capacity
              │                       │
              └──────────┬────────────┘
                         ▼
                    CCCP Decode
                         │
          ┌──────────────┼──────────────┐
          ▼              ▼              ▼
       Wakeup           PDU         Power Limit
          │              │              │
          │              ▼              ▼
          │          Precharge         PFC
          │              │              │
          │              ▼              ▼
          │          Charging          LLC
          │              │
          └──────────────┼──────────────┘
                         ▼
                     OBC Stop
```

------

## 三十三、国标、OEM、代码之间最容易混淆的边界

做项目时，可以用下面这个原则：

### 国标规定

```text
“这个信号应该代表什么”
```

例如 CP：

```text
12V
9V
6V
PWM
```

### OEM规定

```text
“我们的产品具体怎么判”
```

例如：

```text
CP > 10.5V → A
8.0~10.5V → B
...
连续多少ms确认
异常多少ms确认
```

### 硬件决定

```text
“MCU究竟看到多少”
```

例如：

```text
实际 CP = 9V

ADC = 2.24V
```

### Driver 决定

```text
“怎么拿到这个数据”
```

例如：

```text
ADC DMA
TIM Capture
GPIO
```

### CCCP Application 决定

```text
“这个数据意味着什么”
```

### PDU 决定

```text
“接下来系统做什么”
```

### 功率控制决定

```text
“最终允许多少电流”
```

这几个层级一定不要混。

------

## 三十四、代码

按照 OBC/PDU/DCDC 项目的方式，更适合直接做成一个可以落地的最小 CCCP 软件包：

```text
CCCP_Cfg.h
CCCP_Types.h
CCCP.h
CCCP.c

CCCP_Drv.h
CCCP_Drv.c

CCCP_Measure.h
CCCP_Measure.c

CCCP_Decode.h
CCCP_Decode.c

CCCP_State.h
CCCP_State.c

CCCP_Limit.h
CCCP_Limit.c

OBC_PDU.h
OBC_PDU.c

CP_TimCapture.c
CP_Adc.c
CC_Sense.c
```

其中可以进一步按实际 OBC 的 MCU 来写成**接近量产代码**的版本，包括：

- `TIM Input Capture`
- `ADC DMA`
- CP 正/负半周采样
- CP 电压滤波
- CP 1 kHz 频率判断
- Duty 计算
- 5% 数字通信识别
- 10%~85% 电流映射
- CC 电阻/电缆容量识别
- 插枪/拔枪去抖
- CP 状态机
- Wakeup
- PDU 状态机
- BMS 允许充电
- OBC 最大电流
- 温度降额
- CP/CC 限流
- 充电启动
- 充电停止
- CP 异常诊断
- 超时诊断
- 信号合理性检查
- DTC
- CAN 信号接口
- 单元测试接口

而且可以进一步把 **“CP 的 PWM 输出”和“车辆 OBC 的 CP 输入”两个方向都讲清楚**：对于 OBC 本身，通常是**EV 侧接收/解码 EVSE 的 CP**；如果是在开发充电桩/EVSE，则相反是 MCU 通过 TIM/PWM **产生 1 kHz CP**。GB/T 18487.1 对供电设备产生的 PWM 占空比与最大供电电流之间也规定了映射关系。([Scribd](https://www.scribd.com/document/766330125/GB-T-18487-1-2023-电动汽车传导充电系统-第1部分-通用要求?utm_source=chatgpt.com))

另外，若 OBC 是**国标 AC OBC**，建议把 GB/T 27930-2023 从“CCCP核心依据”里拿出来单独放到 **DC充电通信/跨产品架构参考**；它目前是现行标准，规定的是非车载充电机 SECC 与车辆 EVCC 基于 CAN 的数字通信。([Open Standard SAMR](https://openstd.samr.gov.cn/bzgk/std/newGbInfo?hcno=6ECF725CD2BCCA2819082279F6B2E243&utm_source=chatgpt.com))

## 三十五、OBC CCCP Portable Software Package

这是一个面向量产 OBC AC 充电软件架构的 CCCP（CP/CC）可移植软件包。

目标：

1. 公共算法层完全不直接访问 MCU 寄存器。
2. TC377、TI F29P32x、SPC58NN 只实现统一的 `CCCP_Platform` 接口。
3. CP PWM 频率/Duty 使用硬件 capture 数据计算。
4. CP 电压、CC 电阻使用 ADC/板级驱动获得。
5. 公共层完成 CP/CC 解码、去抖、状态机、唤醒请求、电流/功率限值。
6. PDU 层只消费公共 CCCP 状态，不关心 MCU 型号。

### 目录

```text
OBC_CCCP_Portable_Package/
├── common/
│   ├── include/
│   │   ├── CCCP_Types.h
│   │   ├── CCCP_Cfg.h
│   │   ├── CCCP_Platform.h
│   │   └── CCCP.h
│   └── src/
│       ├── CCCP.c
│       ├── CCCP_Measure.c
│       ├── CCCP_Decode.c
│       ├── CCCP_State.c
│       └── CCCP_Limit.c
├── app/
│   ├── OBC_CCCP_App.c
│   ├── OBC_PDU.c
│   └── OBC_PowerLimit.c
├── port/
│   ├── tc377/
│   │   ├── CCCP_Platform_tc377.h
│   │   └── CCCP_Platform_tc377.c
│   ├── f29p32x/
│   │   ├── CCCP_Platform_f29p32x.h
│   │   └── CCCP_Platform_f29p32x.c
│   └── spc58nn/
│       ├── CCCP_Platform_spc58nn.h
│       └── CCCP_Platform_spc58nn.c
└── test/
    ├── mock/
    │   └── CCCP_Platform_mock.c
    └── test_cccp.c
```

### 核心数据流

```text
             CP pin
               │
       ┌───────┴────────┐
       │                │
      ADC          TIM/eCAP/eMIOS
       │                │
 CP voltage        period/high time
       │                │
       └───────┬────────┘
               ▼
         CCCP_Measure
               ▼
         CCCP_Decode
          │          │
          ▼          ▼
       CP State    EVSE Imax
          │          │
          └────┬─────┘
               ▼
          CCCP_State
               │
       ┌───────┼─────────┐
       ▼       ▼         ▼
    Wakeup    PDU    Current Limit
                         │
                         ▼
                    Power Limit
                         │
                         ▼
                      PFC/LLC
```

### 任务建议

- Capture ISR / DMA callback：边沿事件，更新双缓冲 capture。
- `CCCP_1msTask()`：采样快照、计算 CP/CC 原始量。
- `CCCP_10msTask()`：解码、状态确认、限流。
- PDU 10 ms：消费 `CCCP_GetStatus()`。
- PFC/LLC 控制周期：消费已经计算好的功率/电流上限。

### CP PWM

对普通 PWM 区域：

- 10%~85%：`Imax = Duty(%) × 0.6 A`
- 85%~90%：`Imax = (Duty(%) - 64) × 2.5 A`
- 约 5%：标记为数字通信请求，不直接当作 3 A。

具体产品阈值、去抖时间、CC 表、最大电流、功率限制必须由 OEM/国标/硬件误差共同标定。

### MCU 适配原则

公共层只依赖：

```c
CCCP_Platform_ReadCpVoltage();
CCCP_Platform_ReadCcResistance();
CCCP_Platform_GetCpPwmCapture();
CCCP_Platform_RequestWakeup();
CCCP_Platform_SetChargePowerEnable();
```

#### TC377

推荐：

- GTM TIM：CP PWM 输入捕获
- EVADC：CP/CC 模拟量
- SCU/GPIO/PMIC：唤醒

Infineon 的 TC37x 文档和培训资料明确支持 GTM TIM 输入捕获/滤波配置；具体 channel/pin 仍必须根据实际 TC377 封装和 PCB 选择。

#### TI F29P32x

推荐：

- eCAP 或项目选定 capture 外设：CP PWM
- ADC：CP/CC
- GPIO/XINT：唤醒

使用 TI C2000Ware/设备 DriverLib 时，把所有 DriverLib 调用限制在 `port/f29p32x`。

#### SPC58NN

推荐：

- 项目使用的 timer input-capture/MCAL：CP PWM
- ADC/MCAL：CP/CC
- GPIO/IRQ/PMIC：唤醒

SPC58NN 的具体 timer instance、channel 和 MCAL API 随具体 derivative、MCAL package 和项目配置变化，因此全部放在 `port/spc58nn`。

### 注意

本包是“完整的软件架构 + 可编译公共算法 + 三套芯片适配骨架”，不是针对某块 PCB 的最终寄存器工程。

要成为可直接烧录的量产工程，还必须填入：

- CP/CC 实际 ADC channel
- 分压/滤波参数
- timer/capture channel
- pinmux
- interrupt vector
- MCU 时钟
- 实际 wakeup 电路
- PMIC 接口
- OEM 状态机
- BMS CAN 接口
- PFC/LLC enable 接口
- DTC/DEM 接口
- ASIL/safety mechanism