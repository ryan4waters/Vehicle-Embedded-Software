TireMagicFormula_R2025b
=======================

Purpose
-------
Educational/engineering starting point for a tire Magic Formula model
in MATLAB/Simulink R2025b.

Files
-----
MF_TireParams.m
    Example parameter set.

magicFormulaPure.m
    Classic Pacejka Magic Formula:
    y = D*sin(C*atan(Bz-E*(Bz-atan(Bz)))) + Sv

tireMagicFormula.m
    Pure longitudinal/lateral forces plus a simple combined-slip
    friction-capacity coupling.

run_MF_Tire_Demo.m
    Generates longitudinal, lateral and combined-slip plots.

build_TireMagicFormula_Simulink.m
    Creates TireMagicFormula_R2025b.slx programmatically.

TireMagicFormula_Validation.m
    Basic automated symmetry checks.

Quick start in MATLAB R2025b
----------------------------
1. Put this folder on the MATLAB path.
2. Run:
       run_MF_Tire_Demo
3. Build the Simulink model:
       build_TireMagicFormula_Simulink
4. Open:
       TireMagicFormula_R2025b.slx

Simulink interface
------------------
Inputs:
    Kappa     longitudinal slip ratio [-]
    Alpha_rad slip angle [rad]
    Fz_N      vertical tire load [N]

Outputs:
    Fx_N      longitudinal tire force [N]
    Fy_N      lateral tire force [N]
    MuX       load-dependent peak longitudinal friction coefficient
    MuY       load-dependent peak lateral friction coefficient

Important limitation
--------------------
This is NOT a full commercial MF-Tyre/MF5.2/MF6.1 implementation.
It is a clean control-development model for understanding parameter
meaning, force curves, load sensitivity and basic combined slip.

For production vehicle dynamics, the tire parameters should be
identified from measured tire-test data or supplied by a tire vendor.
