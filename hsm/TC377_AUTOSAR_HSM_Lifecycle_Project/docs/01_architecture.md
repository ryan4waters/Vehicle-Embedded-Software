# Architecture

## AUTOSAR path

Application / SecOC / Diagnostic / Bootloader
              |
             CSM
              |
            CryIf
              |
        Crypto Driver
              |
       TC377 HSM Adapter
              |
      TC377 HSM subsystem

AUTOSAR R24-11 CSM uses Crypto Jobs and calls `CryIf_ProcessJob`; CSM queues can schedule jobs and CryIf maps CSM key references to Crypto Driver key IDs.

## Design principle

Application must never know:
- HSM mailbox layout
- HSM private command codes
- actual key material
- physical key slot address

Application should know only:
- service API
- symbolic KeyId
- job/service result

## Recommended separation

AUTOSAR generated:
    Csm_*
    CryIf_*
    Crypto_*
    SecOC_*

Project adapter:
    SecurityPolicy_*
    HsmLifecycle_*
    Tc377_Hsm_Hal_*
    UdsSecurity_*
    BootSecurity_*
    UpdateSecurity_*

TC377-specific:
    UCB configuration
    HSM firmware
    HSM startup/vector configuration
    flash/security driver
