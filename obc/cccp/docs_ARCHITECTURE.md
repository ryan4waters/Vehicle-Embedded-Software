# Architecture and integration rules

## 1. Public dependency rule

```text
app
 ↓
CCCP public API
 ↓
common algorithm
 ↓
CCCP_Platform interface
 ↓
TC377 / F29P32x / SPC58NN
 ↓
MCU SDK / MCAL / register
```

Never:

```text
common/src/*.c -> IfxGtm_*.h
common/src/*.c -> driverlib.h
common/src/*.c -> Mcal.h
```

## 2. CP PWM calculation

Suppose capture timer = 1 MHz:

```text
Rising1 = 1000
Falling = 1500
Rising2 = 2000

Period = 2000 - 1000 = 1000 ticks
High   = 1500 - 1000 = 500 ticks

T = 1000 us
Ton = 500 us
f = 1 kHz
Duty = 50%
```

Then:

```text
I_EVSE = 50 × 0.6 = 30 A
```

## 3. Current limit

```text
Ilim = min(
    I_EVSE,
    I_CC,
    I_BMS,
    I_thermal,
    I_OEM,
    I_OBC
)
```

## 4. Power limit

```text
Pcp ≈ Vac_rms × Ilim × PF

Plim = min(
    Pcp,
    P_BMS,
    P_OEM,
    P_OBC
)
```

PFC software can then consume `power_limit_w` and generate its current reference.

## 5. Wakeup

Recommended:

```text
CP/CC edge or PMIC event
       ↓
wakeup
       ↓
MCU initialization
       ↓
CCCP validation
       ↓
PDU state machine
       ↓
precharge
       ↓
power stage enable
```

Wakeup is not equivalent to charge authorization.

## 6. Fault philosophy

For production, add separate flags for:

- CP stuck-high
- CP stuck-low
- CP frequency invalid
- CP duty invalid
- CP signal timeout
- CC invalid
- CC cable capacity conflict
- BMS timeout
- CP/CC state contradiction
- relay/precharge failure
- HVIL
- insulation
- over-temperature
- power-stage fault

Do not use one `fault` bit as the final production diagnosis; replace it with a diagnostic manager / DEM interface.

## 7. Safety

For a safety-related OBC, charging authorization must be independent of a single software path. Hardware shutdown, PMIC watchdog, gate-driver fault, HVIL, relay feedback and safety MCU/monitoring architecture should be handled according to the product's ISO 26262 safety concept.
