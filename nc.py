import pymc3 as pm
import numpy as np

'''
nc_i ~ floor(N(mu, sigma))
mu = alpha * depth_i + beta
sigma ~ HN(10)
alpha ~ N(0, 10)
beta ~ N(0, 50)
'''

with open('nc', 'r') as nc_f:
    lines = [line[:-1] for line in nc_f.readlines()]
    Y = np.array([int(line.split(',')[0]) for line in lines])
    X = np.array([int(line.split(',')[1]) for line in lines])

basic_model = pm.Model()

with basic_model:
    alpha = pm.Normal(mu=0, sd=10, name='alpha')
    beta = pm.Normal(mu=0, sd=50, name='beta')
    mu = alpha * X + beta
    sigma = pm.HalfNormal(sd=10, name='sigma')
    Y_obs = pm.math.floor(pm.Normal(mu=mu, sd=sigma, observed=Y, name='Y_obs'))

map_estimate = pm.find_MAP(model=basic_model)
print(map_estimate)

'''
alpha <- -0.51
beta <- 22.79
sigma <- 1.95
'''