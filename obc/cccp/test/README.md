# Host test

Linux/macOS/MinGW:

```bash
gcc -std=c99 \
  -Icommon/include \
  -Iapp \
  common/src/CCCP.c \
  common/src/CCCP_Measure.c \
  common/src/CCCP_Decode.c \
  common/src/CCCP_State.c \
  common/src/CCCP_Limit.c \
  test/mock/CCCP_Platform_mock.c \
  test/test_cccp.c \
  -lm -o test_cccp

./test_cccp
```

Mock input:

- CP = 6 V
- CP PWM = 1 kHz
- Duty = 50%
- CP current = 30 A
- CC = 220 ohm -> example 32 A
- BMS = 25 A
- Thermal = 24 A

Therefore the final current limit should be 24 A.
