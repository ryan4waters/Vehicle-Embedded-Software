# PowerScope v1

车载电源软件示波器第一版参考实现，定位为“MCU应用层调试组件”。
目标：PFC / CLLC / DCDC 共用一套上层接口，TC377 / TI F29P32x / SPC58NN 通过平台适配层接入。

## 核心能力
- 运行时注册最多 32 个波形通道
- 通道类型：float32 / int32 / uint32 / bool
- 周期采样、软件分频/抽取
- 环形缓冲
- 单通道触发：RISING / FALLING / ABOVE / BELOW
- Pre-trigger + Post-trigger
- 一次采集 / 连续采集
- 快照打包与传输接口抽象
- 统计量：min / max / avg / RMS / peak-to-peak
- PFC / CLLC / DCDC 示例信号配置
- MCU平台完全隔离：PowerScope_Platform.h 只定义接口

## 推荐部署
PowerScope_Task() 放在固定周期任务中，例如 50us / 100us / 1ms。
真正高频的ADC/PWM原始采样不要依赖普通任务调度，建议在ADC DMA完成或PWM同步ADC ISR中把结果写入应用变量，再由 PowerScope_SampleISR() 快速采样。

## 三类MCU适配
- TC377：GTM/EVADC/CPU Timer/SCU等硬件细节放在 powerscope_tc377_port.c
- F29P32x：ePWM/ADC/DMA等硬件细节放在 powerscope_f29p32_port.c
- SPC58NN：eTimer/ADC/eMIOS/DSPI等硬件细节放在 powerscope_spc58nn_port.c

本包不绑定具体MCAL/SDK，因此可以接 AUTOSAR、iLLD、TI DriverLib 或 SPC5 SDK。

## 数据通道设计
建议应用层先提供“工程信号”：
Vbus, Vout, Iin, Iout, Vref, Duty, PwmFreq, PFC_I, LLC_I, DCDC_I, Temp...
而不是让示波器直接访问寄存器。

## 生产代码注意
- Scope只建议在 DEBUG/DEVELOPMENT 编译开关下启用
- 不要在高速ISR里做浮点统计、printf、动态内存
- 环形缓冲使用静态内存
- 若需要ASILD量产版本，必须单独进行资源、时序、Freedom From Interference和安全分析
