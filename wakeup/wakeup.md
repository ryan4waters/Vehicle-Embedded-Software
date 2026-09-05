# ECU WakeUp

Architecture: Infineon TC377 + TLF35584 + TJA1145

## 硬件唤醒阶段

- **唤醒事件检测**
  SBC（System Basis Chip）检测到外部唤醒源（如CAN报文、LIN信号、IGN点火信号等）。
- **INH引脚使能电源**
  SBC的INH引脚输出高电平，使能外部稳压器（LDO/DCDC）给MCU供电。MCU电源上电，POR（上电复位）释放。
- **MCU复位释放**
  MCU内核复位释放，CPU从复位向量地址开始执行代码。

## MCU启动代码（Boot ROM & SSW）

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

## C运行时初始化与早期初始化

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

## 进入EcuM主函数

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

## 关于唤醒原因的区分

在EcuM初始化前后，系统可能检查唤醒源，以便决定进入何种运行模式（如正常启动、快速唤醒、睡眠唤醒等）。这通常通过读取SBC的中断寄存器或MCU的唤醒标志实现，但**不影响**从复位到`EcuM_Init`的基本启动流程。
