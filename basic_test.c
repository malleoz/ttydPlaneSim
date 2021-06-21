/**********************************************************************
  struggle.c
 **********************************************************************

  struggle - Test/example program for GAUL.
  Copyright ©2001-2004, Stewart Adcock <stewart@linux-domain.com>
  All rights reserved.

  The latest version of this program should be available at:
  http://gaul.sourceforge.net/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.  Alternatively, if your project
  is incompatible with the GPL, I will probably agree to requests
  for permission to use the terms of any other license.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY WHATSOEVER.

  A full copy of the GNU General Public License should be in the file
  "COPYING" provided with this distribution; if not, see:
  http://www.gnu.org/

 **********************************************************************

  Synopsis:	Test/example program for GAUL.

		This program is fairly lean, showing how little
		application code is needed when using GAUL.

		This program aims to generate the final sentence from
		Chapter 3 of Darwin's "The Origin of Species",
		entitled "Struggle for Existence".

		This example is explained in docs/html/tutorial/simple.html

 **********************************************************************/

/*
 * Includes
 */
#include "gaul.h"
#include "run_simulation.h"
#include "plane_physics.h"
/*
 * The solution string.
 */
static char *target_text="When we reflect on this struggle, we may console ourselves with the full belief, that the war of nature is not incessant, that no fear is felt, that death is generally prompt, and that the vigorous, the healthy, and the happy survive and multiply.";

typedef struct {
    struct Player *startPoint;
    struct Result *results;
    int8_t *controllerInputs;
    int collideFrame;
} entity_chrom;


//Initialize an entity.
//This will add a custom memory buffer for temporary storage, 
//and it will allocate (but not initialize) the chromosome. 
bool plane_chromosome_constructor(population *pop, entity *embryo){
    int frameIdx; 
    if(!pop) die("No pointer to population!");
    if(!embryo) die("No pointer to adam element.");
    if(embryo->chromosome != NULL) die("This embryo was already initalized!");
    int numFrames = pop->len_chromosomes;
    entity_chrom *mem = malloc(sizeof(entity_chrom));
    if(!mem) die("Failed to allocate memory.");
    
    struct Result *resultArray = malloc(numFrames * sizeof(struct Result));
    if(!resultArray) die("Failed to allocate result array.");
    mem->results = resultArray;
    
    int8_t *inputs = malloc(numFrames * sizeof(int8_t));
    if(!resultArray) die("Failed to allocate result chromosome.");
    mem->controllerInputs = inputs;
    
    mem->startPoint = (struct Player *) pop->data;    

    embryo->chromosome = malloc(sizeof(entity_chrom *));
    if(!embryo->chromosome) die("Could not allocate chromosomes array.");
    embryo->chromosome[0] = mem;
 
    return true;
}

void plane_mutate_point_random(population *pop, entity *father, entity *son){
    printf("Entered mutator, line %d.\n", __LINE__);
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
        

void plane_chromosome_replicate(const population *pop, 
        entity *src, entity *dest, const int chromosomeid){
    printf("Entered chrom replicate, line %d.\n", __LINE__);
    if(!pop) die("No population passed in.");
    if(!src || !dest) die("Null entity passed.");
    if(!src->chromosome || !dest->chromosome) die("Entity passed with no chromosomes.");
    if(!src->chromosome[0] || !dest->chromosome[0]) die("Empty chromosomes.");
    entity_chrom *srcChrom = (entity_chrom *) src->chromosome[0];
    entity_chrom *destChrom = (entity_chrom *) dest->chromosome[0];
    for(int i=0; i<pop->len_chromosomes; i++){
        destChrom->controllerInputs[i] = srcChrom->controllerInputs[i];
        destChrom->results[i] = srcChrom->results[i];
        destChrom->collideFrame = srcChrom->collideFrame;
    }
}


void plane_chromosome_destructor(population *pop, entity *corpse){
    printf("Entered chrom destructor, line %d.\n", __LINE__);
    if(!pop) die("Null pointer to population passed.");
    if(!corpse) die("Null pointer to entity passed.");
    if(corpse->chromosome==NULL) die("Chromosome already deallocated.");
    //Note! Do not free startPoint, since that's a global pointer. 
    free(((entity_chrom *)corpse->chromosome[0])->results);
    free(((entity_chrom *) corpse->chromosome[0])->controllerInputs);
    free(corpse->chromosome[0]);
    free(corpse->chromosome);
}

//Given an entity (adam), randomly initialize a set of controller inputs.
//(Before seeding, the inputs may be invalid.)
boolean plane_seed(population *pop, entity *adam){
    printf("Entered seed, line %d and adam %d\n", __LINE__, adam);
    int frameIdx;
    for(frameIdx = 0; frameIdx < pop->len_chromosomes; frameIdx++){
        int nextValue = random_int(2*72 + 1); //(0-144, inclusive)
        nextValue -= 72; //-72 to 72, inclusive.
        int8_t controllerInput = (int8_t) nextValue;
        ((int8_t *)((entity_chrom *)adam ->chromosome[0])->controllerInputs)[frameIdx] = controllerInput;
    }
    return true;
}


/**********************************************************************
  struggle_score()
  synopsis:	Score solution.
  parameters:
  return:
  updated:	16/06/01
 **********************************************************************/

static boolean struggle_score(population *pop, entity *entity)
  {
  int		k;		/* Loop variable over all alleles. */

  entity->fitness = 0.0;

  /* Loop over alleles in chromosome. */
  for (k = 0; k < pop->len_chromosomes; k++)
    {
    if ( ((char *)entity->chromosome[0])[k] == target_text[k])
      entity->fitness+=1.0;
    /*
     * Component to smooth function, which helps a lot in this case:
     * Comment it out if you like.
     */
    entity->fitness += (127.0-abs((int)(((char *)entity->chromosome[0])[k]-target_text[k])))/50.0;
    }

  return TRUE;
  }


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
static boolean plane_score(population *pop, entity *entity){
    printf("Entered score calculation, line %d.\n", __LINE__);
    printf("Entity has address %d\n", entity);
    entity_chrom *mem = (entity_chrom *) entity->chromosome[0];
    printf("Entity values: mem address %d\n", mem);
    printf("Entity values: results address %d\n", mem->results);
    printf("Chromosome length: %d\n", pop->len_chromosomes);
    int collideFrame = 
            runSimulation(mem->controllerInputs, 
                  mem->results, 
                  mem->startPoint,
                  pop->len_chromosomes);
    double penalty = 0;
    mem->collideFrame = collideFrame;
    printf("Entered main, line %d.\n", __LINE__);
    if(collideFrame == -1){
        //We didn't even make it to the target x coordinate!
        //We should penalize this strongly!
        penalty += pop->len_chromosomes; //It took you a long time.
        //How far was left to go in the x direction? 
        struct Result finalRes =  mem->results[pop->len_chromosomes-1];
        penalty += distance_to_go_x(finalRes.player);
        //You didn't make it. So get a fat penalty for that. 
        penalty += FAIL_PENALTY;
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
            penalty += collideFrame;
            //Usually this will be near zero, but reward the simulation for 
            //getting further anyway. 
            penalty += distance_to_go_x(finalRes.player);
            //And now add in a y-component penalty.
            penalty += distance_to_go_y(finalRes.player);
        }
    }
    entity->fitness = -1.0*penalty;
    printf("Calculated fitness, it's %f\n", entity->fitness);
    return true;
}
        
     

int main(int argc, char **argv){
    printf("Entered main, line %d.\n", __LINE__);
    random_init();
    struct Player initPlayer;
    initPlayer = init();
    
    population *pop = NULL;
    //log_set_level(LOG_NORMAL);
    
    //Create a population with 10 individuals, each with 1 chromosome.
    //(The third argument is ignored.)
    printf("Entered main, line %d.\n", __LINE__);
    pop = ga_population_new(10, 1, 50);
    if(!pop) die("Unable to allocate population.");
    
    printf("Entered main, line %d.\n", __LINE__);
    pop->chromosome_constructor = plane_chromosome_constructor;
    pop->chromosome_destructor = plane_chromosome_destructor;
    pop->chromosome_replicate = plane_chromosome_replicate;
    pop->chromosome_to_bytes = NULL;
    pop->chromosome_from_bytes = NULL;
    pop->chromosome_to_string = NULL;
    pop->generation_hook = NULL;
    pop->iteration_hook = NULL;
    pop->data_destructor = NULL;
    pop->data_ref_incrementor = NULL;
    
    printf("Entered main, line %d.\n", __LINE__);
    pop->evaluate = plane_score;
    pop->seed = plane_seed;
    pop->adapt = NULL;
    pop->select_one = ga_select_one_random;
    pop->select_two = ga_select_two_random;
    pop->mutate = plane_mutate_point_random;
    pop->crossover = plane_crossover_allele_mixing;
    pop->replace=NULL;
    pop->data = &initPlayer;
    printf("Entered main, line %d.\n", __LINE__);
    ga_population_seed(pop);
    
    printf("Population seeded, line %d.\n", __LINE__);
    //Set population parameters. 
    ga_population_set_parameters(pop, GA_SCHEME_DARWIN,
        GA_ELITISM_PARENTS_DIE,
        0.2,
        0.2,
        0.0);
    printf("Begin evolution, line %d.\n", __LINE__);
    ga_evolution(pop, 2);
    printf("End evolution, line %d.\n", __LINE__);
    ga_extinction(pop);
    printf("Entered main, line %d.\n", __LINE__);
    return EXIT_SUCCESS;
}



/**********************************************************************
  main()
  synopsis:	Erm?
  parameters:
  return:
  updated:	19 Aug 2002
 **********************************************************************/

int otherMain(int argc, char **argv)
  {
  int		i;			/* Loop over runs. */
  population	*pop=NULL;		/* Population of solutions. */
  char		*beststring=NULL;	/* Human readable form of best solution. */
  size_t	beststrlen=0;		/* Length of beststring. */

  for (i=0; i<50; i++)
    {
    random_seed(i);

    pop = ga_genesis_char(
       120,			/* const int              population_size */
       1,			/* const int              num_chromo */
       (int) strlen(target_text),	/* const int              len_chromo */
       NULL,		 	/* GAgeneration_hook      generation_hook */
       NULL,			/* GAiteration_hook       iteration_hook */
       NULL,			/* GAdata_destructor      data_destructor */
       NULL,			/* GAdata_ref_incrementor data_ref_incrementor */
       struggle_score,		/* GAevaluate             evaluate */
       ga_seed_printable_random,	/* GAseed                 seed */
       NULL,			/* GAadapt                adapt */
       ga_select_one_sus,	/* GAselect_one           select_one */
       ga_select_two_sus,	/* GAselect_two           select_two */
       ga_mutate_printable_singlepoint_drift,	/* GAmutate               mutate */
       ga_crossover_char_allele_mixing,	/* GAcrossover            crossover */
       NULL,			/* GAreplace		replace */
       NULL			/* vpointer		User data */
            );

    ga_population_set_parameters(
       pop,			/* population      *pop */
       GA_SCHEME_DARWIN,	/* const ga_scheme_type     scheme */
       GA_ELITISM_PARENTS_DIE,	/* const ga_elitism_type   elitism */
       0.9,			/* double  crossover */
       0.2,			/* double  mutation */
       0.0              	/* double  migration */
                              );

    ga_evolution(
       pop,			/* population      *pop */
       500			/* const int       max_generations */
              );

    printf( "The solution with seed = %d was:\n", i );
    beststring = ga_chromosome_char_to_string(pop, ga_get_entity_from_rank(pop,0), beststring, &beststrlen);
    printf("%s\n", beststring);
    printf( "With score = %f\n", ga_entity_get_fitness(ga_get_entity_from_rank(pop,0)) );

    ga_extinction(pop);
    }

  s_free(beststring);

  exit(EXIT_SUCCESS);
  }


