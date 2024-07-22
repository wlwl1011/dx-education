
import matplotlib.pyplot as plt
from sklearn.neighbors import KNeighborsClassifier
import numpy as np

#닥스훈트의 길이와 높이 데이터
dach_length = [77,78,85,83,73,77,73,80]
dach_height = [25,28,29,30,21,22,17,35]

samo_length = [75,77,86,86,79,83,83,88]
samo_height = [56,57,50,53,60,53,49,61]


plt.scatter(dach_length,dach_height,c='red',label='Dachshund')

plt.scatter(samo_length,samo_height,c='blue',label='Samoyed')

plt.xlabel('Length')
plt.ylabel('Height')
plt.title("Dog size")
plt.legend(loc='upper left')

plt.show()