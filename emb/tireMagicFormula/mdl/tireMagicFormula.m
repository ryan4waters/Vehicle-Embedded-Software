function [Fx,Fy,info] = tireMagicFormula(kappa,alpha,Fz,p)
%TIREMAGICFORMULA Pure and simple combined-slip Magic Formula tire model.
%
% Inputs:
%   kappa : longitudinal slip ratio [-]
%   alpha : slip angle [rad]
%   Fz    : vertical tire load [N]
%   p     : parameter structure from MF_TireParams()
%
% Outputs:
%   Fx, Fy : tire forces [N]
%   info   : diagnostic structure
%
% IMPORTANT:
% This is an engineering/teaching implementation of the classic
% Pacejka Magic Formula. It is NOT a full MF-Tyre/MF5.2/MF6.1 model.
% Full industrial MF models use many additional coefficients for load,
% camber, pressure, combined slip, turn-slip, shifts, etc.

Fz = max(Fz,0);

% Load-dependent peak friction coefficient.
loadRatio = max(Fz,1e-6) ./ p.Fz0;
muX = p.muX .* loadRatio.^p.muLoadExpX;
muY = p.muY .* loadRatio.^p.muLoadExpY;

Dx = muX .* Fz;
Dy = muY .* Fz;

Fx0 = magicFormulaPure(kappa,p.Bx,p.Cx,Dx,p.Ex,0,0);
Fy0 = magicFormulaPure(alpha, p.By,p.Cy,Dy,p.Ey,0,0);

if p.combinedSlipEnable
    % Normalized combined-slip coupling.
    % The force vector is smoothly limited to the available friction
    % capacity. This preserves the pure-slip MF shape while preventing
    % unrealistic Fx/Fy simultaneous peaks.
    nx = Fx0 ./ max(abs(Dx),1e-9);
    ny = Fy0 ./ max(abs(Dy),1e-9);

    normF = sqrt(nx.^2 + ny.^2);
    scale = ones(size(normF));
    idx = normF > 1;
    scale(idx) = 1 ./ normF(idx);

    Fx = Fx0 .* scale;
    Fy = Fy0 .* scale;
else
    Fx = Fx0;
    Fy = Fy0;
end

info.muX = muX;
info.muY = muY;
info.FxPure = Fx0;
info.FyPure = Fy0;
info.FrictionNorm = sqrt((Fx./max(abs(Dx),1e-9)).^2 + ...
                         (Fy./max(abs(Dy),1e-9)).^2);
end
