# TIM/eCAP/input-capture implementation guide

## Generic edge algorithm

The hardware capture peripheral should provide:

```text
rising_1
falling
rising_2
```

Then:

```c
period = rising_2 - rising_1;
high   = falling   - rising_1;
duty   = high / period;
freq   = timer_hz / period;
```

Handle timer wrap-around with unsigned subtraction.

## ISR responsibilities

ISR should be short:

1. Read capture registers.
2. Calculate or copy timestamps.
3. Validate basic ordering.
4. Write a coherent snapshot.
5. Clear interrupt.
6. Return.

Do not execute PDU state machine inside the capture ISR.

## Double-buffer recommendation

For production:

```text
TIM ISR
  ↓
capture_buffer_A/B
  ↓
1ms task atomically consumes latest complete snapshot
```

This prevents the 1ms task from reading half-updated period/high values.

## Filtering

Use:

- hardware input filter first
- frequency window
- duty window
- consecutive-valid counter
- timeout
- optional median/EMA for diagnostic values

Do not heavily low-pass the raw edge timestamps; this can distort duty/frequency.
