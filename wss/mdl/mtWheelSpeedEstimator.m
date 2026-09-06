function raw = mtWheelSpeedEstimator(sim, cfg)
% M/T wheel-speed estimator.
%
% T method:
%   f = 1/Tpulse
%
% M method:
%   f = N/Tgate
%
% Mode selection uses hysteresis:
%   T -> M when speed estimate > highSpeed_kph
%   M -> T when speed estimate < lowSpeed_kph

t = sim.t;
edge = sim.edgeTimes_s;
N = numel(t);

speedT = nan(N,1);
speedM = nan(N,1);
speed = nan(N,1);
valid = false(N,1);
method = strings(N,1);

% T method: most recent adjacent same-edge period.
idxEdge = 1;
lastEdge = NaN;
period = NaN;

% M gate state
gateStart = 0;
gateEnd = cfg.mGate_s;
gateCount = 0;

mode = "T";

for k = 1:N
    tk = t(k);

    % Consume edges up to current time.
    while idxEdge <= numel(edge) && edge(idxEdge) <= tk
        currentEdge = edge(idxEdge);

        if ~isnan(lastEdge)
            period = currentEdge - lastEdge;
            if period > 0
                f = 1/period;
                speedT(k) = 216*f*cfg.circumference_m/cfg.ppr;
            end
        end
        lastEdge = currentEdge;
        gateCount = gateCount + 1;
        idxEdge = idxEdge + 1;
    end

    % M gate closes.
    if tk >= gateEnd
        fM = gateCount / (gateEnd-gateStart);
        speedM(k) = 216*fM*cfg.circumference_m/cfg.ppr;

        gateCount = 0;
        gateStart = gateEnd;
        gateEnd = gateEnd + cfg.mGate_s;
    end

    % Basic validity
    tValid = isfinite(speedT(k)) && speedT(k) >= 0 && speedT(k) < 300;
    mValid = isfinite(speedM(k)) && speedM(k) >= 0 && speedM(k) < 300;

    % Hysteresis transition
    refSpeed = NaN;
    if tValid
        refSpeed = speedT(k);
    elseif mValid
        refSpeed = speedM(k);
    end

    if mode == "T" && isfinite(refSpeed) && refSpeed >= cfg.highSpeed_kph && mValid
        mode = "M";
    elseif mode == "M" && isfinite(refSpeed) && refSpeed <= cfg.lowSpeed_kph && tValid
        mode = "T";
    end

    if mode == "T"
        if tValid
            speed(k) = speedT(k);
            valid(k) = true;
        elseif mValid
            speed(k) = speedM(k);
            valid(k) = true;
        end
    else
        if mValid
            speed(k) = speedM(k);
            valid(k) = true;
        elseif tValid
            speed(k) = speedT(k);
            valid(k) = true;
        end
    end

    method(k) = mode;
end

raw.t = t;
raw.speedT_kph = speedT;
raw.speedM_kph = speedM;
raw.speed_kph = speed;
raw.valid = valid;
raw.method = method;
end
