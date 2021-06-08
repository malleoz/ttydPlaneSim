#include <stdlib.h>
#include <inttypes.h>

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

struct hitobj {
    uint32_t attr;
};

struct Player {
    struct Vector wRotationRelated;
    float wDirectionView;
    struct Vector position;
    double dispDirectionCurrent;
    struct MotStruct motStruct;
    uint8_t wStickDir1;
    float baseSpeed;
};

struct Player getPlayer();
void initPlayerStruct();
void init();
double revise360(double param_1);
void sincosf(float __x, float *__sinx, float *__cosx);
int sysMsec2Frame(int param_1);
int frame();