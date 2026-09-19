# model.py
import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import LSTM, Dense, Dropout
import config as cfg

def build_model(input_shape):
    model = Sequential([
        LSTM(cfg.HIDDEN1, return_sequences=True, input_shape=input_shape),
        Dropout(cfg.DROPOUT),
        LSTM(cfg.HIDDEN2, return_sequences=False),
        Dropout(cfg.DROPOUT),
        Dense(16, activation='relu'),
        Dense(1, activation='linear')
    ])
    model.compile(
        optimizer=tf.keras.optimizers.Adam(learning_rate=cfg.LR),
        loss='mse',
        metrics=['mae']
    )
    return model