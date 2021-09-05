#!/usr/bin/env python3
import json
import sys
import re 
def addInterp(lines, sourceDat):
    #0  Interp data
    #1  Father:
    #2  fatherID <<<
    #3  fatherInputs
    #4  Mother:
    #5  motherID <<<
    #6  motherInputs
    #7  Cross info <<<
    #8  Son: 
    #9  sonID <<<
    #10 sonInputs 
    #11 Daughter:
    #12 daughterID <<<
    #13 daughterInputs
    fatherID = int(lines[2])
    motherID = int(lines[5])
    p = re.compile(".*\((\d*), (\d*)\).*\((\d*), (\d*)\)")
    crosses = p.match(lines[7])
    fatherPts = crosses.group(1, 2)
    motherPts = crosses.group(3, 4)
    sonID = int(lines[9])
    daughterID = int(lines[12])
    sourceDat[sonID] = {"type" : "interp", 
        "parent" : fatherID,
        "otherParent" : motherID, 
        "crossStart" : int(fatherPts[0]),
        "crossEnd" : int(fatherPts[1])}
    sourceDat[daughterID] = {"type" : "interp", 
        "parent" : motherID,
        "otherParent" : fatherID, 
        "crossStart" : int(motherPts[0]),
        "crossEnd" : int(motherPts[1])}

def addMutate(lines, sourceDat):
    #0  JOINT_MUTATE ...
    #1  parentID <<<
    #2  parentInputs
    #3  Command <<<
    #4  Final:
    #5  ChildID <<<
    
    parentID = int(lines[1])
    commands = lines[3].split()
    childID = int(lines[5])
    sourceDat[childID] = {"type" : commands[0], 
        "parent" : parentID, 
        "args" : commands[1:]}
def addSeed(lines, sourceDat):
    #SEED: ...
    #entityID <<<
    eid = int(lines[1])
    sourceDat[eid] = {"type" : "seed"}
    

def readEval(lines, sourceDat):
    color = [[0, 100]]
    size = [[0,100]]
    eid = int(lines[0])
    source = sourceDat[eid]
    inputs_t = lines[1].split()
    inputs = [int(x) for x in inputs_t]
    evaluation = float((lines[2].split())[1])
    return {"color" : color, 
            "size" : size, 
            "id" : eid,
            "inputs" : inputs,
            "source" : source,
            "evaluation" : evaluation}

allObjs = []
sourceDat = dict()
inLines = list(sys.stdin)
for i in range(len(inLines)):
    line = inLines[i]
    if(len(line) > 5):
        if(line[0:5] == "EVAL:"):
            idLine = inLines[i+1]
            inpLine = inLines[i+2]
            evalLine = inLines[i+3]
            ret = readEval((idLine, inpLine, evalLine), sourceDat)
            allObjs.append(ret)
        elif(line[0:5] == "Inter"):
            addInterp(inLines[i:], sourceDat)
        elif(line[0:5] == "JOINT"):
            addMutate(inLines[i:], sourceDat)
        elif(line[0:5] == "SEED:"):
            addSeed(inLines[i:], sourceDat)
            
print(json.dumps(allObjs))

            
