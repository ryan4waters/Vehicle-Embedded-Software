# train.py
import numpy as np
import joblib
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
import tensorflow as tf

import config as cfg
from data_generator import generate_dataset
from dataset import create_sequences
from model import build_model

def main():
    tf.random.set_seed(42)
    np.random.seed(42)

    print("生成仿真数据...")
    X_list, y_list = generate_dataset()

    # 合并所有原始时间步，用于拟合标准化器
    X_concat = np.vstack(X_list)
    y_concat = np.concatenate(y_list)

    scaler_X = StandardScaler().fit(X_concat)
    scaler_y = StandardScaler().fit(y_concat.reshape(-1, 1))

    # 对每个样本标准化并滑窗
    X_seq_all, y_seq_all = [], []
    for X, y in zip(X_list, y_list):
        X_scaled = scaler_X.transform(X).astype(np.float32)
        y_scaled = scaler_y.transform(y.reshape(-1, 1)).flatten().astype(np.float32)
        X_seq, y_seq = create_sequences(X_scaled, y_scaled, cfg.L)
        X_seq_all.append(X_seq)
        y_seq_all.append(y_seq)

    X_seq_all = np.concatenate(X_seq_all, axis=0)
    y_seq_all = np.concatenate(y_seq_all, axis=0)

    print("样本形状:", X_seq_all.shape, y_seq_all.shape)

    X_train, X_val, y_train, y_val = train_test_split(
        X_seq_all, y_seq_all, test_size=0.2, random_state=42
    )

    model = build_model(input_shape=(cfg.L, cfg.D))
    model.summary()

    callbacks = [
        tf.keras.callbacks.EarlyStopping(
            monitor='val_loss', patience=15, restore_best_weights=True
        ),
        tf.keras.callbacks.ReduceLROnPlateau(
            monitor='val_loss', factor=0.5, patience=8, min_lr=1e-5
        )
    ]

    history = model.fit(
        X_train, y_train,
        validation_data=(X_val, y_val),
        epochs=cfg.EPOCHS,
        batch_size=cfg.BATCH_SIZE,
        callbacks=callbacks,
        verbose=1
    )

    # 保存模型和标准化器
    model.save("brake_torque_lstm.keras")
    joblib.dump(scaler_X, "scaler_X.pkl")
    joblib.dump(scaler_y, "scaler_y.pkl")

    print("训练完成，模型已保存。")

if __name__ == "__main__":
    main()