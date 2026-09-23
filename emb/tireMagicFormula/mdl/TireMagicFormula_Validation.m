function results = TireMagicFormula_Validation()
%TIREMAGICFORMULA_VALIDATION Basic automated checks for the example model.

p = MF_TireParams();

% Symmetry checks for the generic example model.
k = linspace(-0.2,0.2,401);
a = linspace(-0.2,0.2,401);

[FxP,~,~] = tireMagicFormula(k,zeros(size(k)),4000,p);
[FxN,~,~] = tireMagicFormula(-k,zeros(size(k)),4000,p);

[~,FyP,~] = tireMagicFormula(zeros(size(a)),a,4000,p);
[~,FyN,~] = tireMagicFormula(zeros(size(a)),-a,4000,p);

errFx = max(abs(FxP + FxN));
errFy = max(abs(FyP + FyN));

results.maxOddSymmetryErrorFx_N = errFx;
results.maxOddSymmetryErrorFy_N = errFy;
results.pass = (errFx < 1e-8) && (errFy < 1e-8);

fprintf('Fx odd-symmetry error: %.3e N\n',errFx);
fprintf('Fy odd-symmetry error: %.3e N\n',errFy);
fprintf('Validation: %s\n',string(results.pass));
end
