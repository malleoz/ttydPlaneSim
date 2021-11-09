#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.patches as mpatches
import math
import random
import tqdm
#Same seed every time. 
random.seed(1)

   
def intToColor(colorid):
    colors = [  
                #[0,0,0], 
                [0, 0x49, 0x49],
                #[0, 0x92, 0x92], 
                [0xff, 0x6d, 0xb6],
                #[0xff, 0xb6, 0xdb],
                [0x49, 0, 0x92],
                [0, 0x6d, 0xfb],
                [0xb6, 0x6d, 0xff],
                #[0x6d, 0xb6, 0xff],
                [0xb6, 0xdb, 0xff],
                [0x92, 0, 0],
                #[0x92, 0x49, 0],
                [0xdb, 0x6d, 0],
                [0x24, 0xff, 0x24],
                [0xf0, 0xf0, 0x6d]]
    color = colors[colorid % len(colors)]
    return [color[0] / 256, color[1] / 256, color[2] / 256, 1]

"""
fig,ax = plt.subplots()

for i in range(14):
    circle = plt.Circle((i, 0), 0.5, fc=intToColor(i))
    ax.add_patch(circle)
ax.set_xlim(-1, 15)
ax.set_ylim(-1, 1)
plt.show()


"""


    

SIGMA = 1
EPSILON = 0.1
DX = 0.00001
MASS=1
CHILD_SCALE=1
FPS=60
class Cell:
    def __init__(self, color, boundaryRadius):
        self._color = color
        self.origColor = list(color)
        self.boundaryRadius = boundaryRadius
        radialPosition = boundaryRadius * math.sqrt(random.random())
        theta = random.random() * 2*math.pi
        self.x = radialPosition * math.cos(theta)
        self.y = radialPosition * math.sin(theta)
        self.vx = 0
        self.vy = 0
        self.ax = 0
        self.ay = 0
        self.drawRadius = 0.5
        self.theta = None

    def wallEnergy(self):
        #Use an exponential that crosses +1 at the wall. 
        r = (self.x **2 + self.y **2) ** 0.5
        return math.exp(r - self.boundaryRadius)
    def singleParticleEnergy(self, testX, testY):
        deltaR = ((self.x - testX) **2 + (self.y - testY) **2)**0.5
        lj = 4*EPSILON* ((SIGMA/deltaR)**12 - (SIGMA / deltaR)**6)
        if(lj > 2):
            #print("Overflow; dr = ", deltaR)
            lj = 3 - deltaR
        return lj
    def particleEnergy(self, particles):
        e = sum([self.singleParticleEnergy(other.x, other.y) for other in particles if (other is not self)])
        return e
    
    def getForce(self, particles):
        centerEnergy = self.wallEnergy() + self.particleEnergy(particles)
        origX = self.x
        self.x += DX
        xEnergy = self.wallEnergy() + self.particleEnergy(particles)
        xForce = -(xEnergy - centerEnergy) / DX
        self.x = origX

        origY = self.y
        self.y += DX
        yEnergy = self.wallEnergy() + self.particleEnergy(particles)
        yForce = -( yEnergy - centerEnergy) / DX
        self.y = origY
        return (xForce, yForce)
    def applyForce(self, dt, xForce, yForce):
        vxPlusHalf = self.vx + 0.5 * self.ax * dt
        self.x += vxPlusHalf*dt
        self.ax = xForce/MASS
        self.vx = vxPlusHalf+ 1/2*self.ax*dt
        vyPlusHalf = self.vy + 0.5 * self.ay * dt
        self.y += vyPlusHalf*dt
        self.ay = yForce/MASS
        self.vy = vyPlusHalf+ 1/2*self.ay*dt
    def thermostat(self):
        self.vx *= 0.9
        self.vx += random.random() - 0.5
        self.vy *= 0.9
        self.vy += random.random() - 0.5
    def plot(self, ax):
        if(len(self._color) == 4):
            circle = plt.Circle((self.x, self.y), self.drawRadius, fc=self._color)
            ax.add_patch(circle)
        else: 
            if(self.theta is None):
                self.theta = random.random() * 360
            theta =self.theta
            theta2 = theta - 180 if theta > 180 else theta + 180
            wedge1 = mpatches.Wedge((self.x, self.y), self.drawRadius, theta, theta2, fc=self._color[0])
            wedge2 = mpatches.Wedge((self.x, self.y), self.drawRadius, theta2, theta, fc=self._color[1])
            ax.add_patch(wedge1)
            ax.add_patch(wedge2)
    def _desaturateColor(self, color, desatFrac):
        r,g,b,_ = color
        
        med = (r + g + b) / 3
        r = r * desatFrac + med * (1 - desatFrac)
        g = g * desatFrac + med * (1 - desatFrac)
        b = b * desatFrac + med * (1 - desatFrac)
        return (r,g,b,1)
            

    def desaturate(self, frac):
        self.drawRadius = (frac/2)
        
        if(len(self._color) == 4):
            self._color = self._desaturateColor(self.origColor, frac)
        else:
            color1 = self._desaturateColor(self.origColor[0], frac)
            color2 = self._desaturateColor(self.origColor[1], frac)
            self._color = [color1, color2]
    def driftColor(self):
        prev = self._color[:3]
        diff = [random.random() * 0.5 - 0.25 for x in range(3)]
        def moveColor(prev, diff):
            if(prev + diff < 0):
                return prev - diff
            if(prev + diff > 1):
                return prev - diff
            return prev + diff
        rgb = [moveColor(prev[i], diff[i]) for i in range(3)]
        self._color = rgb + [1]
                
    def getChildren(self, otherCells, acceptedChildren):
        #Pick five directions at random.
        NUM_CHILDREN = 5
        oc = [x for x in otherCells if x is not self]
        angles = [random.random() * 2 * math.pi for x in range(NUM_CHILDREN)]
        childOffsets = [(math.cos(x), math.sin(x)) for x in angles]
        children = [Cell(self._color, self.boundaryRadius) for x in range(NUM_CHILDREN)]
        energies = []
        for i in range(NUM_CHILDREN):
            children[i].x = self.x + childOffsets[i][0] * CHILD_SCALE
            children[i].y = self.y + childOffsets[i][1] * CHILD_SCALE
            energy = children[i].particleEnergy(oc + children) + children[i].wallEnergy()
            if(energy < 2):
                energies.append((energy,i))
        energies.sort(key=lambda x:x[0])
        if(len(energies) > acceptedChildren):
            energies = energies[:acceptedChildren]
        kids = [children[x[1]] for x in energies]
        
        return kids

times = {"showParents" : 15,
        "addChildren" : 3,
        "showChildren" : 11,
        "driftChildren" : 3,
        "showDrift" : 18,
        "addMixing" : 3,
        "showMixing" : 26,
        "desat" : 3,
        "showFinal" : 10
        }
totalTime = sum(times.values()) * FPS
PLOT_LIM=10.1
pbar = tqdm.tqdm(total=totalTime)
def makePlot(cells, fname):
    fig = plt.figure(figsize=(3,3))
    ax = fig.add_axes([0.05, 0.05, 0.9, 0.9])
    ax.axis("off")
    ax.set_xlim(-PLOT_LIM,PLOT_LIM)
    ax.set_ylim(-PLOT_LIM,PLOT_LIM)
    ax.add_patch(plt.Circle((0,0), 10, fill=False, ec='k'))
    for c in cells:
        c.plot(ax)
    fig.savefig(fname, dpi=300)
    plt.close(fig)
    pbar.update()
    
        
NUM_SURVIVORS = 10
cells = [Cell(intToColor(i), 9) for i in range(NUM_SURVIVORS)]
cells = [x for x in cells if x.particleEnergy(cells) < 10]


#ax = plt.axes(xlim=[-PLOT_LIM,PLOT_LIM], ylim=[-PLOT_LIM,PLOT_LIM])
#for c in cells:
#    c.plot(ax)
for i in range(times["showParents"]*FPS):
    #print("Phase A, frame ", i)
    for c in cells:
        fc = c.getForce(cells)
        c.thermostat()
        c.applyForce(0.05, *fc)
    makePlot(cells, "pd_a_{0:04g}.png".format(i))

allKids = []
numChildren = 0
for i, c in enumerate(list(cells)):
    #print("Phase B, frame ", i)
    numKids = random.randrange(1,4)
    kids = c.getChildren(cells, numKids)
    for kid in kids:
        allKids.append(kid)
        numChildren += 1

numChildFrames = times["addChildren"]*FPS
addChildOnFrames = random.sample(range(numChildFrames), numChildren)
kidIdx = 0
for i in range(numChildFrames):
    #print("Phase B, frame ", i)
    if(i in addChildOnFrames):
        cells.append(allKids[kidIdx])
        kidIdx += 1
    makePlot(cells, "pd_b_{0:04g}.png".format(i))

for i in range(times["showChildren"]*FPS):
    #print("Phase C, frame ", i)
    for c in cells:
        fc = c.getForce(cells)
        c.thermostat()
        c.applyForce(0.05, *fc)
    makePlot(cells, "pd_c_{0:04g}.png".format(i))

numDriftFrames = times["driftChildren"]*FPS
kidIdx = 0
addDriftOnFrames = random.sample(range(numDriftFrames), numChildren)
for i in range(numDriftFrames):
    #print("phase D. Frame ", i)
    if( i in addDriftOnFrames):
        allKids[kidIdx].driftColor()
        kidIdx += 1
    makePlot(cells, "pd_d_{0:04g}.png".format(i))

for i in range(times["showDrift"] * FPS):
    #print("Phase E, frame ", i) 
    for c in cells:
        fc = c.getForce(cells)
        c.thermostat()
        c.applyForce(0.05, *fc)
    makePlot(cells, "pd_e_{0:04g}.png".format(i))

i = 0
totalIters = 0
origCells = list(cells)
mixCells = []
while i < 10 and totalIters < 20:
    #print("Phase F, frame ", i)
    totalIters += 1
    cell1 = random.choice(origCells)
    cell2 = random.choice(origCells)
    if(cell1._color == cell2._color):
        continue
    midX = (cell1.x + cell2.x) / 2
    midY = (cell1.y + cell2.y) / 2
    newCell = Cell([cell1._color, cell2._color], 9)
    newCell.x = midX
    newCell.y = midY
    energy = newCell.particleEnergy(cells) + newCell.wallEnergy()
    if(energy > 2):
        continue
    mixCells.append(newCell)
    #makePlot(cells, "pd_f_{0:04g}.png".format(i))
    i += 1

numMixFrames = times["addMixing"]*FPS
kidIdx = 0
addMixOnFrames = random.sample(range(numMixFrames), len(mixCells))
for i in range(numMixFrames):
    #print("phase D. Frame ", i)
    if( i in addMixOnFrames):
        cells.append(mixCells[kidIdx])
        kidIdx += 1
    makePlot(cells, "pd_f_{0:04g}.png".format(i))


for i in range(times["showMixing"] * FPS):
    
    #print("Phase G, frame ", i)
    for c in cells:
        fc = c.getForce(cells)
        c.thermostat()
        c.applyForce(0.05, *fc)
    makePlot(cells, "pd_g_{0:04g}.png".format(i))

survivors = random.sample(range(len(cells)), NUM_SURVIVORS)
desatLength = times["desat"]*FPS
for i in range(desatLength):
    #print("Phase H, frame ", i)
    for j, c in enumerate(cells):
        if(j not in survivors):
            c.desaturate((desatLength - i-1) / desatLength)
        fc = c.getForce(cells)
        c.thermostat()
        c.applyForce(0.05, *fc)
    makePlot(cells, "pd_h_{0:04g}.png".format(i))

cells = [cells[i] for i in range(len(cells)) if i in survivors ]

for i in range(times["showFinal"]*FPS):
    #print("Phase I, frame ", i)
    for c in cells:
        fc = c.getForce(cells)
        c.thermostat()
        c.applyForce(0.05, *fc)
    makePlot(cells, "pd_i_{0:04g}.png".format(i))




