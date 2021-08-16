
void print_entity(population *pop, entity *entity){
    int i;
    entity_chrom *chrom = (entity_chrom *)entity->chromosome[0];
    printf("%d\n", chrom->name);
    for(i = 0; i < pop->len_chromosomes; i++){
        printf("%d ", chrom->controllerInputs[i]);
    }
    printf("\n");
}

//Initialize an entity.
//Allocate (but not initialize) the chromosome's result array and
//stick positions array.
//Copy the data pointer from pop into the embryo's startPoint. 
//You shouldn't need to change this unless you change the 
//entity_chrom struct. 
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
    
    mem->startPoint = ((pop_data *) pop->data)->startPoint;
    //Note the ++ here: every chromosome will get a unique name.
    mem->name = ((pop_data *) pop->data)->curName++;
    printf("CONSTRUCTOR: %d\n", mem->name);
    if(mem->name >= 10000){
        die("Too many chromosomes allocated.");
    }
    embryo->chromosome = malloc(sizeof(entity_chrom *));
    if(!embryo->chromosome) die("Could not allocate chromosomes array.");
    embryo->chromosome[0] = mem;
    
    return true;
}
        
//Copy the chromosome from src into dest. 
//chromosomeid is ignored. 
//You shouldn't need to mess with this unless you change the 
//entity_chrom struct.
void plane_chromosome_replicate(const population *pop, 
        entity *src, entity *dest, const int chromosomeid){
    if(!pop) die("No population passed in.");
    if(!src || !dest) die("Null entity passed.");
    if(!src->chromosome || !dest->chromosome) die("Entity passed with no chromosomes.");
    if(!src->chromosome[0] || !dest->chromosome[0]) die("Empty chromosomes.");
    entity_chrom *srcChrom = (entity_chrom *) src->chromosome[0];
    entity_chrom *destChrom = (entity_chrom *) dest->chromosome[0];
    printf("REPLICATE: %d -> %d\n", srcChrom->name, destChrom->name);
    for(int i=0; i<pop->len_chromosomes; i++){
        destChrom->controllerInputs[i] = srcChrom->controllerInputs[i];
        destChrom->results[i] = srcChrom->results[i];
        destChrom->collideFrame = srcChrom->collideFrame;
    }
}

//Free the memory held by a chromosome. 
//You shouldn't need to change this unless you add an allocated
//region to the entity_chrom struct.
void plane_chromosome_destructor(population *pop, entity *corpse){
    if(!pop) die("Null pointer to population passed.");
    if(!corpse) die("Null pointer to entity passed.");
    if(corpse->chromosome==NULL) die("Chromosome already deallocated.");
    //Note! Do not free startPoint, since that's a global pointer. 
    printf("DESTRUCTOR: %d\n", ((entity_chrom *)corpse->chromosome[0])->name);
    free(((entity_chrom *)corpse->chromosome[0])->results);
    free(((entity_chrom *) corpse->chromosome[0])->controllerInputs);
    free(corpse->chromosome[0]);
    free(corpse->chromosome);
}



