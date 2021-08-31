
void validateNumber(int value, int min, int max, char *name){
    if(value < min){
        printf("Value underflow: %d < %d for %s", value, min, name);
        die("Value underflow.");
    }else if(value > max){
        printf("Value overflow: %d > %d for %s", value, max, name);
        die("Value overflow.");
    }
}
#define MIN_FLIGHT_LENGTH 25
void getCrossPoints(population *pop, entity_chrom *parent, int *cross_start, int *cross_end){
    int len_chrom = parent->collideFrame;
    if(len_chrom < MIN_FLIGHT_LENGTH || len_chrom >= pop->len_chromosomes){
        len_chrom = pop->len_chromosomes;
    }
    getSplitPoints(len_chrom, cross_start, cross_end);
    validateNumber(len_chrom, MIN_FLIGHT_LENGTH, pop->len_chromosomes, "len chrom.");
    if(*cross_end - *cross_start > 40){
        int new_end = (*cross_start + (*cross_end) * 3) / 4;
        *cross_start = (*cross_start * 3 + *cross_end) / 4;
        *cross_end = new_end;
    }
    if(*cross_end < *cross_start + 3){
        if(*cross_start > 10){
            *cross_start -= 10;
        }else{
            *cross_end += 10;
        }
    }
    validateNumber(*cross_start, 0, *cross_end, "Cross start.");
    validateNumber(*cross_end, *cross_start, len_chrom, "Cross end");
}

void copyStretch(int8_t * parent, int8_t *child, int parent_start, int parent_stop, int child_start, int child_stop){
    int len_parent = parent_stop - parent_start;
    int len_child = child_stop - child_start;
    //Now copy in the split to the daughter from the father
    for(int pos =child_start; pos < child_stop; pos++){
        int sourceIndex = parent_start + 
            ((pos - child_start) * len_parent) / (len_child);
        float source_float = parent_start + 
            ((pos - child_start) * len_parent*1.0) / (len_child*1.0);
        float offset = source_float - sourceIndex;

        int8_t sourceValue = parent[sourceIndex];
        int8_t nextValue = parent[sourceIndex+1];
        float interpValue = nextValue * offset + sourceValue * (1-offset);
        validateNumber((int8_t) interpValue, -72, 72, "child interp value.");
        validateNumber(pos, child_start, child_stop, "Daughter write pos");
        child[pos] = (int8_t) interpValue;
    }
}
    

//Choose one random region in each of the parents 
//(The regions will not be of the same size)
//Then swap those regions, stretching or squooshing the 
//regions to fit in the gap in the other sequence.
void plane_crossover_region_scaling(population *pop,             
        entity *father, entity *mother,
        entity *son, entity *daughter){
    if(!father || !mother || !son || !daughter) die("Null entities passed in.");
    int8_t *father_chrom = (int8_t *) ((entity_chrom*)father->chromosome[0])->controllerInputs;
    int8_t *mother_chrom = (int8_t *) ((entity_chrom *)mother->chromosome[0])->controllerInputs;
    int8_t *son_chrom = (int8_t *) ((entity_chrom *)son->chromosome[0])->controllerInputs;
    int8_t *daughter_chrom = (int8_t *) ((entity_chrom *)daughter->chromosome[0])->controllerInputs;
    
    //Start by copying over the inputs. 
    for(int pos = 0; pos < pop->len_chromosomes; pos++){
        son_chrom[pos] = father_chrom[pos];
        daughter_chrom[pos] = mother_chrom[pos];
    }

    int cross_start_mother, cross_end_mother, cross_start_father, cross_end_father;
    getCrossPoints(pop, (entity_chrom *) father->chromosome[0], &cross_start_father, &cross_end_father);
    getCrossPoints(pop, (entity_chrom *) mother->chromosome[0], &cross_start_mother, &cross_end_mother);


    //printf("Split %d %d %d %d %d %d\n", len_father_chrom, cross_start_father, cross_end_father, len_mother_chrom, cross_start_mother, cross_end_mother);

    copyStretch(father_chrom, daughter_chrom, cross_start_father, cross_end_father, cross_start_mother, cross_end_mother);
    copyStretch(mother_chrom, son_chrom, cross_start_mother, cross_end_mother, cross_start_father, cross_end_father);
    
}

