# ISR

不要只从“中断是什么”来理解，而是从 **CPU执行上下文、OS调度、ISR、时间基准、多核同步** 四条线一起看。

```text
外设事件
  │
  ├── CAN RX/TX/ERR
  ├── ADC EOC
  ├── GPT/TIM
  ├── PWM/CCU
  ├── DMA
  ├── GPIO
  └── 软件触发
        │
        ▼
┌──────────────────┐
│ 中断控制器         │  ← 优先级/屏蔽/路由/挂起
└────────┬─────────┘
         │
         ▼
     指定CPU Core
         │
         ▼
   保存当前执行上下文
         │
         ▼
      进入ISR
         │
         ├── ISR处理
         ├── 清中断源
         └── 必要时唤醒Task
         │
         ▼
     ISR返回/调度
         │
         ▼
 ┌─────────────────────┐
 │ 原Task继续执行        │
 │ 或切换到更高优先级Task │
 └─────────────────────┘
```

最重要的一点先纠正：**发生中断时，不是“CPU所有核都卡住执行中断程序”。通常是某一个核响应这个中断，其他核继续运行自己的代码。**

## 1. 中断到底应该怎么分类？

### 1.1 按 AUTOSAR/软件处理方式分类：Cat1 / Cat2

车载软件时最值得掌握的分类。

#### 1.1.1 Category 1 ISR

Cat1 ISR：

```text
硬件中断
   ↓
ISR
   ↓
直接执行
   ↓
ISR结束
```

特点：

- 没有OS调度语义
- 通常执行时间短
- 一般不涉及Task调度
- ISR中不能随意调用OS服务
- 常用于非常底层、非常实时的中断

例如：

```text
GPT底层快速中断
MCU某些硬件异常
非常底层的周期触发
```

#### 1.1.2 Category 2 ISR

Cat2 ISR：

```text
硬件中断
   ↓
OS识别
   ↓
进入ISR2
   ↓
执行
   ↓
可能触发Task
   ↓
ISR退出
   ↓
OS Scheduler判断是否需要切Task
```

它和Cat1最大的区别是：**Cat2属于OS管理范围，可以与Task调度产生关系。**

典型：

```text
CAN ISR
ADC ISR
PWM ISR
GPT ISR
DMA ISR
```

比如：

```c
ISR(CanRx_Isr)
{
    Can_ReadRxBuffer();

    SetEvent(CanTask, CAN_RX_EVENT);
}
```

ISR本身只做快速处理：

```text
接收硬件数据
↓
读取寄存器/FIFO
↓
清中断
↓
通知Task
```

真正复杂的软件逻辑：

```text
CAN协议处理
诊断
信号解析
状态机
```

放到Task。

### 1.2 按“中断来源”分类

| 类型      | 来源        | 典型例子         |
| --------- | ----------- | ---------------- |
| Timer/GPT | 定时器      | 1ms、10ms        |
| PWM       | PWM模块     | PWM周期、ADC SOC |
| ADC       | ADC转换完成 | ADC EOC          |
| CAN       | CAN外设     | RX/TX/ERR        |
| LIN       | LIN模块     | RX/TX            |
| SPI       | SPI外设     | RX/TX            |
| DMA       | DMA完成     | ADC DMA完成      |
| GPIO      | 外部引脚    | Wakeup、Fault    |
| Capture   | 捕获单元    | 转速、频率       |
| Compare   | 比较单元    | 定时事件         |
| Software  | 软件触发    | SW interrupt     |
| Fault     | 硬件异常    | ECC、Bus Fault等 |

所以：

```text
中断
├── Timer/GPT
├── PWM
├── ADC
├── CAN
├── SPI
├── DMA
├── GPIO
├── Capture
├── Compare
├── Software
└── Hardware Fault
```

### 1.3 按优先级分类

还可以从“谁能打断谁”来分类。

例如：

```text
最高
 │
 ├── NMI
 │
 ├── Hard Fault / Safety Fault
 │
 ├── ADC/PWM高速控制ISR
 │
 ├── CAN ISR
 │
 ├── Timer ISR
 │
 └── 普通后台ISR
 │
最低
```

但是具体优先级**不是固定标准**，由MCU、OS和项目配置决定。

例如DCDC：

```text
100kHz PWM/ADC ISR
       ↑
       │ 最高实时性
       │
       ├── 电流采样
       ├── PI
       ├── PWM更新
       └── 保护判断

1ms Task
       ↑
       ├── 状态机
       ├── 电压管理
       └── 功率控制

10ms Task
       ↑
       ├── 诊断
       └── 状态处理

CAN ISR
       ↑
       └── 收数据/通知Task
```

### 1.4 NMI应该单独理解

NMI：Non-Maskable Interrupt，不可屏蔽中断。

它和普通：

```text
CAN ISR
ADC ISR
Timer ISR
```

不是一个层级的概念。

可以理解成：

```text
普通IRQ
    ↓
可以被屏蔽
    ↓
可以被更高优先级中断抢占


NMI
    ↓
特殊异常通道
    ↓
普通IRQ屏蔽通常无法阻止
```

汽车MCU里NMI经常用于：

```text
严重硬件异常
安全监控
时钟异常
某些ECC/Memory异常
外部安全监控
```

具体哪些事件进入NMI，取决于MCU架构。



## 2. 最重要的问题：中断发生后，是所有CPU核都停下来吗？

**不是。**

假设是：

```text
TC377
Core0
Core1
Core2
```

现在CAN中断发生。

可能配置成：

```text
CAN RX
   ↓
Interrupt Router
   ↓
Core1
```

那么：

```text
Core0                  Core1                 Core2
 │                       │                     │
Task_A                   Task_B                Task_C
 │                       │                     │
运行                     CAN IRQ               运行
 │                       ↓                     │
 │                    ISR_CAN                 │
 │                       │                     │
继续运行                  ↓                     继续运行
                     ISR结束
                         │
                         ↓
                      Task_B
```

只有：**被路由到的那个Core响应这个中断。**

### 2.1 多核为什么能做到？

因为多核CPU实际上有：

```text
Core0
 ├──自己的PC
 ├──自己的寄存器
 ├──自己的执行上下文
 └──自己的中断响应

Core1
 ├──自己的PC
 ├──自己的寄存器
 ├──自己的执行上下文
 └──自己的中断响应

Core2
 ├──自己的PC
 ├──自己的寄存器
 ├──自己的执行上下文
 └──自己的中断响应
```

所以：Core0 ≠ Core1 ≠ Core2

不是3个核共用一个PC，而是三个独立执行单元。

### 2.2 那中断是怎么找到Core的？

中间通常存在：

```text
Peripheral
    │
    ▼
Interrupt Controller / Router
    │
    ├── Core0
    ├── Core1
    └── Core2
```

例如：

```text
ADC结果完成
    ↓
ADC IRQ
    ↓
Interrupt Router
    ↓
Core0
    ↓
ADC_ISR()
```

而：

```text
CAN RX
    ↓
CAN IRQ
    ↓
Interrupt Router
    ↓
Core1
    ↓
CAN_ISR()
```

因此多核系统开发非常重要的一件事情就是：**Interrupt Affinity / Interrupt Routing**

也就是：这个中断到底分配给哪个Core。



## 3. “任务挂起 → ISR → 任务下放”，准确过程是什么？

这个地方容易产生一个误解。

严格来说不是：

```text
Task挂起
↓
ISR
↓
Task下放
```

而是：

```text
Task正在运行
      ↓
硬件产生IRQ
      ↓
CPU响应IRQ
      ↓
保存当前执行上下文
      ↓
进入ISR
      ↓
ISR执行
      ↓
ISR结束
      ↓
恢复上下文
      ↓
调度器判断
      ↓
继续原Task
      OR
切换到其他Task
```

例如：

```text
Core0

Task_A
│
│ 正在执行
│
├──────────────┐
│              │
│          CAN IRQ
│              ↓
│         保存上下文
│              ↓
│          CAN_ISR
│              ↓
│         清中断源
│              ↓
│          ISR结束
│              ↓
│       Scheduler判断
│              │
│        ┌─────┴─────┐
│        ↓           ↓
│      Task_A      Task_B
│       继续         执行
```

**ISR结束以后，不一定回原来的Task**

这是非常关键的。

假设：

```text
Task_A：低优先级
Task_B：高优先级
```

Task_A运行：

```text
Task_A
   ↓
CAN ISR
   ↓
ISR中SetEvent(Task_B)
   ↓
ISR结束
```

OS发现：

```text
Task_B ready
Task_B priority > Task_A
```

于是：

```text
Task_A
   ↓
Preempt
   ↓
Task_B
```

所以：ISR结束以后，是恢复原Task还是切换Task，取决于调度器。



## 4. ISR和Task到底是什么关系？

这是做AUTOSAR时非常核心的概念。

可以把它理解成：

```text
硬件事件
    ↓
ISR
    ↓
“快处理”
    ↓
通知
    ↓
Task
    ↓
“慢处理”
```

例如CAN：

```text
CAN RX
 ↓
CAN ISR
 ↓
读取硬件FIFO
 ↓
保存数据
 ↓
通知Can_MainFunction
 ↓
ISR结束
 ↓
Can Task
 ↓
协议栈
 ↓
PduR
 ↓
Com
 ↓
Application
```

所以ISR最好：**短、快、确定。**

不要在ISR里做：

```c
ISR()
{
    ComplexStateMachine();

    HugeCalculation();

    while(...);

    printf(...);

    LongDiagnosticProcess();
}
```

尤其DCDC：100kHz ADC/PWM ISR 更应该严格控制。



## 5. 中断占用率怎么计算？

这个非常重要。

定义：ISR占用率 = ISR实际执行时间 / 测量时间窗口

例如：

```text
100ms测试窗口

ISR总执行时间 = 8ms
```

那么：$CPU_{ISR}= \frac{8ms}{100ms}=8\%$

### 5.1 单个ISR的占用率

例如：

```text
ADC ISR
执行时间 = 2 μs
触发频率 = 100kHz
```

那么：$T_{ISR}=2\mu s$

每秒：$100000\times2\mu s=0.2s$

所以：$CPU_{ADCISR}=20\%$

这意味着：ADC ISR本身就吃掉一个Core大约20%的计算时间。

### 5.2 多个ISR

假设：

| ISR  | 频率   | 单次时间 |
| ---- | ------ | -------- |
| ADC  | 100kHz | 2μs      |
| PWM  | 100kHz | 1μs      |
| CAN  | 2kHz   | 5μs      |
| GPT  | 1kHz   | 3μs      |

计算：

ADC：$100000\times2\mu s=20\%$

PWM：$100000\times1\mu s=10\%$

CAN：$2000\times5\mu s=1\%$

GPT：$1000\times3\mu s=0.3\%$

所以ISR理论占用：$20+10+1+0.3=31.3\%$

但实际系统还要考虑：

```text
ISR嵌套
ISR抢占
Cache
Flash wait
中断进入/退出开销
OS开销
临界区
```

所以实际测量可能不是简单相加。



## 6. ISR占用率和CPU负载率有什么区别？

这是两个概念。

### CPU负载率

整个CPU忙碌程度：$CPU Load= \frac{CPU Busy Time}{Total Time}$

包括：

```text
ISR
Task
OS
调度
驱动
后台任务
```

所以：$CPU Load = ISR Load + Task Load + OS Load + ...$

例如：

```text
ISR       25%
Task      35%
OS         5%
其他       5%
----------------
CPU Load  70%
```

所以：**ISR占用率是CPU负载率的一部分。**

**一个非常典型的汽车电源例子**

假设DCDC Core：

```text
100kHz ADC ISR       20%
100kHz PWM ISR        5%
CAN ISR               1%
GPT ISR               1%

DCDC Control Task     15%
PDU Task               8%
Diagnostic Task        5%
OS                     5%
```

那么：

```text
ISR Load = 27%

Task Load = 28%

OS/Other = 5%

CPU Load = 60%
```

所以不能看到ISR = 27%，就认为CPU = 27%



## 7. 实际项目怎么测试ISR占用率？

TC377 / F29P32 / SPC58，可以采用三种方法。

### 方法1：CPU Cycle Counter

最准确。

进入ISR：

```c
start = CPU_CycleCounter();
```

退出：

```c
end = CPU_CycleCounter();

delta = end - start;
```

得到：$T_{ISR}=\frac{Cycle}{F_{CPU}}$

例如：

```text
CPU = 200MHz
ISR = 400 cycles
```

则：$T=\frac{400}{200M}=2\mu s$

### 方法2：GPIO打点 + 示波器

这是作为电源软件工程师非常推荐的方法。

ISR开始：

```c
GPIO_SET(DEBUG_PIN);
```

ISR结束：

```c
GPIO_CLR(DEBUG_PIN);
```

示波器看到：

```text
        ┌──────┐
        │      │
────────┘      └────────
        ← 2us →
```

那么：ISR执行时间 = 2μs

如果100kHz：周期 = 10μs

那么：ISR占用率 = 2/10 = 20%

对于：

```text
PFC
CLLC
DCDC
```

这种控制软件，这种方式特别直观。

### 方法3：Trace

例如：

```text
MCDS
Trace
ETM
OS Trace
Debugger Profiler
```

可以统计：

```text
ISR名称
进入次数
平均执行时间
最大执行时间
最小执行时间
CPU占用
```

最终得到：

| ISR     | Count  | Avg   | Max   | CPU  |
| ------- | ------ | ----- | ----- | ---- |
| ADC_ISR | 100000 | 2.0μs | 2.4μs | 20%  |
| PWM_ISR | 100000 | 1.0μs | 1.3μs | 10%  |
| CAN_ISR | 2000   | 5μs   | 8μs   | 1%   |

这比单纯看平均值更有意义。



## 8. 为什么“最大ISR时间”比平均ISR时间还重要？

例如：

```text
ADC ISR

平均：2μs
最大：8μs
```

可能觉得：

```text
平均才2μs
没问题。
```

但如果：

```text
PWM周期 = 10μs
```

偶尔出现：

```text
8μs ISR
```

可能导致：

```text
ADC ISR
    ↓
CAN ISR无法及时响应
    ↓
其他ISR延迟
    ↓
Task延迟
```

所以实时系统不仅看：$Average\ CPU\ Load$

还要看：$Worst\ Case\ Execution\ Time$

即：**WCET：Worst Case Execution Time**

这在功能安全和电源控制里非常重要。



## 9. 最后一个问题：ISR发生时，系统时钟还会不会累加？

会。

这个一定要区分：**硬件时钟** 和 **CPU软件执行**

### 9.1 CPU时钟不会因为ISR暂停

例如：

```text
CPU = 200MHz
```

一个ISR运行：

```text
2μs
```

这2μs里面：

```text
CPU clock
↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
200MHz持续运行
```

不是：

```text
Task
 ↓
暂停CPU clock
 ↓
ISR
 ↓
恢复CPU clock
```

而是：

```text
CPU一直运行
│
├── Task
│
├── ISR
│
└── Task
```

### 9.2 Timer也通常继续计数

假设：

```text
GPT Timer = 1MHz
```

：

```text
0us
 ↓
Task
 ↓
ISR开始
 ↓
ISR运行20us
 ↓
ISR结束
```

Timer：

```text
0
1
2
3
...
20
...
```

通常不会因为CPU正在ISR里执行就自动停止。

因此：

```c
start = Timer_Get();
ISR();

elapsed = Timer_Get() - start;
```

仍然能够得到：

```text
ISR执行期间经过的时间
```



## 10. 什么东西是不容易被ISR“卡住”的？

这是理解嵌入式实时系统最关键的地方。

硬件独立运行

例如：

```text
GPT Timer
PWM Timer
ADC采样硬件
DMA
CAN Controller
Capture/Compare
```

这些外设有自己的硬件状态机。

CPU正在：

```text
ISR
```

它们仍然可以继续运行。

例如：

```text
PWM
 ↓
ADC SOC
 ↓
ADC转换
 ↓
DMA搬运
 ↓
下一次PWM
```

整个过程不一定需要CPU每一步都参与。

这就是为什么现代MCU适合做电源控制。



## 11. DMA尤其重要

例如：

```text
ADC
 ↓
DMA
 ↓
RAM
```

CPU：

```text
正在执行ISR
```

同时：

```text
ADC → DMA → RAM
```

仍然可以进行。

完成后：

```text
DMA Complete IRQ
```

再通知CPU。

所以可以形成：

```text
ADC硬件
   ↓
DMA
   ↓
RAM
   ↓
CPU ISR
```

而不是：

```text
ADC
 ↓
CPU
 ↓
CPU搬数据
 ↓
CPU
```

这样CPU负担会小很多。



## 12. 但要注意：中断会“卡住CPU执行流”，不会“卡住整个芯片”

这是最准确的一句话。

例如：

```text
Core0：

Task
 ↓
ISR
 ↓
Task
```

Core0的正常Task执行流被打断。

但是：

```text
Core1
```

可能完全正常。

同时：

```text
GPT
PWM
ADC
DMA
CAN Controller
```

这些硬件模块仍然可能继续工作。

因此应该理解成：**ISR打断的是某个CPU Core上的软件执行流，而不是让整个SoC/MCU停止运行。**



## 13. 车载软件，可以建立这个最终模型

分析任何中断，都按照下面这张表去看：

| 层级                 | 要问的问题                  |
| -------------------- | --------------------------- |
| Hardware             | 谁产生了事件？              |
| Interrupt Controller | IRQ是谁接收？               |
| Core                 | 分配给哪个Core？            |
| Priority             | 优先级是多少？              |
| ISR                  | ISR执行什么？               |
| OS                   | Cat1还是Cat2？              |
| Task                 | ISR是否唤醒Task？           |
| Scheduler            | ISR结束后是否发生Task切换？ |
| Timing               | ISR最长执行多久？           |
| Load                 | ISR占多少CPU？              |
| Peripheral           | ISR期间外设是否继续运行？   |
| Multi-core           | 其他Core是否继续运行？      |
| Safety               | ISR超时/嵌套/丢中断怎么办？ |

