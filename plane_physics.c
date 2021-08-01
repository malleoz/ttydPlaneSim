#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plane_physics.h"
#ifdef DEBUG
#define debug_printf(...) printf(__VA_ARGS__)
#else
#define debug_printf(...)
#endif

typedef uint32_t uint;
typedef struct Player Player;
typedef struct Vector Vector;
typedef struct MotStruct MotStruct;
typedef struct Result Result;

float gPi;
float landingX;
float landingY;

Player init(FILE *playerDat) {
    gPi = 3.1415927;  // The game decides to round to 7 places

    Player player;
    player.baseSpeed = 1.0; // This is always true on init, no need to scan for it

    // Scan for cmdline arguments from export_player.py
    fscanf(playerDat, "%f", &player.position.x);
    fscanf(playerDat, "%f", &player.position.y);
    fscanf(playerDat, "%f", &player.position.z);
    fscanf(playerDat, "%u", &player.motStruct.flags);
    fscanf(playerDat, "%f", &player.motStruct.rotationPivot.x);
    fscanf(playerDat, "%f", &player.motStruct.rotationPivot.y);
    fscanf(playerDat, "%f", &player.motStruct.rotationPivot.z);
    fscanf(playerDat, "%f", &player.motStruct.index4);
    fscanf(playerDat, "%f", &player.motStruct.index5);
    fscanf(playerDat, "%f", &player.motStruct.ySpeed);
    fscanf(playerDat, "%f", &player.motStruct.index7);
    fscanf(playerDat, "%f", &player.motStruct.index8);
    fscanf(playerDat, "%f", &player.motStruct.index9);
    fscanf(playerDat, "%f", &landingX);
    fscanf(playerDat, "%f", &landingY);
    
    return player;
}

// Restrict parameter to 0-360 deg
double revise360(double param_1) {
  while (param_1 < 0) {
    param_1 += 360;
  }
  
  while (360 <= param_1) {
    param_1 -= 360;
  }
  
  return param_1;
}

// In-game function which computes both the sin and cos for a given variable in radians
void sincosf(float x, float *sinx, float *cosx) {
    double rad = (double)((gPi * x ) / 180.0);
    *sinx =  (float)sin(rad);
    *cosx = -(float)cos(rad);
    return;
}

// This was an in-game function but is optimized a bit
// PowerPC does not have a div instruction so there
// were bit-shift artifacts left over
int sysMsec2Frame(int param_1) {
    return (param_1 * 60) / 1000;
}

void neutralCalc(Player *player) {
    debug_printf("[0x8009cc14] BRANCH TAKEN\n");
    MotStruct *motStruct = &player->motStruct;

    // If nosediving or (some form of acceleration?)
    if ((motStruct->rotationPivot.x <= 0.0) || (motStruct->index5 <= 0.0))
    {
        debug_printf("[0x8009cda8] BRANCH TAKEN\n");

        float baseSpeed = 5.0 - player->baseSpeed;
        debug_printf("[0x8009ce00] baseSpeed [f3]:\t\t%10.6f\n", baseSpeed);

        if (baseSpeed < 0.0) {
            baseSpeed = 0.0;
            debug_printf("[0x8009ce10] baseSpeed [f3?]:\t\t%10.6f\n", baseSpeed);
        }

        double dVar8 = sin((double)((gPi * baseSpeed * 18.0) / 180.0));
        debug_printf("[0x8009ce30] dVar8 [f1]:\t\t%10.6f\n", dVar8);

        double dispDirectionCurrent = (double)(-4.0 * (float)dVar8);
        debug_printf("[0x8009ce48] dispDirectionCurrent [f31]:\t\t%10.6f\n", dispDirectionCurrent);
        
        if ((double)(-4.0 * (float)dVar8) <= (double)-2.0) {
            debug_printf("[0x8009ce54] BRANCH NOT TAKEN\n");
            dispDirectionCurrent = (double)-2.0;
            debug_printf("[0x8009ce58] dispDirectionCurrent [f31]:\t\t%10.6f\n", dispDirectionCurrent);
        }

        //bVar1 = false; // 8009ce5c

        // 8009ce78 - We assume that hitobjUnder.attr = 0

        motStruct->index7 = (float)dispDirectionCurrent;
        debug_printf("[0x8009ce8c] motStruct->index7 [f31]:\t\t%10.6f\n", motStruct->index7);

        motStruct->rotationPivot.x = motStruct->rotationPivot.x + motStruct->index7;
        debug_printf("[0x8009cea0] motStruct->rotationPivot.x [f0]:\t\t%10.6f\n", motStruct->rotationPivot.x);
    }
    
    // Restrict maximum (negative) nosedive angle
    if (motStruct->rotationPivot.x <= -45.0) {
        debug_printf("[0x8009cefc] BRANCH NOT TAKEN\n");
        motStruct->rotationPivot.x = -45.0;
    }

    if (6.0 <= motStruct->rotationPivot.x) {
        debug_printf("[0x8009cf18] BRANCH NOT TAKEN\n");
        motStruct->index5 = 0.0;
    }
}

void nonNeutralCalc(Player *player) {
    debug_printf("[0x8009cc24] BRANCH NOT TAKEN\n");

    MotStruct *motStruct = &player->motStruct;
    // Represent the magnitude of the stick input as a float
    float stickXf = fabs((float)player->stickPosition);
    debug_printf("[0x8009cc5c] stickXf [f3]:\t\t\t\t%10.6f\n", stickXf);
    
    // If (flying left and holding right) or (flying right and holding left)
    if (((motStruct->flags & 1) == 0 && 0 < player->stickPosition) || ((motStruct->flags  & 1) == 1 && player->stickPosition < 0)) {
        float xPivot = (motStruct->rotationPivot).x;
        
        if (xPivot < 25.0) {
            debug_printf("[0x8009cd1c/0x8009cc84] BRANCH TAKEN\n");
            (motStruct->rotationPivot).x = 0.04 * stickXf + (motStruct->rotationPivot).x;
            debug_printf("[0x8009cd44] (motStruct->rotationPivot).x [f0]:\t%10.6f\n", (motStruct->rotationPivot).x);
        }
        else {
            debug_printf("[0x8009cd1c/0x8009cc84] BRANCH NOT TAKEN\n");
            (motStruct->rotationPivot).x = 0.1 * (25.0 - (motStruct->rotationPivot).x) + (motStruct->rotationPivot).x;
            debug_printf("[0x8009cd44] (motStruct->rotationPivot).x [f0]:\t%10.6f\n", (motStruct->rotationPivot).x);
        }
    }

    // If flying to the left
    debug_printf("[0x8009cc50] motStruct->flags [r0]:\t\t\t%u\n", motStruct->flags);
    if ((motStruct->flags & 1) == 0 && player->stickPosition < 0) {
        debug_printf("[0x8009cd50] BRANCH NOT TAKEN\n");

        if (-45.0 < motStruct->rotationPivot.x) {
            debug_printf("[0x8009cd70] BRANCH TAKEN\n");
            
            int64_t temp = 0x3FB99999A0000000;
            double DOUBLE_80420d40 = *(double *)&temp;

            debug_printf("[0x8009cd60] xRot: %10.6f\n", motStruct->rotationPivot.x);
            motStruct->rotationPivot.x = -(DOUBLE_80420d40 * stickXf - motStruct->rotationPivot.x);
            
            debug_printf("[0x8009cd8c] (motStruct->rotationPivot).x [f0]:\t%10.6f\n", (motStruct->rotationPivot).x);
        }
        else {
            // Restrict maximum (negative) nosedive
            debug_printf("[0x8009cd70] BRANCH NOT TAKEN\n");
            motStruct->rotationPivot.x = -45.0;
            debug_printf("[0x8009cd74] (motStruct->rotationPivot).x [f0]:\t\t%10.6f\n", (motStruct->rotationPivot).x);
        }
    }
    // If flying to the right
    else if ((motStruct->flags & 1) == 1 && 0 < player->stickPosition) {
        debug_printf("[0x8009ccb8] BRANCH NOT TAKEN\n");
        if (-45.0 < motStruct->rotationPivot.x) {
            debug_printf("[0x8009ccd8] BRANCH TAKEN\n");
            motStruct->rotationPivot.x = -(0.1 * stickXf - motStruct->rotationPivot.x);
            debug_printf("[0x8009ccf4] (motStruct->rotationPivot).x [f0]:\t\t\t%10.6f\n", (motStruct->rotationPivot).x);
        }
        else {
            // Reduce maximum (negative) nosedive
            debug_printf("[0x8009ccd8] BRANCH NOT TAKEN\n");
            motStruct->rotationPivot.x = -45.0;
            debug_printf("[0x8009ccdc] (motStruct->rotationPivot).x [f0]:\t\t\t%10.6f\n", (motStruct->rotationPivot).x);
        }
    }
}

void nosediveCalc(Player *player) {
    MotStruct *motStruct = &player->motStruct;
    motStruct->flags = motStruct->flags & 0xffffffef; // Flag for nosedive

    double dispDirectionCurrent = sin((double)((gPi * 1.5 * fabs(motStruct->rotationPivot.x)) / 180.0));
    debug_printf("[0x8009d258] dispDirectionCurrent [f1]:\t\t\t%10.6f\n", dispDirectionCurrent);

    motStruct->index4 = 0.2 * (float)dispDirectionCurrent;
    debug_printf("[0x8009d278] motStruct->index4 [f0]:\t\t\t%10.6f\n", motStruct->index4);

    player->baseSpeed = player->baseSpeed + motStruct->index4;
    debug_printf("[0x8009d28c] player.baseSpeed [f0]:\t\t\t%10.6f\n", player->baseSpeed);

    dispDirectionCurrent = sin((double)((gPi * 3.4 * fabs(motStruct->rotationPivot.x)) / 180.0));
    debug_printf("[0x8009d2c8] dispDirectionCurrent [f1]:\t\t\t%10.6f\n", dispDirectionCurrent);
    
    double dVar8 = dispDirectionCurrent;
    dispDirectionCurrent = sin((double)((gPi * player->baseSpeed * 0.05) / 180.0));
    debug_printf("[0x8009d2f4] dispDirectionCurrent [f1]:\t\t\t%10.6f\n", dispDirectionCurrent);

    motStruct->index5 = motStruct->index5 + 1.0 * (float)(dispDirectionCurrent * dVar8);
    debug_printf("[0x8009d31c] motStruct->index5 [f0]:\t\t\t%10.6f\n", motStruct->index5);

    // If stick is not neutral
    if (player->stickPosition != 0) {
        debug_printf("[0x8009d328] BRANCH NOT TAKEN\n");

        // If flying to the left, holding left, and index5 >= 1
        debug_printf("[0x8009d334] motStruct->flags [r0]:\t\t\t%d\n", motStruct->flags);
        if ((motStruct->flags & 1) == 0 && player->stickPosition < 0 && 1 <= motStruct->index5) {
            motStruct->index5 = 1;
            debug_printf("[0x8009d37c] motStruct->index5 [f0]:\t\t\t%10.6f\n", motStruct->index5);
        }
        else if (0 < player->stickPosition && 1 <= motStruct->index5) {
            motStruct->index5 = 1;
        }
        
        // Below execution occurs if no plane panel below Mario. Assume this is the case

        float wIndex7 = motStruct->index7;
        debug_printf("[0x8009d3b4] wIndex7 [f2]:\t\t\t%10.6f\n", wIndex7);

        motStruct->index7 = 0.1 * (1 - wIndex7) + wIndex7;
        debug_printf("[0x8009d3c8] motStruct->index7 [f1]:\t\t\t%10.6f\n", motStruct->index7);

        if (wIndex7 <= motStruct->index7) {
            debug_printf("[0x8009d3dc] BRANCH NOT TAKEN\n");

            motStruct->index7 = wIndex7;
            debug_printf("[0x8009d3e0] motStruct->index7 [f0]:\t\t\t%10.6f\n", motStruct->index7);
        }
    }

    // Below execution occurs if no plane panel below Mario. Assume this is the case
    float wIndex9 = motStruct->index9;

    motStruct->index9 = 0.1 * (-45.0 - wIndex9) + wIndex9;
    debug_printf("[0x8009d440] motStruct->index9 [f0]:\t\t\t%10.6f\n", motStruct->index9);

    motStruct->rotationPivot.x = motStruct->rotationPivot.x + motStruct->index5;
    debug_printf("[0x8009d454] motStruct->rotationPivot.x [f0]:\t\t\t%10.6f\n", motStruct->rotationPivot.x);

    if (motStruct->rotationPivot.x <= motStruct->index9) {
        debug_printf("[0x8009d46c] BRANCH NOT TAKEN\n");

        motStruct->rotationPivot.x = motStruct->index9;
        debug_printf("[0x8009d470] motStruct->rotationPivot.x [f1]:\t\t\t%10.6f\n", motStruct->rotationPivot.x);
    }

    float wySpeed = motStruct->ySpeed;
    debug_printf("[0x8009d47c] wySpeed [f2]:\t\t\t%10.6f\n", wySpeed);

    motStruct->ySpeed = 0.1 * -wySpeed + wySpeed;
    debug_printf("[0x8009d488] motStruct->ySpeed [f0]:\t\t\t%10.6f\n", motStruct->ySpeed);
}

void taildiveCalc(Player *player) {
    MotStruct *motStruct = &player->motStruct;
    
    debug_printf("[0x8009cf50] BRANCH NOT TAKEN\n");

    // Set the taildive flag to represent rotationPivot.x > 0
    motStruct->flags = motStruct->flags | 0x10;
    debug_printf("[0x8009cf64] motStruct->flags [r0]:\t\t\t%d\n", motStruct->flags);
    double dispDirectionCurrent = sin((double)((gPi * motStruct->rotationPivot.x * 0.1) / 180.0));
    debug_printf("[0x8009cf80] dispDirectionCurrent [f1]:\t\t\t%10.6f\n", dispDirectionCurrent);

    player->baseSpeed = player->baseSpeed + player->baseSpeed * -0.1 * (float)dispDirectionCurrent;
    debug_printf("[0x8009cfa4] player.baseSpeed [f0]:\t\t\t%10.6f\n", player->baseSpeed);
    
    if ((player->stickPosition != 0) && (25.0 <= motStruct->rotationPivot.x)) {
        debug_printf("[0x8009cfc8] BRANCH NOT TAKEN\n");
        player->baseSpeed = player->baseSpeed - 0.03;
        debug_printf("[0x8009cfd8] player.baseSpeed [f0]:\t\t\t%10.6f\n", player->baseSpeed);
    }

    if (player->baseSpeed <= 0.0) {
        debug_printf("[0x8009cfec] BRANCH NOT TAKEN\n");
        player->baseSpeed = 0.0;
        debug_printf("[0x8009cff0] player.baseSpeed [f0]:\t\t\t%10.6f\n", player->baseSpeed);
    }

    float wIndex5 = 0.0;
    float wySpeed;

    // If holding neutral...
    if (player->stickPosition == 0) {
        debug_printf("[0x8009d000] BRANCH NOT TAKEN\n");
        if (3.9 <= player->baseSpeed) {
            debug_printf("[0x8009d018] BRANCH TAKEN\n");
            motStruct->index5 = 0.118;
            debug_printf("[0x8009d074] motStruct->index5 [f0]:\t\t\t%10.6f\n", motStruct->index5);
        }
        else {
            debug_printf("[0x8009d018] BRANCH NOT TAKEN\n");
            float dVar8 = (double)(5.0 - player->baseSpeed);

            dispDirectionCurrent = sin((double)((gPi * motStruct->rotationPivot.x) / 180.0));
            debug_printf("[0x8009d03c] dispDirectionCurrent [f1]:\t\t\t%10.6f\n", dispDirectionCurrent);

            wIndex5 = (float)(dVar8 * (double)(-0.112 * (float)dispDirectionCurrent)) + motStruct->index5;

            motStruct->index5 = wIndex5;
            debug_printf("[0x8009d064] motStruct->index5 [f31]:\t\t\t%10.6f\n", motStruct->index5);
        }

        motStruct->rotationPivot.x = motStruct->rotationPivot.x + motStruct->index5;
        debug_printf("[0x8009d090] motStruct->rotationPivot.y [f0]:\t\t\t%10.6f\n", motStruct->rotationPivot.x);

        if ((wIndex5 <= 0) && (player->baseSpeed <= 2.5)) {
            debug_printf("[0x8009d0b8] BRANCH NOT TAKEN\n");

            wIndex5 = -(player->baseSpeed * 0.4 - 5.0);
            debug_printf("[0x8009d0c0] wIndex5 [f31]:\t\t\t%10.6f\n", wIndex5);

            if (wIndex5 <= 0) {
                debug_printf("[0x8009d0cc] BRANCH NOT TAKEN\n");
                wIndex5 = 0;
                debug_printf("[0x8009d0d0] wIndex5 [f31]:\t\t\t%10.6f\n", wIndex5);
            }

            dispDirectionCurrent = (double)(wIndex5 * 18.0);
            debug_printf("[0x8009d0c0] dispDirectionCurrent [f31]:\t\t\t%10.6f\n", dispDirectionCurrent);

            // Below assumes that there is no collision below Mario
            dispDirectionCurrent = sin((double)((float)((double)gPi * dispDirectionCurrent) / 180.0));
            debug_printf("[0x8009d11c] dispDirectionCurrent [f1]:\t\t\t%10.6f\n", dispDirectionCurrent);
            
            wIndex5 = -8.0 * (float)dispDirectionCurrent;
            debug_printf("[0x8009d134] wIndex5 [f0]:\t\t\t%10.6f\n", wIndex5);

            if (wIndex5 <= -2.0) {
                debug_printf("[0x8009d140] BRANCH NOT TAKEN\n");
                wIndex5 = -4.0;
                debug_printf("[0x8009d148] wIndex5 [f0]:\t\t\t%10.6f\n", wIndex5);
            }

            motStruct->index7 = wIndex5;
            debug_printf("[0x8009d150] motStruct->index7 [f0]:\t\t\t%10.6f\n", motStruct->index7);
        }

        motStruct->rotationPivot.x = motStruct->rotationPivot.x + motStruct->index7;
        debug_printf("[0x8009d164] motStruct->rotationPivot.x [f0]:\t\t\t%10.6f\n", motStruct->rotationPivot.x);
    }

    // If holding neutral
    if (player->stickPosition == 0) {
        debug_printf("[0x8009d170] BRANCH TAKEN\n");

        wySpeed = motStruct->ySpeed;
        debug_printf("[0x8009d1e4] wySpeed [f2]:\t\t\t%10.6f\n", wySpeed);

        wySpeed = 0.1 * -wySpeed + wySpeed;
        debug_printf("[0x8009d1ec] wySpeed [f2]:\t\t\t%10.6f\n", wySpeed);

        motStruct->ySpeed = wySpeed;
        debug_printf("[0x8009d1f0] motStruct->ySpeed [f2]:\t\t\t%10.6f\n", motStruct->ySpeed);
    }
    // If holding a direction
    else {
        debug_printf("[0x8009d170] BRANCH NOT TAKEN\n");

        dispDirectionCurrent = sin((double)((gPi * (5.0 - player->baseSpeed)) / 180.0));
        debug_printf("[0x8009d194] dispDirectionCurrent [f1]:\t\t\t%10.6f\n", dispDirectionCurrent);

        wySpeed = -0.3 * (float)dispDirectionCurrent;
        debug_printf("[0x8009d1b0] wySpeed [f2]:\t\t\t\t%10.6f\n", wySpeed);

        motStruct->ySpeed = motStruct->ySpeed + wySpeed;
        debug_printf("[0x8009d1b8] motStruct->ySpeed [f2]:\t\t\t%10.6f\n", motStruct->ySpeed);

        if (motStruct->ySpeed <= -5.0) {
            debug_printf("[0x8009d1d0] BRANCH NOT TAKEN\n");
            motStruct->ySpeed = -5.0;
            debug_printf("[0x8009d1d4] motStruct->ySpeed [f1]:\t\t\t%10.6f\n", motStruct->ySpeed);
        }
    }

    motStruct->rotationPivot.x = motStruct->rotationPivot.x + wySpeed;
    debug_printf("[0x8009d200] motStruct->rotationPivot.x [f0]:\t\t%10.6f\n", motStruct->rotationPivot.x);
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
    debug_printf("[0x8009cafc] dispDirectionCurrent[f1]:\t\t\t%10.6f\n", dispDirectionCurrent);
    
    motStruct->rotationPivot.y = (float)dispDirectionCurrent;
    debug_printf("[0x8009cb04] motStruct->rotationPivot.y [f1]:\t\t%10.6f\n", dispDirectionCurrent);

    // Normalize the calculated y-axis rotation
    yPivot = motStruct->rotationPivot.y;
    player.dispDirectionCurrent = revise360((double)yPivot);
    debug_printf("[0x8009cc04] player.dispDirectionCurrent[f1]:\t\t%10.6f\n", player.dispDirectionCurrent);

    debug_printf("[0x8009cc0c] player.stickPosition[r5]:\t\t\t\t0x%X\n", player.stickPosition);
                
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
    debug_printf("[0x8009cf3c] player.baseSpeed [f1]:\t\t\t%10.6f\n", player.baseSpeed);
    
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
        debug_printf("[0x8009d4a0] BRANCH NOT TAKEN (Speedcap reached)\n");
        player.baseSpeed = 5.0;
    }

    (player.position).y = (player.position).y + motStruct->ySpeed;
    debug_printf("[0x8009d4c0] (player.position).y [f0]:\t\t\t%10.6f\n", (player.position).y);

    int temp = 0x3e4ccccd;
    float FLOAT_8031049c = *(float *)&temp;

    (player.position).y = (player.position).y - FLOAT_8031049c;
    debug_printf("[0x8009d4d0] (player.position).y [f0]:\t\t\t%10.6f\n", (player.position).y);
    
    dispDirectionCurrent = revise360(-(double)motStruct->rotationPivot.x);
    debug_printf("[0x8009d4e0] dispDirectionCurrent [f1]:\t\t\t%10.6f\n", dispDirectionCurrent);
    
    (player.wRotationRelated).z = (float)dispDirectionCurrent;
    debug_printf("[0x8009d4e4] (player.wRotationRelated).z [f1]:\t\t%10.6f\n", (player.wRotationRelated).z);

    player.wDirectionView = motStruct->rotationPivot.y;

    // sin(local_34) and cos(local_38)
    sincosf(player.wDirectionView,&local_34,&local_38);
    debug_printf("[0x8009d584] local_34 [f2]:\t\t\t\t%10.6f\n", local_34);
    debug_printf("[0x8009d50c] local_38 [f3]:\t\t\t\t%10.6f\n", local_38);

    yPivot = 90.0;

    (player.position).z = player.baseSpeed * local_38 + (player.position).z;
    debug_printf("[0x8009d51c] (player.position).z [f1]:\t\t\t%10.6f\n", (player.position).z);

    sincosf(yPivot + motStruct->rotationPivot.x,&local_2c,&local_30);
    debug_printf("[0x8009d588] local_2c [f2]:\t\t\t\t%10.6f\n", local_2c);
    debug_printf("[0x8009d534] local_30 [f3]:\t\t\t\t%10.6f\n", local_30);

    yPivot = 1.0;

    if (0.0 <= local_30) {
        debug_printf("[0x8009d544] BRANCH NOT TAKEN\n");
        yPivot = 1.2;
        debug_printf("[0x8009d550] yPivot [f2]:\t\t\t\t%10.6f\n", yPivot);
    }

    (player.position).y = player.baseSpeed * local_30 * yPivot + (player.position).y;
    debug_printf("[0x8009d57c] (player.position).y [f0]:\t\t\t%10.6f\n", (player.position).y);

    (player.position).x = local_2c * player.baseSpeed * local_34 + (player.position).x;
    debug_printf("[0x8009d598] (player.position).x [f0]:\t\t\t%10.6f\n", (player.position).x);

    if (0.8 < player.baseSpeed) {
        debug_printf("[0x8009d5ac] BRANCH TAKEN\n");
        motStruct->index8 = 0;
        debug_printf("[0x8009d69c] motStruct->index8 [f0]:\t\t\t%10.6f\n", motStruct->index8);
    }
    else {
        debug_printf("[0x8009d5ac] BRANCH NOT TAKEN\n");
        iVar4 = sysMsec2Frame(1500);
        debug_printf("[0x8009d5c8] iVar4 [r3]:\t\t\t\t%d\n", iVar4);

        debug_printf("[0x8009d5d4] motStruct->flags [r0]:\t%d\n", motStruct->flags);
        if ((motStruct->flags & 1) == 0) {
            debug_printf("[0x8009d5d8] BRANCH TAKEN\n");

            if ((0 < player.stickPosition) && (uVar3 = motStruct->index8, motStruct->index8 = uVar3 + 1, iVar4 <= (int)(uVar3 + 1))) {
                debug_printf("[0x8009d67c] BRANCH NOT TAKEN\n");
                debug_printf("Frame execution has ended at 0x8009d690");
            }
        }
        else {
            debug_printf("[0x8009d5d8] BRANCH NOT TAKEN\n");

            if ((player.stickPosition < 0) && (uVar3 = motStruct->index8, motStruct->index8 = uVar3 + 1, iVar4 <= (int)(uVar3 + 1))) {
                debug_printf("[0x8009d620] BRANCH NOT TAKEN\n");
                debug_printf("Frame execution has ended at 0x8009d634");
            }
        }
    }

    // 0x8009d6a0
    // If Mario does not collide with obj, jump to 0x8009d718

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

/*
int main() {
    Player cur = init();

    int i;

    for (i=0; i < 145; i++) {
        debug_printf("Loop iteration %d\n", i);
        debug_printf("Stick input %d\n", -72+i);
        Result *result = malloc(sizeof(Result));
        frameSim(-72+i, &cur, result);

        debug_printf("\n\nEnd Frame Position: X: %10.6f | Y: %10.6f | Z: %10.6f\n", result->player.position.x, result->player.position.y, result->player.position.z);
        debug_printf("End Frame X Rotation: %10.6f\n", result->player.motStruct.rotationPivot.x);
        if (result->collided) {
            debug_printf("Collided!\n");
        }
        if (result->landed) {
            debug_printf("Landed!\n");
        }

        cur = result->player;

    }
}
*/
