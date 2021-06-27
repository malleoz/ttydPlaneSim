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

        

//Randomly select a region of the flight, and then add a random offset to 
//all the controller positions in that region. 
void plane_mutate_region_drift(population *pop, entity *father, entity *son){
    int collideFrame = ((entity_chrom*) father->chromosome[0])->collideFrame;
    if(collideFrame <= 0) collideFrame=pop->len_chromosomes;
    int mutate_start, mutate_end;
    getSplitPoints(collideFrame, &mutate_start, &mutate_end);
    int8_t drift = random_int(50) - 25;
    //That will be in [-5, +4], so remove the zero shift case and 
    //make it from [-5,-1] U [1,5]
    pop->chromosome_replicate(pop, father, son, 0);
    if(drift >= 0) drift++;
    for(int pos = mutate_start; pos <= mutate_end; pos++){
        int8_t prev = ((entity_chrom *)father->chromosome[0])
                        ->controllerInputs[pos];
        int8_t new = prev + drift;
        if(new > 72 || new < -72) continue;
        (((entity_chrom *)son->chromosome[0])
                        ->controllerInputs)[pos] 
            = new;
    }
}

void joint_mutate(population *pop, entity *father, entity *son){
    if(random_int(10) < 3){
        plane_mutate_region_drift(pop, father, son);
    }else{ 
        plane_mutate_point_random(pop, father, son);
    }
}
