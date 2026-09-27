# 关键调用链

## Bus-Off
```text
MCMCAN ISR -> CanDrv -> CanIf_ControllerBusOff -> CanSM_ControllerBusOff
-> Controller STOP -> recovery -> START -> successful TX -> DEM PASS
```

## RX Timeout
```text
CAN -> CanIf_RxIndication -> PduR/COM -> deadline monitor -> DEM
```

## TJA1145
```text
TJA1145 -> SPI -> Tja1145_Drv -> CanDiag -> DEM
```

## NM
```text
CAN -> CanIf -> CanNm -> Nm -> ComM
```
