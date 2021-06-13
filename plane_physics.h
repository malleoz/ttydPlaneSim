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
    float index8;
    float index9;
};

struct Player {
    struct Vector wRotationRelated;
    float wDirectionView;
    struct Vector position;
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
};

struct Player init();
double revise360(double param_1);
void sincosf(float x, float *sinx, float *cosx);
int sysMsec2Frame(int param_1);
void neutralCalc(struct Player *player);
void nonNeutralCalc(struct Player *player);
void nosediveCalc(struct Player *player);
void taildiveCalc(struct Player *player);
void frameSim(signed char stickPosition, struct Player *previousFrame, struct Result *nextFrame);

#endif
