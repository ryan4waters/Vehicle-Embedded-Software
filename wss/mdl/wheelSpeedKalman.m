function kf = wheelSpeedKalman(t, z, valid, cfg)
% Constant-acceleration Kalman filter.
%
% State:
%   x = [v; a]
%
% Prediction:
%   x(k+1) = A*x(k)
%   A = [1 dt; 0 1]
%
% Measurement:
%   z = [1 0] * x + noise
%
% The filter predicts every control cycle but only performs a measurement
% update when a new valid wheel-speed measurement is available.

N = numel(t);
x = [0; 0];
P = cfg.kf.P0;

vOut = zeros(N,1);
aOut = zeros(N,1);
rUsed = nan(N,1);
innovation = nan(N,1);
updateFlag = false(N,1);

H = [1 0];

for k = 1:N
    if k == 1
        dt = cfg.dt_s;
    else
        dt = max(1e-6, t(k)-t(k-1));
    end

    A = [1 dt; 0 1];
    Q = [cfg.kf.Q_speed*dt^2, 0; ...
         0, cfg.kf.Q_accel*dt];

    % Predict
    x = A*x;
    P = A*P*A' + Q;

    % Speed-dependent measurement noise.
    vForR = max(0, x(1));
    alpha = min(1, max(0, ...
        (vForR-cfg.kf.rLowSpeed_kph) / ...
        (cfg.kf.rHighSpeed_kph-cfg.kf.rLowSpeed_kph)));
    R = (1-alpha)*cfg.kf.R_low + alpha*cfg.kf.R_high;
    rUsed(k) = R;

    % Measurement update only for valid data.
    if valid(k) && isfinite(z(k))
        % Additional innovation gate prevents a single abnormal pulse from
        % pulling the filter excessively.
        y = z(k) - H*x;
        S = H*P*H' + R;

        if abs(y) <= 4*sqrt(S) + 15
            K = P*H'/S;
            x = x + K*y;
            I = eye(2);
            P = (I-K*H)*P*(I-K*H)' + K*R*K'; % Joseph form
            innovation(k) = y;
            updateFlag(k) = true;
        end
    end

    % Physical constraints
    x(1) = max(0, min(300, x(1)));
    x(2) = max(-150, min(150, x(2)));

    vOut(k) = x(1);
    aOut(k) = x(2);
end

kf.speed_kph = vOut;
kf.accel_kph_s = aOut;
kf.R = rUsed;
kf.innovation = innovation;
kf.updateFlag = updateFlag;
end
