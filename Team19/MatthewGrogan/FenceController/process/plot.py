import sys
import numpy
import math
import time
import pylab

pylab.plot([30.621375,30.621417],[-96.304884,-96.304838],linewidth=2,color='black')
pylab.plot([30.621417,30.621313],[-96.304838,-96.304807],linewidth=2,color='black')
pylab.plot([30.621313,30.621272],[-96.304807,-96.304853],linewidth=2,color='black')
pylab.plot([30.621272,30.621375],[-96.304853,-96.304884],linewidth=2,color='black')

pylab.plot([30.6214,30.62140902],[-96.3048,-96.3048],linewidth=3,color='black')
pylab.plot([30.6214,30.6214],[-96.3048,-96.30481043],linewidth=3,color='black')
pylab.text(30.6214,-96.3047965,u'1 meter',fontsize=25)

#file_list = [("../data/data1/IN.txt",'green'),("../data/data1/OUT.txt",'red')]
file_list = [("../data/data2/IN.txt",'green'),("../data/data2/ON.txt",'orange'),("../data/data2/OUT.txt",'red')]
#file_list = [("../Subsystem/solution/goot.pos",'blue')]
data_list = [(pylab.loadtxt(filename), color) for filename, color in file_list] 

for data, color in data_list:
    pylab.scatter(data[:,0], data[:,1], s=3, color=color)

pylab.title("Position results",fontsize=30)
pylab.xlabel("Lat (decimal deg)",fontsize=30)
pylab.ylabel("Lon (decimal deg)",fontsize=30)
pylab.xlim([30.62126,30.62143])
pylab.ylim([-96.30478,-96.3049])
pylab.gca().axes.get_xaxis().set_ticks([])
pylab.gca().axes.get_yaxis().set_ticks([])
pylab.show()
