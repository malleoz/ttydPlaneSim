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
            
