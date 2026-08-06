from numpy import *
from matplotlib.pylab import *
import sys

CelestialBodies = int(sys.argv[2])
N = linspace(0,4*(CelestialBodies-1),CelestialBodies)
dt = 0.00005

for j in (N):
  j = int(j)
  x = []
  y = [] 
   
  infile = open(sys.argv[1], 'r')
  for line in infile:           
         a = line.split() 
         x.append(float(a[0+j]) )
         y.append(float(a[1+j]) )
  infile.close()

x = array(x)
y = array(y)
r = sqrt(x**2 + y**2)

n = 500
val2 = linspace (len(r)-n,len(r)-1, n)
val1 = linspace (0,n-1, n)


x0 = r[1]
for i in val1:
  if ( abs(r[i] < x0)):
    x0 = r[i]
    xp1 = x[i]
    yp1 = y[i]
    t1 = i*0.001

theta1 = arctan(yp1/xp1)

x0 = r[i]
for i in val2:
  
  if ( abs(r[i] < x0)):
    x0 = r[i]
    xp2 = x[i]
    yp2 = y[i]
    t2 = i*dt

theta2 = arctan(yp2/xp2)

print abs(theta1-theta2)*(3600*180/pi), 'arcseconds in', abs(t1-t2), 'years' 
