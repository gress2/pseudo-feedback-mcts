import pysal
import numpy as np

w = [[0.1, 1, 1, 1, 1], [1, 0.1, 1, 1, 1], [1, 1, 0.1, 1, 1], [1, 1, 1, 0.1, 1], [1, 1, 1, 1, 0.1]]
x = [1, 1, 1999, 0, 1]

y = np.array(x)
w = pysal.weights.util.full2W(np.array(w))

mi = pysal.Moran(y, w, two_tailed=False)
print(mi.I)

