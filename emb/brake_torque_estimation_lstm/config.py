# config.py

# 时间窗口长度
L = 50
# 输入特征维度
D = 9

# LSTM 网络参数
HIDDEN1 = 64
HIDDEN2 = 32
DROPOUT = 0.2
BATCH_SIZE = 64
EPOCHS = 100
LR = 0.001

# 车辆参数
RE = 0.3          # 轮胎有效滚动半径 m
M = 1500.0        # 整车质量 kg
G = 9.81          # 重力加速度
DT = 0.01         # 采样周期 s

# 数据生成参数
NUM_SAMPLES = 80  # 生成多少个制动工况样本
T_TOTAL = 3.0     # 每个样本时长 s