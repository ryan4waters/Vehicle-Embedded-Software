# UDS -> HSM Mapping

| UDS | Typical security relevance | HSM/CSM action |
|---|---|---|
| 0x10 | session transition | security policy |
| 0x27 | authentication | RNG + challenge/response |
| 0x34 | download start | update authorization/policy |
| 0x36 | data transfer | flash write + integrity tracking |
| 0x37 | transfer exit | SHA-256 / signature verify |
| 0x31 | routine control | verify / activate / rollback |
| 0x22 | read data | protect security-sensitive identifiers |
| 0x2E | write data | authenticated authorization |

This is an architectural mapping, not a claim that every ECU must implement every service with HSM.
