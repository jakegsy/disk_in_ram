//
//  block_ops.c
//  assignment3
//
//  Created by Jake Goh on 24/2/17.
//  Copyright © 2017 Jake Goh Si Yuan. All rights reserved.
//

#include "block_ops.h"



int get_block(void * partition, int parent_block){ //can return array of ints in the future
    //Nothing to do with metadata.
    int relative_index;
    master_block * mb = (master_block *) partition;
    block_map * bmap = (block_map *) partition + mb->current_free_map_block;
    
    //If no blocks left
    if(mb->total_free_blocks <= 0){
        //perror("No free blocks remaining\n");
        return 0;
    }
    // Reduce total_free_blocks
    mb->total_free_blocks = mb->total_free_blocks - 1;
    
    //Look for block in bmap with guaranteed free block;
    for(int i=0; i<BLOCK_MAP_SIZE; i++){
        if(bmap->block_map[i]==0){
            bmap->block_map[i] = parent_block;
            relative_index = i;
            bmap->free_blocks = bmap->free_blocks - 1;
            break;
        }
    }
    //If using this block clears all remaining in bmap, find new free block map;
    if(bmap->free_blocks <= 0){
        mb->total_free_maps = mb->total_free_maps - 1;
        if(mb->total_free_maps > 0){
            mb->current_free_map_block = find_new_free_map(partition);
        } else{
            mb->current_free_map_block = 0;
        }
    }
    //Index is computed by taking the location of first data block, summing up with the total number of block maps used(which signify number of data blocks taken) * BLOCK_MAP_SIZE, along with the relative index of this block with its block map.
    
    int return_index = (bmap->block_id - 1) * BLOCK_MAP_SIZE + mb->start_data_block + relative_index;
    
    //Set up the new block with valid information.
    block * new_block = (block *) partition + return_index;
    new_block->parent_block = parent_block;
    new_block->meta_block = ((block *) partition + parent_block)->meta_block;
    return return_index;
    
}

void initialize_master_block(master_block * mb, int num_blocks, int num_maps){
    mb->current_free_map_block = 1; //By default, block 1 is the first free block_map
    mb->total_num_blocks = num_blocks;
    mb->total_free_blocks = mb->total_num_blocks - 1; //for master_block;
    mb->total_num_maps = num_maps;
    mb->total_free_maps = num_maps;
    mb->start_data_block = mb->total_num_maps + 1; //+1 for 1 index after the last block_map.

}

int initialize_partition(void * partition, int size){
    block_map * partition_to_use = (block_map *) partition;
    unsigned int num_blocks = modulo(size,BLOCK_SIZE); //1 less than.
    unsigned int num_maps = modulo(num_blocks,BLOCK_MAP_SIZE) + 1; //1 default;
    master_block * mb = (master_block *) partition;
    
    //To initialize BLOCK_0 or Master Block;
    initialize_master_block(mb,num_blocks,num_maps);
    
    //To initialize all block_maps;
    for(int j=1; j<=num_maps;j++){
        mb->total_free_blocks = mb->total_free_blocks - 1; //Use up 1 for block_map
        
        block_map * curr = partition_to_use + j;
        initialize_block_map(curr, j);
        
        if(j==num_maps){ //special case of last map, remainder of blocks.
            int free_blocks_remaining = num_blocks - (num_maps - 1) * BLOCK_MAP_SIZE - num_maps - 1;
            partition_to_use[j].free_blocks = free_blocks_remaining;
            partition_to_use[j].num_blocks = free_blocks_remaining;
        } else{
            partition_to_use[j].free_blocks = BLOCK_MAP_SIZE;
            partition_to_use[j].num_blocks = BLOCK_MAP_SIZE;
        }
    }
    
    //To initialize all normal blocks;
    for(int i=num_maps+1;i<num_blocks;i++){
        block * current = (block *) partition + i;
        current->this_block = i;
        initialize_block(current);
    }
    
    return 1;
}






void initialize_block_map(block_map * block_map, int id){
    block_map->block_id = id;
    for(int i=0; i<BLOCK_MAP_SIZE; i++){
        block_map->block_map[i] = 0;
    }
}


void initialize_block(block * block){
    block->free_index = 0;
    block->meta_block = 0;
    block->next_block = 0;
    block->parent_block = 0;
    for(int i=0;i<BLOCK_CONTENT_SIZE;i++){
        block->content[i] = 0;
    }
}


int release_block(void * partition, int block_index){
    master_block * mb = (master_block *) partition;
    int start_data_block = mb->start_data_block;
    int bmap_id = modulo(block_index - start_data_block,BLOCK_MAP_SIZE) + 1;
    int relative_index = block_index - (bmap_id-1) * BLOCK_MAP_SIZE - start_data_block;
    //Update block map and master block;
    block_map * bmap = (block_map *) partition + bmap_id;
    if(bmap->free_blocks == 0){
        mb->total_free_maps += 1;
        mb->current_free_map_block = bmap_id;
    }
    bmap->free_blocks = bmap->free_blocks + 1;
    //printf("Freeing block %d\n",bmap->free_blocks);
    //printf("Norm_block %d\n",bmap->num_blocks);
    bmap->block_map[relative_index] = 0;
    mb->total_free_blocks = mb->total_free_blocks + 1;
    
    //Reinitalize block;
    block * b = (block *) partition + block_index;
    initialize_block(b);
    
    return 1;
}
