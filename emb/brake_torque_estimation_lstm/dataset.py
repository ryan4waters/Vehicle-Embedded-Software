# dataset.py
import numpy as np

def create_sequences(X, y, L):
    """
    对单个时间序列做滑动窗口。
    X: (N, d)
    y: (N,)
    L: 窗口长度
    返回:
        X_seq: (N-L+1, L, d)
        y_seq: (N-L+1,)
    """
    X_seq, y_seq = [], []
    for i in range(len(X) - L + 1):
        X_seq.append(X[i:i + L])
        y_seq.append(y[i + L - 1])  # 预测窗口最后一个时刻的扭矩
    return np.array(X_seq, dtype=np.float32), np.array(y_seq, dtype=np.float32)