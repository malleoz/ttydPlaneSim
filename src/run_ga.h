#ifndef RUN_GA_H
#define RUN_GA_H

#include <stdio.h>

/*
The main method used for optimizing flights. 
Arguments: 
playerDat : a file pointer to a player.dat file exported by export_player.py. 
goodControllerInputs: A file pointer to a set of inputs that should be used to seed the algorithm. This file should have one integer per line, and should have at most maxFrames lines. If this is NONE, then all flights will be randomly seeded. 
outputFile: A file pointer to where the flight logs will be written. This should of course have been opened for writing. 

popSize: The number of individuals in the population that will be evolved. Fewer individuals means faster generations but slower convergence. I find a happy value to be around 3,000. 
maxFrames: If the flight lasts longer than this many frames, terminate it. This will depend on the particular flight being run, but it never hurts to be generous with this parameter. 
numGenerations: How long should the GA chew on the problem? You typically want a large number here, on the order of 100,000 generations. 
Returns: EXIT_SUCCESS on successful optimization. 
*/
int run_ga (FILE *playerDat, FILE *goodControllerInputs, 
        FILE *outputFile, int popSize, int maxFrames, 
        int numGenerations);

#endif
