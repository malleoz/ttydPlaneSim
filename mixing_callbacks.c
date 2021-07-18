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


void plane_crossover_doublepoint(population *pop,             
        entity *father, entity *mother,
        entity *son, entity *daughter){
    if(!father || !mother || !son || !daughter) die("Null entities passed in.");
    int8_t *father_chrom = (int8_t *) ((entity_chrom*)father->chromosome[0])->controllerInputs;
    int8_t *mother_chrom = (int8_t *) ((entity_chrom *)mother->chromosome[0])->controllerInputs;
    int8_t *son_chrom = (int8_t *) ((entity_chrom *)son->chromosome[0])->controllerInputs;
    int8_t *daughter_chrom = (int8_t *) ((entity_chrom *)daughter->chromosome[0])->controllerInputs;
    int cross_start, cross_end;
    getSplitPoints(pop->len_chromosomes, &cross_start, &cross_end);
    for(int pos = 0; pos < pop->len_chromosomes; pos++){
        if(pos < cross_start || pos > cross_end){
            son_chrom[pos] = father_chrom[pos];
            daughter_chrom[pos] = mother_chrom[pos];
        }else{
            son_chrom[pos] = mother_chrom[pos];
            daughter_chrom[pos] = father_chrom[pos];
        }
    }
}

void validateNumber(int value, int min, int max, char *name){
    if(value < min){
        printf("Value underflow: %d < %d for %s", value, min, name);
        die("Value underflow.");
    }else if(value > max){
        printf("Value overflow: %d > %d for %s", value, max, name);
        die("Value overflow.");
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

    int cross_start_mother, cross_end_mother, cross_start_father, cross_end_father, len_mother, len_father, sourceIndex, sourceValue, nextValue;
    float source_float, offset, interpValue;
    int len_father_chrom = ((entity_chrom *) father->chromosome[0])->collideFrame;
    int len_mother_chrom = ((entity_chrom *) mother->chromosome[0])->collideFrame;
    if(len_father_chrom < 3 || len_father_chrom >= pop->len_chromosomes) 
        len_father_chrom = pop->len_chromosomes;
    if(len_mother_chrom < 3 || len_mother_chrom >= pop->len_chromosomes) 
        len_mother_chrom = pop->len_chromosomes;

    getSplitPoints(len_father_chrom, &cross_start_father, &cross_end_father);
    validateNumber(len_father_chrom, 3, pop->len_chromosomes, "len father chrom.");
    if(cross_end_father > cross_start_father + 40){
        int new_cross_end_father = (cross_start_father + cross_end_father * 3) / 4;
        cross_start_father = (cross_start_father * 3 + cross_end_father) / 4;
        cross_end_father = new_cross_end_father;
    }
    if(cross_end_mother > cross_start_mother + 40){
        int new_cross_end_mother = (cross_start_mother + cross_end_mother * 3) / 4;
        cross_start_mother = (cross_start_mother * 3 + cross_end_mother) / 4;
        cross_end_mother = new_cross_end_mother;
    }
    
    if(cross_end_father < cross_start_father + 3){
        if(cross_start_father > 20){
            cross_start_father -= 10;
        }else{
            cross_end_father += 10;
        }
    }

    if(cross_end_mother < cross_start_mother + 3){
        if(cross_start_mother > 20){
            cross_start_mother -= 10;
        }else{
            cross_end_mother += 10;
        }
    }

    validateNumber(cross_start_father, 0, cross_end_father, "Cross start father");
    validateNumber(cross_end_father, cross_start_father, len_father_chrom, "cross end father");


    getSplitPoints(len_mother_chrom, &cross_start_mother, &cross_end_mother);
    validateNumber(len_mother_chrom, 3, pop->len_chromosomes, "len mother chrom.");
    validateNumber(cross_start_mother, 0, cross_end_mother, "Cross start mother");
    validateNumber(cross_end_mother, cross_start_mother, len_mother_chrom, "cross end mother");
    //printf("Split %d %d %d %d %d %d\n", len_father_chrom, cross_start_father, cross_end_father, len_mother_chrom, cross_start_mother, cross_end_mother);
    len_mother = cross_end_mother - cross_start_mother;
    len_father = cross_end_father - cross_start_father;
    validateNumber(len_father, 1, len_father_chrom, "Father length");
    validateNumber(len_mother, 1, len_mother_chrom, "Mother length");
    //Now copy in the split to the daughter from the father
    for(int pos = cross_start_mother; pos < cross_end_mother; pos++){
        sourceIndex = cross_start_father + 
            ((pos - cross_start_mother) * len_father) / (len_mother);
        source_float = cross_start_father + 
            ((pos - cross_start_mother) * len_father*1.0) / (len_mother*1.0);
        offset = source_float - sourceIndex;

        sourceValue = mother_chrom[sourceIndex];
        nextValue = mother_chrom[sourceIndex+1];
        interpValue = nextValue * offset + sourceValue * (1-offset);
        //if(sourceIndex > pop->len_chromosomes) 
        //printf("%d %d %d %d %d %d %d\n", sourceIndex, cross_start_father, pos, cross_start_mother, len_father, len_mother, (int8_t) interpValue);
        /*interpValue = sourceValue; */
        validateNumber((int8_t) interpValue, -72, 72, "daughter interp value.");
        validateNumber(pos, cross_start_mother, cross_end_mother, "Daughter write pos");
        daughter_chrom[pos] = (int8_t) interpValue;//father_chrom[sourceIndex];
    }
    
    for(int pos = cross_start_father; pos < cross_end_father; pos++){
        sourceIndex = cross_start_mother + ((pos - cross_start_father) * len_mother) / len_father;
        if(sourceIndex > pop->len_chromosomes) printf("%d %d %d %d %d %d\n", sourceIndex, cross_start_father, pos, cross_start_mother, len_father, len_mother);
        source_float = cross_start_mother + 
            ((pos - cross_start_father) * len_mother * 1.0) / (len_father*1.0);
        offset = source_float - sourceIndex;
        sourceValue = father_chrom[sourceIndex];
        nextValue = father_chrom[sourceIndex + 1];
        interpValue = nextValue * offset + sourceValue *(1-offset);
        validateNumber((int8_t) interpValue, -72, 72, "son interp value.");
        validateNumber(pos, cross_start_father, cross_end_father, "Son write pos");
        son_chrom[pos] = (int8_t) interpValue;// mother_chrom[sourceIndex];
    }
    
}

