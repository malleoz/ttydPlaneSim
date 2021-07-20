//Randomly mutate one of the controller inputs in father and 
//store the new chromosome in son. 
//You may want to play with this one. I have a couple versions of this. 
void plane_mutate_point_random(population *pop, entity *father, entity *son){
    //Select a frame that occurs before the simulation collides. 
    int collideFrame =((entity_chrom*) father->chromosome[0])->collideFrame;
    int maxFrame = pop->len_chromosomes;
    if(collideFrame > 0) maxFrame = collideFrame;
    if(!father || !son) die("Null pointer to entity passed.");
    
    //Copy over the data. 
    pop->chromosome_replicate(pop, father, son, 0);
    //And perform the mutation. 
    //for(int i = 0; i < random_int(5)+1; i++){
        int mutate_point = random_int(maxFrame);
        int nextValue = random_int(2*72 + 1); //(0-144, inclusive)
        nextValue -= 72; //-72 to 72, inclusive.
        int8_t controllerInput = (int8_t) nextValue;
        ((int8_t *)((entity_chrom *)son->chromosome[0])->controllerInputs)[mutate_point] = controllerInput;
    //}
}

        

//Randomly select a region of the flight, and then add a random offset to 
//all the controller positions in that region. 
void plane_mutate_region_drift(population *pop, entity *father, entity *son){
    int collideFrame = ((entity_chrom*) father->chromosome[0])->collideFrame;
    if(collideFrame <= 0) collideFrame=pop->len_chromosomes;
    int mutate_start, mutate_end;
    getSplitPoints(collideFrame, &mutate_start, &mutate_end);
    if(mutate_end > mutate_start + 50){
        mutate_end = mutate_start + 10;
    }
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


//Choose a random region in the flight, and set it to a random value. 
void plane_mutate_region_set(population *pop, entity *father, entity *son){
    int collideFrame = ((entity_chrom*) father->chromosome[0])->collideFrame;
    if(collideFrame <= 0) collideFrame=pop->len_chromosomes;
    int mutate_start, mutate_end;
    getSplitPoints(collideFrame, &mutate_start, &mutate_end);
    if(mutate_end - mutate_start > 80){
        mutate_end = mutate_start + 10;
    }
    int nextValue = random_int(2*72 + 1); //(0-144, inclusive)
    nextValue -= 72; //-72 to 72, inclusive.
    pop->chromosome_replicate(pop, father, son, 0);
    for(int pos = mutate_start; pos <= mutate_end; pos++){
        (((entity_chrom *)son->chromosome[0])
                        ->controllerInputs)[pos] 
            = nextValue;
    }
}

int compInts(const void *elem1, const void *elem2){
    int f = *((int *)elem1);
    int s = *((int *)elem2);
    if(f > s) return 1;
    if(f < s) return -1;
    return 0;
}
//Duplicate or delete some random frames from the flight. 
void plane_mutate_dilate(population *pop, entity *father, entity *son){
    //TRUE means we're deleting. 
    //FALSE means we're inserting.
    int indel_mode = random_boolean();
    int num_indels = random_int(10);
    int indelPoses[10];
    for(int i = 0; i < num_indels; i++){
        indelPoses[i] = random_int(pop->len_chromosomes-1);
    }
    qsort(indelPoses, num_indels, sizeof(int), compInts);
    int curIndelPos = 0;
    int writeHead = 0;
    int8_t * fatherInputs = ((entity_chrom *)father->chromosome[0])->controllerInputs;
    int8_t * sonInputs = ((entity_chrom *)son->chromosome[0])->controllerInputs;
    for(int i = 0; i < pop->len_chromosomes; i++){  
        if(i == indelPoses[curIndelPos]){
            if(indel_mode){//Just move to the next input in the father.
                continue;
            }else{//Duplicate the father at this position. 
                int writeVal;
                if(i < pop->len_chromosomes - 1){
                    writeVal = ((int) fatherInputs[i] + (int) fatherInputs[i+1]) / 2;
                }else{
                    writeVal = fatherInputs[i];
                }
                sonInputs[writeHead] = writeVal;
                if(++writeHead == pop->len_chromosomes) break; //We finished! 
            }
            curIndelPos++;
        }
        sonInputs[writeHead++] = fatherInputs[i];
        if(writeHead == pop->len_chromosomes) break; //We finished! 
    }
    if(indel_mode){
        //We were deleting, so fill in the inputs in the son that were omitted.
        //I do this by just copying over father inputs. Simplistic,
        //but most of these inputs will be from after the collision. 
        for(int i = writeHead; i < pop->len_chromosomes; i++){
            sonInputs[i] = fatherInputs[i];
        }
    }
}
                
    
    
//A combination of all the above methods, applied several times. 
void joint_mutate(population *pop, entity *father, entity *son){
    int mode = random_int(15);
    if(mode < 5){
        plane_mutate_region_drift(pop, father, son);
    }else if (mode < 5){
        plane_mutate_region_set(pop, father, son);
    }else if (mode < 10){
        plane_mutate_dilate(pop, father, son);
    }else { 
        plane_mutate_point_random(pop, father, son);
    }
}
