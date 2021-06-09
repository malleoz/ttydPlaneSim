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
    uint8_t stickPosition;
    float baseSpeed;
};

// Returns from main sim function
struct Result {
    struct Player player;
    bool collided;
    bool landed;
};

struct Player getPlayer();
struct Player init();
double revise360(double param_1);
void sincosf(float __x, float *__sinx, float *__cosx);
int sysMsec2Frame(int param_1);
void frameSim(signed char stickPosition, int numFrames, struct Player *previousFrame, struct Result *nextFrame);