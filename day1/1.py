
#1
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Step 1: 데이터 읽기 및 생성
# Step 1: 데이터 읽기 및 생성
data_home = 'D:\\'
lin_data = pd.read_csv(data_home + 'linear.csv')

x = lin_data['X'].to_numpy()
y_true = lin_data['Y'].to_numpy()

np.random.seed(0)
noise = np.random.normal(0, 1, len(x))
y = 5 * x + 50 * noise

# Step 2: MSE 함수 정의def mse(y_true, y_pred):
    return np.mean((y_true - y_pred) ** 2)

# Step 3: 모델 함수 정의def model(x, w, b):
    return w * x + b

# w와 b의 초기값 설정
w = 0.5
b = 0.5

# y_hat 계산
y_hat = model(x, w, b)

# Step 4: 데이터 시각화 및 MSE 출력
error = mse(y, y_hat)
print(f'MSE: {error}')

plt.scatter(x, y, label='Data')
plt.plot(x, y_hat, color='red', label='Model: y = wx + b')
plt.xlabel('Input (x)')
plt.ylabel('Output (y)')
plt.title('Data and Linear Model')
plt.legend()
plt.show()

######################################################################

# learning_iteration = 1000 #하이퍼파라미터 : 학습반복횟수
# learning_rate = 0.0025

# param = [1,1]

# x = lin_data['input'].to_numpy()
# y = lin_data['pollution'].to_numpy()

# for i in range(learning_iteration):
#     if i % 200 == 0:
#         lin_data.plot(kind = 'scatter', x = 'input', y = 'pollution')
#         plt.plot([0,1], [h(0,param), h(1,param)])
    
#     error = ( h(x,param) - y)
#     param[0] -= learning_rate * (error*x).sum()
#     param[1] -= learning_rate * error.sum()
    
    
# x = np.array([1,4.5,9,10,13])
# y = np.array([0,0.2,2.5,5.4,7.3])

# w_list = np.arange(1.0,0.2,-0.1)

# print('start!')
# print(w_list)
# for w in list(w_list):
#     print(w)
#     y_hat = w*x
#     print('w = {:.1f}, 평균제곱 오차: {:.2f}'.format(w,mse(y_hat,y)))

#data_home = 'https://github.com/dknife/ML/raw/main/data/'
#lin_data = pd.read_csv(data_home+'pollution.csv')
#print(lin_data)



#w,b = -3,6
#x0, x1 = 0.0, 1.0

#def h(x,w,b):
#    return w*x + b


#lin_data.plot(kind = 'scatter', x = 'input', y='pollution')
#plt.plot([x0,x1],[h(x0,w,b), h(x1,w,b)])