import numpy as np
import matplotlib.pyplot as plt

data = list()
for _ in range(150000):
    data.append(np.random.negative_binomial(4, 0.4) + 5)

plt.hist(data, 100)
plt.show()

