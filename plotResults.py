#!/usr/bin/env python3
import numpy as np 
import matplotlib.pyplot as plt

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
    

inData = open("sim_results.txt", "r")

flights = [Flight(x) for x in inData]

fig,((axInp, axX), (axVel, axY)) = plt.subplots(nrows=2, ncols=2)
for flight in flights:
    AX_MAXVAL=550
    axInp.cla()
    axInp.plot(flight.controllerInput)
    axInp.set_xlim((0, AX_MAXVAL))
    axInp.set_ylim((-72, 72))

    axVel.cla()
    axVel.plot(flight.baseSpeed)
    axVel.set_xlim((0, AX_MAXVAL))

    axX.cla()
    axX.plot(flight.xPos)
    axX.set_xlim((0, AX_MAXVAL))

    axY.cla()
    axY.plot(flight.yPos)
    axY.set_xlim((0, AX_MAXVAL))

    plt.pause(0.1)

axInp.cla()
axX.cla()
axVel.cla()
axY.cla()
        
for flight in flights[len(flights)//2:]:
    AX_MAXVAL=550
    axInp.plot(flight.controllerInput, '.')
    axInp.set_xlim((0, AX_MAXVAL))
    axInp.set_ylim((-72, 72))

    axVel.plot(flight.baseSpeed, '.')
    axVel.set_xlim((0, AX_MAXVAL))

    axX.plot(flight.xPos, '.')
    axX.set_xlim((0, AX_MAXVAL))

    axY.plot(flight.yPos,'.')
    axY.set_xlim((0, AX_MAXVAL))

plt.show()
