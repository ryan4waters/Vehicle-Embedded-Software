# 基于 LSTM + 魔术公式的车辆制动扭矩估算与校验

## 运行步骤

1. 安装依赖：
   ```bash
   pip install -r requirements.txt
	```
	
2. 训练模型：
   ```bash
	python train.py
	```

3. 推理与校验可视化：
   ```bash
	python inference.py
	```

## 文件说明
config.py：全局参数
magic_formula.py：魔术公式纵向力模型
data_generator.py：仿真数据生成
dataset.py：滑动窗口构造
model.py：LSTM 网络
train.py：训练脚本
fusion.py：校验与融合模块
inference.py：推理与对比可视化

## 输出
brake_torque_lstm.keras：训练好的 LSTM 模型
scaler_X.pkl、scaler_y.pkl：标准化器
result.png：LSTM、魔术公式、融合输出对比图

## 打包成 zip
在项目根目录执行：

```bash
zip -r brake_torque_estimation_lstm.zip brake_torque_estimation_lstm/
```

然后就可以把 brake_torque_estimation_lstm.zip 发给其他人或部署到其他机器。

## 运行顺序
```bash
cd brake_torque_estimation_lstm
pip install -r requirements.txt
python train.py
python inference.py
```

运行 inference.py 后，会生成 result.png，图中包含：

* 真实制动扭矩
* LSTM 估计扭矩
* 魔术公式参考扭矩
* 融合后的最终扭矩
* LSTM 与魔术公式的偏差
* 可信度 alpha 随时间变化