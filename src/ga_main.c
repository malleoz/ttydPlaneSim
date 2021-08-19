#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "run_ga.h"

void readArguments(int argc, char **argv, FILE **playerDat, FILE **goodControllerInputs, FILE **outputFile, int *popSize, int *maxFrames, int *numGenerations){
    int c;
    while(1){
        static struct option longopts [] = {
            {"player-dat", required_argument, NULL, 'p'},
            {"good-inputs", required_argument, NULL, 'g'},
            {"output-file", required_argument, NULL, 'o'},
            {"pop-size", required_argument, NULL, 's'},
            {"max-frames", required_argument, NULL, 'f'},
            {"num-generations", required_argument, NULL, 'n'},
            {0, 0, 0, 0}};
        int option_index = 0;
        c = getopt_long_only(argc, argv, "", longopts, &option_index);
        if(c == -1){
            break;
        }
        switch(c){
            case 'p':
                *(playerDat) = fopen(optarg, "r"); break;
            case 'g':
                *(goodControllerInputs) = fopen(optarg, "r"); break;
            case 'o':
                *(outputFile) = fopen(optarg, "w"); break;
            case 's':
                *popSize = atoi(optarg); break;
            case 'f':
                *maxFrames = atoi(optarg); break;
            case 'n':
                *numGenerations = atoi(optarg);  break;
            default:
                printf("Unrecognized option.\n");
                exit(-1);
        }
    }
}
    


int main(int argc, char **argv){
    FILE *playerDat = NULL;
    FILE *goodControllerInputs = NULL; 
    FILE *outputFile = NULL;
    int popSize = -1;
    int maxFrames = -1;
    int numGenerations = -1;

    readArguments(argc, argv, &playerDat, &goodControllerInputs, &outputFile, &popSize, &maxFrames, &numGenerations);
    return run_ga(playerDat, goodControllerInputs, outputFile,
        popSize, maxFrames, numGenerations);
}
