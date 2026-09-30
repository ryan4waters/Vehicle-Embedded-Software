# UDS

UDS协议贯穿车载电源ECU全生命周期，各阶段侧重点不同：

- **研发**：侧重灵活读写、控制，方便调试。
- **供应商产线**：侧重刷写、配置、EOL测试。
- **OEM产线**：侧重整车集成配置、功能验证、基础诊断。
- **售后维护**：侧重故障诊断、数据监控、软件升级。

## 1 研发调试阶段

**目标**：快速验证硬件功能、调试软件逻辑、标定参数。

**常用UDS服务**：

- `0x10` 诊断会话控制：切换到扩展会话/编程会话，解锁特殊功能。
- `0x27` 安全访问：解锁受保护的服务（如写入标定数据、控制IO）。
- `0x22` 按标识符读取数据：读取内部变量、状态、标定值。
- `0x2E` 按标识符写入数据：修改标定参数、临时覆盖配置。
- `0x31` 例程控制：启动特定测试例程（如自检、PWM输出测试、ADC采样）。
- `0x34/0x36/0x37` 数据传输：用于调试阶段烧写小段代码或参数（也可用0x2E代替）。
- `0x3E` 待机握手：保持诊断会话活跃。

**代码示例**（以C语言实现UDS服务处理框架）：

```c
// UDS服务分发函数（简化版）
void UDS_MainFunction(void) {
    uint8_t sid = received_msg[0];  // 服务ID
    switch (sid) {
        case 0x10: UDS_Service10_DiagnosticSessionControl(); break;
        case 0x27: UDS_Service27_SecurityAccess(); break;
        case 0x22: UDS_Service22_ReadDataByIdentifier(); break;
        case 0x2E: UDS_Service2E_WriteDataByIdentifier(); break;
        case 0x31: UDS_Service31_RoutineControl(); break;
        case 0x3E: UDS_Service3E_TesterPresent(); break;
        // 其他服务...
        default: UDS_SendNegativeResponse(sid, 0x11); // 服务不支持
    }
}

// 0x22 示例：读取标定值（DID=0xF190 代表输出电压目标值）
void UDS_Service22_ReadDataByIdentifier(void) {
    uint16_t did = (received_msg[1] << 8) | received_msg[2];
    switch (did) {
        case 0xF190: {
            uint16_t voltage = get_target_voltage(); // 从内存读取
            UDS_SendPositiveResponse(0x22, (uint8_t*)&voltage, 2);
            break;
        }
        case 0xF191: { /* 读取温度 */ break; }
        default: UDS_SendNegativeResponse(0x22, 0x31); // 请求超出范围
    }
}

// 0x2E 示例：写入标定值（DID=0xF190）
void UDS_Service2E_WriteDataByIdentifier(void) {
    uint16_t did = (received_msg[1] << 8) | received_msg[2];
    if (did == 0xF190) {
        uint16_t new_voltage = (received_msg[3] << 8) | received_msg[4];
        set_target_voltage(new_voltage);  // 写入RAM/EEPROM
        UDS_SendPositiveResponse(0x2E, NULL, 0);
    } else {
        UDS_SendNegativeResponse(0x2E, 0x31);
    }
}

// 0x31 例程控制：启动自检（RoutineID=0x0201）
void UDS_Service31_RoutineControl(void) {
    uint8_t ctrl_type = received_msg[1]; // 0x01启动 0x02停止 0x03请求结果
    uint16_t routine_id = (received_msg[2] << 8) | received_msg[3];
    if (routine_id == 0x0201) {
        if (ctrl_type == 0x01) {
            start_self_test();
            UDS_SendPositiveResponse(0x31, NULL, 0);
        } else if (ctrl_type == 0x03) {
            uint8_t result = get_self_test_result();
            UDS_SendPositiveResponse(0x31, &result, 1);
        }
    } else {
        UDS_SendNegativeResponse(0x31, 0x31);
    }
}
```

## 2 供应商产线阶段

**目标**：对ECU进行出厂编程、配置、功能测试（EOL，End-of-Line）。

**常用UDS服务**：

- `0x10 02` 编程会话：进入Bootloader刷写模式。
- `0x27` 安全访问：解锁编程权限（种子-密钥机制）。
- `0x2E` 写入配置：写入零件号、序列号、标定数据。
- `0x34/0x36/0x37` 数据传输：批量下载应用软件或标定数据。
- `0x31` 例程控制：执行EOL测试（如继电器吸合测试、绝缘检测）、校验CRC、复位ECU。
- `0x22` 读取标识：读取软件版本、硬件版本、序列号以追溯。
- `0x11` ECU复位：刷写后复位进入应用。

**代码示例**（安全访问和刷写流程简化）：

```c
// 安全访问 0x27
void UDS_Service27_SecurityAccess(void) {
    uint8_t sub_func = received_msg[1];
    if (sub_func == 0x01) { // 请求种子
        uint32_t seed = generate_random_seed();
        store_seed(seed);
        UDS_SendPositiveResponse(0x27, (uint8_t*)&seed, 4);
    } else if (sub_func == 0x02) { // 发送密钥
        uint32_t key = (received_msg[2]<<24) | (received_msg[3]<<16) | (received_msg[4]<<8) | received_msg[5];
        if (verify_key(key)) {
            security_unlocked = 1;
            UDS_SendPositiveResponse(0x27, NULL, 0);
        } else {
            UDS_SendNegativeResponse(0x27, 0x35); // 密钥错误
        }
    }
}

// 数据下载流程（以请求下载0x34为例）
void UDS_Service34_RequestDownload(void) {
    // 解析数据格式、地址、长度
    uint8_t dataFormat = received_msg[1];
    uint32_t addr = (received_msg[2]<<24)|(received_msg[3]<<16)|(received_msg[4]<<8)|received_msg[5];
    uint32_t size = (received_msg[6]<<24)|(received_msg[7]<<16)|(received_msg[8]<<8)|received_msg[9];
    if (is_valid_flash_range(addr, size)) {
        flash_erase(addr, size);
        current_download_addr = addr;
        remaining_bytes = size;
        UDS_SendPositiveResponse(0x34, (uint8_t*)&max_block_len, 2); // 回复最大块长度
    } else {
        UDS_SendNegativeResponse(0x34, 0x31); // 地址无效
    }
}
```


## 3 主机厂OEM产线阶段

**目标**：在整车装配线上对ECU进行整车集成后的配置、功能验证和基础诊断。

**常用UDS服务**：

- `0x22` 读取数据：读取ECU状态、软件版本、故障状态。
- `0x2E` 写入数据：写入VIN码、整车配置参数（如电池类型、容量）。
- `0x31` 例程控制：执行整车级功能测试（如充电握手模拟、高压互锁检测）。
- `0x19` 读取DTC信息：检测ECU是否存储故障码。
- `0x14` 清除DTC：修复后清除历史故障。
- `0x28` 通信控制：临时关闭某些报文发送以便测试。

**代码示例**（写入VIN和读取DTC）：


```c
// 0x2E 写入VIN（DID=0xF190）
void UDS_Service2E_WriteDataByIdentifier(void) {
    uint16_t did = (received_msg[1]<<8) | received_msg[2];
    if (did == 0xF190) { // VIN标识符
        if (security_unlocked) {
            memcpy(vin_buffer, &received_msg[3], 17); // 17字节VIN
            save_vin_to_eeprom(vin_buffer);
            UDS_SendPositiveResponse(0x2E, NULL, 0);
        } else {
            UDS_SendNegativeResponse(0x2E, 0x33); // 安全访问拒绝
        }
    }
}

// 0x19 读取DTC信息（子功能0x02按状态掩码读取）
void UDS_Service19_ReadDTCInformation(void) {
    uint8_t sub_func = received_msg[1];
    if (sub_func == 0x02) { // 按状态掩码读取
        uint8_t status_mask = received_msg[2];
        uint8_t dtc_list[100];
        uint8_t count = get_dtc_by_status(status_mask, dtc_list);
        // 响应格式：状态可用性掩码 + DTC格式标识 + DTC数量 + DTC列表
        uint8_t resp[103];
        resp[0] = 0x59; // 正响应SID
        resp[1] = 0x02;
        resp[2] = 0xFF; // 状态可用性掩码
        resp[3] = 0x01; // DTC格式（ISO14229-1）
        resp[4] = count >> 8;
        resp[5] = count & 0xFF;
        for (int i=0; i<count; i++) {
            resp[6+i*4]   = (dtc_list[i] >> 16) & 0xFF;
            resp[7+i*4]   = (dtc_list[i] >> 8) & 0xFF;
            resp[8+i*4]   = dtc_list[i] & 0xFF;
            resp[9+i*4]   = get_dtc_status(dtc_list[i]);
        }
        UDS_SendRawBytes(resp, 6+count*4);
    }
}
```


## 4 售后维护阶段

**目标**：服务站故障诊断、维修后的验证、软件升级（OTA或线下刷写）。

**常用UDS服务**：

- `0x19` 读取DTC及快照/扩展数据：深入分析故障原因。
- `0x22` 读取实时数据：监控电压、电流、温度等。
- `0x2E` 写入配置：更换ECU后写入车辆配置。
- `0x31` 例程控制：执行特定动作（如继电器测试、风扇测试、清除学习值）。
- `0x34/0x36/0x37` 数据传输：软件升级（SBL）。
- `0x27` 安全访问：保护高权限操作。
- `0x11` ECU复位：维修后复位。
- `0x28` 通信控制：屏蔽干扰报文。
- `0x85` 控制DTC设置：临时禁用某些DTC检测。

**代码示例**（实时数据读取和例程控制）：

```c
// 0x22 读取实时数据（DID=0x2001 输出电压，DID=0x2002 输出电流）
void UDS_Service22_ReadDataByIdentifier(void) {
    uint16_t did = (received_msg[1]<<8) | received_msg[2];
    if (did == 0x2001) {
        uint16_t vout = adc_read_voltage();
        UDS_SendPositiveResponse(0x22, (uint8_t*)&vout, 2);
    } else if (did == 0x2002) {
        int16_t iout = adc_read_current();
        UDS_SendPositiveResponse(0x22, (uint8_t*)&iout, 2);
    } else {
        UDS_SendNegativeResponse(0x22, 0x31);
    }
}

// 0x31 例程控制：执行继电器测试（RoutineID=0x0301）
void UDS_Service31_RoutineControl(void) {
    uint8_t ctrl = received_msg[1];
    uint16_t rid = (received_msg[2]<<8)|received_msg[3];
    if (rid == 0x0301) {
        if (ctrl == 0x01) {
            relay_test_start();
            UDS_SendPositiveResponse(0x31, NULL, 0);
        } else if (ctrl == 0x02) {
            relay_test_stop();
            UDS_SendPositiveResponse(0x31, NULL, 0);
        } else if (ctrl == 0x03) {
            uint8_t result = relay_test_get_result();
            UDS_SendPositiveResponse(0x31, &result, 1);
        }
    } else {
        UDS_SendNegativeResponse(0x31, 0x31);
    }
}
```