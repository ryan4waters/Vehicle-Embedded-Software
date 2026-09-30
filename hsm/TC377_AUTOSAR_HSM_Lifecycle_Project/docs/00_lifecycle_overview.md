# ECU HSM Lifecycle

```text
             ┌──────────────────────────────┐
             │       Factory / Production   │
             │ identity + key provisioning  │
             └──────────────┬───────────────┘
                            │
                            ▼
             ┌──────────────────────────────┐
             │          Startup             │
             │ HSM init + Secure Boot       │
             └──────────────┬───────────────┘
                            │
                            ▼
             ┌──────────────────────────────┐
             │          Runtime             │
             │ SecOC / Diag / RNG / HSM     │
             └──────────────┬───────────────┘
                            │
             ┌──────────────┴───────────────┐
             │                              │
             ▼                              ▼
       Flash / Update                  After Sales
       UDS secure update               secure service
             │                              │
             └──────────────┬───────────────┘
                            ▼
                       Secure Boot
                            │
                            └── cycle
```

The same HSM root of trust is reused across the ECU lifecycle, but the allowed operations and key states differ by phase.
