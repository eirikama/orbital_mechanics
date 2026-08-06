from numpy import *
from matplotlib.pylab import *
import sys

CelestialBodies = int(sys.argv[2])
N = linspace(0,4*(CelestialBodies-1),CelestialBodies)


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

  plot(x,y)
title('The SolarSystem]',fontsize=16)
#legend([ r'Sun $\approx$ CM',  'Jupiter ','Earth'])
xlabel('$x$ [AU]',fontsize=16)
ylabel('$y$ [AU]',fontsize=16)
#axis([-1.2, 1.2, -1.2, 1.4])
show()

# x = array(x)
# y = array(y)
# r = sqrt(x**2 + y**2)
# l = zeros(len(r))

# for j in (N):
#   j = int(j)
#   vx = []
#   vy = []

#   infile = open(sys.argv[1], 'r')
#   for line in infile:           
#       a = line.split() 
#       vx.append(float(a[2+j]) )
#       vy.append(float(a[3+j]) )
#   infile.close


# plot(vx)
# plot(vy)
# legend(['vx','vy'])
# xlabel('x-axis [AU]')
# ylabel('y-axis [AU]')
# show()

# vx = array(vx)
# vy = array(vy)
# v = sqrt(vx**2 + vy**2)
# t = linspace(0,100,len(r))

# for i in range(len(r)):
#   theta = arctan(y[i]/x[i])
#   l[i] = (3e-6)*r[i]*v[i]*sin(theta)

# l0 = l[0]
# plot(t,abs((l0-l)/l0))
# title('Angular Momentum')
# legend(['$|l_0 - l(t))/l_0|$'])
# xlabel('$t$ [years]')
# show()


# plot(t,v)
# plot(t,r)
# title('Radial distance of Earth-Sun [RK4]',fontsize=16)
# legend(['$r$'])
# xlabel('$t$ [years]',fontsize=16)
# show()

# E0 = 3e-6*0.5*v[0]**2 - (3e-6*4*pi**2)/r[0]


# plot(t,abs((E0 - (3e-6*0.5*v**2 - (3e-6*4*pi**2)/r) )/E0))
# legend(['$|(E_0 - E(t))/E_0$|'])
# title('Relative Error in Total Energy [RK4]',fontsize=16)
# xlabel('$t$ [years]',fontsize=16) 
# show()

