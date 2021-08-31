#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plane_physics.h"
#include <assert.h>
#include <time.h>

typedef uint32_t uint;
typedef struct Player Player;
typedef struct Vector Vector;
typedef struct MotStruct MotStruct;
typedef struct Result Result;

float gPi;
float landingX;
float landingY;
float platformX; // The x-axis boundary of the plane panel before physics change
bool interferencePresent;
float interferenceX1;
float interferenceX2;
float interferenceY;

Player init(FILE *playerDat) {
    gPi = 3.1415927;  // The game decides to round to 7 places
    Player player;  

    // Scan for cmdline arguments from export_player.py
    fscanf(playerDat, "%f", &player.position.x);
    fscanf(playerDat, "%f", &player.position.y);
    fscanf(playerDat, "%f", &player.position.z);
    fscanf(playerDat, "%f", &player.baseSpeed);
    fscanf(playerDat, "%u", &player.motStruct.flags);
    fscanf(playerDat, "%f", &player.motStruct.rot.x);
    fscanf(playerDat, "%f", &player.motStruct.rot.y);
    fscanf(playerDat, "%f", &player.motStruct.rot.z);
    fscanf(playerDat, "%f", &player.motStruct.index4);
    fscanf(playerDat, "%f", &player.motStruct.pitchRate);
    fscanf(playerDat, "%f", &player.motStruct.ySpeed);
    fscanf(playerDat, "%f", &player.motStruct.index7);
    fscanf(playerDat, "%f", &player.motStruct.index9);
    fscanf(playerDat, "%f", &landingX);
    fscanf(playerDat, "%f", &landingY);
    fscanf(playerDat, "%f", &platformX);

    // Is there an interfering piece of collision duriing the flight that we need to avoid?
    if (EOF != fscanf(playerDat, "%f", &interferenceX1)) {
      interferencePresent = true;
      if (EOF == fscanf(playerDat, "%f", &interferenceX2) || EOF == fscanf(playerDat, "%f", &interferenceY)) {
        printf("Interference boundary missing!\n \
	        Check that you have a left and right x-axis bound, as well as y-axis in the .dat file.\n");
        exit(1);
      }
    }
    return player;
}

bool usingInterference(){
    return interferencePresent;
}

// Restrict parameter to 0-360 deg
// Original in-game instruction uses sets of while-loops to recreate modulo operator
// This is just a more efficient mod function
double revise360(double param_1) {
    while (param_1 < 0) {
      param_1 += 360;
    }
    while (360 <= param_1) {
      param_1 -= 360;
    }
    
    return param_1;
}

// In-game function which computes both the sin and cos for a given float
void sincosf(float x, float *sinx, float *cosx) {
    double rad = (double)((gPi * x ) / 180.0);
    *sinx =  (float)sin(rad);
    *cosx = -(float)cos(rad);
    return;
}

// A simple function which accepts an angle (in degrees) as inputs and computes the radian sine
double dsin(double x) {
    double rad = (gPi * x) / 180;
    return sin(rad);
}

// This function is reached in all scenarios in which the Gamecube analog stick x-axis is set to neutral
void neutralCalc(Player *player) {
    MotStruct *motStruct = &player->motStruct;

    if ((motStruct->rot.x <= 0.0) || (motStruct->pitchRate <= 0.0)) // If nosediving or (some form of acceleration?)
    {
        float speedRemaining = 5.0 - player->baseSpeed;

        if (speedRemaining < 0.0) {
            speedRemaining = 0.0;
        }
	
    	bool flyRight = motStruct->flags & 1;
    	bool above_panel = flyRight ? player->position.x < platformX : player->position.x > platformX;
    
    	if (above_panel) {
            motStruct->rot.x -= 0.001;
            player->baseSpeed += 0.5;
            if (2.0 < player->baseSpeed) {
                player->baseSpeed = 2.0;
            }
        }
        else {
	    motStruct->index7 = -4.0 * dsin(speedRemaining * 18.0);
	    
	    if (motStruct->index7 <= -2.0) {
		motStruct->index7 = -2.0;
	    }

            motStruct->rot.x += motStruct->index7;
        }
    }
    
    if (motStruct->rot.x <= -45.0) {
        motStruct->rot.x = -45.0; // Restrict maximum (negative) nosedive angle
    }
    else if (6.0 <= motStruct->rot.x) {
        motStruct->pitchRate = 0.0;
    }
}

// This function is reached when the Gamecube analog x-axis is not neutral
void nonNeutralCalc(Player *player) {
    MotStruct *motStruct = &player->motStruct;
    int stickMag = abs(player->stickPosition); // magnitude
    bool flyRight = motStruct->flags & 1;
    
    if (flyRight ? player->stickPosition < 0 : player->stickPosition > 0) { // If (flying left and holding right) or (flying right and holding left)
        if (motStruct->rot.x < 25.0) {
            motStruct->rot.x = 0.04 * stickMag + motStruct->rot.x;
        }
        else {
            motStruct->rot.x = 0.1 * (25.0 - motStruct->rot.x) + motStruct->rot.x;
        }
    }
    
    if (flyRight ? player->stickPosition > 0 : player->stickPosition < 0) { // if (flying left and holding left) or (flying right and holding right)
        if (-45.0 < motStruct->rot.x) {
            motStruct->rot.x = -(0.1 * stickMag - motStruct->rot.x);
        }
        else {
            motStruct->rot.x = -45.0; // Restrict maximum (negative) nosedive angle
        }
    }
}

// This function is reached when the plane's rotation is negative
void nosediveCalc(Player *player) {
    MotStruct *motStruct = &player->motStruct;
    
    motStruct->flags = motStruct->flags & 0xffffffef; // Flag for nosedive
    motStruct->index4 = 0.2 * dsin(1.5 * fabs(motStruct->rot.x));
    player->baseSpeed += motStruct->index4;
    motStruct->pitchRate = motStruct->pitchRate + 1.0 * dsin(0.05 * player->baseSpeed) * dsin(3.4 * fabs(motStruct->rot.x));

    bool flyLeft = (motStruct->flags & 1) == 0 ? true : false;
    bool above_panel = flyLeft ? player->position.x > platformX : player->position.x < platformX;

    if (player->stickPosition != 0) { // If stick is not neutral
        if (1 <= motStruct->pitchRate) {
            if (player->stickPosition > 0 || (player->stickPosition < 0 && (motStruct->flags & 1) == 0)) { // If (holding right) OR (holding left and flying left)
                motStruct->pitchRate = 1;
            }
        }
        
    	if (!above_panel && motStruct->index7 > 1) {
                motStruct->index7 += 0.1 * (1 - motStruct->index7);
        }
    }
     
    if (above_panel) {
    	motStruct->index9 = -22.0;
    }
    else {
        motStruct->index9 = 0.1 * (-45.0 - motStruct->index9) + motStruct->index9;
    }

    motStruct->rot.x = motStruct->rot.x + motStruct->pitchRate;

    if (motStruct->rot.x <= motStruct->index9) {
        motStruct->rot.x = motStruct->index9;
    }
    
    motStruct->ySpeed -= 0.1 * motStruct->ySpeed;
}

// This function is reached when the plane's rotation is positive
void taildiveCalc(Player *player) {
    MotStruct *motStruct = &player->motStruct;
    
    motStruct->flags = motStruct->flags | 0x10; // Set the taildive flag to represent rot.x > 0

    player->baseSpeed += -0.1 * player->baseSpeed * dsin(motStruct->rot.x * 0.1);
    
    if ((player->stickPosition != 0) && (25.0 <= motStruct->rot.x)) {
        player->baseSpeed -= 0.03;
    }

    if (player->baseSpeed <= 0.0) {
        player->baseSpeed = 0.0;
    }

    float pitchRate = 0.0;
    float speedRemaining = 5.0 - player->baseSpeed;

    // If holding neutral...
    if (player->stickPosition == 0) {
        if (3.9 <= player->baseSpeed) {
            motStruct->pitchRate = 0.118;
        }
        else {
            pitchRate = speedRemaining * -0.112 * dsin(motStruct->rot.x) + motStruct->pitchRate;
            motStruct->pitchRate = pitchRate;
        }

        motStruct->rot.x += motStruct->pitchRate;

        if ((pitchRate <= 0) && (player->baseSpeed <= 2.5)) {

            pitchRate = 5.0 - 0.4 * player->baseSpeed;

            if (pitchRate <= 0) {
                pitchRate = 0;
            }

            bool flyLeft = (motStruct->flags & 1) == 0;
            bool above_panel = flyLeft ? player->position.x > platformX : player->position.x < platformX;
        
	    if (!above_panel) {
            	motStruct->index7 = -8.0 * dsin(pitchRate * 18.0);

            	if (motStruct->index7 <= -2.0) {
                    motStruct->index7 = -4.0;
                }
	    }
        }

        motStruct->rot.x += motStruct->index7;
    }
     
    // If holding neutral
    if (player->stickPosition == 0) {
        pitchRate = -0.1 * motStruct->ySpeed + motStruct->ySpeed;
        motStruct->ySpeed = pitchRate;
    }
    // If holding a direction
    else {
        pitchRate = -0.3 * dsin(speedRemaining);
        motStruct->ySpeed += pitchRate;

        if (motStruct->ySpeed <= -5.0) {
            motStruct->ySpeed = -5.0;
        }
    }

    motStruct->rot.x += pitchRate;
}

void updatePos(Player *player) {
    MotStruct *motStruct = &player->motStruct;
    float local_2c;
    float local_30;
    float local_34;
    float local_38;
    
    player->position.y = player->position.y + motStruct->ySpeed - 0.2; // Written out as the shorthand += results in floating point inaccuracy
   
    player->wDirectionView = motStruct->rot.y;
    
    sincosf(player->wDirectionView,&local_34,&local_38);
    player->position.z += player->baseSpeed * local_38;
    
    sincosf(90.0 + motStruct->rot.x,&local_2c,&local_30);
    
    if (local_30 > 0.0) {
        local_30 *= 1.2;
    }
    
    player->position.y += player->baseSpeed * local_30;
    player->position.x += local_2c * player->baseSpeed * local_34;
}

void interferenceCalc(Player *player, Result *nextFrame, bool rightFlight) {
    if  (rightFlight ? player->position.x > interferenceX1 : player->position.x < interferenceX2) { // If player has reached the edge of interference
        nextFrame->reachedInterference = true;
    	    
    	if (player->position.y < interferenceY && (rightFlight ? player->position.x < interferenceX2 : player->position.x > interferenceX1)) {
    	    nextFrame->collidedInterference = true;
    	    
    	    if (rightFlight ? (player->position.x - interferenceX1 > 5) : (interferenceX2 - player->position.x > 5)) {
    	        // If x is sufficiently far into collision, then we must have landed on top
    	        nextFrame->landedInterference = true;
    	    }
        }
    }
}

// Main simulation routine
// Given a Gamecube x-axis analog stick position and the previousFrame state, compute the nextFrame state
void frameSim(signed char stickPosition, Player *previousFrame, Result *nextFrame) {
    Player player = *previousFrame; // Copy player obj to later store in nextFrame
    player.stickPosition = stickPosition; // Store this frame's input
    MotStruct *motStruct = &player.motStruct; // shorthand for later
    
    motStruct->rot.y = revise360(motStruct->rot.z * 0.025 + motStruct->rot.y);

    // If analog stick is neutral
    if (player.stickPosition == 0) {
        neutralCalc(&player);
    }
    // If speed is not 0 (we are mid-flight)
    else if (player.baseSpeed != 0.0) {
        nonNeutralCalc(&player);   
    }
    
    player.baseSpeed -= 0.02 * player.baseSpeed; // static speed decrease, before factoring in rotation
    
    bool nosediving = motStruct->rot.x < 0;
    if (nosediving) {
        nosediveCalc(&player);
    }
    else {
        taildiveCalc(&player);
    }
    
    if (5.0 <= player.baseSpeed) {
        player.baseSpeed = 5.0; // Enforce speedcap
    }
    
    updatePos(&player); // Given the prior physics calcs, compute new position vals

    // The frame has ended.
    // set up the result struct
    nextFrame->player   = player;
    nextFrame->collided = false;
    nextFrame->landed   = false;
    nextFrame->reachedInterference = false;
    nextFrame->collidedInterference = false;
    nextFrame->landedInterference = false;

    // Determine if we have made contact with the landing wall, with varying logic depending on direction of flight
    bool rightFlight = motStruct->flags & 1;
    if (rightFlight ? (player.position.x >= landingX) : (player.position.x <= landingX)) {
        nextFrame->collided = true;

        // If we're also above the landing platform, then we have landed
        if (player.position.y >= landingY) {
            nextFrame->landed = true;
        }
    }

    // Determine if we have intersected any interfering piece of collision
    if (interferencePresent) {
        interferenceCalc(&player, nextFrame, rightFlight);
    }

    return;
}

// returns < 0 if we overshot, > 0 if we haven't hit it yet
float distance_to_go_x(Player p) {
    bool rightFlight = p.motStruct.flags & 1;

    if (rightFlight) {
        return landingX - p.position.x;
    }
    else {
        return p.position.x - landingX;
    }
}

// returns > 0 if we are below platform, < 0 if we're over platform
float distance_to_go_y(Player p) {
    return landingY - p.position.y;
}

float interference_distance_to_go_x(Player p) {
    bool flyRight = p.motStruct.flags & 1;

    if (flyRight) {
        return interferenceX1 - p.position.x;
    }
    else {
    	return p.position.x - interferenceX2;
    }
}

// returns > 0 if we are below platform
float interference_distance_to_go_y(Player p) {
    return interferenceY - p.position.y;
}
