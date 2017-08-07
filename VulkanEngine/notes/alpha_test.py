# -*- coding: utf-8 -*-
"""
Created on Mon Aug  7 16:45:31 2017

@author: Thomas Duborgel
"""

import numpy as np

r = np.array([1.0, 0.0, 0.0, 1.0])
t = np.array([0.5, 0.0, 0.0, 0.5])
n = np.array([0.0, 0.0, 0.0, 0.0])
w = np.array([1.0, 1.0, 1.0, 1.0])

back = np.array([0.25, 0.25, 0.25, 1.0])

def blend(src, dst):
    result = np.array([0,4])
    result = dst * (1-src[3])
    result += src
    return result

print(r)
print(back,"\n")
print(blend(r, back))
print(blend(t, back))
print(blend(n, back))
print(blend(w, back))