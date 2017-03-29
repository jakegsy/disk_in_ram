//
//  filesys_types.h
//  assignment3
//
//  Created by Jake Goh on 24/2/17.
//  Copyright © 2017 Jake Goh Si Yuan. All rights reserved.
//

#ifndef filesys_types_h
#define filesys_types_h

#define BLOCK_SIZE 8000
#define BLOCK_MAP_SIZE (BLOCK_SIZE - 3 * sizeof(int))/(sizeof(short))
#define MAX_SIZE_IN_KB BLOCK_MAP_SIZE * BLOCK_MAP_SIZE * BLOCK_SIZE
#define BLOCK_CONTENT_SIZE (BLOCK_SIZE - 5 * sizeof(int))/(sizeof(char))
#define NUM_FILES_IN_CONTENT BLOCK_CONTENT_SIZE / sizeof(int)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


//Only BLOCK_MAP_SIZE amount of BLOCK_MAPS allowed.
//Easy to scale up size via first bit-map optimizing and then increasing size of master_block;
typedef struct {
    void * first_partition;
    void * second_partition;
    unsigned int size;
} my_file_system;
//Size 24 B

typedef struct block {
    int parent_block;
    int this_block;
    int next_block;
    int meta_block;
    int free_index;
    char content[BLOCK_CONTENT_SIZE];
} block;

typedef struct {
    int parent_block;
    int this_block;
    int next_block;
    int meta_block;
    int free_index;
    int content[NUM_FILES_IN_CONTENT];
} f_block;
// Size 8000 B, if next block then positive non-zero next_block
// next_block of master_block is always root.
// meta_block of -1 means this is the meta_block, else, points to actual block.

typedef struct {
    unsigned int num_blocks;
    unsigned int free_blocks;
    unsigned int block_id;
    short block_map[BLOCK_MAP_SIZE] ; //implement it in bit-map optimization.
} block_map;

typedef struct {
    unsigned int total_num_blocks;
    unsigned int total_free_blocks;
    unsigned int total_num_maps;
    unsigned int total_free_maps;
    unsigned int current_free_map_block;
    unsigned int start_data_block;
} master_block;

// If -(n), then it's a master block. Where n = 1 is the first block, n = 2 is the second master etc.
// Number in block_map connects to the metadata location of file, which is also the first block of the file.
// To locate files on blocks after the master(n=1), offset with location = BLOCK_MAP_SIZE * (block_id - 1) + offset.

typedef struct {
    char name[255];
    time_t date_of_creation;
    time_t date_of_mod;
    long actual_size;//in Bytes
    long partitioned_size;
    char read;
    char write;
    char exec;
    char folder;
    unsigned int parent;
    unsigned int start_block;
    unsigned int last_block;
    unsigned int this_block;
} file_metadata;

typedef struct {
    char content[BLOCK_CONTENT_SIZE];
    int point_index;
} buffer;

typedef struct {
    char permission;
    buffer buffer;
    int * block_tab;
    int max_block_tab;
    int relative_curr_block;
    int num_blocks;
    file_metadata * meta;
    block * partition;
} file_process;

#define SIZE_OF_METADATA sizeof(file_metadata)


#include "misc_ops.h"
#include "file_io.h"
#include "block_ops.h"
#include "file_ops.h"

#endif /* filesys_types_h */
