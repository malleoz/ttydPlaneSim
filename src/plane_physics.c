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
        float baseSpeed = 5.0 - player->baseSpeed;

        if (baseSpeed < 0.0) {
            baseSpeed = 0.0;
        }

        double dVar8 = dsin(baseSpeed * 18.0);
        double dispDirectionCurrent = -4.0 * dVar8;
        
        if (-4.0 * dVar8 <= -2.0) {
            dispDirectionCurrent = -2.0;
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
	    motStruct->index7 = -4.0 * dVar8;
	    
	    if (motStruct->index7 <= -2.0) {
		motStruct->index7 = -2.0;
	    }

            motStruct->rot.x = motStruct->rot.x + motStruct->index7;
        }
    }
    
    if (motStruct->rot.x <= -45.0) {
        motStruct->rot.x = -45.0; // Restrict maximum (negative) nosedive angle
    }

    if (6.0 <= motStruct->rot.x) {
        motStruct->pitchRate = 0.0;
    }
}

// This function is reached when the Gamecube analog x-axis is not neutral
void nonNeutralCalc(Player *player) {
    MotStruct *motStruct = &player->motStruct;
    int stickMag = abs(player->stickPosition); // magnitude
    bool flyRight = motStruct->flags & 1;
    
    if ((!flyRight && player->stickPosition > 0) || (flyRight && player->stickPosition < 0)) { // If (flying left and holding right) or (flying right and holding left)
        if (motStruct->rot.x < 25.0) {
            motStruct->rot.x = 0.04 * stickMag + motStruct->rot.x;
        }
        else {
            motStruct->rot.x = 0.1 * (25.0 - motStruct->rot.x) + motStruct->rot.x;
        }
    }
    
    if (!flyRight && player->stickPosition < 0) { // If flying to the left
        if (-45.0 < motStruct->rot.x) {
            motStruct->rot.x = -(0.1 * stickMag - motStruct->rot.x);
        }
        else {
            motStruct->rot.x = -45.0; // Restrict maximum (negative) nosedive angle
        }
    }
    else if (flyRight && 0 < player->stickPosition) { // If flying to the right
        if (-45.0 < motStruct->rot.x) {
            motStruct->rot.x = -(0.1 * stickMag - motStruct->rot.x);
        }
        else {
            motStruct->rot.x = -45.0; // Reduce maximum (negative) nosedive angle
        }
    }
}

// This function is reached when the plane's rotation is negative
void nosediveCalc(Player *player) {
    MotStruct *motStruct = &player->motStruct;
    motStruct->flags = motStruct->flags & 0xffffffef; // Flag for nosedive
    motStruct->index4 = 0.2 * dsin(1.5 * fabs(motStruct->rot.x));

    player->baseSpeed = player->baseSpeed + motStruct->index4;

    double dispDirectionCurrent = dsin(3.4 * fabs(motStruct->rot.x));
    
    double dVar8 = dispDirectionCurrent;
    dispDirectionCurrent = dsin(player->baseSpeed * 0.05);

    motStruct->pitchRate = motStruct->pitchRate + 1.0 * dispDirectionCurrent * dVar8;

    bool flyLeft = (motStruct->flags & 1) == 0 ? true : false;
    bool above_panel = flyLeft ? player->position.x > platformX : player->position.x < platformX;

    if (player->stickPosition != 0) { // If stick is not neutral
        if (1 <= motStruct->pitchRate) {
            if (player->stickPosition > 0 || (player->stickPosition < 0 && (motStruct->flags & 1) == 0)) { // If (holding right) OR (holding left and flying left)
                motStruct->pitchRate = 1;
            }
        }
        
    	if (!above_panel) {
            float temp7 = 0.1 * (1 - motStruct->index7) + motStruct->index7;

	    if (motStruct->index7 > temp7) {
                motStruct->index7 = temp7;
	    }
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
    
    motStruct->ySpeed = motStruct->ySpeed - (0.1 * motStruct->ySpeed);
}

// This function is reached when the plane's rotation is positive
void taildiveCalc(Player *player) {
    MotStruct *motStruct = &player->motStruct;
    motStruct->flags = motStruct->flags | 0x10; // Set the taildive flag to represent rot.x > 0

    player->baseSpeed = player->baseSpeed + player->baseSpeed * -0.1 * dsin(motStruct->rot.x * 0.1);
    
    if ((player->stickPosition != 0) && (25.0 <= motStruct->rot.x)) {
        player->baseSpeed = player->baseSpeed - 0.03;
    }

    if (player->baseSpeed <= 0.0) {
        player->baseSpeed = 0.0;
    }

    float pitchRate = 0.0;
    float wySpeed;

    // If holding neutral...
    if (player->stickPosition == 0) {
        if (3.9 <= player->baseSpeed) {
            motStruct->pitchRate = 0.118;
        }
        else {
            float dVar8 = (double)(5.0 - player->baseSpeed);
            pitchRate = dVar8 * -0.112 * dsin(motStruct->rot.x) + motStruct->pitchRate;

            motStruct->pitchRate = pitchRate;
        }

        motStruct->rot.x = motStruct->rot.x + motStruct->pitchRate;

        if ((pitchRate <= 0) && (player->baseSpeed <= 2.5)) {

            pitchRate = -(player->baseSpeed * 0.4 - 5.0);

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

        motStruct->rot.x = motStruct->rot.x + motStruct->index7;
    }

    // If holding neutral
    if (player->stickPosition == 0) {

        wySpeed = motStruct->ySpeed;

        wySpeed = 0.1 * -wySpeed + wySpeed;

        motStruct->ySpeed = wySpeed;
    }
    // If holding a direction
    else {
        wySpeed = -0.3 * sin((double)((gPi * (5.0 - player->baseSpeed)) / 180.0));

        motStruct->ySpeed = motStruct->ySpeed + wySpeed;

        if (motStruct->ySpeed <= -5.0) {
            motStruct->ySpeed = -5.0;
        }
    }

    motStruct->rot.x = motStruct->rot.x + wySpeed;
}

// Main simulation routine
// Given a Gamecube x-axis analog stick position and the previousFrame state, compute the nextFrame state
void frameSim(signed char stickPosition, Player *previousFrame, Result *nextFrame) {
    Player player = *previousFrame;
    player.stickPosition = stickPosition;
    MotStruct *motStruct = &player.motStruct; // shorthand for later

    float yPivot;
    float local_2c;
    float local_30;
    float local_34;
    float local_38;
    
    motStruct->rot.y = revise360((double)(motStruct->rot.z * 0.025 + motStruct->rot.y));

    // Normalize the calculated y-axis rotation
    player.dispDirectionCurrent = revise360(motStruct->rot.y);

    // If analog stick is neutral
    if (player.stickPosition == 0) {
        neutralCalc(&player);
    }
    // If speed is not 0 (we are mid-flight)
    else if (player.baseSpeed != 0.0) {
        nonNeutralCalc(&player);   
    }
    
    // Apply a static speed decrease, before factoring in rotation
    player.baseSpeed = player.baseSpeed * -0.02 + player.baseSpeed;
    
    // If nosediving
    if (motStruct->rot.x < 0) {
        nosediveCalc(&player);
    }
    // If we're not nosediving...
    else {
        taildiveCalc(&player);
    }
    
    // Enforce speedcap
    if (5.0 <= player.baseSpeed) {
        player.baseSpeed = 5.0;
    }

    player.position.y = player.position.y + motStruct->ySpeed - 0.2;

    player.wDirectionView = motStruct->rot.y;

    sincosf(player.wDirectionView,&local_34,&local_38);

    (player.position).z += player.baseSpeed * local_38;

    sincosf(90.0 + motStruct->rot.x,&local_2c,&local_30);
    
    yPivot = 1.0;
    
    if (local_30 > 0.0) {
        yPivot = 1.2;
    }

    (player.position).y = player.baseSpeed * local_30 * yPivot + (player.position).y;
    
    player.position.x += local_2c * player.baseSpeed * local_34;

    // Code relating to a "timeout" has been ommitted.
    // If Mario has speed < 0.8 for 90 frames, he will be forced out of plane.
    // Under no circumstance will this happen.

    // The frame has ended.
    // set up the result struct
    nextFrame->player   = player;
    nextFrame->collided = false;
    nextFrame->landed   = false;

    //Check to see if we have landed or bonked

    // Determine if we have made contact with the landing wall, with varying logic depending on direction of flight
    bool leftFlight = (motStruct->flags & 1) == 0;
    if (leftFlight ? (player.position.x <= landingX) : (player.position.x >= landingX)) {
        nextFrame->collided = true;

        // If we're also above the landing platform, then we have landed
        if (player.position.y >= landingY) {
            nextFrame->landed = true;
        }
    }

    // Determine if we have intersected any interfering piece of collision
    nextFrame->reachedInterference = false;
    nextFrame->collidedInterference = false;
    nextFrame->landedInterference = false;
    
    if (interferencePresent) {
    	if (leftFlight) {
    	    if (player.position.x < interferenceX2) {
    		nextFrame->reachedInterference = true;
    		
    		if (player.position.x > interferenceX1 && player.position.y < interferenceY) {
    		    nextFrame->collidedInterference = true;
    		    
    		    // If x is sufficiently far into collision, then we must have landed on top
    		    if (interferenceX2 - player.position.x > 5)  {
    		        nextFrame->landedInterference = true;
    		    }
    		}
    	    }
    	}
    	else {
    	    if (player.position.x > interferenceX1) {
    	        nextFrame->reachedInterference =  true;
    	        
    	        if (player.position.x < interferenceX2 && player.position.y < interferenceY) {
    	            nextFrame->collidedInterference = true;
    	            
    	            if (player.position.x - interferenceX1 > 5) {
    	                nextFrame->landedInterference = true;
    	            }
    	        }
    	    }
    	}
    }

    return;
}

// returns < 0 if we overshot, > 0 if we haven't hit it yet
float distance_to_go_x(Player p) {
    bool flyLeft = (p.motStruct.flags & 1) == 0 ? true : false; // Flight direction

    if (flyLeft) {
        return p.position.x - landingX;
    }
    else {
        return landingX - p.position.x;
    }
}

// returns > 0 if we are below platform, < 0 if we're over platform
float distance_to_go_y(Player p) {
    return landingY - p.position.y;
}

float interference_distance_to_go_x(Player p) {
    bool flyLeft = (p.motStruct.flags & 1) == 0 ? true : false;

    if (flyLeft) {
        return p.position.x - interferenceX2;
    }
    else {
    	return interferenceX1 - p.position.x;
    }
}

// returns > 0 if we are below platform
float interference_distance_to_go_y(Player p) {
    return interferenceY - p.position.y;
}
