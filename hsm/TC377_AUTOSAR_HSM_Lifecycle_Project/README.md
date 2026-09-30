# TC377 + AUTOSAR HSM Lifecycle Security Project

## 目标

本版本按照 ECU 全生命周期组织 HSM 使用场景：

01_factory_production
    - 工厂烧录
    - Device Identity / Key Provisioning
    - HSM状态切换
    - Production Lock 前检查

02_startup
    - HSM启动
    - Secure Boot
    - Firmware Hash
    - Signature Verify
    - Anti-Rollback
    - Recovery

03_runtime
    - CAN SecOC
    - Freshness Counter
    - Diagnostic Authentication
    - Random Number
    - Runtime HSM health monitor

04_flash_update
    - UDS 0x10 / 0x27 / 0x34 / 0x36 / 0x37 / 0x31
    - Download authorization
    - Erase / TransferData
    - Hash
    - Signature
    - Version / Anti-Rollback
    - Atomic commit / rollback

05_after_sales
    - Service diagnostics
    - Secure diagnostic unlock
    - Key/certificate update
    - Secure recovery
    - Failure/event logging

common
    - CSM / CryIf / Crypto Driver abstraction
    - TC377 HSM HAL
    - Key IDs / Key lifecycle
    - IPC / asynchronous job
    - Security policy
    - platform adapters

examples
    - complete end-to-end scenarios

## Important

This is a production-oriented architecture skeleton, not vendor-generated AUTOSAR BSW.

The exact function prototypes and ECUC configuration depend on:
- AUTOSAR CP release
- MCAL/BSW vendor
- EB tresos / DaVinci configuration
- Infineon HSM firmware/package
- bootloader implementation
- project PKI and security concept

TC377 HSM register-level/private firmware APIs are deliberately isolated in `common/platform/Tc377_Hsm_Hal.c`.

Never place production secret keys in source code.
Never copy real production certificates/private keys into this repository.
Development and production UCB/HSM locking must be treated as different configurations.
