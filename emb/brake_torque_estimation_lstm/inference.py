# inference.py
import numpy as np
import joblib
import matplotlib.pyplot as plt
import tensorflow as tf

import config as cfg
from data_generator import generate_sample
from magic_formula import magic_formula_longitudinal
from fusion import TorqueFusion

def main():
    # 加载模型和标准化器
    model = tf.keras.models.load_model("brake_torque_lstm.keras")
    scaler_X = joblib.load("scaler_X.pkl")
    scaler_y = joblib.load("scaler_y.pkl")

    # 生成一个测试工况
    mu = 0.6
    v0 = 30.0
    brake_intensity = 0.8
    X_raw, y_true = generate_sample(mu, v0, brake_intensity)

    # 标准化
    X_scaled = scaler_X.transform(X_raw).astype(np.float32)

    # 滑窗推理
    L = cfg.L
    N = len(X_scaled)
    T_lstm_list = []
    T_mf_list = []
    T_final_list = []
    alpha_list = []

    fusion = TorqueFusion(window_size=20, sigma_e=50.0)

    for i in range(L - 1, N):
        x_seq = X_scaled[i - L + 1:i + 1].reshape(1, L, cfg.D)
        y_pred_scaled = model.predict(x_seq, verbose=0)
        T_lstm = scaler_y.inverse_transform(y_pred_scaled).flatten()[0]

        # 当前时刻魔术公式参考扭矩
        kappa = X_raw[i, 2]
        Fz = X_raw[i, 5]
        mu_est = X_raw[i, 6]
        Fx_mf = magic_formula_longitudinal(kappa, Fz, mu_est)
        T_mf = Fx_mf * cfg.RE

        T_final, alpha, e_bar = fusion.update(T_lstm, T_mf)

        T_lstm_list.append(T_lstm)
        T_mf_list.append(T_mf)
        T_final_list.append(T_final)
        alpha_list.append(alpha)

    # 时间轴对齐
    t = np.arange(L - 1, N) * cfg.DT

    plt.figure(figsize=(12, 8))

    plt.subplot(3, 1, 1)
    plt.plot(t, y_true[L - 1:], 'k-', label='真实扭矩')
    plt.plot(t, T_lstm_list, 'r--', label='LSTM 估计')
    plt.plot(t, T_mf_list, 'b-.', label='魔术公式参考')
    plt.plot(t, T_final_list, 'g-', label='融合输出')
    plt.ylabel('制动扭矩 (N·m)')
    plt.legend()
    plt.grid(True)

    plt.subplot(3, 1, 2)
    plt.plot(t, np.array(T_lstm_list) - np.array(T_mf_list), 'm-')
    plt.ylabel('LSTM - 魔术公式 (N·m)')
    plt.grid(True)

    plt.subplot(3, 1, 3)
    plt.plot(t, alpha_list, 'c-')
    plt.ylabel('可信度 alpha')
    plt.xlabel('时间 (s)')
    plt.grid(True)

    plt.tight_layout()
    plt.savefig("result.png")
    plt.show()

    print("推理完成，结果已保存为 result.png")

if __name__ == "__main__":
    main()