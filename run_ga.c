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

#include "boring_callbacks.c"
#include "mutate_callbacks.c"
#include "mixing_callbacks.c"
#include "util_callbacks.c"

//Randomly mutate one of the controller inputs in father and 
//store the new chromosome in son. 
//You may want to play with this one. I have a couple versions of this. 
void plane_mutate_point_random(population *pop, entity *father, entity *son){
    //Select a frame that occurs before the simulation collides. 
    int collideFrame =((entity_chrom*) father->chromosome[0])->collideFrame;
    int maxFrame = pop->len_chromosomes;
    if(collideFrame > 0) maxFrame = collideFrame;
    int mutate_point = random_int(maxFrame);
    if(!father || !son) die("Null pointer to entity passed.");
    
    //Copy over the data. 
    pop->chromosome_replicate(pop, father, son, 0);
    //And perform the mutation. 
    int nextValue = random_int(2*72 + 1); //(0-144, inclusive)
    nextValue -= 72; //-72 to 72, inclusive.
    int8_t controllerInput = (int8_t) nextValue;
    ((int8_t *)((entity_chrom *)son->chromosome[0])->controllerInputs)[mutate_point] = controllerInput;
}
        


//Given an entity (adam), randomly initialize a set of controller inputs.
//(Before seeding, the inputs may be invalid.)
//You may want to change this to start with a known-good flight path.
boolean plane_seed(population *pop, entity *adam){
    int frameIdx;
    for(frameIdx = 0; frameIdx < pop->len_chromosomes; frameIdx++){
        int nextValue = random_int(2*72 + 1); //(0-144, inclusive)
        nextValue -= 72; //-72 to 72, inclusive.
        int8_t controllerInput = (int8_t) nextValue;
        ((int8_t *)((entity_chrom *)adam ->chromosome[0])->controllerInputs)[frameIdx] = controllerInput;
    }
    return true;
}



//Given two parents, randomly partition the parents inputs
//into the children. 
//This is one you might want to mess with. 
void plane_crossover_allele_mixing(population *pop, 
        entity *father, entity *mother,
        entity *son, entity *daughter){
    if(!father || !mother || !son || !daughter) die("Null entities passed in.");
    int8_t *father_chrom = (int8_t *) ((entity_chrom*)father->chromosome[0])->controllerInputs;
    int8_t *mother_chrom = (int8_t *) ((entity_chrom *)mother->chromosome[0])->controllerInputs;
    int8_t *son_chrom = (int8_t *) ((entity_chrom *)son->chromosome[0])->controllerInputs;
    int8_t *daughter_chrom = (int8_t *) ((entity_chrom *)daughter->chromosome[0])->controllerInputs;
    for(int i = 0; i < pop->len_chromosomes; i++){
        if(random_boolean()){
            son_chrom[i] = father_chrom[i];
            daughter_chrom[i] = mother_chrom[i];
        }else{
            son_chrom[i] = mother_chrom[i];
            daughter_chrom[i] = father_chrom[i];
        }
    }
}
            

#define FAIL_PENALTY 10000
//Given a flight, determine how good it is and store that 
//value in the entity's fitness. 
//This one could definitely be changed to help drive the 
//simulation in the right direction. 
static boolean plane_score(population *pop, entity *entity){
    entity_chrom *mem = (entity_chrom *) entity->chromosome[0];
    int collideFrame = 
            runSimulation(mem->controllerInputs, 
                  mem->results, 
                  mem->startPoint,
                  pop->len_chromosomes);
    double penalty = 0;
    mem->collideFrame = collideFrame;
    if(collideFrame == -1){
        //We didn't even make it to the target x coordinate!
        //We should penalize this strongly!
        //How far was left to go in the x direction? 
        struct Result finalRes =  mem->results[pop->len_chromosomes-1];
        penalty += distance_to_go_x(finalRes.player);
        //You didn't make it. So get a fat penalty for that. 
        penalty += FAIL_PENALTY * 2;
    }else{
        struct Result finalRes =  mem->results[collideFrame];
        if(finalRes.landed){
            //How long did it take you? 
            penalty += collideFrame;
            //And how far beyond the platform were you? 
            //Note that this rewards going extra far. 
            //TODO: Add a weight to this parameter. 
            penalty += distance_to_go_x(finalRes.player);
        } else {
            //We hit the platform but didn't land. 
            penalty += FAIL_PENALTY;
            //Usually this will be near zero, but reward the simulation for 
            //getting further anyway.
            penalty += distance_to_go_x(finalRes.player);
            //And now add in a y-component penalty.
            penalty += distance_to_go_y(finalRes.player)*10;
            //At this point, prefer longer flights but only a bit.
            penalty -= collideFrame / 50; 
        }
    }
    entity->fitness = -1.0*penalty;
    //Since we account for failing to land at all with the penalty function
    //there's no case where we'd need to return false. 
    return true;
}

//This is just a callback that's run at the end of each generation.         
bool plane_generation_hook(int generation, population *pop){
    entity *best;
    best = ga_get_entity_from_rank(pop, 0);
    int collide = ((entity_chrom *) best->chromosome[0])->collideFrame;
    printf("Generation %d, Collided on %d, best fitness %f\n", generation, 
        collide,    
        best->fitness);
    struct Player final;
    if(collide == -1){
        final = ((entity_chrom *)best->chromosome[0])
                ->results[pop->len_chromosomes-1].player;
    }else {
        final = ((entity_chrom *)best->chromosome[0])
                ->results[collide].player;
    }
    printf("\tdist_x: %f dist_y %f\n", distance_to_go_x(final),
        distance_to_go_y(final));

}

int main(int argc, char **argv){
    random_init();
    struct Player initPlayer;
    initPlayer = init();
    
    population *pop = NULL;
    
    //Create a population with 10 individuals, each with 1 chromosome.
    pop = ga_population_new(1000, 1, 800);
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
    pop->select_one = ga_select_one_aggressive;
    pop->select_two = ga_select_two_aggressive;
    pop->mutate = plane_mutate_point_random;
    pop->crossover = plane_crossover_allele_mixing;
    pop->replace=NULL;
    pop->data = &initPlayer;
    ga_population_seed(pop);
    
    //Set population parameters. 
    ga_population_set_parameters(pop, GA_SCHEME_DARWIN,
        GA_ELITISM_PARENTS_DIE,
        0.5,
        0.2,
        0.0);
    ga_evolution(pop, 200);
    ga_extinction(pop);
    return EXIT_SUCCESS;
}

