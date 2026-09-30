# Key Lifecycle

Example state model:

KEY_EMPTY
   |
   | factory provisioning
   v
KEY_PROVISIONED
   |
   | validation
   v
KEY_VALID
   |
   | rotation
   v
KEY_RETIRED
   |
   | secure erase / replacement
   v
KEY_DESTROYED

Typical keys:

BOOT_VERIFY_PUBLIC_KEY
    Used for firmware signature verification.

SEC_OC_CMAC_KEY
    Used for CAN authentication.

DIAG_AUTH_KEY
    Used by secure diagnostic access.

UPDATE_AUTH_KEY
    Used to authorize firmware update.

DEVICE_ID / DEVICE_CERT
    Device identity/certificate.

Important:
- Private/symmetric secrets should remain inside protected key storage.
- Key IDs are references, not key material.
- Factory provisioning must authenticate the provisioning station.
- Production lock must happen only after recovery/debug policy is validated.
