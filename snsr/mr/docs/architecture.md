# 工程架构

## 1. 推荐分层

MR_HwIf
  ↓
MR_AngleSensor
  ↓
MR_Calibration
  ↓
MR_Diagnostic
  ↓
MR_Nvm
  ↓
Motor Position Manager
  ↓
FOC / commutation / application

## 2. ADC设计

建议：
- SIN/COS 使用同一 ADC group 或同步触发；
- DMA 搬运成固定结构；
- ISR/DMA callback 只做采样帧发布，不做 atan2；
- 控制任务中执行角度计算；
- 高速 FOC 则可把角度计算放到 PWM/ADC 同步控制 ISR；
- ADC 原始码先进行电气标定，再进入 atan2。

## 3. 诊断树

SensorPower
 ├─ VDD low/high
 └─ GND/reference abnormal

Signal
 ├─ SIN out-of-range
 ├─ COS out-of-range
 ├─ stuck-at
 └─ ADC plausibility

Magnetic
 ├─ vector magnitude too low
 ├─ vector magnitude too high
 ├─ eccentricity / ellipse
 └─ external magnetic disturbance

Angle
 ├─ jump
 ├─ overspeed
 ├─ direction mismatch
 └─ redundant-channel mismatch

Calibration
 ├─ CRC
 ├─ range
 ├─ version
 └─ zero-learning validity

## 4. 零位学习

典型流程：
1. 电机进入安全静止状态；
2. 施加已知机械参考位置/电角度；
3. 等待转子稳定；
4. 连续采样 N 点；
5. 对角度做 circular mean；
6. 与参考角比较；
7. 若误差超过阈值则学习失败；
8. 计算 zero_offset；
9. CRC 后写 NVM；
10. 重启/重新加载验证。

## 5. 高精度版本

如果仅做 offset + gain：
    x = (X-Xoff)/Ax
    y = (Y-Yoff)/Ay
    theta = atan2(y,x)

如果存在明显椭圆/非正交误差：
    [x';y'] = M * ([x;y]-offset)

M 可由工装采集 360°参考角后通过最小二乘/椭圆拟合获得。
对于高安全等级项目，建议把传感器双通道独立处理，再进行角度差异诊断，而不是先融合。
