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
    //Now seed with known good inputs, if there are any. 
    pop_data *pd = ((pop_data *)pop->data);
    if(pd->goodControllerInputs != NULL){
        //If we've already seeded one input, no need to seed any more. 
        if(!pd->seededGood){
            pd->seededGood = true;
            char inBuf[10];
            int frameIdx = 0;
            while(fgets(inBuf, 9, pd->goodControllerInputs)){
                ((entity_chrom *) adam->chromosome[0])->controllerInputs[frameIdx] = 
                    (int8_t) atoi(inBuf);
                frameIdx++;
            }
        }
    }
    return true;
}


#define FAIL_PENALTY 5000
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
    double score = 3*FAIL_PENALTY;
    mem->collideFrame = collideFrame;
    if(collideFrame == -1){
        //We didn't even make it to the target x coordinate!
        //We should penalize this strongly!
        //How far was left to go in the x direction? 
        struct Result finalRes =  mem->results[pop->len_chromosomes-1];
        score -= distance_to_go_x(finalRes.player);
        //You didn't make it. So get a fat penalty for that. 
        score -= FAIL_PENALTY * 2;
    }else{
        struct Result finalRes =  mem->results[collideFrame];
        if(finalRes.landed){
            //How long did it take you? 
            score -= collideFrame;
            //And how far beyond the platform were you? 
            //Note that this rewards going extra far. 
            //TODO: Add a weight to this parameter. 
            score -= distance_to_go_y(finalRes.player) / 50.;
            score -= distance_to_go_x(finalRes.player) / 10.;
        } else {
            //We hit the platform but didn't land. 
            score -= FAIL_PENALTY;
            //And now add in a y-component penalty.
            score -= distance_to_go_y(finalRes.player)*10;
        }
    }
    entity->fitness = score;
    //Since we account for failing to land at all with the penalty function
    //there's no case where we'd need to return false. 
    return true;
}

//This is just a callback that's run at the end of each generation.         
bool plane_generation_hook(int generation, population *pop){
    entity *best;
    best = ga_get_entity_from_rank(pop, 0);
    fprintf(stderr, "Generation %d\r", generation);
    if(((pop_data *)pop->data)->lastScore != best->fitness){
        ((pop_data *)pop->data)->lastScore = best->fitness;

        int collide = ((entity_chrom *) best->chromosome[0])->collideFrame;
        printf("Generation %d, Collided on %d, best fitness %f\n", generation, 
            collide,    
            best->fitness);
        struct Player final;
        if (collide == -1) collide = pop->len_chromosomes-1;
        final = ((entity_chrom *)best->chromosome[0])
                ->results[collide].player;
        printf("\tdist_x: %f dist_y %f\n", distance_to_go_x(final),
            distance_to_go_y(final));
        FILE *outFile = ((pop_data *)pop->data)->outFile;
        fprintf(outFile, "%d,%d,%f,%f,%f", generation, collide, best->fitness,
                distance_to_go_x(final), distance_to_go_y(final));
        for(int i = 0; i <= (collide); i++){
            struct Player curPlayer = ((entity_chrom *)best->chromosome[0])->results[i].player;
            fprintf(outFile, ",%d,%f,%f,%f,%f", 
                ((entity_chrom *)best->chromosome[0])->controllerInputs[i],
                curPlayer.position.x,
                curPlayer.position.y,
                curPlayer.position.z,
                curPlayer.baseSpeed);
        }
        fprintf(outFile, "\n", generation);
    }
}
