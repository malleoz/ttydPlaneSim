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
    fscanf(playerDat, "%f", &player.motStruct.rotationPivot.x);
    fscanf(playerDat, "%f", &player.motStruct.rotationPivot.y);
    fscanf(playerDat, "%f", &player.motStruct.rotationPivot.z);
    fscanf(playerDat, "%f", &player.motStruct.index4);
    fscanf(playerDat, "%f", &player.motStruct.index5);
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

void neutralCalc(Player *player) {
    MotStruct *motStruct = &player->motStruct;

    // If nosediving or (some form of acceleration?)
    if ((motStruct->rotationPivot.x <= 0.0) || (motStruct->index5 <= 0.0))
    {
        float baseSpeed = 5.0 - player->baseSpeed;

        if (baseSpeed < 0.0) {
            baseSpeed = 0.0;
        }

        double dVar8 = sin((double)((gPi * baseSpeed * 18.0) / 180.0));
        double dispDirectionCurrent = (double)(-4.0 * (float)dVar8);
        
        if ((double)(-4.0 * (float)dVar8) <= (double)-2.0) {
            dispDirectionCurrent = (double)-2.0;
        }
	
    	bool flyLeft = (motStruct->flags & 1) == 0 ? true : false;
    	bool above_panel = flyLeft ? player->position.x > platformX : player->position.x < platformX;
    
    	if (above_panel) {
            motStruct->rotationPivot.x += -0.001;
            player->baseSpeed += 0.5;
            if (2.0 < player->baseSpeed) {
                player->baseSpeed = 2.0;
            }
        }
        else {
            motStruct->index7 = (float)dispDirectionCurrent;

            motStruct->rotationPivot.x = motStruct->rotationPivot.x + motStruct->index7;
        }
    }
    
    // Restrict maximum (negative) nosedive angle
    if (motStruct->rotationPivot.x <= -45.0) {
        motStruct->rotationPivot.x = -45.0;
    }

    if (6.0 <= motStruct->rotationPivot.x) {
        motStruct->index5 = 0.0;
    }
}

void nonNeutralCalc(Player *player) {

    MotStruct *motStruct = &player->motStruct;
    // Represent the magnitude of the stick input as a float
    float stickXf = fabs((float)player->stickPosition);
    
    // If (flying left and holding right) or (flying right and holding left)
    if (((motStruct->flags & 1) == 0 && 0 < player->stickPosition) || ((motStruct->flags  & 1) == 1 && player->stickPosition < 0)) {
        float xPivot = (motStruct->rotationPivot).x;
        
        if (xPivot < 25.0) {
            (motStruct->rotationPivot).x = 0.04 * stickXf + (motStruct->rotationPivot).x;
        }
        else {
            (motStruct->rotationPivot).x = 0.1 * (25.0 - (motStruct->rotationPivot).x) + (motStruct->rotationPivot).x;
        }
    }

    // If flying to the left
    if ((motStruct->flags & 1) == 0 && player->stickPosition < 0) {

        if (-45.0 < motStruct->rotationPivot.x) {
            
            int64_t temp = 0x3FB99999A0000000;
            double DOUBLE_80420d40 = *(double *)&temp;

            motStruct->rotationPivot.x = -(DOUBLE_80420d40 * stickXf - motStruct->rotationPivot.x);
            
        }
        else {
            // Restrict maximum (negative) nosedive
            motStruct->rotationPivot.x = -45.0;
        }
    }
    // If flying to the right
    else if ((motStruct->flags & 1) == 1 && 0 < player->stickPosition) {
        if (-45.0 < motStruct->rotationPivot.x) {
            motStruct->rotationPivot.x = -(0.1 * stickXf - motStruct->rotationPivot.x);
        }
        else {
            // Reduce maximum (negative) nosedive
            motStruct->rotationPivot.x = -45.0;
        }
    }
}

void nosediveCalc(Player *player) {
    MotStruct *motStruct = &player->motStruct;
    motStruct->flags = motStruct->flags & 0xffffffef; // Flag for nosedive

    double dispDirectionCurrent = sin((double)((gPi * 1.5 * fabs(motStruct->rotationPivot.x)) / 180.0));

    motStruct->index4 = 0.2 * (float)dispDirectionCurrent;

    player->baseSpeed = player->baseSpeed + motStruct->index4;

    dispDirectionCurrent = sin((double)((gPi * 3.4 * fabs(motStruct->rotationPivot.x)) / 180.0));
    
    double dVar8 = dispDirectionCurrent;
    dispDirectionCurrent = sin((double)((gPi * player->baseSpeed * 0.05) / 180.0));

    motStruct->index5 = motStruct->index5 + 1.0 * (float)(dispDirectionCurrent * dVar8);

    bool flyLeft = (motStruct->flags & 1) == 0 ? true : false;
    bool above_panel = flyLeft ? player->position.x > platformX : player->position.x < platformX;

    // If stick is not neutral
    if (player->stickPosition != 0) {
        // If flying to the left, holding left, and index5 >= 1
        if ((motStruct->flags & 1) == 0 && player->stickPosition < 0 && 1 <= motStruct->index5) {
            motStruct->index5 = 1;
        }
        else if (0 < player->stickPosition && 1 <= motStruct->index5) {
            motStruct->index5 = 1;
        }
        
    	if (!above_panel) {
            // Below execution occurs if no plane panel below Mario.

            float wIndex7 = motStruct->index7;

            motStruct->index7 = 0.1 * (1 - wIndex7) + wIndex7;

            if (wIndex7 <= motStruct->index7) {
                motStruct->index7 = wIndex7;
            }
	}
    }
     
    if (above_panel) {
    	motStruct->index9 = -22.0;
    }
    else {
        // Below execution occurs if no plane panel below Mario. Assume this is the case
        float wIndex9 = motStruct->index9;

        motStruct->index9 = 0.1 * (-45.0 - wIndex9) + wIndex9;
    }

    motStruct->rotationPivot.x = motStruct->rotationPivot.x + motStruct->index5;

    if (motStruct->rotationPivot.x <= motStruct->index9) {
        motStruct->rotationPivot.x = motStruct->index9;
    }

    float wySpeed = motStruct->ySpeed;

    motStruct->ySpeed = 0.1 * -wySpeed + wySpeed;
}

void taildiveCalc(Player *player) {
    MotStruct *motStruct = &player->motStruct;

    // Set the taildive flag to represent rotationPivot.x > 0
    motStruct->flags = motStruct->flags | 0x10;
    double dispDirectionCurrent = sin((double)((gPi * motStruct->rotationPivot.x * 0.1) / 180.0));

    player->baseSpeed = player->baseSpeed + player->baseSpeed * -0.1 * (float)dispDirectionCurrent;
    
    if ((player->stickPosition != 0) && (25.0 <= motStruct->rotationPivot.x)) {
        player->baseSpeed = player->baseSpeed - 0.03;
    }

    if (player->baseSpeed <= 0.0) {
        player->baseSpeed = 0.0;
    }

    float wIndex5 = 0.0;
    float wySpeed;

    // If holding neutral...
    if (player->stickPosition == 0) {
        if (3.9 <= player->baseSpeed) {
            motStruct->index5 = 0.118;
        }
        else {
            float dVar8 = (double)(5.0 - player->baseSpeed);

            dispDirectionCurrent = sin((double)((gPi * motStruct->rotationPivot.x) / 180.0));

            wIndex5 = (float)(dVar8 * (double)(-0.112 * (float)dispDirectionCurrent)) + motStruct->index5;

            motStruct->index5 = wIndex5;
        }

        motStruct->rotationPivot.x = motStruct->rotationPivot.x + motStruct->index5;

        if ((wIndex5 <= 0) && (player->baseSpeed <= 2.5)) {

            wIndex5 = -(player->baseSpeed * 0.4 - 5.0);

            if (wIndex5 <= 0) {
                wIndex5 = 0;
            }

            dispDirectionCurrent = (double)(wIndex5 * 18.0);

            bool flyLeft = (motStruct->flags & 1) == 0 ? true : false;
            bool above_panel = flyLeft ? player->position.x > platformX : player->position.x < platformX;
        
	    if (!above_panel) {
            	// Below assumes that there is no collision below Mario
            	dispDirectionCurrent = sin((double)((float)((double)gPi * dispDirectionCurrent) / 180.0));
            
            	wIndex5 = -8.0 * (float)dispDirectionCurrent;

            	if (wIndex5 <= -2.0) {
                    wIndex5 = -4.0;
                }

                motStruct->index7 = wIndex5;
	    }
        }

        motStruct->rotationPivot.x = motStruct->rotationPivot.x + motStruct->index7;
    }

    // If holding neutral
    if (player->stickPosition == 0) {

        wySpeed = motStruct->ySpeed;

        wySpeed = 0.1 * -wySpeed + wySpeed;

        motStruct->ySpeed = wySpeed;
    }
    // If holding a direction
    else {
        dispDirectionCurrent = sin((double)((gPi * (5.0 - player->baseSpeed)) / 180.0));

        wySpeed = -0.3 * (float)dispDirectionCurrent;

        motStruct->ySpeed = motStruct->ySpeed + wySpeed;

        if (motStruct->ySpeed <= -5.0) {
            motStruct->ySpeed = -5.0;
        }
    }

    motStruct->rotationPivot.x = motStruct->rotationPivot.x + wySpeed;
}

// Main simulation routine
void frameSim(signed char stickPosition, Player *previousFrame, Result *nextFrame) {
    Player player = *previousFrame;
    player.stickPosition = stickPosition;
    MotStruct *motStruct = &player.motStruct;

    uint uVar3;
    float yPivot;
    //float baseSpeed; //Unused.
    float local_2c;
    float local_30;
    float local_34;
    float local_38;
    int iVar4;

    // 0x8009cafc
    double dispDirectionCurrent = revise360((double)(motStruct->rotationPivot.z * 0.025 + motStruct->rotationPivot.y));
    
    motStruct->rotationPivot.y = (float)dispDirectionCurrent;

    // Normalize the calculated y-axis rotation
    yPivot = motStruct->rotationPivot.y;
    player.dispDirectionCurrent = revise360((double)yPivot);

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
    if (motStruct->rotationPivot.x < 0) {
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

    (player.position).y = (player.position).y + motStruct->ySpeed;

    int temp = 0x3e4ccccd;
    float FLOAT_8031049c = *(float *)&temp;

    (player.position).y = (player.position).y - FLOAT_8031049c;
    
    dispDirectionCurrent = revise360(-(double)motStruct->rotationPivot.x);
    
    (player.wRotationRelated).z = (float)dispDirectionCurrent;

    player.wDirectionView = motStruct->rotationPivot.y;

    // sin(local_34) and cos(local_38)
    sincosf(player.wDirectionView,&local_34,&local_38);

    yPivot = 90.0;

    (player.position).z = player.baseSpeed * local_38 + (player.position).z;

    sincosf(yPivot + motStruct->rotationPivot.x,&local_2c,&local_30);

    yPivot = 1.0;

    if (0.0 <= local_30) {
        yPivot = 1.2;
    }

    (player.position).y = player.baseSpeed * local_30 * yPivot + (player.position).y;

    (player.position).x = local_2c * player.baseSpeed * local_34 + (player.position).x;

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
