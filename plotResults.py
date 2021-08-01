#!/usr/bin/env python3
import numpy as np 
import matplotlib.pyplot as plt
import sys
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
    

inData = open("tmp_results.txt", "r")
lines = [x for x in inData]
sliceStart = int(sys.argv[1]) if len(sys.argv) > 1 else (int(len(lines)* 0.8))    
flights = [Flight(x) for x in lines[sliceStart:]]
generations = [f.generation for f in flights]
fitnesses = [f.fitness for f in flights]
fig,((axInp, axX, axPos), (axVel, axY, axFrames)) = plt.subplots(nrows=2, ncols=3)


AX_MAXVAL=600
"""for flight in flights:
    axInp.cla()
    axInp.plot(flight.controllerInput, '.')
    axInp.set_xlim((0, AX_MAXVAL))
    axInp.set_ylim((-72, 72))

    axVel.cla()
    axVel.plot(flight.baseSpeed, '.')
    axVel.set_xlim((0, AX_MAXVAL))

    axX.cla()
    axX.plot(flight.xPos, '.')
    axX.set_xlim((0, AX_MAXVAL))

    axY.cla()
    axY.plot(flight.yPos, '.')
    axY.set_xlim((0, AX_MAXVAL))

    plt.pause(0.1)
"""
axInp.cla()
axX.cla()
axVel.cla()
axY.cla()
sliceStart = int(sys.argv[1]) if len(sys.argv) > 1 else (int(len(flights)* 0.8))   
finalXVals = []
finalYVals = []
for flight in flights:
    if(flight == flights[-1]):
        dispMode = '.-'
    else: 
        dispMode = ','
    axInp.plot(flight.controllerInput, dispMode)
    axVel.plot(flight.baseSpeed, dispMode)
    axX.plot(flight.xPos, dispMode)
    axY.plot(flight.yPos, dispMode)
    axPos.plot(flight.xPos, flight.yPos, dispMode)
    finalXVals.append(flight.xPos[-1])
    finalYVals.append(flight.yPos[-1])
    

axPos.plot(finalXVals, finalYVals, '-')

axInp.set_xlim((0, AX_MAXVAL))
axInp.set_ylim((-73, 73))
axInp.set_xlabel("Frame")
axInp.set_ylabel("Controller position")
axVel.set_xlim((0, AX_MAXVAL))
axVel.set_xlabel("Frame")
axVel.set_ylabel("Base speed")
axX.set_xlim((0, AX_MAXVAL))
axX.set_xlabel("frame")
axX.set_ylabel("x position")
axY.set_xlim((0, AX_MAXVAL))
axY.set_xlabel("frame")
axY.set_ylabel("y position")
axPos.set_xlabel("x position")
axPos.set_ylabel("y position")
axFrames.plot(generations, fitnesses, ".")
axFrames.set_xlabel("Generation number")
axFrames.set_ylabel("Fitness score")
plt.show()
