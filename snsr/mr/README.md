# MR Angle Sensor Software Package

适用于汽车电机/执行器的模拟 SIN/COS MR（AMR/GMR/TMR）角度传感器软件框架。
默认假设：
- MCU ADC 同步采样 SIN/COS；
- 传感器输出经过差分/单端模拟前端；
- 角度计算采用 atan2；
- 支持 offset、gain、正交误差、零位 offset、方向、双通道一致性诊断；
- MCU HAL 与算法解耦，可适配 TC377 / F29P32 / SPC58NN。

## 软件链路
ADC -> raw -> electrical calibration -> normalized SIN/COS
-> atan2 -> unwrap -> mechanical zero offset -> angle/speed
-> plausibility/diagnostic -> application

## 典型参数
TMR/GMR/AMR 角度传感器通常提供正交 SIN/COS。实际器件参数必须以具体 datasheet 为准。
以 TLE5502D 为例，器件输出差分 SIN/COS，360°测量，且其 datasheet 明确要求 MCU 外部实现诊断。 

## 标定
1. 机械夹具将转子旋转至少 1~2 圈；
2. 采集 SIN/COS min/max；
3. 计算 offset；
4. 计算 amplitude；
5. 根据需要增加 orthogonality/phase correction；
6. 保存 NVM；
7. 机械零位在已知转子参考位置执行 zero learning；
8. 运行时加载参数并做 plausibility check。

## 安全注意
本包是通用算法骨架，不等同于某个 OEM 的 ISO 26262 safety implementation。
量产项目必须结合具体传感器 safety manual、磁铁/磁路公差、ADC诊断、MCU ADC自检、供电诊断、冗余通道和系统安全目标进行设计。
