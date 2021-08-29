
#include <stdio.h>
#include <stdlib.h>
#include "run_simulation.h"
#include "plane_physics.h"
#include <getopt.h>
#include <string.h>

void readArguments(int argc, char **argv, FILE **playerDat){
    int c;
    while(1){
        static struct option longopts [] = {
            {"player-dat", required_argument, NULL, 'p'},
            {0,0,0,0}};
        int option_index = 0;
        c = getopt_long_only(argc, argv, "", longopts, &option_index);
        if(c == -1){
            break;
        }
        switch(c){
            case('p'):
                *(playerDat) = fopen(optarg, "r"); break;
        }
    }
}
int readInputs(int8_t **ret_inputs){
    //Sets ret_inputs to be the stick positions read from stdin. 
    //Returns the length of the resulting array. 
    unsigned int stickStrBufSize = 10;
    char * line = malloc(stickStrBufSize * sizeof(char *));
    int inputsBufSize = 10;
    int8_t * inputs = malloc(inputsBufSize * sizeof(int8_t));
    int numCharsRead;
    int inputIdx = 0;
    while((numCharsRead = getline(&line, &stickStrBufSize, stdin)) > 0){
        inputs[inputIdx++] = atoi(line);
        if(inputIdx >= inputsBufSize){
            inputsBufSize *= 2;
            inputs = realloc(inputs, inputsBufSize);
        }
    }
    free(line);
    *(ret_inputs) = malloc(inputIdx * sizeof(int8_t));
    memcpy(*ret_inputs, inputs, inputIdx);
    free(inputs);    

    return inputIdx;
    
}
        
    
    


int main(int argc, char **argv){
    //Process command line arguments.
    FILE *playerDat; 
    struct Player initPlayer; 
    readArguments(argc, argv, &playerDat);
    initPlayer = init(playerDat);
    fclose(playerDat);
    int8_t *inputs;
    int numFrames = readInputs(&inputs);
    /*printf("%d frames\n", numFrames);
    for(int i = 0; i < numFrames; i++){
        printf("%d\n", inputs[i]);
    }*/
    //Allocate the results. 
    struct Result * results = malloc(numFrames * sizeof(struct Result));
    
    //Now run the simulation. 
    int collideFrame = runSimulation(inputs, results, &initPlayer, numFrames);
    //Now print out the flight information. 
    for(int i = 0; i < collideFrame; i++){
        //      i  c  l  s  x  y  z  v  θ
        printf("%d %d %d %d %f %f %f %f %f\n", 
            i, 
            results[i].collided,
            results[i].landed,
            inputs[i],
            results[i].player.position.x,
            results[i].player.position.y,
            results[i].player.position.z,
            results[i].player.baseSpeed,
            results[i].player.wDirectionView);

    }   
    return EXIT_SUCCESS;
}

