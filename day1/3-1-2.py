import numpy as np
import matplotlib.pyplot as plt
from sklearn import cluster

def kmeas_predict_plot(X,k):
    model = cluster.KMeans(n_clusters=k)
    model.fit(X)
    labels = model.predict(X)
    
    colors = np.array(['red','green','blue','magenta'])
    plt.suptitle('K-means clustering,k={}'.format(k))
    plt.scatter(X[:,0], X[:,1], color=colors[labels])             

# Provided data for the dogs
samo_length = [75,77,83,81,73,99,72,83]
samo_height = [24,29,19,32,21,22,19,34]

dach_length = [76,78,82,88,76,83,81,89]
dach_height = [55,58,53,54,61,52,57,64]

maltese_length = [35,39,38,41,30,57,41,35]
maltese_height = [23,26,19,30,21,24,28,20]

# Data for A, B, C, D
data_A = [58, 30]
data_B = [61, 26]
data_C = [80, 41]
data_D = [75, 55]


dog_length = np.array(dach_length + samo_length + [data_A[0]] + [data_B[0]] + [data_C[0]] + [data_D[0]])
dog_height = np.array(dach_height + samo_height + [data_A[1]] + [data_B[1]] + [data_C[1]] + [data_D[1]])

dog_data = np.column_stack((dog_length, dog_height))

kmeas_predict_plot(dog_data, k=2)

#plt.title(("Dog data without label"))
#plt.scatter(dog_length, dog_height)

