# data_generator.py
import numpy as np
import config as cfg
from magic_formula import magic_formula_longitudinal

def generate_sample(mu, v0, brake_intensity):
    """
    生成一个制动工况的时间序列。
    返回:
        X: (N, D)
        y: (N,) 制动扭矩
    """
    t = np.arange(0, cfg.T_TOTAL, cfg.DT)
    N = len(t)

    # 车速从 v0 线性降到接近 0
    vx = np.maximum(v0 - (v0 / cfg.T_TOTAL) * t, 0.5)

    # 滑移率：用正弦变化模拟制动过程，始终为负
    kappa_max = 0.3 * brake_intensity
    kappa = -kappa_max * np.sin(np.pi * t / cfg.T_TOTAL)

    # 垂向载荷，四轮平均
    Fz = cfg.M * cfg.G / 4.0 * np.ones(N)

    # 魔术公式计算纵向力与制动扭矩
    Fx = magic_formula_longitudinal(kappa, Fz, mu)
    Tb = Fx * cfg.RE   # 负值表示制动扭矩

    # 轮速
    omega = vx * (1.0 + kappa) / cfg.RE

    # 制动轮缸压力，与滑移率幅值成比例，加噪声
    p_b = -kappa * 5e6 + 0.1e6 * np.random.randn(N)
    p_b = np.clip(p_b, 0, None)

    # 纵向加速度
    ax = Fx / cfg.M

    # 横摆角速度、方向盘转角，简单加小噪声
    r = 0.01 * np.random.randn(N)
    delta = 0.001 * np.random.randn(N)

    # 组装特征矩阵
    X = np.column_stack([
        vx,
        omega,
        kappa,
        p_b,
        ax,
        Fz,
        mu * np.ones(N),
        r,
        delta
    ])
    y = Tb
    return X.astype(np.float32), y.astype(np.float32)


def generate_dataset(num_samples=cfg.NUM_SAMPLES):
    """
    生成多个工况样本，返回列表。
    """
    X_list, y_list = [], []
    rng = np.random.default_rng(42)

    for _ in range(num_samples):
        mu = rng.uniform(0.2, 1.0)
        v0 = rng.uniform(15.0, 40.0)
        brake_intensity = rng.uniform(0.3, 1.0)

        X, y = generate_sample(mu, v0, brake_intensity)
        X_list.append(X)
        y_list.append(y)

    return X_list, y_list