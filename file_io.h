//
//  file_io.h
//  assignment3
//
//  Created by Jake Goh on 24/2/17.
//  Copyright © 2017 Jake Goh Si Yuan. All rights reserved.
//

#ifndef file_io_h
#define file_io_h
#include "filesys_types.h"
#include <stdio.h>

file_process * open_file(void * partition, int meta_block, char op);
int rewind_to_top(file_process *fp);
int close_file(file_process * fp);
char read_one(file_process * fp);
int write_one(char w, file_process * fp);
int write_flush(file_process * fp);
int * double_block_tab(int max_block_size, int * previous);


#endif /* file_io_h */

