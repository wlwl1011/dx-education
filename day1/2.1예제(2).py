
import matplotlib.pyplot as plt
from sklearn.neighbors import KNeighborsClassifier
import numpy as np

#닥스훈트의 길이와 높이 데이터
dach_length = [77,78,85,83,73,77,73,80]
dach_height = [25,28,29,30,21,22,17,35]

samo_length = [75,77,86,86,79,83,83,88]
samo_height = [56,57,50,53,60,53,49,61]

d_data = np.column_stack((dach_length,dach_height))
d_label = np.zeros(len(d_data))
s_data = np.column_stack((samo_length,samo_height))
s_label = np.ones(len(s_data))

newdata = [[75,35]]

dogs = np.concatenate((d_data,s_data))
labels = np.concatenate((d_label,s_label))

dog_classes = {0:'Dachshund', 1: 'Samoyed'}

k = 3
knn = KNeighborsClassifier(n_neighbors = k)
knn.fit(dogs, labels)
y_pred = knn.predict(newdata)
print('데이터', newdata,', 판정결과:',dog_classes[y_pred[0]])

