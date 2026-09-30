# Minimum HSM Security Test Set

## Factory
1. Correct device identity
2. Wrong provisioning authorization
3. Duplicate key provisioning
4. EOL HSM self-test
5. Production lock pre-check failure
6. Recovery path

## Startup
1. Valid image
2. Hash mismatch
3. Signature mismatch
4. Unsupported key
5. Rollback version
6. Corrupted metadata
7. Invalid memory range
8. HSM unavailable
9. HSM timeout
10. Power loss during update

## Runtime CAN
1. Valid MAC
2. Wrong MAC
3. Wrong Data ID
4. Wrong freshness
5. Replay
6. Counter wrap policy
7. HSM busy
8. Queue full
9. callback timeout

## Update
1. Unauthorized tester
2. Wrong security response
3. invalid address
4. oversized transfer
5. dropped block
6. hash mismatch
7. signature mismatch
8. rollback version
9. power loss
10. interrupted commit

## After-sales
1. unauthorized service tool
2. authorization timeout
3. expired certificate
4. update authorization failure
5. recovery
6. audit event
