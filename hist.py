import matplotlib.pyplot as plt

with open('td') as td:
    tds = [ int(line[:-1]) for line in td.readlines() ]

plt.hist(tds, 100)
plt.show()
