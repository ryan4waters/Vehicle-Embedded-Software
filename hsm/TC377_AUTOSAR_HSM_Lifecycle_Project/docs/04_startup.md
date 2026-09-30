# Startup / Secure Boot

Recommended sequence:

Reset
 |
 +--> early startup
 |
 +--> HSM init
 |
 +--> HSM ready?
 |       |
 |       +-- NO --> recovery
 |
 +--> boot metadata valid?
 |
 +--> image address/size valid?
 |
 +--> SHA-256 image
 |
 +--> ECDSA-P256 signature verify in HSM
 |
 +--> anti-rollback
 |
 +--> vector / image consistency checks
 |
 +--> start application
 |
 +--> runtime HSM self-check

Failure must not simply jump to an unverified image.

For TC377, HSM/UCB security configuration is device-specific and should be applied using the exact Infineon-supported toolchain and project security concept.
