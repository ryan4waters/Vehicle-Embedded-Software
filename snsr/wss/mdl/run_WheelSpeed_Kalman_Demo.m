%% WheelSpeed Kalman compensation demo
% MATLAB R2025b
clear; clc; close all;

cfg = struct();

% Physical / sensor parameters
cfg.ppr = 48;                  % pulses/revolution
cfg.circumference_m = 2.0;     % wheel circumference
cfg.timerFreqHz = 1e6;         % 1 MHz capture timer

% Estimator parameters
cfg.mGate_s = 0.010;           % M-method gate = 10 ms
cfg.lowSpeed_kph = 5.0;        % T -> M only above this high threshold
cfg.highSpeed_kph = 20.0;      % M -> T only below this low threshold

% Simulation
cfg.simDuration_s = 30;
cfg.dt_s = 0.001;              % 1 ms application/Kalman period
cfg.edgeJitter_us = 8;         % timestamp jitter
cfg.timerQuantization_us = 1;  % capture timer resolution
cfg.missingPulseProbability = 0.025;
cfg.extraPulseProbability = 0.006;
cfg.randomSeed = 20250906;

% Kalman parameters: x=[speed_kph; acceleration_kph_s]
cfg.kf.Q_speed = 0.20;
cfg.kf.Q_accel = 20.0;
cfg.kf.R_low = 9.0;
cfg.kf.R_high = 1.5;
cfg.kf.rLowSpeed_kph = 5;
cfg.kf.rHighSpeed_kph = 20;
cfg.kf.P0 = diag([25, 400]);

% Generate scenario and simulated capture data
sim = wheelSpeedScenario(cfg);

% M/T raw speed estimate
raw = mtWheelSpeedEstimator(sim, cfg);

% Kalman compensation
kf = wheelSpeedKalman(sim.t, raw.speed_kph, raw.valid, cfg);

% Metrics
metrics = wheelSpeedMetrics(sim.truth_kph, raw.speed_kph, raw.valid, kf.speed_kph, sim.t);

% Display metrics
fprintf('\n===== Wheel Speed Kalman Compensation =====\n');
fprintf('Overall raw M/T RMSE : %8.3f km/h\n', metrics.raw.rmse);
fprintf('Overall KF RMSE      : %8.3f km/h\n', metrics.kf.rmse);
fprintf('Overall raw M/T MAE  : %8.3f km/h\n', metrics.raw.mae);
fprintf('Overall KF MAE       : %8.3f km/h\n', metrics.kf.mae);
fprintf('Low-speed raw RMSE   : %8.3f km/h\n', metrics.low.rawRmse);
fprintf('Low-speed KF RMSE    : %8.3f km/h\n', metrics.low.kfRmse);
fprintf('Max raw error        : %8.3f km/h\n', metrics.raw.maxAbsError);
fprintf('Max KF error         : %8.3f km/h\n', metrics.kf.maxAbsError);
fprintf('Raw valid ratio      : %8.2f %%\n', 100*mean(raw.valid));
fprintf('KF improvement       : %8.2f %% RMSE\n', ...
    100*(1-metrics.kf.rmse/metrics.raw.rmse));

plotWheelSpeedResults(sim, raw, kf, metrics);

% Save results
results = table(sim.t, sim.truth_kph, raw.speed_kph, raw.valid, ...
    string(raw.method), kf.speed_kph, kf.accel_kph_s, ...
    'VariableNames', {'Time_s','Truth_kph','RawMT_kph','RawValid', ...
    'Method','Kalman_kph','KalmanAccel_kph_s'});

writetable(results, 'wheel_speed_kalman_results.csv');
save('wheel_speed_kalman_demo.mat', 'cfg', 'sim', 'raw', 'kf', 'metrics');

fprintf('\nFiles generated:\n');
fprintf('  wheel_speed_kalman_results.csv\n');
fprintf('  wheel_speed_kalman_demo.mat\n');

% Automated acceptance test
test_WheelSpeed_Kalman(sim, raw, kf, metrics);
