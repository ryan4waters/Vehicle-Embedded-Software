function test_WheelSpeed_Kalman(sim, raw, kf, metrics)
% Simple regression-style acceptance test.

assert(all(isfinite(kf.speed_kph)), ...
    'FAIL: Kalman output contains NaN/Inf.');

assert(all(kf.speed_kph >= 0), ...
    'FAIL: negative wheel speed detected.');

assert(metrics.kf.rmse < metrics.raw.rmse, ...
    'FAIL: Kalman RMSE is not better than raw M/T RMSE.');

assert(metrics.low.kfRmse < metrics.low.rawRmse, ...
    'FAIL: low-speed Kalman RMSE is not improved.');

assert(max(kf.speed_kph) < 300, ...
    'FAIL: Kalman output exceeds configured physical range.');

fprintf('\nPASS: Kalman compensation improves both overall and low-speed RMSE.\n');
end
