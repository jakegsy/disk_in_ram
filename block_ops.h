//
//  block_ops.h
//  assignment3
//
//  Created by Jake Goh on 24/2/17.
//  Copyright © 2017 Jake Goh Si Yuan. All rights reserved.
//

#ifndef block_ops_h
#define block_ops_h
#include "filesys_types.h"
#include <stdio.h>

int get_block(void * partition, int parent_block);
int initialize_partition(void * partition, int size);
void initialize_block_map(block_map * block_map, int id);
void initialize_block(block * block_map);
int release_block(void * partition, int block_index);

#endif /* block_ops_h */
