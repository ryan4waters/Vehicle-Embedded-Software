function y = magicFormulaPure(x,B,C,D,E,Sv,Sh)
%MAGICFORMULAPURE Classic 4-parameter Pacejka Magic Formula.
%
% y = D*sin(C*atan(B*z - E*(B*z - atan(B*z)))) + Sv
% z = x + Sh
%
% Inputs may be scalar or arrays of equal size.

if nargin < 6, Sv = 0; end
if nargin < 7, Sh = 0; end

z = x + Sh;
Bx = B .* z;
y = D .* sin(C .* atan(Bx - E .* (Bx - atan(Bx)))) + Sv;
end
