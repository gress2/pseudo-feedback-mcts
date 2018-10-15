from scipy.special import gamma
import numpy as np
import math

tds = dict()
with open('td', 'r') as td_f:
    lines = [int(line) for line in td_f.readlines()]

for line in lines:
    if line not in tds:
        tds[line] = 1
    else:
        tds[line] += 1

def likelihood(x, r, p):
    if r == 0 or p == 1 or p == 0:
        return float("-inf")
    return math.log(gamma(r + x) * p**r * (1 - p)**x) - math.log(gamma(r) * gamma(x + 1))

# grid search
max_r = min(tds.keys())
rs = list(range(1, max_r))
ps = np.linspace(0, 1, 50) 

max_r = rs[0]
max_p = ps[0]
max_ll = float("-inf") 

for r in rs:
    for p in ps:
        ll = 0
        for key in tds:
            ll += tds[key] * likelihood(key, r, p)
        if ll > max_ll:
            max_ll = ll
            max_r = r
            max_p = p

print("max_ll: " + str(max_ll))
print("max_r: " + str(max_r))
print("max_p: " + str(max_p))
