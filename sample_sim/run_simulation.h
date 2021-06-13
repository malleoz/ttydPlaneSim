#ifndef RUN_SIMULATION_H
#define RUN_SIMULATION_H

#include "sample_physics.h"

/** Runs frameSim repeatedly, and once the simulation
indicates that the player has collided, stops the simulation.
The returned int is the index into output where the collision happened. 
results in output after this index are invalid. 
A result of -1 indicates that the simulation never collided. 
Arguments:
stickPositions is an array (of length maxFrames) of the input stick positions at each frame. These will be fed into the frameSim function. 
Result *output is an array (of length maxFrames) into which the result of 
each frame of simulation will be inserted. 
startPoint is the initial player struct. This will be copied into the 
first element of output. (startPoint will not be modified by this code.)
maxFrames is simply the length of output. After the simulation has run for maxFrames without colliding, it will be terminated.  
*/
int runSimulation(int8_t *stickPositions,
                  struct Result * output, 
                  struct Player *startPoint, 
                  int maxFrames);


#endif
