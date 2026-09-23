function p = MF_TireParams()
%MF_TireParams  Example Pacejka Magic Formula tire parameters.
% MATLAB R2025b compatible.
%
% Sign convention used by this demo:
%   kappa > 0 : driving/traction slip
%   alpha > 0 : tire slip angle producing positive lateral force
%   Fz > 0    : vertical tire load [N]
%
% The parameter set is intentionally generic and for algorithm study,
% NOT a validated tire identification dataset.

p.Fz0 = 4000;          % nominal vertical load [N]

% Pure longitudinal MF parameters
p.Bx = 10.0;
p.Cx = 1.65;
p.Ex = 0.97;
p.muX = 1.05;          % peak friction coefficient at Fz0

% Pure lateral MF parameters
p.By = 7.0;
p.Cy = 1.30;
p.Ey = -1.60;
p.muY = 1.00;

% Simple load sensitivity:
% mu(Fz) = mu0 * (Fz/Fz0)^muLoadExp
% An exponent < 0 represents decreasing peak friction with load.
p.muLoadExpX = -0.08;
p.muLoadExpY = -0.10;

% Combined-slip weighting factors.
% This demo uses a normalized friction-ellipse style coupling around
% pure-slip Magic Formula outputs; it is deliberately simpler than
% the full Pacejka MF5.x/MF6.x combined-slip parameterization.
p.combinedSlipEnable = true;
end
