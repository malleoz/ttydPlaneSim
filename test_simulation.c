#include "run_simulation.h"
#include "plane_physics.h"
#include <stdlib.h>
#include <stdio.h>
//A very simple loop that tries to run a simulation. 
int main(){
    int numFrames = 1000;
    int8_t *stickPoses = malloc(numFrames*sizeof(int8_t));
    for(int i = 0; i < numFrames; i++){
        stickPoses[i] = 10;
    }
    printf("Stick positions allocated.\n");
    struct Result *results = malloc(numFrames*sizeof(struct Result));
    printf("Results allocated!\n");
    struct Player startPoint = init();
    printf("Starting player allocated.\n");
    int res = runSimulation(stickPoses, results, &startPoint, numFrames);
    int lastFrame = res > 0? res+1 : numFrames;
    for(int i = 0; i < lastFrame; i++){
        struct Vector rv;
        rv = results[i].player.position;
        char land = results[i].landed?'L':'l';
        char collide = results[i].collided?'C':'c';
        printf("%10d     %10f     %10f     %10f %c%c\n",i, rv.x, rv.y, rv.z, land, collide);
    }
    
    printf("\nres = %d\n", res);
}

