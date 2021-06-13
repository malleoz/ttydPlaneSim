#ifndef SAMPLE_PHYSICS_H
#define SAMPLE_PHYSICS_H
/**This file defines a very simple model with known behavior:
It collides once the sum of stick positions is equal to 1000. 
If it reaches frame 600 and hasn't reached 1000, then the 
distance covered is the sum of the stick positions. 
*/
#include <inttypes.h>
#include <stdbool.h>

struct Player {
    int position;
    int frameNumber;
    //No need for the player struct to know the stick position, since
    //that'll be provided to frameSim
    //int8_t stickPosition; // -72 to +72 inclusive
};

// Returns from main sim function
struct Result {
    struct Player player;
    bool collided;
    bool landed;
};

struct Player init();
void frameSim(signed char stickPosition, 
              struct Player *previousFrame, 
              struct Result *nextFrame);

#endif
