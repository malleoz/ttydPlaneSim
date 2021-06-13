#include "run_simulation.h"
#include "sample_physics.h"
#include <stdlib.h>
#include <stdio.h>
//A very simple loop that tries to run a simulation. 
void main(){
    int numFrames = 1000;
    int8_t *stickPoses = malloc(numFrames*sizeof(int8_t));
    for(int i = 0; i < numFrames; i++){
        stickPoses[i] = 10;
    }
    struct Result *results = malloc(numFrames*sizeof(struct Result));
    struct Player startPoint = init();
    
    int res = runSimulation(stickPoses, results, &startPoint, numFrames);
    for(int i = 0; i < res; i++){
        printf("%d  ", results[i].player.position);
    }
    
    printf("\nres = %d\n", res);
}

