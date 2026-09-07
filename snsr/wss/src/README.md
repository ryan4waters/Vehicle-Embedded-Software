# Wheel Speed Final Design

## 1. Two hardware variants

### Variant A: SPC58 + AK
Sensor -> AK electrical interface -> SPC58 input -> eMIOS/GTM capture -> DMA -> timestamp buffer -> AK parser -> speed/diagnosis.

### Variant B: SR5E1 + PWM
SR5E1 PWM -> input protection/conditioning -> MCU timer input capture -> DMA -> timestamp buffer -> PWM parser -> period/high-time -> speed/diagnosis.

The two variants share the timestamp/capture and physical-speed layers.

## 2. M method

Fixed measurement window Tg:
    N = number of pulses in Tg
    f = N / Tg

Strength:
- Good at high speed because many pulses arrive in one gate.
- Averaging over multiple pulses reduces single-period quantization/noise.

Weakness:
- At low speed N may be 0 or 1.
- Resolution is limited by the fixed gate.
- If Tg is shortened for fast dynamics, low-speed resolution becomes worse.

## 3. T method

Measure time T between adjacent pulses:
    f = 1 / T

Strength:
- Very high speed resolution at low speed.
- A single pulse period can still provide a speed estimate.

Weakness:
- At low speed the update interval becomes long.
- Timer quantization and edge jitter create large relative speed error at very short periods.
- A single bad edge can create a large speed jump.

## 4. M/T method

Use M at high speed and T at low speed, with a transition/blending region.

Recommended practical rule:
- low speed: T
- high speed: M
- transition: blend or hysteresis to prevent mode chattering

The thresholds must be calibrated using actual wheel-speed data.

## 5. Kalman filter

State:
    x = [v, a]^T

Prediction:
    v(k+1) = v(k) + a(k)*dt
    a(k+1) = a(k)

Measurement:
    z = v + noise

The filter is useful because low-speed pulse measurements can be sparse/noisy. It does NOT replace signal diagnostics. Invalid/stuck/timeout measurements must be rejected before entering the filter.

## 6. Important production changes

This package is a hardware-independent final architecture/template, not a drop-in SPC58NN register driver.

Before production:
- bind eMIOS/GTM channels to actual pins
- configure exact DMA request sources
- handle DMA circular-buffer read/write indexes
- use the real free-running timer for g_nowTicks
- implement exact AK protocol state machine from the sensor/OEM specification
- expose parser snapshots into WheelSpeedData_t
- tune M/T thresholds and Kalman Q/R using logged vehicle data
- add four-wheel plausibility and vehicle-level diagnostics
- add counter rollover/overflow handling and DMA overflow diagnostics
