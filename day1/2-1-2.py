import numpy as np
import matplotlib.pyplot as plt
from sklearn.neighbors import KNeighborsClassifier
from sklearn.metrics import confusion_matrix, ConfusionMatrixDisplay

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
m_label = np.full(len(m_data), 2)

# 데이터 합치기
dogs = np.concatenate((d_data, s_data, m_data))
labels = np.concatenate((d_label, s_label, m_label))

# 클래스 정의
dog_classes = {0: 'Dachshund', 1: 'Samoyed', 2: 'Maltese'}

# k-NN 모델 학습
k = 3
knn = KNeighborsClassifier(n_neighbors=k)
knn.fit(dogs, labels)
y_pred = knn.predict(dogs)  # 예측을 위해 전체 데이터를 사용

# Confusion Matrix 생성
cm = confusion_matrix(labels, y_pred)
disp = ConfusionMatrixDisplay(confusion_matrix=cm, display_labels=['Dachshund', 'Samoyed', 'Maltese'])

# Confusion Matrix 출력print("Confusion Matrix:")
print(cm)

# Confusion Matrix 시각화 (첫 번째 색상 형태: Blues)
disp.plot(cmap=plt.cm.Blues)
plt.title('Confusion Matrix (Blues)')
plt.show()

# Confusion Matrix 시각화 (두 번째 색상 형태: Greys)
disp.plot(cmap=plt.cm.Greys)
plt.title('Confusion Matrix (Greys)')
plt.show()

# Confusion Matrix 시각화 (세 번째 색상 형태: Purples)
disp.plot(cmap=plt.cm.Purples)
plt.title('Confusion Matrix (Purples)')
plt.show()
