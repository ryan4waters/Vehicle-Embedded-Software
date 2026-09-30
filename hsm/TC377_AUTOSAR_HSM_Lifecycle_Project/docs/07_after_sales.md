# After Sales / Service

Permitted service operations should be policy-controlled.

Examples:
- secure diagnostic unlock
- DTC read/clear
- secure ECU replacement
- certificate/key rotation
- authenticated software update
- recovery/reprogramming

A service station should not receive permanent ECU secrets.

Recommended:
Tester authentication
    -> temporary authorization
    -> limited service capability
    -> audit/event record
    -> authorization timeout
    -> locked again
