#not use clikit-learn

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Step 1: 데이터 읽기 및 생성
data_home = 'D:\\'
lin_data = pd.read_csv(data_home + 'linear.csv')

x = lin_data['X'].to_numpy()
y_true = lin_data['Y'].to_numpy()

np.random.seed(0)
noise = np.random.normal(0, 1, len(x))
y = 5 * x + 50 * noise

# Step 2: MSE 함수 정의
def mse(y_true, y_pred):
    return np.mean((y_true - y_pred) ** 2)

# Step 3: 모델 함수 정의
def model(x, w, b):
    return w * x + b

# 경사하강법 파라미터 설정
learning_rate = 1e-4
n_iterations = 100

# 초기 파라미터 설정
w = 0.5
b = 0.5

# 경사하강법 학습
m = len(x)
for i in range(n_iterations):
    y_hat = model(x, w, b)
    dw = (2/m) * np.sum((y_hat - y) * x)
    db = (2/m) * np.sum(y_hat - y)
    w -= learning_rate * dw
    b -= learning_rate * db

# 최종 모델 출력 및 MSE 계산
y_hat = model(x, w, b)
final_mse = mse(y, y_hat)
print(f'Final w: {w}, Final b: {b}')
print(f'Final MSE: {final_mse}')

# 데이터와 학습된 모델 시각화
plt.scatter(x, y, label='Data')
plt.plot(x, y_hat, color='red', label='Model: y = wx + b')
plt.xlabel('Input (x)')
plt.ylabel('Output (y)')
plt.title('Data and Learned Linear Model')
plt.legend()
plt.show()

#use
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from sklearn.linear_model import LinearRegression
from sklearn.metrics import mean_squared_error

# Step 1: 데이터 읽기
data_home = 'D:\\'
lin_data = pd.read_csv(data_home + 'linear.csv')

x = lin_data['X'].to_numpy().reshape(-1, 1)  # Scikit-learn은 2D 배열을 입력으로 받음
y_true = lin_data['Y'].to_numpy()

# Step 2: Scikit-learn 모델 학습
model = LinearRegression()
model.fit(x, y_true)

# 예측값 계산
y_pred = model.predict(x)

# MSE 계산
mse_value = mean_squared_error(y_true, y_pred)
print(f'Final w: {model.coef_[0]}, Final b: {model.intercept_}')
print(f'Final MSE: {mse_value}')

# Step 3: 데이터와 학습된 모델 시각화
plt.scatter(x, y_true, label='Data')
plt.plot(x, y_pred, color='red', label='Model: y = wx + b')
plt.xlabel('Input (x)')
plt.ylabel('Output (y)')
plt.title('Data and Learned Linear Model (Scikit-learn)')
plt.legend()
plt.show()

