#include <stdlib.h>
#include <stdio.h>
#include "plane_physics.h"
#include <string.h>
#include <math.h>
#include <stdbool.h>

typedef uint64_t ulonglong;
typedef int64_t longlong;
typedef uint32_t uint;
typedef struct Player Player;
typedef struct Vector Vector;
typedef struct MotStruct MotStruct;

double DOUBLE_802c4168;
float  gPi;
Player player; // Allocate on stack to appease trebuchet...

Player getPlayer() {
    return player;
}

// TODO: Read from RAM dump to initialize player struct parameters (namely motStruct and position)
void initPlayerStruct() {
    // IF FLYING TO THE RIGHT:
    // Seems that flags = 1
    // rotationPivot.y = 90
    // else: flags = 0, y = 270
    player.motStruct.flags = 1;
    player.motStruct.rotationPivot = (Vector){0, 90, 0};
    player.motStruct.index4 = 2;
    player.motStruct.index5 = 0;
    player.motStruct.ySpeed = 0;
    player.motStruct.index7 = 0; // TODO
    player.motStruct.index8 = 0; // TODO
    player.motStruct.index9 = 0; // TODO

    player.wStickDir1 = 0xB8; // THE GENETIC ALGORITHM WILL NEED TO MODIFY THIS VALUE
    // FULL LEFT IS 0xB8
    // MIN LEFT IS 0xFF
    // NEUTRAL IS 0x00
    // MIN RIGHT IS 0x01
    // MAX RIGHT IS 0x48

    (player.position).x = -355.383118;
    (player.position).y = 192.5;
    (player.position).z = 49.462914;

    player.baseSpeed = 1.0;
    return;
}

void init() {
    DOUBLE_802c4168 = 0x4330000080000000;
    gPi             = 3.1415927;

    player.baseSpeed = 1.0;
    initPlayerStruct(player);

    return;
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
void sincosf(float __x, float *__sinx, float *__cosx) {
    double rad = (double)((gPi * __x ) / 180.0);
    *__sinx = (float)sin(rad);
    * __cosx = -(float)cos(rad);
    return;
}

// This was an in-game function but is optimized a bit
// PowerPC does not have a div instruction so there
// were bit-shift artifacts left over
int sysMsec2Frame(int param_1) {
    return (param_1 * 60) / 1000;
}

// Main simulation routine
int frame() {
    MotStruct motStruct = player.motStruct;

    bool bVar1 = false;
    uint uVar3;
    float xPivot;
    float yPivot;
    float baseSpeed;
    float stickXf;
    float wIndex5;
    float wySpeed;
    float wIndex7;
    float wIndex9;
    int8_t wStickDirX;
    double local_28;
    float local_2c;
    float local_30;
    float local_34;
    float local_38;
    int iVar4;

    // 0x8009cafc
    double dispDirectionCurrent = revise360((double)(motStruct.rotationPivot.z * 0.025 + motStruct.rotationPivot.y));
    printf("[0x8009cafc] dispDirectionCurrent[f1]:\t\t\t%10.6f\n", dispDirectionCurrent);
    
    motStruct.rotationPivot.y = (float)dispDirectionCurrent;
    printf("[0x8009cb04] motStruct.rotationPivot.y [f1]:\t\t%10.6f\n", dispDirectionCurrent);

    yPivot = motStruct.rotationPivot.y;

    printf("[0x8009cb0c] motStruct.flags [r0]:\t\t\t%u\n", motStruct.flags);
    if ((motStruct.flags & 1) == 0) {
        // This likely means that we are flying to the RIGHT
        printf("[0x8009cb14] BRANCH TAKEN\n");

        // These two if-statements seem to handle cases where the plane panel is not flying directly left,
        // rather it is flying at a slight angle pivoting around the (upwards) Y-axis (think Glitzville's panel)
        if (330.0 <= yPivot) {
            printf("[0x8009cb78]: 330.0 < yPivot\n");
            (motStruct.rotationPivot).y = 0.1 * (330.0 - yPivot) + yPivot;
            printf("[0x8009cb88] motStruct.rotationPivot.y [f0]:\t\t%10.6f\n", motStruct.rotationPivot.y);
            yPivot = motStruct.rotationPivot.y;
        }
        
        if (yPivot <= 210.0) {
            printf("[0x8009cba0]: yPivot <= 210.0\n");
            motStruct.rotationPivot.y = 0.1 * (210.0 - yPivot) + yPivot;
            printf("[0x8009cbb0] motStruct.rotationPivot.y [f0]:\t\t%10.6f\n", motStruct.rotationPivot.y);
        }
    }
    else {
        // If plane panel is facing left
        printf("[0x8009cb14] BRANCH NOT TAKEN\n");
        
        // Same as above. These if-statements handle rotated plane panel flights.
        if (yPivot <= 60.0) {
            printf("[0x8009cb28]: yPivot <= 60.0\n");
            (motStruct.rotationPivot).y = 0.1 * (60.0 - yPivot) + yPivot;
            printf("[0x8009cb38] motStruct.rotationPivot.y [f0]:\t\t%10.6f\n", motStruct.rotationPivot.y);
            yPivot = motStruct.rotationPivot.y;
        }
        
        if (150.0 <= yPivot) {
            printf("[0x8009cb50]: 150.0 <= yPivot\n");
            motStruct.rotationPivot.y = 0.1 * (150.0 - yPivot) + yPivot;
            printf("[0x8009cb60] motStruct.rotationPivot.y [f0]:\t\t%10.6f\n", motStruct.rotationPivot.y);
        }
    }
    
    yPivot = motStruct.rotationPivot.y;

    // If flying parallel to or towards camera
    if ((90.0 <= yPivot) && (yPivot <= 270.0)) {
        printf("[0x8009cbe0] bVar1 = true\n");
        bVar1 = true;
    }

    // bVar1 is true most of the time
    if (bVar1) {
        printf("[0x8009cbec] -yPivot - 90.0\n");
        yPivot = -yPivot - 90.0;
    }
    else {
        printf("[0x8009cbfc] -yPivot + 270.0\n");
        yPivot = 270.0 - yPivot;
    }

    // Normalize the calculated y-axis rotation
    player.dispDirectionCurrent = revise360((double)yPivot);
    printf("[0x8009cc04] player.dispDirectionCurrent[f1]:\t\t\t%10.6f\n", player.dispDirectionCurrent);

    wStickDirX = player.wStickDir1;
    printf("[0x8009cc0c] wStickDirX[r5]:\t\t\t\t0x%X\n", wStickDirX);
                
    // If analog stick is neutral
    if (wStickDirX == 0) {
        printf("[0x8009cc14] BRANCH TAKEN\n");

        // If nosediving or (some form of acceleration?)
        if ((motStruct.rotationPivot.x <= 0.0) || (motStruct.index5 <= 0.0))
        {
            printf("[0x8009cda8] BRANCH TAKEN\n");

            baseSpeed = 5.0 - player.baseSpeed;
            printf("[0x8009ce00] baseSpeed [f3]:\t\t%10.6f\n", baseSpeed);

            if (baseSpeed < 0.0) {
                baseSpeed = 0.0;
                printf("[0x8009ce10] baseSpeed [f3?]:\t\t%10.6f\n", baseSpeed);
            }

            double dVar8 = sin((double)((gPi * baseSpeed * 18.0) / 180.0));
            printf("[0x8009ce30] dVar8 [f1]:\t\t%10.6f\n", dVar8);

            dispDirectionCurrent = (double)(-4.0 * (float)dVar8);
            printf("[0x8009ce48] dispDirectionCurrent [f31]:\t\t%10.6f\n", dispDirectionCurrent);
            
            if ((double)(-4.0 * (float)dVar8) <= (double)-2.0) {
                printf("[0x8009ce54] BRANCH NOT TAKEN\n");
                dispDirectionCurrent = (double)-2.0;
                printf("[0x8009ce58] dispDirectionCurrent [f31]:\t\t%10.6f\n", dispDirectionCurrent);
            }

            //bVar1 = false; // 8009ce5c

            // 8009ce78 - We assume that hitobjUnder.attr = 0

            motStruct.index7 = (float)dispDirectionCurrent;
            printf("[0x8009ce8c] motStruct.index7 [f31]:\t\t%10.6f\n", motStruct.index7);

            motStruct.rotationPivot.x = motStruct.rotationPivot.x + motStruct.index7;
            printf("[0x8009cea0] motStruct.rotationPivot.x [f0]:\t\t%10.6f\n", motStruct.rotationPivot.x);
        }
        
        // Restrict maximum (negative) nosedive angle
        if (motStruct.rotationPivot.x <= -45.0) {
            printf("[0x8009cefc] BRANCH NOT TAKEN\n");
            motStruct.rotationPivot.x = -45.0;
        }

        if (6.0 <= motStruct.rotationPivot.x) {
            printf("[0x8009cf18] BRANCH NOT TAKEN\n");
            motStruct.index5 = 6.0;
        }
    }
    // If speed is not 0 (we are mid-flight and have not bonked yet)
    else if (player.baseSpeed != 0.0) {
        printf("[0x8009cc24] BRANCH NOT TAKEN\n");

        // Represent the magnitude of the stick input as a float
        stickXf = fabs((float)wStickDirX);
        printf("[0x8009cc5c] stickXf [f3]:\t\t\t\t%10.6f\n", stickXf);
        
        // If flying to the left
        printf("[0x8009cc50] motStruct.flags [r0]:\t\t\t%u\n", motStruct.flags);
        if ((motStruct.flags & 1) == 0) {
            // If analog stick is right
            if (0 < wStickDirX) {
                printf("[0x8009cd00] BRANCH NOT TAKEN\n");
                xPivot = (motStruct.rotationPivot).x;
                
                if (xPivot < 25.0) {
                    printf("[0x8009cd1c] BRANCH TAKEN\n");
                    (motStruct.rotationPivot).x = 0.04 * stickXf + xPivot;
                    printf("[0x8009cd44] (motStruct.rotationPivot).x [f0]:\t%10.6f\n", (motStruct.rotationPivot).x);
                }
                else {
                    printf("[0x8009cd1c] BRANCH NOT TAKEN\n");
                    (motStruct.rotationPivot).x = 0.1 * (25.0 - xPivot) + xPivot;
                    printf("[0x8009cd44] (motStruct.rotationPivot).x [f0]:\t%10.6f\n", (motStruct.rotationPivot).x);
                }
            }
            // If analog stick is left
            else if (wStickDirX < 0) {
                printf("[0x8009cd50] BRANCH NOT TAKEN\n");

                if (-45.0 < motStruct.rotationPivot.x) {
                    printf("[0x8009cd70] BRANCH TAKEN\n");
                    
                    int64_t temp = 0x3FB99999A0000000;
                    double DOUBLE_80420d40 = *(double *)&temp;

                    printf("[0x8009cd60] xRot: %10.6f\n", motStruct.rotationPivot.x);
                    motStruct.rotationPivot.x = -(DOUBLE_80420d40 * stickXf - motStruct.rotationPivot.x);
                    
                    printf("[0x8009cd8c] (motStruct.rotationPivot).x [f0]:\t%10.6f\n", (motStruct.rotationPivot).x);
                }
                else {
                    // Restrict maximum (negative) nosedive
                    printf("[0x8009cd70] BRANCH NOT TAKEN\n");
                    motStruct.rotationPivot.x = -45.0;
                    printf("[0x8009cd74] (motStruct.rotationPivot).x [f0]:\t\t%10.6f\n", (motStruct.rotationPivot).x);
                }
            }
        }
        // If flying to the right
        else {
            // If stick is left
            if (wStickDirX < 0) {
                printf("[0x8009cc68] BRANCH NOT TAKEN\n");

                xPivot = (motStruct.rotationPivot).x;
                printf("[0x8009cc74] xPivot [f2]:\t\t\t\t%10.6f\n", xPivot);

                if (xPivot < 25.0) {
                    printf("[0x8009cc84] BRANCH TAKEN\n");

                    (motStruct.rotationPivot).x = 0.04 * stickXf + xPivot;
                    printf("[0x8009ccac] (motStruct.rotationPivot).x [f0]:\t\t%10.6f\n", (motStruct.rotationPivot).x);
                }
                else {
                    printf("[0x8009cc84] BRANCH NOT TAKEN\n");
                    (motStruct.rotationPivot).x = 0.1 * (25.0 - xPivot) + xPivot;
                    printf("[0x8009cc94] (motStruct.rotationPivot).x [f0]:\t\t\t%10.6f\n", (motStruct.rotationPivot).x);
                }
            }
            // If stick is right
            else if (0 < wStickDirX) {
                printf("[0x8009ccb8] BRANCH NOT TAKEN\n");
                if (-45.0 < motStruct.rotationPivot.x) {
                    printf("[0x8009ccd8] BRANCH TAKEN\n");
                    motStruct.rotationPivot.x = -(0.1 * stickXf - motStruct.rotationPivot.x);
                    printf("[0x8009ccf4] (motStruct.rotationPivot).x [f0]:\t\t\t%10.6f\n", (motStruct.rotationPivot).x);
                }
                else {
                    // Reduce maximum (negative) nosedive
                    printf("[0x8009ccd8] BRANCH NOT TAKEN\n");
                    motStruct.rotationPivot.x = -45.0;
                    printf("[0x8009ccdc] (motStruct.rotationPivot).x [f0]:\t\t\t%10.6f\n", (motStruct.rotationPivot).x);
                }
            }
        }   
    }
    
    xPivot = 0.0;
    printf("[0x8009cf30] xPivot [f0]:\t\t\t\t%10.6f\n", xPivot);
    
    // Apply a static speed decrease, before factoring in xPivot
    player.baseSpeed = player.baseSpeed * -0.02 + player.baseSpeed;
    printf("[0x8009cf3c] player.baseSpeed [f1]:\t\t\t%10.6f\n", player.baseSpeed);
    
    float fVar2 = 180.0;
    
    // If nosediving
    if (motStruct.rotationPivot.x < 0) {
        printf("[0x8009cf50] BRANCH TAKEN\n");

        // There is no instance where this condition will not be satisfied
        // since nosedive cap is at -45.0
        if (-80.0 <= motStruct.rotationPivot.x) {
            printf("[0x8009d214] BRANCH NOT TAKEN\n");

            // flags & 0x10 == 1 if xPivot > 0
            if ((motStruct.flags & 0x10) != 0) {
                printf("[0x8009d220] BRANCH NOT TAKEN\n");
                motStruct.flags = motStruct.flags & 0xffffffef;
                printf("[0x8009d228] motStruct.flags [r0]:\t\t\t%d\n", motStruct.flags);
            }

            dispDirectionCurrent = sin((double)((gPi * 1.5 * fabs((double)motStruct.rotationPivot.x)) / 180.0));
            printf("[0x8009d258] dispDirectionCurrent [f1]:\t\t\t%10.6f\n", dispDirectionCurrent);

            xPivot = 0.0;
            motStruct.index4 = 0.2 * (float)dispDirectionCurrent;
            printf("[0x8009d278] motStruct.index4 [f0]:\t\t\t%10.6f\n", motStruct.index4);

            player.baseSpeed = player.baseSpeed + motStruct.index4;
            printf("[0x8009d28c] player.baseSpeed [f0]:\t\t\t%10.6f\n", player.baseSpeed);
            
            // There was a redundant if statement here, which checked that rotationPivot.x < 0
            xPivot = fabs(motStruct.rotationPivot.x);
            printf("[0x8009d2a4] xPivot [f3]:\t\t\t%10.6f\n", xPivot);
        
            dispDirectionCurrent = sin((double)((gPi * 3.4 * xPivot) / 180.0));
            printf("[0x8009d2c8] dispDirectionCurrent [f1]:\t\t\t%10.6f\n", dispDirectionCurrent);
            
            double dVar8 = dispDirectionCurrent;
            dispDirectionCurrent = sin((double)((gPi * player.baseSpeed * 0.05) / 180.0));
            printf("[0x8009d2f4] dispDirectionCurrent [f1]:\t\t\t%10.6f\n", dispDirectionCurrent);

            motStruct.index5 = motStruct.index5 + 1.0 * (float)(dispDirectionCurrent * dVar8);
            printf("[0x8009d31c] motStruct.index5 [f0]:\t\t\t%10.6f\n", motStruct.index5);

            // If stick is not neutral
            if (wStickDirX != 0) {
                printf("[0x8009d328] BRANCH NOT TAKEN\n");

                // If flying to the left, holding left, and index5 >= 1
                printf("[0x8009d334] motStruct.flags [r0]:\t\t\t%d\n", motStruct.flags);
                if ((motStruct.flags & 1) == 0 && wStickDirX < 0 && 1 <= motStruct.index5) {
                    motStruct.index5 = 1;
                    printf("[0x8009d37c] motStruct.index5 [f0]:\t\t\t%10.6f\n", motStruct.index5);
                }
                else if (0 < wStickDirX && 1 <= motStruct.index5) {
                    motStruct.index5 = 1;
                }
                
                // Below execution occurs if no plane panel below Mario. Assume this is the case

                wIndex7 = motStruct.index7;
                printf("[0x8009d3b4] wIndex7 [f2]:\t\t\t%10.6f\n", wIndex7);

                motStruct.index7 = 0.1 * (1 - wIndex7) + wIndex7;
                printf("[0x8009d3c8] motStruct.index7 [f1]:\t\t\t%10.6f\n", motStruct.index7);

                if (wIndex7 <= motStruct.index7) {
                    printf("[0x8009d3dc] BRANCH NOT TAKEN\n");

                    motStruct.index7 = wIndex7;
                    printf("[0x8009d3e0] motStruct.index7 [f0]:\t\t\t%10.6f\n", motStruct.index7);
                }
            }

            // Below execution occurs if no plane panel below Mario. Assume this is the case
            wIndex9 = motStruct.index9;

            motStruct.index9 = 0.1 * (-45.0 - wIndex9) + wIndex9;
            printf("[0x8009d440] motStruct.index9 [f0]:\t\t\t%10.6f\n", motStruct.index9);

            motStruct.rotationPivot.x = motStruct.rotationPivot.x + motStruct.index5;
            printf("[0x8009d454] motStruct.rotationPivot.x [f0]:\t\t\t%10.6f\n", motStruct.rotationPivot.x);

            if (motStruct.rotationPivot.x <= motStruct.index9) {
                printf("[0x8009d46c] BRANCH NOT TAKEN\n");

                motStruct.rotationPivot.x = motStruct.index9;
                printf("[0x8009d470] motStruct.rotationPivot.x [f1]:\t\t\t%10.6f\n", motStruct.rotationPivot.x);
            }

            wySpeed = motStruct.ySpeed;
            printf("[0x8009d47c] wySpeed [f2]:\t\t\t%10.6f\n", wySpeed);

            motStruct.ySpeed = 0.1 * -wySpeed + wySpeed;
            printf("[0x8009d488] motStruct.ySpeed [f0]:\t\t\t%10.6f\n", motStruct.ySpeed);
        }
    }
    // If we're not nosediving...
    else {
        printf("[0x8009cf50] BRANCH NOT TAKEN\n");

        // Set the taildive flag to represent rotationPivot.x > 0
        motStruct.flags = motStruct.flags | 0x10;
        printf("[0x8009cf64] motStruct.flags [r0]:\t\t\t%d\n", motStruct.flags);

        dispDirectionCurrent = sin((double)((gPi * motStruct.rotationPivot.x * 0.1) / fVar2));
        printf("[0x8009cf80] dispDirectionCurrent [f1]:\t\t\t%10.6f\n", dispDirectionCurrent);

        player.baseSpeed = player.baseSpeed + player.baseSpeed * -0.1 * (float)dispDirectionCurrent;
        printf("[0x8009cfa4] player.baseSpeed [f0]:\t\t\t%10.6f\n", player.baseSpeed);
        
        if ((wStickDirX != 0) && (25.0 <= motStruct.rotationPivot.x)) {
            printf("[0x8009cfc8] BRANCH NOT TAKEN\n");
            player.baseSpeed = player.baseSpeed - 0.03;
            printf("[0x8009cfd8] player.baseSpeed [f0]:\t\t\t%10.6f\n", player.baseSpeed);
        }

        if (player.baseSpeed <= 0.0) {
            printf("[0x8009cfec] BRANCH NOT TAKEN\n");
            player.baseSpeed = 0.0;
            printf("[0x8009cff0] player.baseSpeed [f0]:\t\t\t%10.6f\n", player.baseSpeed);
        }

        wIndex5 = 0.0;

        // If holding neutral...
        if (wStickDirX == 0) {
            printf("[0x8009d000] BRANCH NOT TAKEN\n");
            if (3.9 <= player.baseSpeed) {
                printf("[0x8009d018] BRANCH TAKEN\n");
                motStruct.index5 = 0.118;
                printf("[0x8009d074] motStruct.index5 [f0]:\t\t\t%10.6f\n", motStruct.index5);
            }
            else {
                printf("[0x8009d018] BRANCH NOT TAKEN\n");
                float dVar8 = (double)(5.0 - player.baseSpeed);

                dispDirectionCurrent = sin((double)((gPi * motStruct.rotationPivot.x) / 180.0));
                printf("[0x8009d03c] dispDirectionCurrent [f1]:\t\t\t%10.6f\n", dispDirectionCurrent);

                wIndex5 = (float)(dVar8 * (double)(-0.112 * (float)dispDirectionCurrent)) + motStruct.index5;

                motStruct.index5 = wIndex5;
                printf("[0x8009d064] motStruct.index5 [f31]:\t\t\t%10.6f\n", motStruct.index5);
            }

            motStruct.rotationPivot.x = motStruct.rotationPivot.x + motStruct.index5;
            printf("[0x8009d090] motStruct.rotationPivot.y [f0]:\t\t\t%10.6f\n", motStruct.rotationPivot.x);

            if ((wIndex5 <= 0) && (player.baseSpeed <= 0.25)) {
                printf("[0x8009d0b8] BRANCH NOT TAKEN\n");

                wIndex5 = -(player.baseSpeed * 0.4 - 5.0);
                printf("[0x8009d0c0] wIndex5 [f31]:\t\t\t%10.6f\n", wIndex5);

                if (wIndex5 <= 0) {
                    printf("[0x8009d0cc] BRANCH NOT TAKEN\n");
                    wIndex5 = 0;
                    printf("[0x8009d0d0] wIndex5 [f31]:\t\t\t%10.6f\n", wIndex5);
                }

                dispDirectionCurrent = (double)(wIndex5 * 18.0);
                printf("[0x8009d0c0] dispDirectionCurrent [f31]:\t\t\t%10.6f\n", dispDirectionCurrent);

                // Below assumes that there is no collision below Mario
                dispDirectionCurrent = sin((double)((float)((double)gPi * dispDirectionCurrent) / 180.0));
                printf("[0x8009d11c] dispDirectionCurrent [f1]:\t\t\t%10.6f\n", dispDirectionCurrent);
                
                wIndex5 = -8.0 * (float)dispDirectionCurrent;
                printf("[0x8009d134] wIndex5 [f0]:\t\t\t%10.6f\n", wIndex5);

                if (wIndex5 <= -2.0) {
                    printf("[0x8009d140] BRANCH NOT TAKEN\n");
                    wIndex5 = -4.0;
                    printf("[0x8009d148] wIndex5 [f0]:\t\t\t%10.6f\n", wIndex5);
                }

                motStruct.index7 = wIndex5;
                printf("[0x8009d150] motStruct.index7 [f0]:\t\t\t%10.6f\n", motStruct.index7);
            }

            motStruct.rotationPivot.x = motStruct.rotationPivot.x + motStruct.index7;
            printf("[0x8009d164] motStruct.rotationPivot.x [f0]:\t\t\t%10.6f\n", motStruct.rotationPivot.x);
        }

        // If holding neutral
        if (wStickDirX == 0) {
            printf("[0x8009d170] BRANCH TAKEN\n");

            wySpeed = motStruct.ySpeed;
            printf("[0x8009d1e4] wySpeed [f2]:\t\t\t%10.6f\n", wySpeed);

            wySpeed = 0.1 * -wySpeed + wySpeed;
            printf("[0x8009d1ec] wySpeed [f2]:\t\t\t%10.6f\n", wySpeed);

            motStruct.ySpeed = wySpeed;
            printf("[0x8009d1f0] motStruct.ySpeed [f2]:\t\t\t%10.6f\n", motStruct.ySpeed);
        }
        // If holding a direction
        else {
            printf("[0x8009d170] BRANCH NOT TAKEN\n");

            dispDirectionCurrent = sin((double)((gPi * (5.0 - player.baseSpeed)) / 180.0));
            printf("[0x8009d194] dispDirectionCurrent [f1]:\t\t\t%10.6f\n", dispDirectionCurrent);

            wySpeed = -0.3 * (float)dispDirectionCurrent;
            printf("[0x8009d1b0] wySpeed [f2]:\t\t\t\t%10.6f\n", wySpeed);

            motStruct.ySpeed = motStruct.ySpeed + wySpeed;
            printf("[0x8009d1b8] motStruct.ySpeed [f2]:\t\t\t%10.6f\n", motStruct.ySpeed);

            if (motStruct.ySpeed <= -5.0) {
                printf("[0x8009d1d0] BRANCH NOT TAKEN\n");
                motStruct.ySpeed = -5.0;
                printf("[0x8009d1d4] motStruct.ySpeed [f1]:\t\t\t%10.6f\n", motStruct.ySpeed);
            }
        }

        motStruct.rotationPivot.x = motStruct.rotationPivot.x + wySpeed;
        printf("[0x8009d200] motStruct.rotationPivot.x [f0]:\t\t%10.6f\n", motStruct.rotationPivot.x);
    }
    
    // Enforce speedcap
    if (5.0 <= player.baseSpeed) {
        printf("[0x8009d4a0] BRANCH NOT TAKEN (Speedcap reached)\n");
        player.baseSpeed = 5.0;
    }

    (player.position).y = (player.position).y + motStruct.ySpeed;
    printf("[0x8009d4c0] (player.position).y [f0]:\t\t\t%10.6f\n", (player.position).y);

    int temp = 0x3e4ccccd;
    float FLOAT_8031049c = *(float *)&temp;

    (player.position).y = (player.position).y - FLOAT_8031049c;
    printf("[0x8009d4d0] (player.position).y [f0]:\t\t\t%10.6f\n", (player.position).y);
    
    dispDirectionCurrent = revise360(-(double)motStruct.rotationPivot.x);
    printf("[0x8009d4e0] dispDirectionCurrent [f1]:\t\t\t%10.6f\n", dispDirectionCurrent);
    
    (player.wRotationRelated).z = (float)dispDirectionCurrent;
    printf("[0x8009d4e4] (player.wRotationRelated).z [f1]:\t\t%10.6f\n", (player.wRotationRelated).z);

    player.wDirectionView = motStruct.rotationPivot.y;

    // sin(local_34) and cos(local_38)
    sincosf(player.wDirectionView,&local_34,&local_38);
    printf("[0x8009d584] local_34 [f2]:\t\t\t\t%10.6f\n", local_34);
    printf("[0x8009d50c] local_38 [f3]:\t\t\t\t%10.6f\n", local_38);

    yPivot = 90.0;

    (player.position).z = player.baseSpeed * local_38 + (player.position).z;
    printf("[0x8009d51c] (player.position).z [f1]:\t\t\t%10.6f\n", (player.position).z);

    sincosf(yPivot + motStruct.rotationPivot.x,&local_2c,&local_30);
    printf("[0x8009d588] local_2c [f2]:\t\t\t\t%10.6f\n", local_2c);
    printf("[0x8009d534] local_30 [f3]:\t\t\t\t%10.6f\n", local_30);

    yPivot = 1.0;

    if (0.0 <= local_30) {
        printf("[0x8009d544] BRANCH NOT TAKEN\n");
        yPivot = 1.2;
        printf("[0x8009d550] yPivot [f2]:\t\t\t\t%10.6f\n", yPivot);
    }

    (player.position).y = player.baseSpeed * local_30 * yPivot + (player.position).y;
    printf("[0x8009d57c] (player.position).y [f0]:\t\t\t%10.6f\n", (player.position).y);

    (player.position).x = local_2c * player.baseSpeed * local_34 + (player.position).x;
    printf("[0x8009d598] (player.position).x [f0]:\t\t\t%10.6f\n", (player.position).x);

    if (0.8 < player.baseSpeed) {
        printf("[0x8009d5ac] BRANCH TAKEN\n");
        motStruct.index8 = 0;
        printf("[0x8009d69c] motStruct.index8 [f0]:\t\t\t%10.6f\n", motStruct.index8);
    }
    else {
        printf("[0x8009d5ac] BRANCH NOT TAKEN\n");
        local_28 = 1500.0;
        iVar4 = sysMsec2Frame(1500);
        printf("[0x8009d5c8] iVar4 [r3]:\t\t\t\t%d\n", iVar4);

        printf("[0x8009d5d4] motStruct.flags [r0]:\t%d\n", motStruct.flags);
        if ((motStruct.flags & 1) == 0) {
            printf("[0x8009d5d8] BRANCH TAKEN\n");

            local_28 = (double)wStickDirX;
            printf("[0x8009d65c] local_28 [r0]:\t\t\t\t%10.6f\n", local_28);

            if ((0.0 < local_28) && (uVar3 = motStruct.index8, motStruct.index8 = uVar3 + 1, iVar4 <= (int)(uVar3 + 1))) {
                printf("[0x8009d67c] BRANCH NOT TAKEN\n");
                return 0;
                printf("Frame execution has ended at 0x8009d690");
            }
        }
        else {
            printf("[0x8009d5d8] BRANCH NOT TAKEN\n");

            local_28 = (double)wStickDirX;
            printf("[0x8009d600] local_28 [r0]:\t\t\t\t%10.6f\n", local_28);

            if ((local_28 < 0.0) && (uVar3 = motStruct.index8, motStruct.index8 = uVar3 + 1, iVar4 <= (int)(uVar3 + 1))) {
                printf("[0x8009d620] BRANCH NOT TAKEN\n");
                return 0;
                printf("Frame execution has ended at 0x8009d634");
            }
        }
    }

    // 0x8009d6a0
    // If Mario does not collide with obj, jump to 0x8009d718


    return 0;
}

int main() {
    init();
    printf("Global variables initialized.\n");
    
    initPlayerStruct();
    printf("Player structure initialized.\n");

    //struct Vector destination = (struct Vector){-369, 100, 0};

    int i;
    for (i=0; i < 1; i++) {
        printf("Loop iteration %d\n", i);
        frame();

        printf("\n\nEnd Frame Position: X: %10.6f | Y: %10.6f | Z: %10.6f\n", player.position.x, player.position.y, player.position.z);
    }
}