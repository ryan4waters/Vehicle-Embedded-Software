# WheelSpeed Kalman Compensation — MATLAB R2025b

## 1. Purpose
This project demonstrates the low-speed wheel-speed compensation strategy discussed in the C implementation:

1. Construct a ground-truth vehicle/wheel speed profile.
2. Generate pulse timestamps from the physical speed.
3. Add edge timestamp jitter, quantization and occasional missing/irregular pulses.
4. Calculate raw wheel speed with M/T:
   - low speed: T method
   - high speed: M method
   - transition: hysteresis
5. Reject obviously invalid measurements.
6. Run a constant-acceleration Kalman filter:
       x = [v; a]
       v(k+1) = v(k) + a(k)*dt
       a(k+1) = a(k)
7. Compare truth / raw M-T / Kalman compensated speed.
8. Display RMSE, MAE, maximum error, low-speed RMSE and plots.

## 2. MATLAB version
Designed for MATLAB R2025b. The code uses basic MATLAB functionality and does not require Simulink.

## 3. Run
Open this folder in MATLAB and execute:

    run_WheelSpeed_Kalman_Demo

The script generates:
- wheel_speed_kalman_demo.mat
- wheel_speed_kalman_results.csv
- several figures

## 4. Main files
- run_WheelSpeed_Kalman_Demo.m : one-click demo
- wheelSpeedScenario.m         : test scenario and pulse timestamp generation
- mtWheelSpeedEstimator.m      : M/T estimator
- wheelSpeedKalman.m           : [v,a] Kalman filter
- wheelSpeedMetrics.m          : error metrics
- plotWheelSpeedResults.m      : result plots
- test_WheelSpeed_Kalman.m    : automatic pass/fail test

## 5. Recommended interpretation
The important comparison is not only whether the filtered curve looks smoother.

Check:
- low-speed RMSE
- low-speed MAE
- maximum error
- response delay around acceleration/deceleration
- behavior during sparse pulses
- rejection of abnormal measurements

The Kalman filter is not a fault detector. Invalid/stuck/timeout measurements should be rejected before the Kalman measurement update.
