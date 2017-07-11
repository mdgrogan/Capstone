import sys
import numpy
import math
import time
import pylab
from geopy.distance import vincenty

file_list = [("../misc/7-10_test/test1_point1.pos",(30.664772,-96.323129),'Point 1','blue'),("../misc/7-10_test/test1_point2.pos",(30.664791,-96.323181),'Point 2','green'),("../misc/7-10_test/test1_point3.pos",(30.664743,-96.323175),'Point 3','purple')]
#file_list = [("../misc/7-10_test/test2_point1.pos",(30.664772,-96.323129)),("../misc/7-10_test/test2_point2.pos",(30.664791,-96.323181)),("../misc/7-10_test/test2_point3.pos",(30.664743,-96.323175))]
data_list = [(pylab.loadtxt(filename), point, label, color) for filename, point, label, color in file_list] 

count = 1
sd = 0.0
maxd = 0.0
dlist = []
for data, point, label, color in data_list:
    for line in data:
        dlist.append(vincenty((line[0],line[1]),point).meters)
#        count += 1
#        tmp = vincenty((line[0],line[1]),point).meters
#        maxd = tmp if tmp>maxd else maxd
#        sd += tmp**2

    # Subtract known point 
#    data[:,0] -= point[0]
#    data[:,1] -= point[1]
    # Convert to meters. Conversion accurate locally.
    # Constants arrived at using vincenty approx.
#    data[:,0] /= 0.00000902
#    data[:,1] /= 0.00001043
#    pylab.scatter(data[:,0], data[:,1], label=label, s=1, color=color)

#sd = sd/count
#sd = math.sqrt(sd)
#rms2 = sd*2

#l1 = "r="+str('%.3f'%rms2)+", 95%"
#l2 = "r="+str('%.3f'%maxd)+", 100%"
#circle1=pylab.Circle((0,0),radius=rms2,color='orange',fill=False,label=l1)
#circle2=pylab.Circle((0,0),radius=maxd,color='red',fill=False,label=l2)
#pylab.gca().add_patch(circle1)
#pylab.gca().add_patch(circle2)

pylab.legend(loc='upper right',prop={'size':20})
#pylab.axhline(y=0, color='k')
#pylab.axvline(x=0, color='k')
pylab.title("Position results",fontsize=15)
#pylab.xlabel("W (m)",fontsize=15)
#pylab.xlim([-1,1])
#pylab.ylabel("N (m)",fontsize=15)
#pylab.ylim([-1,1])

pylab.xlabel("Distance from point (m)",fontsize=15)
pylab.ylabel("Frequency of occurance",fontsize=15)
bins = numpy.linspace(0,0.7,150)
pylab.hist(dlist, bins)
pylab.show()
