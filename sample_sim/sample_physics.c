#include "sample_physics.h"

struct Player init(){
    struct Player p; 
    p.position = 0;
    p.frameNumber = 0;
    return p;
}

void frameSim(int8_t stickPosition, 
        struct Player *previousFrame,
        struct Result *nextFrame){
    nextFrame ->player. position = previousFrame->position + stickPosition;
    nextFrame -> player.frameNumber = previousFrame -> frameNumber + 1;
    if(nextFrame->player.frameNumber > 600){
        nextFrame->collided = true;
    } else {
        nextFrame->collided = false;
    }
    if(nextFrame->player.position >= 1000){
        nextFrame -> landed = true;
        nextFrame -> collided = true;
    }
}

