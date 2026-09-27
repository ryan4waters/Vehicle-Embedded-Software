# TC377 + TJA1145 CAN 通讯诊断实例

这是一个 AUTOSAR Classic 风格的参考工程，重点展示 TC377 + TJA1145 下：Bus-Off、Error Passive、TJA1145 收发器故障、Rx Timeout、Tx Confirmation Timeout、NM Timeout、DEM/DTC，以及故障恢复。

> 不是供应商 MCAL/BSW 的替代品。生产工程应把 Adapter 层替换为实际 Can/CanIf/CanSM/Com/CanNm/DEM/DCM/EcuM API；TJA1145 寄存器必须按实际 TJA1145/TJA1145A/FD 器件 datasheet 校核。

## 故障链
```text
MCMCAN -> CanDrv -> CanIf -> CanSM -> Bus-Off recovery
TJA1145 -> SPI -> Tja1145_Drv -> CanDiag -> DEM
CAN Rx -> CanIf -> PduR -> COM -> Rx Deadline -> DEM
CAN NM -> CanIf -> CanNm -> Nm -> ComM
DEM -> DCM -> UDS 0x19
```

AUTOSAR CanIf 会通过 `CanIf_ControllerBusOff()` 向 CanSM 通知控制器 Bus-Off；CanSM 再按配置执行 Bus-Off recovery。TJA1145 支持 SPI 诊断、TXD dominant timeout、过温、欠压以及本地/远程唤醒。生产项目需依据具体 AUTOSAR release 和器件版本配置。 

## 10ms任务
```c
CanSM_MainFunction_10ms();
Com_MainFunction_10ms();
CanNm_MainFunction_10ms();
CanDiag_MainFunction_10ms();
Dem_MainFunction_10ms();
```

## Bus-Off
```text
MCMCAN Bus-Off IRQ
 -> CanDrv
 -> CanIf_ControllerBusOff
 -> CanSM_ControllerBusOff
 -> Controller STOP
 -> recovery timer
 -> Controller START
 -> successful TX
 -> DEM event PASS
```

## Rx Timeout
```text
PDU收到 -> Com_RxIndication -> timer=0
无PDU -> timer += 10ms
>= timeout -> DEM FAILED
恢复收到 -> DEM PASSED
```

## TJA1145
Driver 只负责 SPI、模式、状态和唤醒；不要把应用降级策略塞进 driver。诊断层读取欠压、过温、TXD dominant timeout 等状态并交给 DEM。

## DTC
实例用 `FAILED/PASSED` 演示。真实 DEM 还应配置 debounce、operation cycle、pending/confirmed、healing、aging 等状态。

## 测试
1. 正常通信
2. 无 ACK -> TEC 增加 -> Error Passive/Bus-Off
3. Bus-Off recovery
4. BMS_Status 停止 -> Rx Timeout
5. NM 停止 -> NM Timeout
6. TJA1145 欠压/过温/TXD dominant timeout
7. 受控 CANH/CANL 物理故障
