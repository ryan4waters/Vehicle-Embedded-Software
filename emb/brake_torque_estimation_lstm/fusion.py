# fusion.py
import numpy as np

class TorqueFusion:
    """
    校验与融合模块。
    维护滑动窗口误差，计算可信度 alpha，并输出融合扭矩。
    """
    def __init__(self, window_size=20, sigma_e=50.0):
        self.window_size = window_size
        self.sigma_e = sigma_e
        self.errors = []

    def update(self, T_lstm, T_mf):
        e = T_lstm - T_mf
        self.errors.append(e)
        if len(self.errors) > self.window_size:
            self.errors.pop(0)

        e_bar = np.mean(self.errors)
        # 可信度：偏差越小，alpha 越接近 1
        alpha = np.exp(- (e_bar ** 2) / (2.0 * self.sigma_e ** 2))
        T_final = alpha * T_lstm + (1.0 - alpha) * T_mf
        return T_final, alpha, e_bar