function run_MF_Tire_Demo()
%RUN_MF_TIRE_DEMO Plot the main characteristics of the example tire model.
%
% MATLAB R2025b.

p = MF_TireParams();

%% 1. Longitudinal force vs slip ratio
FzList = [2000 4000 6000];
kappa = linspace(-0.25,0.25,1001);

figure('Name','Magic Formula - Longitudinal');
hold on; grid on;
for Fz = FzList
    [Fx,~,~] = tireMagicFormula(kappa,zeros(size(kappa)),Fz,p);
    plot(kappa,Fx/1000,'LineWidth',1.4, ...
        'DisplayName',sprintf('F_z = %.0f N',Fz));
end
xlabel('\kappa [-]');
ylabel('F_x [kN]');
title('Pacejka Magic Formula - Longitudinal Force');
legend('Location','best');

%% 2. Lateral force vs slip angle
alphaDeg = linspace(-15,15,1001);
alpha = deg2rad(alphaDeg);

figure('Name','Magic Formula - Lateral');
hold on; grid on;
for Fz = FzList
    [~,Fy,~] = tireMagicFormula(zeros(size(alpha)),alpha,Fz,p);
    plot(alphaDeg,Fy/1000,'LineWidth',1.4, ...
        'DisplayName',sprintf('F_z = %.0f N',Fz));
end
xlabel('\alpha [deg]');
ylabel('F_y [kN]');
title('Pacejka Magic Formula - Lateral Force');
legend('Location','best');

%% 3. Combined slip surface
kappa2 = linspace(-0.20,0.20,81);
alpha2 = deg2rad(linspace(-12,12,81));
[K,A] = meshgrid(kappa2,alpha2);

[FX,FY,~] = tireMagicFormula(K,A,p.Fz0,p);

figure('Name','Magic Formula - Combined Slip');
surf(K,A*180/pi,FX/1000,'EdgeColor','none');
grid on;
xlabel('\kappa [-]');
ylabel('\alpha [deg]');
zlabel('F_x [kN]');
title('Combined Slip: F_x');

figure('Name','Magic Formula - Combined Slip Fy');
surf(K,A*180/pi,FY/1000,'EdgeColor','none');
grid on;
xlabel('\kappa [-]');
ylabel('\alpha [deg]');
zlabel('F_y [kN]');
title('Combined Slip: F_y');

%% 4. Simulink model
fprintf('\nTo build the Simulink model, run:\n');
fprintf('  build_TireMagicFormula_Simulink\n\n');
end
