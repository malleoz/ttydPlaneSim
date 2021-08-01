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
    //One copy of this struct is given to the population. It contains
    //miscellaneous information that is needed by the callbacks. 
    //Since all the callbacks include a pointer to the population, this is 
    //a convenient place to put that information. 
    
    //The Player that is used as the initial coordinate for every simulation.
    //Note that there is one copy of this for every chromosome, so don't mess 
    //with it unless you really mean it. 
    struct Player *startPoint;
    //Where should the logs from the optimization be written? 
    FILE *outFile;
    //If NULL, then don't seed the algorithm with known good inputs. 
    //If not NULL, then read in the lines from this file during the seeding
    //process and use those lines for the initial controller inputs. 
    FILE *goodControllerInputs;
    //Last time we evaluated the population, what was the best score? 
    //This is used to make sure we only write outputs when the population has 
    //improved. 
    double lastScore;
    //Has one of the chromosomes been seeded with known good inputs? 
    boolean seededGood;
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


int run_ga(FILE * playerDat, FILE *goodControllerInputs, 
        FILE *outputFile, int popSize, int maxFrames, 
        int numGenerations){
    //Mandatory initialization of the rng. 
    //Note that the rng starts at its default seed, so runs with the 
    //same settings should always generate the same results. 
    random_init();
    struct Player initPlayer;
    initPlayer = init(playerDat);
    fclose(playerDat); //We don't need it anymore. 

    pop_data popData;
    popData.startPoint = &initPlayer;
    popData.outFile = outputFile;
    popData.lastScore = -1;
    popData.goodControllerInputs = goodControllerInputs;
    popData.seededGood = false;

    population *pop;
    
    pop = ga_population_new(popSize, 1, maxFrames);
    if(!pop) die("Unable to allocate population.");
    
    pop->chromosome_constructor = plane_chromosome_constructor;
    pop->chromosome_destructor = plane_chromosome_destructor;
    pop->chromosome_replicate = plane_chromosome_replicate;
    pop->chromosome_to_bytes = NULL; //We don't need to serialize the chromosome.
    pop->chromosome_from_bytes = NULL; //We also don't need to deserialize.
    pop->chromosome_to_string = NULL; //We write to string during generation_hook. 
    pop->generation_hook = plane_generation_hook;
    pop->iteration_hook = NULL; //Do nothing at the end of each evaluation.
    pop->data_destructor = NULL; //We aren't using entity->vdata for anything.
    pop->data_ref_incrementor = NULL; //Ditto.
    
    pop->evaluate = plane_score; //How do we evaluate a chromosome? By simulating it! 
    pop->seed = plane_seed; //Read in previous known good inputs if available, otherwise seed randomly. 
    pop->adapt = NULL; //No adaptation - this is a purely Darwinian method. 
    //For selections, prefer high-scoring entities. 
    pop->select_one = ga_select_one_randomrank;
    pop->select_two = ga_select_two_randomrank;
    //See mutate_callbacks.c for this monster.
    pop->mutate = joint_mutate;
    //If there's a bug, it's probably in my crossover function. It's hairy. 
    pop->crossover = plane_crossover_region_scaling;
    pop->replace=NULL; //We don't do replacement. 
    pop->data = &popData; //Store that extra data that will be needed by callbacks.
    ga_population_seed(pop); //Seed the entities. 
    
    //Set population parameters. 
    ga_population_set_parameters(
        pop, // population *
        GA_SCHEME_DARWIN, //ga_class_type  - just leave alone. 
        GA_ELITISM_ONE_PARENT_SURVIVES, //ga_elitism_type 
                    //Maybe consider using GA_ELITISM_PARENTS_SURVIVE, 
                    //but this incurs a performance penalty. 
        0.7,    //double crossover - The frequency of the crossover events.
        0.7,    //double mutation - The frequency of mutation events.
        0.0);   //double migration - Only used when multiple populations
                    //exist, which is not the case here. 


    //If you run into problems that are caused by multithreading
    //(Or you're struggling to debug the multithreaded program 
    //and want something simpler to debug), change this to 
    //ga_evolution(pop, numGenerations);
    ga_evolution_threaded(pop, numGenerations);

    fclose(popData.outFile);
    ga_extinction(pop);
    return EXIT_SUCCESS;
}

