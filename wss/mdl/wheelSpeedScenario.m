function sim = wheelSpeedScenario(cfg)
% Generate a realistic wheel-speed profile and pulse capture timestamps.

rng(cfg.randomSeed);

t = (0:cfg.dt_s:cfg.simDuration_s)';
N = numel(t);

% Ground-truth speed profile, km/h.
% Contains low-speed operation, acceleration, cruise, deceleration and stop.
truth = zeros(N,1);

for k = 1:N
    tk = t(k);
    if tk < 3
        truth(k) = 0.8*tk;                         % 0 -> 2.4
    elseif tk < 8
        truth(k) = 2.4 + (15-2.4)*(tk-3)/5;      % accelerate
    elseif tk < 12
        truth(k) = 15;
    elseif tk < 15
        truth(k) = 15 - 12*(tk-12)/3;             % 15 -> 3
    elseif tk < 19
        truth(k) = 3 + 12*(tk-15)/4;              % 3 -> 15
    elseif tk < 23
        truth(k) = 15;
    elseif tk < 27
        truth(k) = 15 - 15*(tk-23)/4;             % 15 -> 0
    else
        truth(k) = 0.3;                           % near standstill
    end
end

% Add a small sinusoidal wheel-speed disturbance to make the filter useful.
truth = max(0, truth + 0.35*sin(2*pi*0.7*t));

% Pulse frequency f = v_kph*ppr/(216*C)
pulseFreq = truth * cfg.ppr / (216*cfg.circumference_m);

% Generate ideal edge times from the varying instantaneous pulse frequency.
% One timestamp per rising edge.
edgeTimes = [];
phase = 0;
lastT = 0;

for k = 1:N-1
    dt = cfg.dt_s;
    fmid = max(0, (pulseFreq(k)+pulseFreq(k+1))/2);
    phase = phase + fmid*dt;

    if phase >= 1
        nPulses = floor(phase);
        phase = phase - nPulses;
        % Approximate placement within this control interval.
        for p = 1:nPulses
            frac = p/nPulses;
            edgeTimes(end+1,1) = t(k) + frac*dt; %#ok<AGROW>
        end
    end
end

% Inject missing and extra pulses.
keep = rand(size(edgeTimes)) > cfg.missingPulseProbability;
edgeTimes = edgeTimes(keep);

if ~isempty(edgeTimes)
    extraMask = rand(size(edgeTimes)) < cfg.extraPulseProbability;
    extra = edgeTimes(extraMask) + 0.35e-3;
    edgeTimes = sort([edgeTimes; extra]);
end

% Quantization + timestamp jitter.
edgeTimes = round(edgeTimes/cfg.timerQuantization_us*1e6) ...
          * cfg.timerQuantization_us/1e6;
edgeTimes = edgeTimes + randn(size(edgeTimes))*cfg.edgeJitter_us*1e-6;

% Keep monotonic timestamps.
edgeTimes = sort(edgeTimes);
edgeTimes = edgeTimes([true; diff(edgeTimes) > 1e-7]);

sim.t = t;
sim.truth_kph = truth;
sim.edgeTimes_s = edgeTimes;
sim.pulseFreq_Hz = pulseFreq;
end
