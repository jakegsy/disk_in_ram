//
//  misc_ops.h
//  assignment3
//
//  Created by Jake Goh on 24/2/17.
//  Copyright © 2017 Jake Goh Si Yuan. All rights reserved.
//

#ifndef misc_ops_h
#define misc_ops_h

#include <stdio.h>
#include "filesys_types.h"

int get_num_blocks(int partitioned_size);
unsigned int modulo(unsigned int index, unsigned long mod);
my_file_system * create_new_file_system(unsigned int s);
int save_partition_to_disk(my_file_system * fs, int partition);
int load_partition_from_disk(my_file_system * fs, int partition);
int find_new_free_map(void * partition);


#endif /* misc_ops_h */
