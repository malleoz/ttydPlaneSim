/*
 * Includes
 */
#include "gaul.h"
#include "run_simulation.h"
#include "plane_physics.h"

typedef struct {
    //This is a shared pointer that is read-only. 
    //It comes from init(). 
    struct Player *startPoint;
    //An array of the results from the simulation. 
    //Changing the controller inputs invalidates this 
    //array until the entity has been evaluated again. 
    struct Result *results;
    //The vector of stick positions. 
    int8_t *controllerInputs;
    //After running the simulation, this is the frame where it
    //hit the target platform, or -1 if it failed. 
    int collideFrame;
} entity_chrom;

typedef struct {
    struct Player *startPoint;
    FILE *outFile;
    double lastScore;
} pop_data;


//Pick two numbers less than maxSize. Store the smaller 
//in start, and store the larger in end. 
void getSplitPoints(int maxSize, int *start, int *end){
    if(maxSize < 3) die("Span not large enough.");
    int point1 = random_int(maxSize);
    int point2 = random_int(maxSize);
    if(point2 == point1) point2 = (point2 + 1) % maxSize;
    if (point2 < point1) {
        *start=point2; *end=point1;
    }else{
        *start=point1; *end=point2;
    }
}
        


#include "boring_callbacks.c"
#include "mutate_callbacks.c"
#include "mixing_callbacks.c"
#include "util_callbacks.c"


int main(int argc, char **argv){
    random_init();
    struct Player initPlayer;
    initPlayer = init();
    pop_data popData;
    popData.startPoint = &initPlayer;
    if(argc < 2){
        die("Need to provide an output file name.");
    }
    popData.outFile = fopen(argv[1], "w");
    popData.lastScore = -1;

    population *pop = NULL;
    
    //Create a population with 10 individuals, each with 1 chromosome.
    pop = ga_population_new(400, 1, 600);
    if(!pop) die("Unable to allocate population.");
    
    pop->chromosome_constructor = plane_chromosome_constructor;
    pop->chromosome_destructor = plane_chromosome_destructor;
    pop->chromosome_replicate = plane_chromosome_replicate;
    pop->chromosome_to_bytes = NULL;
    pop->chromosome_from_bytes = NULL;
    pop->chromosome_to_string = NULL;
    pop->generation_hook = plane_generation_hook;
    pop->iteration_hook = NULL;
    pop->data_destructor = NULL;
    pop->data_ref_incrementor = NULL;
    
    pop->evaluate = plane_score;
    pop->seed = plane_seed;
    pop->adapt = NULL;
    pop->select_one = ga_select_one_randomrank;
    //pop->select_one = ga_select_one_roulette;
    pop->select_two = ga_select_two_randomrank;
    //pop->mutate = plane_mutate_point_random;
    pop->mutate = joint_mutate;
    //pop->crossover = plane_crossover_allele_mixing;
    pop->crossover = plane_crossover_region_scaling;
    pop->replace=NULL;
    pop->data = &popData;
    ga_population_seed(pop);
    
    //Set population parameters. 
    ga_population_set_parameters(pop, GA_SCHEME_DARWIN,
        //GA_ELITISM_PARENTS_SURVIVE,
        GA_ELITISM_ONE_PARENT_SURVIVES,
        0.7,
        0.3,
        0.0);
    //Run a *lot* of cycles. This takes a long time to converge!     
    ga_evolution_threaded(pop, 200000);
    fclose(popData.outFile);
    ga_extinction(pop);
    return EXIT_SUCCESS;
}

