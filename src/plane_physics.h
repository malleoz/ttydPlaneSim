#ifndef PLANE_PHYSICS_H
#define PLANE_PHYSICS_H

#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

struct Vector {
    float x;
    float y;
    float z;
};

struct MotStruct {
    uint32_t flags;
    struct Vector rotationPivot;
    float index4;
    float index5;
    float ySpeed;
    float index7;
    float index9;
};

struct Player {
    struct Vector wRotationRelated;
    float wDirectionView;
    struct Vector position;
    struct Vector speed;
    double dispDirectionCurrent;
    struct MotStruct motStruct;
    int8_t stickPosition; // -72 to +72 inclusive
    float baseSpeed;
};

// Returns from main sim function
struct Result {
    struct Player player;
    bool collided;
    bool landed;
    bool reachedInterference;
    bool collidedInterference;
    bool landedInterference;
};

/**
Initialization function, should be called once per program run. 
This reads in arguments from stdin and creates a player struct containing the 
starting player state. 

Returns: 
A Player struct that represents the initial player state. You should probably
not clobber this struct. 

Caveats:
This function also reads in (and sets) the global landingX and landingY. 
If you are using this code in a multiprocess environment, you must set these 
variables in every process. 

Author: Malleo
*/
struct Player init();
/** ???
Author: Malleo
*/
double revise360(double param_1);
/** ???
Author: Malleo */
void sincosf(float x, float *sinx, float *cosx);
/** ???
Author: Malleo */
int sysMsec2Frame(int param_1);
/** ???
Author: Malleo */
void neutralCalc(struct Player *player);
/** ???
Author: Malleo */
void nonNeutralCalc(struct Player *player);
/** ???
Author: Malleo */
void nosediveCalc(struct Player *player);
/** ???
Author: Malleo */
void taildiveCalc(struct Player *player);
/** 
Run a single frame of the game simulation. 
Arguments:
signed char (i.e., int8_t) stickPosition. 
    The current input from the analog stick. Range is from -72 to 72, inclusive.
Player * previousFrame.
    The result from the previous simulation frame, or the initial Player 
    returned by init(). This value is NOT clobbered by this function. 
Result *nextFrame. 
    OUT parameter. This is where the result of this frame of simulation will be 
    stored. This will be clobbered by the function. 

Author: Malleo
*/
void frameSim(signed char stickPosition, struct Player *previousFrame, struct Result *nextFrame);
/** 
How far is left to go before we reach the x-coordinate of the target platform? 
Arguments:
Player p: the current player struct. 
Returns:
The distance left. This value will be positive if we still need to get more distance to reach the platform, and negative if we have overshot. 
Author: Malleo
*/
float distance_to_go_x(struct Player p);
/** 
How far below the platform are we? 
Arguments: 
Player p: The current player struct.
Returns:
The distance we are below the platform. This value will be positive if the 
player is below the target, and negative if we're above it. 
Author: Malleo
*/
float distance_to_go_y(struct Player p);
/**
How far away are we from an interfering piece of collision?
Arguments:
Player p: The current player struct
Returns:
The distance we are away from the x-coordinate of the interfering collision.
This value is positive if the player has not contacted the wall yet, and
negative if we have gone through it
*/
float interference_distance_to_go_x(struct Player p);
/**
How far away are we from an interfering piece of collision?
Arguments:
Player p: The current player struct
Returns:
The distance we are away from the y-coordinate of the interfering collision.
This value is positive if the player has not contacted the wall yet, and
negative if we have gone below it
*/
float interference_distance_to_go_y(struct Player p);

/** Simply returns true if this flight should use the interference model. 
*/
bool usingInterference();
#endif
