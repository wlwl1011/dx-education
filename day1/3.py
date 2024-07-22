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

#닥스훈트의 길이와 높이 데이터
dach_length = [77,78,85,83,73,77,73,80]
dach_height = [25,28,29,30,21,22,17,35]

samo_length = [75,77,86,86,79,83,83,88]
samo_height = [56,57,50,53,60,53,49,61]


dog_length = np.array(dach_length + samo_length)
dog_height = np.array(dach_height + samo_height)
cyan
dog_data = np.column_stack((dog_length, dog_height))

kmeas_predict_plot(dog_data, k=2)

#plt.title(("Dog data without label"))
#plt.scatter(dog_length, dog_height)

