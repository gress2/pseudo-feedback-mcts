import pymc3 as pm
import numpy as np
import ast

'''
reward_i,g = N(a_g + b_g * depth, eps_g)
a_g ~ N(mu_a, sigma_a**2)
b_g ~ N(mu_b, sigma_b**2)
mu_a ~ N(0, 100)
sigma_a ~ HalfCauchy(5)
mu_b ~ N(0, 100)
sigma_b ~ HalfCauchy(5)
'''

with open('gr', 'r') as gr_f:
    lines = [line[:-1] for line in gr_f.readlines()]

num_groups = len(lines)
group_idx = list()
rewards = list()
depths = list()

idx = 0
for line in lines:
    group, depth = line.split(';')
    depth = int(depth)
    group = ast.literal_eval(group)
    group_idx += [idx] * len(group)
    depths += [depth] * len(group)
    rewards += group
    idx += 1 

group_idx = np.array(group_idx)
rewards = np.array(rewards)
depths = np.array(depths)

model = pm.Model()

with model:
    mu_a = pm.Normal('mu_a', mu=0, sd=100)
    sigma_a = pm.HalfCauchy('sigma_a', 5)
    mu_b = pm.Normal('mu_b', mu=0, sd=100)
    sigma_b = pm.HalfCauchy('sigma_b', 5)
    a = pm.Normal('a', mu=mu_a, sd=sigma_a, shape=num_groups) 
    b = pm.Normal('b', mu=mu_b, sd=sigma_b, shape=num_groups)
    eps = pm.HalfCauchy('eps', 5) 
    reward_est = a[group_idx] + b[group_idx] * depths
    reward_like = pm.Normal('reward_like', mu=reward_est, sd=eps, observed=rewards) 

map_estimate = pm.find_MAP(model=model)
print(map_estimate)

