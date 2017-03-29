//
//  file_ops.h
//  assignment3
//
//  Created by Jake Goh on 24/2/17.
//  Copyright © 2017 Jake Goh Si Yuan. All rights reserved.
//

#ifndef file_ops_h
#define file_ops_h

#include <stdio.h>
#include "filesys_types.h"

int extend_file(void * partition, int meta_block);
int create_file(void * partition, char * filename, unsigned int size, unsigned int parent, char folder);
int copy_to_new_partition(file_process *fp, int new_parent, void * new_partition);
int move_to_new_partition(file_process * fp, int new_parent, void * new_partition);
int copy_to_new_parent(file_process * fp, int new_parent);
int move_to_new_parent(file_process *fp, int new_parent);

int add_to_folder(void * partition, int folder_meta, int add_meta);
int delete_file(void * partition, int meta_block);
int find_free_folder_index(f_block * f_b);
int num_files_from_folder(void * partition, int folder_meta);
char is_folder(void * partition, int parent);
int search_in_folder_block(f_block * f_b, int meta_to_find);

#endif /* file_ops_h */
