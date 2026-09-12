# PowerScope Protocol v1

## Header
| Byte | Length | Meaning |
|---|---:|---|
| 0..1 | 2 | 0xAA 0x55 |
| 2..3 | 2 | Sequence |
| 4..5 | 2 | Channel count |
| 6..7 | 2 | Sample count |
| 8..11 | 4 | Timestamp us |

Payload:
`sample0[ch0..chN-1], sample1[ch0..chN-1] ...`

当前版本统一采用float32，优先保证PC端简单解析。
量产/高速版本建议增加：
- channel ID
- sample period
- trigger position
- CRC16/CRC32
- module ID
- signal engineering unit
- scaling
- compression
- dropped-frame counter

## 带宽估算
例如 8通道 × 20kS/s × 4Byte = 640kB/s，仅适合高速UART/USB/Ethernet/CAN-FD分帧上传。
因此推荐：
- ADC/控制环：10~100kHz采样
- MCU本地RAM缓存
- PC上传：按触发抓取后的窗口数据，而不是连续上传所有原始点
