function plotWheelSpeedResults(sim, raw, kf, metrics)

figure('Name','Wheel speed compensation');
plot(sim.t, sim.truth_kph, 'LineWidth', 1.5); hold on;
plot(sim.t, raw.speed_kph, '.');
plot(sim.t, kf.speed_kph, 'LineWidth', 1.2);
grid on;
xlabel('Time (s)');
ylabel('Wheel speed (km/h)');
legend('Truth','Raw M/T','Kalman compensated','Location','best');
title('Wheel-speed M/T + Kalman compensation');

figure('Name','Low speed zoom');
mask = sim.t <= 12;
plot(sim.t(mask), sim.truth_kph(mask), 'LineWidth', 1.5); hold on;
plot(sim.t(mask), raw.speed_kph(mask), '.');
plot(sim.t(mask), kf.speed_kph(mask), 'LineWidth', 1.2);
grid on;
xlabel('Time (s)');
ylabel('Wheel speed (km/h)');
legend('Truth','Raw M/T','Kalman','Location','best');
title('Low-speed / acceleration region');

figure('Name','Wheel speed error');
plot(sim.t, metrics.rawError, '.'); hold on;
plot(sim.t, metrics.kfError, 'LineWidth', 1.1);
yline(0);
grid on;
xlabel('Time (s)');
ylabel('Error (km/h)');
legend('Raw M/T error','Kalman error','Location','best');
title('Estimation error');

figure('Name','M/T method selection');
stairs(sim.t, double(raw.method=="M"), 'LineWidth', 1.1);
grid on;
ylim([-0.1 1.1]);
yticks([0 1]);
yticklabels({'T','M'});
xlabel('Time (s)');
ylabel('Method');
title('M/T adaptive method selection');

figure('Name','Kalman acceleration estimate');
plot(sim.t, kf.accel_kph_s, 'LineWidth', 1.1);
grid on;
xlabel('Time (s)');
ylabel('Acceleration (km/h/s)');
title('Kalman estimated acceleration');

fprintf('\n===== Low-speed result =====\n');
fprintf('Raw low-speed RMSE = %.3f km/h\n', metrics.low.rawRmse);
fprintf('KF  low-speed RMSE = %.3f km/h\n', metrics.low.kfRmse);
fprintf('Raw low-speed MAE  = %.3f km/h\n', metrics.low.rawMae);
fprintf('KF  low-speed MAE  = %.3f km/h\n', metrics.low.kfMae);
end
