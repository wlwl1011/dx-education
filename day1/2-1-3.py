import numpy as np
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split
from sklearn.neighbors import KNeighborsClassifier
from sklearn.metrics import confusion_matrix, ConfusionMatrixDisplay, precision_score, recall_score, accuracy_score, f1_score

# 데이터 생성
dach_length = [75,77,83,81,73,99,72,83]
dach_height = [24,29,19,32,21,22,19,34]

samo_length = [76,78,82,88,76,83,81,89]
samo_height = [55,58,53,54,61,52,57,64]

mal_length = [35, 39, 38, 41, 30, 57, 41, 35]
mal_height = [23, 26, 19, 30, 21, 24, 28, 20]

# 데이터 준비
d_data = np.column_stack((dach_length, dach_height))
s_data = np.column_stack((samo_length, samo_height))
m_data = np.column_stack((mal_length, mal_height))

# Label 생성
d_label = np.zeros(len(d_data))  # 닥스훈트: 0
s_label = np.ones(len(s_data))   # 사모예드: 1
m_label = np.full(len(m_data), 2)  # 말티즈: 2

# 모든 데이터와 라벨 결합
dogs = np.concatenate((d_data, s_data, m_data))
labels = np.concatenate((d_label, s_label, m_label))

# 데이터 분할
X_train, X_test, y_train, y_test = train_test_split(dogs, labels, test_size=0.3)

# k-NN 모델 학습
k = 3
knn = KNeighborsClassifier(n_neighbors=k)
knn.fit(X_train, y_train)

# 예측
y_pred = knn.predict(X_test)

# Confusion Matrix 생성
cm = confusion_matrix(y_test, y_pred, labels=[0, 1, 2])
disp = ConfusionMatrixDisplay(confusion_matrix=cm, display_labels=['Dachshund', 'Samoyed', 'Maltese'])

# Confusion Matrix 출력
print("Confusion Matrix:")
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

# 닥스훈트와 말티즈의 결과값만 추출
cm_dach_malt = cm[[0, 2]][:, [0, 2]]

# 직접 계산
TP_dach = cm_dach_malt[0, 0]
FP_dach = cm_dach_malt[1, 0]
FN_dach = cm_dach_malt[0, 1]
TN_dach = cm_dach_malt[1, 1]

precision_dach = TP_dach / (TP_dach + FP_dach)
recall_dach = TP_dach / (TP_dach + FN_dach)
accuracy_dach = (TP_dach + TN_dach) / cm_dach_malt.sum()
f1_dach = 2 * (precision_dach * recall_dach) / (precision_dach + recall_dach)

# 직접 계산한 값 출력
print("\n직접 계산한 값:")
print(f"Precision: {precision_dach:.3f}")
print(f"Recall: {recall_dach:.3f}")
print(f"Accuracy: {accuracy_dach:.3f}")
print(f"F1-score: {f1_dach:.3f}")

# Scikit-learn 함수를 사용한 계산
y_true_dach_malt = y_test[(y_test == 0) | (y_test == 2)]
y_pred_dach_malt = y_pred[(y_test == 0) | (y_test == 2)]

precision_sklearn = precision_score(y_true_dach_malt, y_pred_dach_malt, average='binary', pos_label=0)
recall_sklearn = recall_score(y_true_dach_malt, y_pred_dach_malt, average='binary', pos_label=0)
accuracy_sklearn = accuracy_score(y_true_dach_malt, y_pred_dach_malt)
f1_sklearn = f1_score(y_true_dach_malt, y_pred_dach_malt, average='binary', pos_label=0)

# Scikit-learn 함수 결과 출력
print("\nScikit-learn 함수 결과:")
print(f"Precision: {precision_sklearn:.3f}")
print(f"Recall: {recall_sklearn:.3f}")
print(f"Accuracy: {accuracy_sklearn:.3f}")
print(f"F1-score: {f1_sklearn:.3f}")
