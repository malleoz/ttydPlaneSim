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

/*
 * The solution string.
 */
static char *target_text="When we reflect on this struggle, we may console ourselves with the full belief, that the war of nature is not incessant, that no fear is felt, that death is generally prompt, and that the vigorous, the healthy, and the happy survive and multiply.";

typedef struct {
    struct Player startPoint;
    struct Result *results;
} entity_mem;


//Initialize an entity.
//This will add a custom memory buffer for temporary storage, 
//and it will allocate (but not initialize) the chromosome. 
bool plane_chromosome_constructor(population *pop, entity *embryo){
    int frameIdx; 
    if(!pop) die("No pointer to population!");
    if(!embryo) die("No pointer to adam element.");
    if(embryo->chromosome != NULL) die("This embryo was already initalized!");
    int numFrames = pop->len_chromosomes;
    entity_mem *mem = malloc(sizeof(entity_mem));
    if(!mem) die("Failed to allocate memory.");
    embryo->data = (void *)mem;
    
    struct Result *resultArray = malloc(numFrames * sizeof(struct Result));
    if(!resultArray) die("Failed to allocate result array.");
    mem->results = resultArray;
    
    int8_t *chrom = malloc(numFrames * sizeof(int8_t));
    if(!resultArray) die("Failed to allocate result chromosome.");
    embryo->chromosome = malloc(sizeof(int8_t *));
    if(!embryo->chromosome) die("Could not allocate chromosomes array.");
    embryo->chromosome[0] = chrom;
 
    return true;
}

void plane_chromosome_replicate(const population *pop, 
        entity *src, entity *dest, const int chromosomeid){
    if(!pop) die("No population passed in.");
    if(!src || !dest) die("Null entity passed.");
    if(!src->chromosome || !dest->chromosome) die("Entity passed with no chromosomes.");
    if(!src->chromosome[0] || !dest->chromosome[0]) die("Empty chromosomes.");
    for(int i=0; i<pop->len_chromosomes; i++){
        dest->chromosome[0][i] = src->chromosome[0][i];
    }
}


void plane_chromosome_destructor(population *pop, entity *corpse){
    if(!pop) die("Null pointer to population passed.");
    if(!corpse) die("Null pointer to entity passed.");
    if(corpse->chromosome==NULL) die("Chromosome already deallocated.");
    if(corpse->data == NULL) die("Entity has null data pointer.");
    //need to free entity_mem, the result array, the chromosome, 
    //and the chromosome array.
    free(((entity_mem *) corpse->data)->results);
    free(corpse->data);
    free(corpse->chromosome[0]);
    free(corpse->chromosome);
}

//Given an entity (adam), randomly initialize a set of controller inputs.
//(Before seeding, the inputs may be invalid.)
boolean plane_seed(population *pop, entity *adam){
    int frameIdx;
    for(frameIdx = 0; frameIdx < pop->len_chromosomes; frameIdx++){
        int nextValue = random_int(2*72 + 1); //(0-144, inclusive)
        nextValue -= 72; //-72 to 72, inclusive.
        int8_t controllerInput = (int8_t) nextValue;
        ((int8_t *)adam ->chromosome[0])[frameIdx] = controllerInput;
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

static boolean plane_score(population *pop, entity *entity){
    entity_mem *mem = (entity_mem *) entity->mem;
    int collideFrame = 
            runSimulation(entity->chromosome[0], 
                  mem->results, 
                  &(mem->startPoint),
                  pop->len_chromosomes);
    double penalty = 0
    if(collideFrame == -1){
        //We didn't even make it to the target x coordinate!
        //We should penalize this strongly!
        penalty += pop->len_chromosomes; //It took you a long time.
        //How far was left to go in the x direction? 
        penalty += 
        
     




/**********************************************************************
  main()
  synopsis:	Erm?
  parameters:
  return:
  updated:	19 Aug 2002
 **********************************************************************/

int main(int argc, char **argv)
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


