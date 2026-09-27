# 测试用例

1. 正常通信：所有诊断 PASS。
2. 无 ACK：观察 ACK Error、TEC、Error Passive、Bus-Off。
3. Bus-Off：验证 CanSM stop/recovery/start。
4. Rx Timeout：停止 BMS_Status 100 ms 后应 FAILED，恢复后 PASSED。
5. Tx Confirmation Timeout：不产生 Tx confirmation，超过 50 ms 应 FAILED。
6. NM Timeout：停止 NM，300 ms 后应 FAILED。
7. TJA1145 欠压/过温/TXD dominant timeout：SPI 状态应映射到 DEM。
8. CANH/CANL 故障：在受控实验台观察错误计数器和 Bus-Off。
