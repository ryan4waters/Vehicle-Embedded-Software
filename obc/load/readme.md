# OBC / DCDC LOAD

**CC、CV、CR 是“测试设备对被测电源施加的负载模型”，不是简单地等同于“车上某一种负载”。**

对 OBC 来说，真正的车上负载主要是**动力电池**，它在充电过程中表现为一个随 SOC、温度、BMS 限值、极化、电芯一致性变化的动态电气系统，典型充电过程是 **CC → CV**。
对 DCDC 来说，车上负载则是大量 ECU、传感器、执行器、灯具、继电器、风扇、电机以及低压蓄电池组成的**混合动态负载**，很少是真正意义上的纯 CC / CV / CR。

## 一、OBC / DCDC 的“负载地图”

可以先把两类产品分开看：

```text
                    新能源汽车电源系统
                           │
          ┌────────────────┴────────────────┐
          │                                 │
         OBC                               DCDC
          │                                 │
      AC → HV DC                         HV DC → LV DC
          │                                 │
          ▼                                 ▼
      动力电池 HV                         12V/48V电源网络
          │                                 │
    ┌─────┴─────┐              ┌────────────┼────────────┐
    │           │              │            │            │
   BMS        电池包         低压蓄电池    ECU/负载    瞬态负载
    │           │              │
    └─────┬─────┘              ├── MCU/ECU
          │                    ├── ADAS
       CC → CV                 ├── IVI
                               ├── 灯
                               ├── 风扇
                               ├── 泵
                               ├── 继电器
                               └── 其他执行器
```

因此：

| 产品 | DUT输出端真正面对的对象  | 实验室主要模拟方式                   |
| ---- | ------------------------ | ------------------------------------ |
| OBC  | HV动力电池 + BMS         | 电池模拟器 / 蓄电池 + BMS / 双向电源 |
| DCDC | 12V/48V电池 + 车载用电器 | 电子负载 + 蓄电池/电池模拟器         |
| OBC  | 不仅仅是“一个CV负载”     | CV电子负载只能近似                   |
| DCDC | 不仅仅是“一个CC负载”     | 动态电子负载只能近似                 |

------

## 二、CC / CV / CR 到底分别代表什么

电子负载的基本模型非常明确：

### 1. CC——恒流负载

```text
DUT ───────► Electronic Load
               │
               └── Iload = Constant
```

例如：

```text
DCDC = 14V
CC = 100A

P = 14 × 100 = 1.4kW
```

电压变化的时候，电子负载仍然努力保持 100 A。

CC 模式非常适合：

- 电源最大输出能力
- 恒流工作点
- OCP测试
- DCDC负载阶跃
- 电池放电
- 功率器件热测试

电子负载厂家也通常把 CC 作为电源、转换器以及电池测试的基础模式。([Keysight United States](https://www.keysight.com/au/en/products/dc-electronic-loads.html?utm_source=chatgpt.com))

------

### 2. CV——恒压负载

CV 是：

```text
DUT ───────► Electronic Load
                │
                └── Vload = Constant
```

例如：

```text
OBC → 400V
电子负载设定 CV = 400V
```

电子负载会根据 DUT 输出能力不断调整吸收电流：

```text
DUT能力增加
      ↓
Load吸收电流增加

DUT能力降低
      ↓
Load吸收电流降低
```

所以：**CV 是非常重要的“电池模拟”方法。**

但是这里有一个非常重要的区别：**CV电子负载 ≠ 真实动力电池。**

后面会重点讲。

------

### 3. CR——恒阻负载

CR：$I=\frac{V}{R}$

例如：

```text
R = 0.14Ω

14V → 100A
15V → 107.1A
12V → 85.7A
```

它的特点是：

```text
电压 ↑
   ↓
电流 ↑
```

因此 CR 更接近：**被动电阻型负载**

例如：

- 加热器
- 电阻丝
- 某些简单功率电阻
- 一部分灯丝类负载的近似模型

但现代汽车绝大多数电子负载，并不是纯 CR。

------

### 4. CP——恒功率

如果准备建立一套 OBC/DCDC 最小测试用例，强烈建议不要只考虑：CC / CV / CR

还应该加入：**CP——Constant Power，恒功率**

因为很多车载电子设备实际上更接近恒功率负载。

例如：

```text
DCDC = 14V
负载 = 700W

I = 50A
```

当 DCDC 电压下降到 12V：

```text
I ≈ 58.3A
```

即：

```text
V ↓
I ↑
P ≈ Constant
```

这就是典型的 **负阻抗特性**：$I=\frac{P}{V}$

现代 ECU、DC/DC 输入级、逆变器、电机控制器等内部都有闭环，所以不能简单地认为它们是一个电阻。

电子负载本身也普遍提供 CC/CV/CR/CP 等模式。([Keysight United States](https://www.keysight.com/au/en/products/dc-electronic-loads.html?utm_source=chatgpt.com))

------

## 三、OBC 的真正车载负载是什么？

这个地方一定要和 DCDC 分开。

OBC：

```text
AC
 ↓
PFC
 ↓
DC BUS
 ↓
LLC / CLLC
 ↓
HV Battery
```

最终负载是：**动力电池。**

但动力电池不是CC负载，也不是CV负载

而是一个：**随 SOC / 温度 / SOH / 电流 / BMS限制 / 电芯状态变化的动态电化学系统。**

------

## 四、OBC真实充电过程为什么是 CC → CV？

典型动力电池充电过程可以抽象为：

```text
          CC阶段                    CV阶段

I
│       ┌───────────────┐
│       │               │
│       │               └───────────────╲
│       │                                ╲
│       │                                 ╲
└───────┴───────────────────────────────────→ t


V
│                         ┌──────────────────
│                    ┌────┘
│                ┌───┘
│            ┌───┘
│        ┌───┘
│    ┌───┘
└────┴─────────────────────────────────────→ t
```

### CC阶段

OBC：

```text
控制 Icharge = Constant
```

例如：

```text
400V
200A
80kW
```

电池电压逐渐上升。

------

### CV阶段

达到电池/BMS允许的最高充电电压：

```text
Vbat = Vmax
```

之后：

```text
V = Constant
I ↓↓↓
```

最终：

```text
I < I_end
```

充电结束。

这也是动力电池常见的 CC-CV 充电方式。([PubMed Central (PMC)](https://pmc.ncbi.nlm.nih.gov/articles/PMC10447943/?utm_source=chatgpt.com))

------

## 五、OBC实验室到底应该怎么测试？

### Level 1：电子负载

最容易做。

```text
AC Source
    ↓
   OBC
    ↓
Electronic Load
```

主要用于：

#### A. CC负载

例如：

```text
400V / 50A
400V / 100A
400V / 150A
400V / 200A
```

测试：

- 最大输出能力
- 电流环
- 电压环
- 稳态精度
- 纹波
- 效率
- 热性能
- OCP
- 动态响应

------

#### B. CV负载

例如：

```text
CV = 350V
CV = 400V
CV = 450V
```

测试：

- OBC CV控制
- 电压调节
- CV → CC转移
- 充电结束行为
- 电流下降过程

------

#### C. CC → CV自动切换

这是 OBC 非常重要的用例。

例如：

```text
Vtarget = 450V
Ilimit = 15A

        CC
         │
         │
         ▼
     Vbat ↑
         │
         │
      450V
         │
         ▼
        CV
         │
         │
      I ↓
         │
         ▼
     Iend
         │
         ▼
       STOP
```

这比单纯测几个静态工作点更有价值。

------

### Level 2：真实蓄电池

如果实验室有条件，更建议：

```text
OBC
 │
 ▼
HV Battery
 │
 ▼
BMS
```

因为这时候可以真正观察：

```text
OBC
 ↕
BMS
 ↕
Battery
```

之间的交互。

例如：

```text
BMS：
允许充电电流 = 200A
允许充电电压 = 450V

       ↓

OBC:
Iref = min(OBC能力，BMS允许电流)
Vref = BMS允许电压
```

然后随着 SOC 上升：

```text
BMS允许电流
200A
 ↓
180A
 ↓
150A
 ↓
100A
 ↓
50A
 ↓
0A
```

这个过程是单纯电子负载无法完整模拟的。

------

## 六、OBC最小实验室用例集

如果做一个 **OBC软件工程师最小可用测试集**，会分成下面几类。

### 基础启动

| 用例   | 工况                 |
| ------ | -------------------- |
| OBC-01 | AC输入正常，输出空载 |
| OBC-02 | AC输入正常，低功率   |
| OBC-03 | AC输入正常，50%功率  |
| OBC-04 | AC输入正常，100%功率 |

检查：

```text
PFC启动
DC BUS
LLC启动
输出电压
输出电流
PWM
故障状态
```

------

### 静态负载测试

#### CC

```text
10%
20%
50%
75%
100%
110%（受保护条件下）
```

分别测试：

- 输出电压
- 输出电流
- 输入电流
- 效率
- PFC PF
- THD
- 温升
- 纹波

------

#### CV

```text
Vmin
Vnom
Vmax
```

例如某个平台：

```text
250V
400V
450V
```

验证：

```text
CV控制
电压精度
电流下降
环路稳定
```

------

#### CR

主要作为：

```text
动态负载
异常负载
启动特性
```

而不是作为动力电池的真实模型。

------

### 动态测试

比如：

```text
10A → 100A
100A → 10A
20A → 200A
200A → 20A
```

测试：

```text
ΔV
overshoot
undershoot
settling time
oscillation
```

电子负载的快速电流阶跃是测试电源环路和动态响应的典型方法。([Keysight United States](https://www.keysight.com/il/en/use-cases/perform-dc-load-mode-testing.html?utm_source=chatgpt.com))

------

### OBC的真实充电场景

这部分应该使用：**电池模拟器 / 实车动力电池**，而不是简单电子负载。

例如：

```text
SOC 10%
   ↓
CC 200A
   ↓
SOC 50%
   ↓
CC 200A
   ↓
SOC 80%
   ↓
BMS逐渐降低允许电流
   ↓
CV
   ↓
电流逐渐下降
   ↓
Charge Complete
```

这里可以测试：

- BMS限流
- BMS限压
- OBC与BMS通信
- CC/CV切换
- 动态电流限值
- 充电结束
- 故障退出
- 重新充电
- 充电暂停/恢复

------

## 七、DCDC与OBC负载区别

DCDC 和 OBC 最大的区别是：

```text
OBC：

AC → OBC → Battery
                 ↑
             主要负载


DCDC：

HV Battery → DCDC → 12V Battery + LV loads
                          │
              ┌───────────┼────────────┐
              ↓           ↓            ↓
             ECU         灯           风扇
```

所以 DCDC 面对的是：**“电池 + 大量电子设备组成的低压电网”。**

------

## 八、DCDC实际车载负载有哪些？

可以把汽车低压负载分成 6 类。

### 第一类：近似电阻型

例如：

- 加热器
- 电阻
- 某些灯丝负载

比较接近：CR

------

### 第二类：近似恒流

某些驱动器在特定工作区间可以近似：CC

例如某些电流控制型执行器。

------

### 第三类：恒功率

这是现代汽车电子非常重要的一类。

例如：

- ECU内部DC/DC
- ADAS计算平台
- Infotainment
- 显示系统
- 功放
- 某些电机控制器

其输入可能表现为：

```text
P ≈ Constant
```

所以更接近：CP

------

### 第四类：脉冲型负载

例如：

```text
继电器
电磁阀
电机
压缩机
泵
风扇
```

它们可能表现为：0A → 50A → 0A

这种负载对 DCDC 的环路、输出电容、MOSFET、磁性器件都很重要。

------

### 第五类：电池负载

DCDC输出端通常还连着：12V Battery

所以它本身就是一个动态储能单元。这意味着：**DCDC实际输出端不是简单电子负载。**而是：

```text
DCDC
 │
 ├── Battery
 │
 ├── ECU
 ├── ADAS
 ├── Lamp
 ├── Fan
 ├── Pump
 └── etc.
```

------

### 第六类：启动/唤醒类负载

这个对 DCDC 软件特别重要。

比如：

```text
Sleep
 ↓
KL15 ON
 ↓
大量ECU同时Wakeup
 ↓
瞬间负载增加
```

可能出现：

```text
2A
 ↓
20A
 ↓
80A
 ↓
150A
```

然后：

```text
部分ECU启动完成
 ↓
负载下降
```

这种工况电子负载可以**近似模拟电流波形**，但是不能完整模拟车载网络行为。

------

## 九、DCDC最小实验室测试集

建议做 DCDC 软件 bring-up 时直接按照这个矩阵走。

### 第一层：静态

| 用例 | 输入    | 输出负载 |
| ---- | ------- | -------- |
| D01  | Vin_min | 0%       |
| D02  | Vin_nom | 0%       |
| D03  | Vin_max | 0%       |
| D04  | Vin_min | 25%      |
| D05  | Vin_nom | 25%      |
| D06  | Vin_max | 25%      |
| D07  | Vin_min | 50%      |
| D08  | Vin_nom | 50%      |
| D09  | Vin_max | 50%      |
| D10  | Vin_min | 100%     |
| D11  | Vin_nom | 100%     |
| D12  | Vin_max | 100%     |

这个矩阵非常重要。

因为做 DCDC 控制设计时：Vin 和 Vo 都是动态的。

所以不能只测：

```text
400V → 12V → 100%
```

而应该至少覆盖：

```text
Vin_min
Vin_nom
Vin_max
```

×

```text
No load
25%
50%
75%
100%
```

------

### 第二层：CC负载

例如：

```text
10A
20A
50A
100A
150A
Imax
```

测试：

- 电压精度
- 电流精度
- 输出纹波
- 效率
- MOS温升
- 变压器温升
- SR波形
- Active Clamp波形
- 电流环
- 电压环

------

### 第三层：负载阶跃

这个是做软件示波器时特别值得记录的一类。

例如：

```text
10A ─────────────┐
                 │
                 └──────── 100A
```

然后：

```text
100A
 │
 │
 └────────────── 10A
```

重点测：$\Delta V$

以及：

```text
Rise Time
Fall Time
Overshoot
Undershoot
Settling Time
Ring Frequency
```

这些测试直接帮助判断：电压环带宽到底够不够。

------

### 第四层：CP负载

强烈建议增加：

```text
CP = 100W
CP = 500W
CP = 1kW
CP = 2kW
...
```

然后做：

```text
Vin变化
+
CP负载
```

例如：

```text
Vin = 250V → 400V → 500V

Pload = Constant
```

这比单纯 CC 更接近大量车载电子负载。

------

### 第五层：CR

用于验证：Rload变化

例如：

```text
轻载
中载
重载
```

观察：

```text
Vout
Iout
Pout
```

CR 对验证：

- 输出特性
- 启动
- 限流
- 输出短路趋势
- 稳态性能

很有价值。

------

### 第六层：蓄电池测试

这是非常关键的。

实验室可以：

```text
HV DC Supply
       ↓
     DCDC
       ↓
12V Battery
       ↓
Electronic Load
```

此时可以模拟：

```text
Battery + Vehicle Loads
```

例如：

```text
Battery = 12.0V
Load = 100A
```

然后：

```text
Battery = 11.5V
Load = 100A
```

测试 DCDC：

```text
低压输入/输出
限流
启动
恢复
电池充电
电池纹波
```

------

## 十、OBC和DCDC的最小用例矩阵

如果把它压缩成一个研发团队可以直接执行的版本：

| 测试类别      | OBC   | DCDC  |
| ------------- | ----- | ----- |
| 空载          | ✓     | ✓     |
| 10%负载       | ✓     | ✓     |
| 25%           | ✓     | ✓     |
| 50%           | ✓     | ✓     |
| 75%           | ✓     | ✓     |
| 100%          | ✓     | ✓     |
| Vin/Vac min   | ✓     | ✓     |
| Vin/Vac nom   | ✓     | ✓     |
| Vin/Vac max   | ✓     | ✓     |
| CC            | ★★★★★ | ★★★★★ |
| CV            | ★★★★★ | ★★★   |
| CR            | ★★★   | ★★★★  |
| CP            | ★★★   | ★★★★★ |
| Load Step     | ★★★★★ | ★★★★★ |
| Soft Start    | ★★★★★ | ★★★★★ |
| OCP           | ★★★★★ | ★★★★★ |
| OVP           | ★★★★★ | ★★★★★ |
| UVP           | ★★★★★ | ★★★★★ |
| OTP           | ★★★★★ | ★★★★★ |
| Short Circuit | ★★★★★ | ★★★★★ |
| 电池模拟器    | ★★★★★ | ★★★★  |
| 真实电池      | ★★★★★ | ★★★★  |
| BMS交互       | ★★★★★ | ★★★   |
| 车载网络负载  | ★★    | ★★★★★ |

这里的星号不是性能排名，而是**该工况作为研发测试项目的优先程度**。

------

## 十一、最关键的问题：电子负载和真实汽车负载到底差在哪里？

可以从五个维度理解。

------

### 1. 静态 I-V 特性不同

电子负载：

```text
CC：

I = Constant
```

汽车负载：

```text
ECU：
P ≈ Constant
```

所以：

```text
电子负载：

V ↓
I ≈ Constant


真实ECU：

V ↓
I ↑
```

这会导致 DCDC 控制器看到完全不同的：$Z_{load}=\frac{\Delta V}{\Delta I}$

------

### 2. 动态特性不同

电子负载：

```text
10A → 100A
```

可以做到非常干净。

但车上：

```text
ECU A启动
      ↓
ECU B启动
      ↓
继电器闭合
      ↓
风扇启动
      ↓
泵启动
```

是一个复杂的：

```text
随机 + 周期 + 状态相关
```

动态过程。

------

### 3. 真实汽车有储能元件

这是非常大的区别。

真实车上：

```text
DCDC
 │
 ├── Battery
 │
 ├── Cable L/R
 ├── ECU input C
 ├── DC/DC input C
 └── distributed capacitors
```

实验室：

```text
DCDC
 │
 └── Electronic Load
```

因此真实车上存在：

```text
L
C
ESR
Battery
Cable
Connector
```

共同组成的网络。

------

### 4. 电子负载没有“控制器之间的控制环”

真实车上：

```text
DCDC
 ↑ ↓
Battery
 ↑ ↓
BMS
 ↑ ↓
ECU
 ↑ ↓
CAN
```

每一个 ECU 都可能存在：

```text
内部DC/DC
电流环
电压环
限流
欠压保护
睡眠
唤醒
```

所以整个汽车低压电网实际上是：**多个闭环控制系统互相耦合。**

而普通电子负载只是：一个可编程受控负载

------

### 5. 真实汽车存在通信和状态机

这个对于做 OBC / DCDC 软件尤其重要。

例如 OBC：

```text
插枪
 ↓
CP检测
 ↓
BMS握手
 ↓
充电请求
 ↓
允许充电
 ↓
OBC启动
 ↓
CC
 ↓
CV
 ↓
充电结束
```

电子负载只能告诉 OBC：

```text
“我现在是400V、100A”
```

但是它无法天然告诉 OBC：

```text
SOC = 75%
电池温度 = 35℃
BMS允许电流 = 120A
BMS允许电压 = 448V
电池故障 = xxx
充电暂停
充电恢复
```

所以：**电子负载主要验证功率级和控制算法；真实电池/BMS才能验证完整充电系统。**

------

## 十二、哪些车上场景实验室很难真正复现？

这个问题非常重要。把它分成：

### A类：电子负载可以高度模拟

例如：

```text
稳态负载
CC
CR
CV
CP
负载阶跃
过载
短路
```

这些实验室完全可以做好。

------

### B类：电子负载可以近似模拟

例如：

```text
ECU启动
风扇启动
泵启动
继电器动作
ADAS负载变化
Infotainment负载变化
```

可以通过：

```text
Load Sequence
```

做：

```text
0A
 ↓
10A
 ↓
30A
 ↓
80A
 ↓
30A
 ↓
10A
```

但它只能模拟**电气结果**。

------

### C类：普通电子负载很难模拟

#### ① 真实动力电池

因为涉及：

```text
SOC
SOH
温度
OCV
内阻
极化
电芯一致性
BMS
```

------

#### ② BMS动态限流

例如：

```text
Icharge_max = 200A

温度升高

↓
Icharge_max = 150A

SOC升高

↓
Icharge_max = 100A
```

这需要电池模拟器/BMS闭环，而不是普通电子负载。

------

#### ③ 车辆整车唤醒

```text
KL15 ON

↓

BCM
VCU
BMS
OBC
DCDC
ADAS
IVI
Gateway
...
```

大量模块同时启动。

这种负载不是简单：100A step

而是：**软件状态机驱动的动态负载。**

------

#### ④ 车辆网络通信导致的负载变化

例如：

```text
CAN
LIN
Ethernet
```

通信状态变化：

```text
Sleep
 ↓
Wakeup
 ↓
Network Active
 ↓
High Traffic
 ↓
Sleep
```

对应 ECU 功耗变化。

电子负载只能模拟最后的：I(t)

却不能验证：通信 → ECU状态 → 功耗 → DCDC响应，这一整条链路。

------

#### ⑤ 冷启动 / 极端温度

这个也很重要。

例如 DCDC：-40℃

会同时影响：

```text
Battery ESR
MOS Rds_on
Transformer
Capacitor ESR
Gate Driver
MCU ADC
Current Sensor
```

实验室电子负载可以设置：100A

但无法单独通过电子负载模拟：**整车在 -40℃ 下电池 + 线束 + ECU + DCDC 的完整电气行为。**

------

#### ⑥ 线束压降

真实车辆：

```text
DCDC
 │
 ├──── 1m cable ──── ECU
 │
 ├──── 2m cable ──── Fan
 │
 └──── 3m cable ──── Battery
```

存在：$V_{drop}=I R_{cable}$

同时还有：$V=L\frac{di}{dt}$

所以大电流快速变化时会产生明显的：Voltage Spike

实验室如果：DCDC → 很短的铜排 → Electronic Load

那么测出来的结果往往比整车“干净”。

------

## 十三、这也是为什么实验室测试经常“很好看”，上车却出问题

例如实验室：

```text
DCDC
 ↓
Electronic Load

100A step

Vout：
12.00
11.98
12.01
12.00
```

看起来非常漂亮。

上车：

```text
DCDC
 ↓
1.5m Cable
 ↓
Fuse
 ↓
Connector
 ↓
Busbar
 ↓
ECU
 ↓
ECU内部DC/DC
```

可能变成：

```text
12.0V
 ↓
11.3V
 ↓
12.4V
 ↓
11.6V
 ↓
12.0V
```

因为系统已经不是：**DCDC + Load**

而是：**DCDC + Battery + Cable + Fuse + Connector + Multiple Loads + Multiple Control Loops**

------

## 十四、把测试分成“三层”

这个非常适合目前 OBC/DCDC 软件开发流程。

```text
                 软件/功率开发验证
                       │
          ┌────────────┼────────────┐
          │            │            │
       Level 1       Level 2      Level 3
       台架器件       半实物        整车
          │            │            │
       电子负载       电池模拟器     实际电池
          │            │            │
       CC/CV/CR/CP   BMS模拟       实车BMS
          │            │            │
       Load Step      动态SOC       整车负载
          │            │            │
       环路验证       通信验证       整车状态机
          │            │            │
       功率级         系统级         整车级
```

------

## 十五、对应 OBC 软件开发流程

可以直接建立：

```text
OBC Level 1

① PWM
② ADC
③ 电流环
④ 电压环
⑤ PFC
⑥ LLC/CLLC
⑦ CC
⑧ CV
⑨ CC→CV
⑩ OCP/OVP/OTP
⑪ Load Step
⑫ Efficiency
```

然后：

```text
OBC Level 2

① BMS模拟
② SOC
③ 电池温度
④ 允许充电电流
⑤ 允许充电电压
⑥ CC/CV
⑦ BMS限流
⑧ BMS停止
⑨ BMS故障
⑩ CAN通信
```

最后：

```text
OBC Level 3

① 插枪
② CP/CC
③ BMS
④ VCU
⑤ OBC
⑥ DCDC
⑦ 整车网络
⑧ 整车充电状态机
```

------

## 十六、DCDC则可以建立另一套

### Level 1：功率级

```text
Vin
 ↓
DCDC
 ↓
Electronic Load
```

测试：

```text
No Load
CC
CR
CP
Load Step
OVP
OCP
OTP
Short
Startup
Shutdown
```

------

### Level 2：电池 + 电子负载

```text
HV DC Source
      ↓
    DCDC
      ↓
12V Battery
      ↓
Electronic Load
```

验证：

```text
Battery charging
Battery discharge
Load transient
Battery voltage variation
DCDC current limit
```

------

### Level 3：整车负载

```text
              ┌── ECU
              │
DCDC ── LV Bus ├── ADAS
              │
              ├── Fan
              │
              ├── Pump
              │
              ├── Lamp
              │
              └── Battery
```

再加入：

```text
KL15
CAN Wakeup
NM
Sleep
Wakeup
Fault
Derating
```

这才是整车级 DCDC 验证。

------

## 十七、最后给一个非常实用的判断口诀

以后遇到一个车上工况，先问四个问题：

### 第一问

> **这个负载的 I-V 特性是什么？**

```text
CC？
CV？
CR？
CP？
```

### 第二问

> **这个负载是静态还是动态？**

```text
Static
Step
Pulse
Ramp
Random
```

### 第三问

> **这个负载后面有没有自己的控制环？**

如果有：

```text
ECU
DC/DC
Motor Controller
Inverter
```

就不能简单当成 CR。

### 第四问

> **这个负载是不是和电池、线束、BMS、CAN状态耦合？**

如果是：电子负载只能验证局部电气特性，必须进一步进入 HIL / 电池模拟器 / 半实物 / 整车测试。

------

## 十八、 OBC / DCDC 的“负载真实性”最终理解

```text
                    真实性
                       ↑
                       │
             ┌─────────┴─────────┐
             │                   │
        电子负载               实际车辆
             │                   │
        CC/CV/CR/CP          Battery
             │                   │
        Load Step             BMS
             │                   │
        功率级验证            Cable
             │                   │
        环路验证              ECU
             │                   │
        保护验证              CAN
             │                   │
             └───────→───────────┘
                    差异越来越大
```

**所以实验室电子负载不是“模拟不了汽车”，而是它模拟的是汽车负载的某一个电气维度。**