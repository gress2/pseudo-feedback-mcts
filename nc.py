import pymc3 as pm
import numpy as np

'''
nc_i ~ Poisson(theta)
theta = exp(beta * d_i + alpha)
alpha ~ N(0, 10)
beta ~ N(0, 10)
'''

with open('nc', 'r') as nc_f:
    lines = [line[:-1] for line in nc_f.readlines()]

Y = np.array([int(line.split(',')[0]) for line in lines])
X = np.array([int(line.split(',')[1]) for line in lines])

basic_model = pm.Model()

with basic_model:
    beta = pm.Normal(mu=0, sd=10, name='beta')
    alpha = pm.Normal(mu=0, sd=10, name='alpha')
    theta = beta * X + alpha
    like = pm.Poisson(mu=np.exp(theta), observed=Y, name='like')

map_estimate = pm.find_MAP(model=basic_model)
print(map_estimate)

'''
beta = -0.066
alpha = 3.567
'''
