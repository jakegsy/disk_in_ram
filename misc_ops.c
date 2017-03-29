//
//  misc_ops.c
//  assignment3
//
//  Created by Jake Goh on 24/2/17.
//  Copyright © 2017 Jake Goh Si Yuan. All rights reserved.
//
#include "filesys_types.h"
#include "misc_ops.h"


my_file_system * create_new_file_system(unsigned int s) { //Verified
    my_file_system * ret = (my_file_system *)
    malloc(sizeof(my_file_system));
    ret->size = s;
    ret->first_partition = calloc(s,1);
    ret->second_partition = calloc(s,1);
    return ret;
}


int get_num_blocks(int partitioned_size){ //Verified
    return partitioned_size/BLOCK_SIZE;
}


unsigned int modulo(unsigned int index, unsigned long mod){ //Verified
    unsigned int counter = 0;
    unsigned int current = index;
    while(1){
        if(current < mod) return counter;
        else{current -= mod;
            counter++;}
    }
}


int save_partition_to_disk(my_file_system * fs, int partition){
    char path[255];
    void * partition_to_use = NULL;
    FILE * file;
    if(partition == 1){
        partition_to_use = fs->first_partition;
    }
    if(partition == 2){
        partition_to_use = fs->second_partition;
    }
    printf("Enter path of file to save to: \n");
    scanf("%s",path);
    //char * patha = "/Users/jakegsy/Documents/Yale-NUS/OS/assignment3/binary.txt";
    file = fopen(path,"wb");
    if(file==NULL){
        return 1;
    }
    master_block * mb = (master_block *) partition_to_use;
    block_map * bm = (block_map *) partition_to_use;
    block * b = (block *) partition_to_use;
    fwrite(mb, sizeof(master_block), 1, file);
    fwrite(bm+1,sizeof(block_map),mb->total_num_maps,file);
    fwrite(b +mb->start_data_block,sizeof(block),mb->total_num_blocks-mb->total_num_maps-1,file);
    //for(int i=0; i<fs->size; i++){
     //   fputc(partition_to_use[i],file);
    //}
    fclose(file);
    return 0;
}

int load_partition_from_disk(my_file_system * fs, int partition){
    char path[255];
    void * partition_to_use = NULL;
    FILE * file;
    int i = 0;
    if(partition == 1){
        partition_to_use = fs->first_partition;
    }
    if(partition == 2){
        partition_to_use = fs->second_partition;
    }
    printf("Enter path of file to load from: \n");
    scanf("%s",path);
    //char * patha = "/Users/jakegsy/Documents/Yale-NUS/OS/assignment3/binary.txt";
    file = fopen(path,"rb");
    if(file==NULL){
        return 1;
    }
    master_block * mb = (master_block * )partition_to_use;
    block_map * bm = (block_map *) partition_to_use;
    block * b = (block *) partition_to_use;
    
    fread(mb,sizeof(master_block),1,file);
    
    for(int i=0;i<mb->total_num_maps;i++){
        fread(bm+1+i,sizeof(block_map),1,file);
    }
    for(int i=0;i<mb->total_num_blocks-mb->total_num_maps-1;i++){
        fread(b+mb->start_data_block+i,sizeof(block),1,file);
    }
    //fread(bm+1,sizeof(block_map),mb->total_num_maps,file);
    
    //fread(b+mb->start_data_block,sizeof(block),mb->total_num_blocks-mb->total_num_maps-1,file);
    //for(int i=0; i<fs->size; i++){
     //   partition_to_use[i] = fgetc(file);
    //}
    fclose(file);
    return 0;
}


int find_new_free_map(void * partition){
    master_block * mb = (master_block *)partition;
    block_map * bmap = (block_map *) partition + 1;
    for(int i=0;i<mb->total_num_maps;i++){
        block_map * curr = bmap + i;
        if(curr->free_blocks > 0){
            return (i + 1);
        }
    }
    return 0;
}


