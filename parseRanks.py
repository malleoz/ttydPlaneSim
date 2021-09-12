#!/usr/bin/env python3
import sys
liveEntities = []
dyingEntities = []
entityScores = dict()
nextScore = 0
colorInfo = open("colorInfo.dat", "w")
liveEntitiesFp = open("liveEntities.dat", "w")
generation = 0
for line in sys.stdin:
    if(len(line) <= 6):
        continue
    if(line[0:5] == "CONST") :
        liveEntities.append(int(line[13:]))
    elif(line[0:5] == "EVAL:"):
        nextScore = int(line[13:])
    elif(line[0:5] == "Evalu"):
        entityScores[nextScore] = float(line[12:])
    elif(line[0:5] == "DESTR"):
        dyingEntities.append(int(line[12:]))
    elif(line[0:5] == "Gener"):
        survivors = []
        liveEntitiesFp.write("L {0:d} ".format(generation))
        for elem in liveEntities:
            liveEntitiesFp.write("{0:d} ".format(elem))
            if(elem not in dyingEntities):
                survivors.append((elem, entityScores[elem]))
        liveEntitiesFp.write("\n")
        liveEntitiesFp.write("S {0:d} ".format(generation))
        for elem in survivors:
            liveEntitiesFp.write("{0:d} ".format(elem[0]))
        liveEntitiesFp.write("\n")
        sortedEntities = sorted(survivors, key=lambda x: x[1])
        liveEntities = []
        for i, elem in enumerate(sortedEntities):
            colorInfo.write("{0:d} {1:d} {2:d}\n".format(elem[0], i, generation))
            liveEntities.append(elem[0])
        generation += 1
colorInfo.close()
liveEntitiesFp.close()
        
            
        
        
