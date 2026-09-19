# magic_formula.py
import numpy as np

def magic_formula_longitudinal(kappa, Fz, mu):
    """
    简化 Pacejka 魔术公式，计算轮胎纵向力。
    kappa: 滑移率，制动时为负
    Fz: 垂向载荷 N
    mu: 路面附着系数
    返回: Fx 纵向力 N，制动时为负
    """
    # 简化参数
    C = 1.5
    D = mu * Fz
    # 避免除零
    B = 10.0 / (C * D) if D > 1e-6 else 0.0
    E = 0.97

    Bx = B * kappa
    Fx = D * np.sin(C * np.arctan(Bx - E * (Bx - np.arctan(Bx))))
    return Fx