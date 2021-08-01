#!/usr/bin/env python3
import numpy as np 
import matplotlib.pyplot as plt
import sys

if len(sys.argv) == 1:
    print("""Usage: 
    head -n -1 sim_result.txt > tmp_results.txt; 
    ./plotResults.py tmp_results.txt <<n>>
    where n is the milestone number you want to start plotting at. For example,
if the optimizer has gone through 5,000 generations and created 1000 lines of output, then you would use an n of 500 to look at the second half of the milestone outputs. 
    The first line just trims off the last line of the results file, since it may not be a complete line and therefore it would be unparseable.
    The file sim_result.txt is the file that ga_main writes as its --output-file argument. 
    Once the optimization has completed, you can run this script on the whole output file, of course. """)
    sys.exit(1)

class Flight:
    def __init__(self, flightLine):
        dats = [float(x) for x in flightLine.split(',')]
        self.generation = dats[0];
        self.collideFrame = dats[1];
        self.fitness = dats[2];
        self.distance_to_go_x = dats[3]
        self.distance_to_go_y = dats[4]
        flat_remaining = np.array(dats[5:])
        smoosh_remaining = np.reshape(flat_remaining, (-1, 5))
        self.controllerInput = smoosh_remaining[:,0]
        self.xPos = smoosh_remaining[:,1]
        self.yPos = smoosh_remaining[:,2]
        self.zPos = smoosh_remaining[:,3]
        self.baseSpeed = smoosh_remaining[:,4]
    

inData = open(sys.argv[1], "r")
lines = [x for x in inData]
sliceStart = int(sys.argv[2]) if len(sys.argv) > 2 else (int(len(lines)* 0.8))    
flights = [Flight(x) for x in lines[sliceStart:]]
print("Loaded flight data.")
generations = [f.generation for f in flights]
fitnesses = [f.fitness for f in flights]
grid_space = 0.04
gridspec = {"left" : grid_space, "right" : 1 - grid_space, "top" : 1 - grid_space, "bottom" : grid_space}
fig,((axInp, axX, axPos), (axVel, axY, axFrames)) = \
    plt.subplots(nrows=2, ncols=3, gridspec_kw = gridspec)

minX = 1e10
maxX = -1e10
minY = 1e10
maxY = -1e10
maxlen = 0
for flight in flights:
    fmaxX = np.max(flight.xPos)
    fminX = np.min(flight.xPos)
    fmaxY = np.max(flight.yPos)
    fminY = np.min(flight.yPos)
    minX = min(fminX, minX)
    maxX = max(fmaxX, maxX)
    minY = min(fminY, minY)
    maxY = max(fmaxY, maxY)
    maxlen = max(maxlen, len(flight.xPos))
print("Maxima & minima calculated.")
AX_MAXVAL=maxlen + 5
minX -= 0.04 * (maxX - minX)
maxX += 0.04 * (maxX - minX)
minY -= 0.04 * (maxY - minY)
maxY += 0.04 * (maxY - minY)
prevFlight = flights[0]
if False:
    for flight in flights:
        def plotInfo(ax, fn, xlim, ylim):
            ax.cla()
            ax.plot(fn(flight), '.', color='blue', markersize=5)
            ax.plot(fn(prevFlight), '.', color='red', markersize=1)
            ax.set_xlim(xlim)
            ax.set_ylim(ylim)
        plotInfo(axInp, lambda x : x.controllerInput, \
            (0, AX_MAXVAL), (-72,72))
        
        plotInfo(axVel, lambda x : x.baseSpeed, \
            (0, AX_MAXVAL), (0, 5.1))
        
        plotInfo(axX, lambda x : x.xPos, \
            (0, AX_MAXVAL), (minX, maxX))
    
        plotInfo(axY, lambda x : x.yPos, \
            (0, AX_MAXVAL), (minY, maxY))
    
    
        axPos.cla()
        axPos.plot(flight.xPos, flight.yPos, '.', color='blue', markersize = 5)
        axPos.plot(prevFlight.xPos, prevFlight.yPos, '.', color='red', markersize = 1)
        axPos.set_xlim((minX, maxX))
        axPos.set_ylim((minY, maxY))
        prevFlight = flight
        plt.pause(0.1)

axInp.cla()
axX.cla()
axVel.cla()
axY.cla()
finalXVals = []
finalYVals = []
finalFramenos = []
from scipy.ndimage import gaussian_filter1d, median_filter
filt = lambda x:  gaussian_filter1d(x, 3)
for flight in flights:
    dispMode = '.'
    if(flight == flights[-1]):
        markersize = 5
        markercolor = 'blue'
    else: 
        markersize = 0.5
        markercolor = 'red'
    axInp.plot(flight.controllerInput, dispMode, markersize = markersize, color=markercolor)
    axVel.plot(flight.baseSpeed, dispMode, markersize = markersize, color=markercolor)
    axX.plot(flight.xPos, dispMode, markersize = markersize, color=markercolor)
    axY.plot(flight.yPos, dispMode, markersize = markersize, color=markercolor)
    axPos.plot(flight.xPos, flight.yPos, dispMode, markersize = markersize, color=markercolor)
    finalXVals.append(flight.xPos[-1])
    finalYVals.append(flight.yPos[-1])
    finalFramenos.append(len(flight.xPos)-1)
    
axInp.plot(filt(flights[-1].controllerInput), '-', color='green')

axX.plot(finalFramenos, finalXVals, '-')
axY.plot(finalFramenos, finalYVals, '-')

axInp.set_xlim((0, AX_MAXVAL))
axInp.set_ylim((-73, 73))
axInp.set_xlabel("Frame")
axInp.set_ylabel("Controller position")
axVel.set_xlim((0, AX_MAXVAL))
axVel.set_ylim((0, 5.1))
axVel.set_xlabel("Frame")
axVel.set_ylabel("Base speed")
axX.set_xlim((0, AX_MAXVAL))
axX.set_ylim((minX, maxX))
axX.set_xlabel("frame")
axX.set_ylabel("x position")
axY.set_xlim((0, AX_MAXVAL))
axY.set_ylim((minY, maxY))
axY.set_xlabel("frame")
axY.set_ylabel("y position")
axPos.set_xlabel("x position")
axPos.set_ylabel("y position")
axPos.set_xlim((minX, maxX))
axPos.set_ylim((minY, maxY))
axFrames.plot(generations, fitnesses, ".")
axFrames.set_xlabel("Generation number")
axFrames.set_ylabel("Fitness score")
plt.show()
