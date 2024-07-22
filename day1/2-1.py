import matplotlib.pyplot as plt
from sklearn.neighbors import KNeighborsClassifier
import numpy as np

# 닥스훈트의 길이와 높이 데이터
dach_length = [77,78,85,83,73,77,73,80]
dach_height = [25,28,29,30,21,22,17,35]

# 사모예드의 길이와 높이 데이터
samo_length = [75,77,86,86,79,83,83,88]
samo_height = [56,57,50,53,60,53,49,61]

# 몰티즈의 길이와 높이 데이터
mal_length = [35,39,38,41,30,57,41,35]
mal_height = [23,26,19,30,21,24,28,20]

# 데이터 준비
d_data = np.column_stack((dach_length, dach_height))
d_label = np.zeros(len(d_data))

s_data = np.column_stack((samo_length, samo_height))
s_label = np.ones(len(s_data))

m_data = np.column_stack((mal_length, mal_height))
m_label = np.full(len(m_data), 2)  # np.twos 대신 np.full 사용

# 출력
print("Dachshund(",d_label[0],") : ", d_data,sep="")
print("Samoyed(",s_label[0],") : ", s_data,sep="")
print("Maltese(",m_label[0],") : ", m_data,sep="")


# 새로운 데이터 포인트
newdata = [[75, 35]]

# 데이터 합치기
dogs = np.concatenate((d_data, s_data, m_data))
labels = np.concatenate((d_label, s_label, m_label))

# 클래스 정의
dog_classes = {0: 'Dachshund', 1: 'Samoyed', 2: 'Maltese'}

# k-NN 모델 학습
k = 3
knn = KNeighborsClassifier(n_neighbors=k)
knn.fit(dogs, labels)
y_pred = knn.predict(newdata)

print('데이터', newdata, ', 판정결과:', dog_classes[int(y_pred[0])])

# 데이터 시각화
plt.scatter(dach_length, dach_height, c='red', label='Dachshund')
plt.scatter(samo_length, samo_height, c='blue', label='Samoyed')
plt.scatter(mal_length, mal_height, c='green', label='Maltese')
plt.scatter([newdata[0][0]], [newdata[0][1]], c='black', label='New Data', marker='x')
plt.xlabel('Length')
plt.ylabel('Height')
plt.legend()
plt.title('Dog Breed Classification')
plt.show()
