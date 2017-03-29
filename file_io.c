//
//  file_io.c
//  assignment3
//
//  Created by Jake Goh on 24/2/17.
//  Copyright © 2017 Jake Goh Si Yuan. All rights reserved.
//

#include "file_io.h"



int rewind_to_top(file_process *fp){
    int start_block = fp->meta->start_block;
    block * sb = fp->partition + start_block;
    memcpy(fp->buffer.content,sb->content,BLOCK_CONTENT_SIZE);
    fp->buffer.point_index = 0;
    fp->relative_curr_block = 0;
    return 1;
}


file_process * open_file(void * partition, int meta_block, char op){
    file_process * return_fp = malloc(sizeof(file_process));
    block * partition_block = (block *) partition;
    return_fp->meta = (file_metadata *)(partition_block + meta_block)->content;
    
    if(op=='w'||op=='a'){
        if(return_fp->meta->write!=1){perror("File does not have permission to write");return NULL;}
    }
    if(op=='r'){
        if(return_fp->meta->read!=1){perror("File does not have permission to read");return NULL;}
    }
    if(return_fp->meta->this_block != meta_block){perror("File cannot be read, invalid meta_block");return NULL;}
    
    int num_blocks = get_num_blocks((int) return_fp->meta->partitioned_size);
    int next_block = return_fp->meta->start_block;
    return_fp->block_tab = malloc(sizeof(int) * num_blocks);
    
    for(int i=0;i<num_blocks;i++){ //Store block index onto block_tab for quick access;
        if(next_block==0){printf("No next block\n"); break;}
        return_fp->block_tab[i] = next_block;
        next_block = (partition_block + next_block)->next_block;
    }
    
    return_fp->relative_curr_block = 0;
    return_fp->permission = op;
    return_fp->partition = (block *) partition;
    return_fp->num_blocks = num_blocks;

  
    if(op=='a'){
        //case of writing at the end.
        block * last_block = partition_block + return_fp->block_tab[num_blocks - 1];//wrong index?
        memcpy(return_fp->buffer.content,last_block->content,BLOCK_CONTENT_SIZE);
        return_fp->buffer.point_index = last_block->free_index;
    }
    else{
        block * first_block = (block *) partition + return_fp->block_tab[0];
        memcpy(return_fp->buffer.content,first_block->content,BLOCK_CONTENT_SIZE); //not copied over?
        return_fp->buffer.point_index = 0;
    }
    return return_fp;
    
}

int close_file(file_process * fp){
    //printf("Closing file..\n");
    if(fp->permission!='r'){
        //printf("Attempting flush...\n");
        write_flush(fp);
        
    }
    //free(fp->block_tab);
    free(fp);
    return 1;
}


char read_one(file_process * fp){
    if(fp->permission!='r'){perror("File is not opened for reading");return 0;}
    if(fp->buffer.point_index>=BLOCK_CONTENT_SIZE){
        fp->relative_curr_block = fp->relative_curr_block + 1;
        //Case of over-reading will not be reached, because of EOF, but just in case
        if(fp->relative_curr_block > fp->num_blocks){
            perror("Reading out of allocated space");
            return 0;
        }
        int next_block = fp->block_tab[fp->relative_curr_block];
        block * nb = (fp->partition + next_block);
        memcpy(fp->buffer.content,nb->content,BLOCK_CONTENT_SIZE);
        fp->buffer.point_index = 0;
    }
    char returnc = fp->buffer.content[fp->buffer.point_index];
    fp->buffer.point_index = fp->buffer.point_index + 1;
    return returnc;
}

int write_one(char w, file_process * fp){
    if(fp->permission=='r') return 0;
    //Allow only 'w' and 'a';
    
    //Case where buffer is full
    if(fp->buffer.point_index>=BLOCK_CONTENT_SIZE){
        int flush_block = write_flush(fp);
        int new_block = extend_file(fp->partition, fp->meta->this_block);
        if(new_block == 0){
            fp->relative_curr_block = -1;
            fp->buffer.point_index = BLOCK_CONTENT_SIZE;
            perror("Partition is out of space, out of blocks");
            //printf("Actual size: %ld\n",fp->meta->actual_size);
            return 0;}

        fp->relative_curr_block = fp->relative_curr_block + 1;
        if(fp->num_blocks == fp->max_block_tab){
            fp->max_block_tab = 2 * fp->num_blocks;
            fp->block_tab = double_block_tab(fp->max_block_tab, fp->block_tab);
        }
        fp->num_blocks += 1;
        fp->meta->partitioned_size = fp->num_blocks * BLOCK_SIZE;
        fp->block_tab[fp->relative_curr_block] = new_block;
        block * nb = fp->partition + new_block;
        nb->parent_block = flush_block;
        nb->meta_block = fp->meta->this_block;
    }
    fp->buffer.content[fp->buffer.point_index] = w;
    fp->buffer.point_index = fp->buffer.point_index + 1;
    fp->meta->actual_size = fp->meta->actual_size + sizeof(char);
    return 1;
}

int * double_block_tab(int max_block_size, int * previous){
    int * return_tab = malloc(sizeof(int) * max_block_size);
    memcpy(return_tab, previous, max_block_size/2);
    free(previous);
    return return_tab;
}


int write_flush(file_process * fp){
    if(fp->relative_curr_block == -1)return 0;
    //If there are no more next block;
    int block_at = fp->block_tab[fp->relative_curr_block];
    block * address = fp->partition + block_at;
    memcpy(address->content, fp->buffer.content, fp->buffer.point_index);
    address->free_index = fp->buffer.point_index;
    fp->buffer.point_index = 0;
    memset(fp->buffer.content,0,BLOCK_CONTENT_SIZE);
    fp->meta->date_of_mod = time(NULL);

    return block_at; //returns the block index flushed to
}

