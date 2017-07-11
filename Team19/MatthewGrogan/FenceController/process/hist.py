import sys
import numpy
import math
import time
import pylab
from geopy.distance import vincenty

#file_list = [("../data/Data2147/sol_edit.pos",(30.621375,-96.304884))]
#file_list = [("../../../misc/7-10_test/test1_point1.pos",(30.664772,-96.323129)),("../../../misc/7-10_test/test1_point2.pos",(30.664791,-96.323181)),("../../../misc/7-10_test/test1_point3.pos",(30.664743,-96.323175))]
file_list = [("../data/Data2147/sol_edit.pos",(30.621355,-96.304884))]
data_list = [(pylab.loadtxt(filename), point) for filename, point in file_list] 

dlist = []
for data, point in data_list:
    for line in data:
        dlist.append(vincenty((line[0],line[1]),point).meters)


pylab.xlabel("Distance from point (m)",fontsize=28)
pylab.xticks(fontsize=20)
pylab.ylabel("Number of Occurances",fontsize=28)
pylab.yticks(fontsize=20)
bins = numpy.linspace(0,3,100)
pylab.hist(dlist, bins)
pylab.show()
