# Flash Update / UDS

Typical flow:

0x10 DiagnosticSessionControl
        |
0x27 SecurityAccess
        |
0x31 optional precondition/auth service
        |
0x34 RequestDownload
        |
0x36 TransferData
        |
0x37 RequestTransferExit
        |
Hash verification
        |
Signature verification
        |
Anti-rollback
        |
0x31 programming dependency/commit
        |
Reset
        |
Secure Boot verifies image again

Production architecture should preferably use a staging/A-B partition or equivalent power-loss-safe mechanism.

CRC can detect accidental transmission errors but is not a replacement for cryptographic authenticity.
