import pymc3 as pm
import numpy as np
import ast

'''
y_g_i ~ floor(N(mu_g, sigma_g))
mu_g ~ N(beta_mu * X + alpha_mu, ?)
sigma_g ~ N(beta_sigma * X + alpha_sigma, ?)
'''

with open('gr', 'r') as gr_f:
    lines = [line[:-1] for line in gr_f.readlines()]

groups = np.array([ast.literal_eval(line.split(';')[0]) for line in lines]) 
print(groups)
