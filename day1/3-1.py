import matplotlib.pyplot as plt

# Provided data for the dogs
samo_length = [75,77,83,81,73,99,72,83]
samo_height = [24,29,19,32,21,22,19,34]

dach_length = [76,78,82,88,76,83,81,89]
dach_height = [55,58,53,54,61,52,57,64]

maltese_length = [35,39,38,41,30,57,41,35]
maltese_height = [23,26,19,30,21,24,28,20]

# Data for A, B, C, D
data_A = (58, 30)
data_B = (61, 26)
data_C = (80, 41)
data_D = (75, 55)

# Plotting the data
plt.figure(figsize=(10, 6))

# Scatter plot for each dog breed
plt.scatter(samo_length, samo_height, color='blue', label='Samoyed', s=200)
plt.scatter(dach_length, dach_height, color='red', marker='^', label='Dachshund', s=100)
plt.scatter(maltese_length, maltese_height, color='green', marker='s', label='Maltese', s=100)

# Scatter plot for A, B, C, D with larger sizes and specific colors
plt.scatter(data_A[0], data_A[1], color='magenta', label='A', s=700)
plt.scatter(data_B[0], data_B[1], color='grey', label='B', s=700)
plt.scatter(data_C[0], data_C[1], color='cyan', label='C', s=700)
plt.scatter(data_D[0], data_D[1], color='green', label='D', s=700)

# Adding labels and title
plt.xlabel('Length')
plt.ylabel('Height')
plt.title('Dog size')
plt.legend()
plt.grid(True)

# Show the plot
plt.show()
