#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np
import subprocess
import json
from io import StringIO

def intToSize(sizeid):
    return [30, 50, 70, 200][sizeid]

entityColors = dict()
for line in open("colorInfo.dat", "r"):
    lsp = line.split()
    eid = int(lsp[0])
    if(eid not in entityColors):
        entityColors[eid] = dict()
    entityColors[eid][int(lsp[2])] = int(lsp[1])
def entityToColorID(entityId, generation):
    possibles = entityColors[entityId]
    rv = -1
    print(entityId, generation, possibles)
    for i in range(0, generation+1):
        if(i in possibles):
            rv = possibles[i]
    return rv

   
def intToColor(colorid):
    colors = [  
                #[0,0,0], 
                #[0, 0x49, 0x49],
                [0, 0x92, 0x92], 
                [0xff, 0x6d, 0xb6],
                #[0xff, 0xb6, 0xdb],
                [0x49, 0, 0x92],
                [0, 0x6d, 0xfb],
                #[0xb6, 0x6d, 0xff],
                #[0x6d, 0xb6, 0xff],
                #[0xb6, 0xdb, 0xff],
                [0x92, 0, 0],
                #[0x92, 0x49, 0],
                #[0xdb, 0x6d, 0],
                [0x24, 0xff, 0x24],
                [0xff, 0xff, 0x6d]]
    color = colors[colorid % len(colors)]
    
    return [color[0] / 256, color[1] / 256, color[2] / 256, 1]
class Flight:
    def __init__(self, inJson, generation, colorByParents=False):
        self._inputs = np.array(inJson["inputs"])
        self._colors = []
        source = inJson["source"]
        if(colorByParents and source["type"] != "seed"):
            color1 = intToColor(entityToColorID(source["parent"], generation-1))
            if(source["type"] == "interp"):
                color2 = intToColor(entityToColorID(source["otherParent"], generation-1))
                otherSize = source["crossEnd"] - source["crossStart"]
                finalSize = 100 - source["crossEnd"]
                self._colors = [color1] * source["crossStart"] +\
                                [color2] * otherSize + \
                                [color1] * finalSize
            else:
                self._colors = [color1] * 100
        else:
            color1 = intToColor(entityToColorID(inJson["id"], generation))
            self._colors = [color1] * 100
        print(len(self._colors))
        self._sizes = [intToSize(0)] * 100
        bigSize = intToSize(3)
        if(colorByParents):
            if(source["type"] in ["ROFF", "RPOI"]):
                self._sizes[int(source["args"][0])] = bigSize
            elif(source["type"] in ["RDRI", "RSET"]):
                for i in range(int(source["args"][0]), int(source["args"][1])):
                    self._sizes[i] = bigSize
            elif(source["type"] == "DILA"):
                for arg in source["args"][1:]:
                    self._sizes[int(arg)] = bigSize
        
        self.collideFrame = -1
        self.evaluation = inJson["evaluation"]
        self.source = inJson["source"]
        self.id = inJson["id"]

    def runSimulation(self):
        with open("inputs.txt", "w") as outFile:
            for elem in self._inputs:
                outFile.write("{0:d}\n".format(elem))
        with open("inputs.txt", "r") as inFile:
            ret = subprocess.run(["./simulate_single", "--player-dat", "playerdats/blooper.dat"], stdin = inFile, capture_output = True, text=True)
            so = StringIO(ret.stdout)
            simRes = np.loadtxt(so)
            collideStats = simRes[:,1]
            self.collideFrame = np.argmax(collideStats)
            self.landed = simRes[self.collideFrame, 2]
            self.xPoses = simRes[:self.collideFrame+1, 4]
            self.yPoses = simRes[:self.collideFrame+1, 5]
            self.zPoses = simRes[:self.collideFrame+1, 6]
            self.speeds = simRes[:self.collideFrame+1, 7]
            self.dirs = simRes[:self.collideFrame+1, 8]
        

    def plotxy(self, ax):
        ax.scatter(self.xPoses, self.yPoses, c=self._colors[:self.collideFrame+1], s=self._sizes)

    def plotInputs(self, ax, offset, collideSize = False):
        sizes = self._sizes
        if(collideSize):
            sizes = [intToSize(1)] * self.collideFrame + \
                    [intToSize(0)] * (100 - self.collideFrame)
        ax.scatter(range(100), self._inputs + offset, \
                    c=self._colors, s=sizes)
        ax.plot(range(100), self._inputs + offset, "k-")

def prepXy(ax, yMin, yMax):
    ax.plot([296.15, 296.15], [yMin, yMax], "b--")
    ax.plot([525, 525, 530], [yMin, 10, 10], "b--")

def showStart():
    vals = json.load(open("fs_init.json"))
    flights = [Flight(x) for x in vals]
    [x.runSimulation() for x in flights]

    fig, (ax1, ax2) = plt.subplots(nrows=1, ncols=2)
    yMin = min([min(f.yPoses) for f in flights])
    yMax = max([max(f.yPoses) for f in flights])
    prepXy(ax1, yMin, yMax)
    for i, flight in enumerate(flights):
        flight.plotInputs(ax2, 200*i, True)
        flight.plotxy(ax1)
    plt.show()

def showByEval(inJson, entityIds, generations, outFile):
    flights = []
    for elem in inJson:
        for i in range(len(entityIds)):
            if(elem["id"] == entityIds[i]):
                flight = Flight(elem, generations[i], False)
                flight.runSimulation()
                flights.append(flight)
    fig, (ax1, ax2) = plt.subplots(nrows=1, ncols=2)
    yMin = min([min(f.yPoses) for f in flights])
    yMax = max([max(f.yPoses) for f in flights])
    prepXy(ax1, yMin, yMax)
    for i, flight in enumerate(flights):
        flight.plotInputs(ax2, 200*i, True)
        flight.plotxy(ax1)
    fig.savefig(outFile)
    plt.close(fig)
        
def showByParents(inJson, entityIds, generations, outFile):
    flights = []
    for elem in inJson:
        for i in range(len(entityIds)):
            if(elem["id"] == entityIds[i]):
                flight = Flight(elem, generations[i], True)
                flight.runSimulation()
                flights.append(flight)
    fig, (ax1, ax2) = plt.subplots(nrows=1, ncols=2)
    yMin = min([min(f.yPoses) for f in flights])
    yMax = max([max(f.yPoses) for f in flights])
    prepXy(ax1, yMin, yMax)
    for i, flight in enumerate(flights):
        flight.plotInputs(ax2, 200*i, False)
        flight.plotxy(ax1)
    fig.savefig(outFile)
    plt.close(fig)
        
    
    

def showFirstMixes():
    vals = json.load(open("fs_firstmut.json"))
    flights = [Flight(x) for x in vals[:15]]
    [x.runSimulation() for x in flights]
    fig, (ax1, ax2) = plt.subplots(nrows=1, ncols=2)
    yMin = min([min(f.yPoses) for f in flights])
    yMax = max([max(f.yPoses) for f in flights])
    prepXy(ax1, yMin, yMax)
    for i, flight in enumerate(flights):
        flight.plotInputs(ax2, 200*i, False)
        flight.plotxy(ax1)
    plt.show()
    
    

def makeAll():
    inFp = open("liveEntities.dat", "r")
    firstGenerations = dict()
    inJson = json.load(open("blog.json", "r"))
    for line in inFp:
        lsp = line.split()
        mode = lsp[0]
        gen = int(lsp[1])
        if(mode == "L"):
            for elem in lsp[2:]:
                if(int(elem) not in firstGenerations):
                    firstGenerations[int(elem)] = gen
            showByParents(inJson, [int(x) for x in lsp[2:]],\
                         [firstGenerations[int(x)] for x in lsp[2:]], \
                          "L_{0:02d}.png".format(gen))
        elif(mode == "S"):
            showByEval(inJson, [int(x) for x in lsp[2:]], [gen] * 5, "S_{0:02d}".format(gen))

#showByEval(json.load(open("blog.json", "r")), [0,1,2,3,4], [0,0,0,0,0])
#showByParents(json.load(open("blog.json", "r")), [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14], [0]*5 + [1]*10)
#showByEval(json.load(open("blog.json", "r")), [0,6,8,12,14], [1,1,1,1,1])
#showByParents(json.load(open("blog.json", "r")), [0, 12, 6, 14, 8, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26], [0] + [1] * 4 + [2] * 12)
makeAll()
