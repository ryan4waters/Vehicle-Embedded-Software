# Runtime / CAN Security

## SecOC

TX:
Application
 -> SecOC
 -> freshness
 -> CSM MAC Generate
 -> CryIf
 -> Crypto Driver
 -> HSM AES-CMAC
 -> Authenticator
 -> CAN

RX:
CAN
 -> CanIf/PduR
 -> SecOC
 -> reconstruct/check freshness
 -> CSM MAC Verify
 -> HSM
 -> accept/reject

Important:
CMAC alone does not provide anti-replay. Freshness state and synchronization are part of the security design.

## Diagnostic

UDS 0x27 SecurityAccess:
Tester -> ECU seed request
ECU -> random/challenge
Tester -> key/response
ECU/HSM -> verify
ECU -> unlocked session

Do not expose the diagnostic secret key to the application.
