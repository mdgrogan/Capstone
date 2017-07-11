import sys
import numpy
import math
import time
import pylab
from geopy.distance import vincenty

file_list = [("../data/Data2147/sol_edit.pos",(30.621355,-96.304884),'blue')]
data_list = [(pylab.loadtxt(filename), point, color) for filename, point, color in file_list] 

count = 1
sd = 0.0
maxd = 0.0
dlist = []
for data, point, color in data_list:
    for line in data:
        dlist.append(vincenty((line[0],line[1]),point).meters)
        count += 1
        tmp = vincenty((line[0],line[1]),point).meters
        maxd = tmp if tmp>maxd else maxd
        sd += tmp**2
    # Subtract known point 
    data[:,0] -= point[0]
    data[:,1] -= point[1]
    # Convert to meters. Conversion accurate locally.
    # Constants arrived at using vincenty approx.
    data[:,0] /= 0.00000902
    data[:,1] /= 0.00001043
    pylab.scatter(data[:,0], data[:,1], s=1, color=color)

sd = sd/count
sd = math.sqrt(sd)
rms90 = sd*1.64
rms2 = sd*2

l1 = "r="+str('%.3f'%rms90)+", 90%"
l2 = "r="+str('%.3f'%rms2)+", 95%"
l3 = "r="+str('%.3f'%maxd)+", 100%"
circle1=pylab.Circle((0,0),radius=rms90,color='green',fill=False,label=l1)
circle2=pylab.Circle((0,0),radius=rms2,color='orange',fill=False,label=l2)
circle3=pylab.Circle((0,0),radius=maxd,color='red',fill=False,label=l3)
pylab.gca().add_patch(circle1)
pylab.gca().add_patch(circle2)
pylab.gca().add_patch(circle3)

pylab.legend(loc='upper right',prop={'size':20})
pylab.axhline(y=0, color='k')
pylab.axvline(x=0, color='k')
pylab.title("Position results",fontsize=15)
pylab.xlabel("W (m)",fontsize=15)
pylab.xlim([-5,5])
pylab.ylabel("N (m)",fontsize=15)
pylab.ylim([-5,5])

pylab.show()
