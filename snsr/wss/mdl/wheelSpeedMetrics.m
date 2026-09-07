function metrics = wheelSpeedMetrics(truth, raw, rawValid, kf, t)

rawErr = raw - truth;
kfErr = kf - truth;

metrics.raw.rmse = sqrt(mean(rawErr(rawValid).^2, 'omitnan'));
metrics.raw.mae = mean(abs(rawErr(rawValid)), 'omitnan');
metrics.raw.maxAbsError = max(abs(rawErr(rawValid)), [], 'omitnan');

metrics.kf.rmse = sqrt(mean(kfErr.^2, 'omitnan'));
metrics.kf.mae = mean(abs(kfErr), 'omitnan');
metrics.kf.maxAbsError = max(abs(kfErr), [], 'omitnan');

lowMask = truth <= 5 & truth > 0.5;
metrics.low.rawRmse = sqrt(mean(rawErr(lowMask & rawValid).^2, 'omitnan'));
metrics.low.kfRmse = sqrt(mean(kfErr(lowMask).^2, 'omitnan'));
metrics.low.rawMae = mean(abs(rawErr(lowMask & rawValid)), 'omitnan');
metrics.low.kfMae = mean(abs(kfErr(lowMask)), 'omitnan');

metrics.t = t;
metrics.rawError = rawErr;
metrics.kfError = kfErr;
end
