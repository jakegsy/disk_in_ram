//
//  file_ops.c
//  assignment3
//
//  Created by Jake Goh on 24/2/17.
//  Copyright © 2017 Jake Goh Si Yuan. All rights reserved.
//

#include "file_ops.h"


int delete_folder_and_files(void * partition, file_metadata * folder_meta){
    int next_block = folder_meta->start_block;
    for(int i=0;i<get_num_blocks((int)folder_meta->partitioned_size);i++){
        f_block * curr = (f_block *) partition + next_block;
        int old_block = next_block;
        next_block = curr->next_block;
        for(int j=0;j<NUM_FILES_IN_CONTENT;j++){
            int curr_file = curr->content[j];
            //printf("curr_file %d\n",curr_file);
            if(curr_file){
                delete_file(partition,curr_file);
            }
        }
        release_block(partition, old_block);
    }
    release_block(partition, folder_meta->this_block);
    return 1;
}

int delete_file(void * partition, int meta_block){
    //printf("Deleting Block : %d\n",meta_block);
    block * b = (block *) partition;
    file_metadata * md = (file_metadata *) ((b + meta_block)->content);
    if(md->folder==1) delete_folder_and_files(partition, md);
    else{
        int next_block = md->start_block;
        int old_block = 0;
        for(int i=0;i<get_num_blocks((int)md->partitioned_size);i++){
            block * curr = b + next_block;
            old_block = next_block;
            next_block = curr->next_block;
            if(old_block){
                release_block(partition, old_block);
            }
        }

        release_block(partition,meta_block);
    }
    return 1;
}


int add_to_folder(void * partition, int folder_meta, int add_meta){
    f_block * f_b = (f_block *) partition;
    block * b_f_meta = (block *) partition + folder_meta;
    file_metadata * f_meta = (file_metadata * )b_f_meta->content;
    if(f_meta->folder!=1){return 0;}
    
    f_block * free_fb = f_b + f_meta->last_block;
    free_fb->content[free_fb->free_index] = add_meta;
    free_fb->free_index = find_free_folder_index(free_fb);
    if(free_fb->free_index >= NUM_FILES_IN_CONTENT){
        extend_file(partition, folder_meta); //****
    }
    return 1;
}

int find_free_folder_index(f_block * f_b){
    for(int i=0;i<NUM_FILES_IN_CONTENT;i++)if(f_b->content[i]==0)return i;
    return NUM_FILES_IN_CONTENT;
}

int num_files_from_folder(void * partition, int folder_meta){
    f_block * f_b = (f_block *) partition;
    block * b_f_meta = (block *) partition + folder_meta;
    file_metadata * f_meta = (file_metadata * )b_f_meta->content;
    if(f_meta->folder!=1){return 0;}
    int counter = 0;
    int next_block = f_meta->start_block;
    for(int i=0;i<get_num_blocks((int)f_meta->partitioned_size);i++){
        f_block * curr = f_b + next_block;
        if(curr->free_index>=NUM_FILES_IN_CONTENT){
            counter += NUM_FILES_IN_CONTENT;
        }else{
            counter += curr->free_index;
        }
    }
    return counter;
}

int search_in_folder_block(f_block * f_b, int meta_to_find){
    for(int i=0;i<NUM_FILES_IN_CONTENT;i++)if(f_b->content[i] == meta_to_find)return i+1;
    //remember -1 with all usage of index.
    return 0;
}


int remove_from_folder(void * partition, int folder_meta, int to_be_removed){
    f_block * f_b = (f_block *) partition;
    block * b_f_meta = (block *) partition + folder_meta;
    file_metadata * f_meta = (file_metadata * )b_f_meta->content;
    if(f_meta->folder!=1){perror("This is not a folder.");return 0;}
    
    int next_block = f_meta->start_block;
    for(int i=0;i<get_num_blocks((int)f_meta->partitioned_size);i++){
        f_block * curr = f_b + next_block;
        int is_in_folder = search_in_folder_block(curr, to_be_removed);
        if(is_in_folder){
            curr->content[is_in_folder - 1] = 0;
            curr->free_index = is_in_folder - 1;
            return 1;
        }
    }
    perror("File not in folder.");
    return 0;
}




int copy_to_new_partition(file_process *fp, int new_parent, void * new_partition){
    master_block * mb = (master_block *) new_partition;
    block * b = (block *) new_partition;
    if(!(fp->num_blocks + 1 <=mb->total_free_blocks)) return 0; //+1 for metablock
    
    int free_block = get_block(new_partition,new_parent);
    memcpy((b+free_block)->content,fp->meta,BLOCK_CONTENT_SIZE);
    file_metadata * new_md = (file_metadata *) (b+free_block)->content;
    new_md->date_of_creation = time(NULL);
    new_md->date_of_mod = time(NULL);
    new_md->parent = new_parent;
    new_md->this_block = free_block;
    new_md->start_block = get_block(new_partition,free_block);
    new_md->last_block = new_md->start_block;
    
    for(int i=0;i<fp->num_blocks;i++){
        block * curr = b + new_md->last_block;
        block * tb_copied = fp->partition + fp->block_tab[i];
        curr->meta_block = new_md->this_block;
        curr->parent_block = free_block;
        curr->next_block = get_block(new_partition, new_md->last_block);
        memcpy(curr->content,tb_copied->content,BLOCK_CONTENT_SIZE);
        if(i==fp->num_blocks -1){continue;} //skip final block creation for senseless.
        if(curr->next_block == 0){
            perror("Partition out of space, copied till max-size.");
            return new_md->this_block;}
        new_md->last_block = curr->next_block;
    }
    add_to_folder(new_partition, new_parent, new_md->this_block);
    return new_md->this_block;
}


int copy_to_new_parent(file_process * fp, int new_parent){
    return copy_to_new_partition(fp,new_parent,fp->partition);
}


int move_to_new_parent(file_process * fp, int new_parent){
    int meta_block = fp->meta->this_block;
    int old_block = fp->meta->parent;
    //attach to new parent
    fp->meta->parent = new_parent;
    add_to_folder(fp->partition, new_parent, meta_block);
    //remove from old folder content
    remove_from_folder(fp->partition, old_block, meta_block);
    return 1;
}



int move_to_new_partition(file_process * fp, int new_parent, void * new_partition){
    int meta_block = fp->meta->this_block;
    int old_block = fp->meta->parent;
    int new_block = copy_to_new_partition(fp,new_parent,new_partition);
    remove_from_folder(fp->partition, old_block, meta_block);
    delete_file(fp->partition, meta_block);
    fp->partition = (block *) new_partition;
    return new_block;
}







int create_file(void * partition, char * filename, unsigned int size, unsigned int parent, char folder){
    file_metadata meta;
    if(size<=0)size=0; //Default 1 block;
    for(int i=0;i<255;i++){
        if(filename[i]=='\0')break;
        meta.name[i] = filename[i];
    }
    meta.date_of_creation = meta.date_of_mod = time(NULL);
    meta.read = meta.write = 1;
    meta.exec = 0;
    meta.folder = folder;
    meta.parent = parent;
    meta.actual_size = SIZE_OF_METADATA + size;
    
    meta.partitioned_size = 8000*(modulo((int)meta.actual_size,8000) + 1);
    meta.this_block = get_block(partition,parent);
    meta.start_block = get_block(partition, meta.this_block);
    meta.last_block = meta.start_block;
    
    
    block * location = (block *) partition + meta.this_block;
    memcpy(location->content,&meta,sizeof(file_metadata));
    
    location->free_index = SIZE_OF_METADATA;
    location->meta_block = meta.this_block;
    location->parent_block = parent;
    
    if(is_folder(partition,parent)==1){
        add_to_folder(partition, parent, meta.this_block);
    }
    //next-block updated in get_block func;
    return meta.this_block;
}

char is_folder(void * partition, int parent){
    block * b = (block *) partition;
    file_metadata * md = (file_metadata *) (b + parent)->content;
    return md->folder;
}

int extend_file(void * partition, int meta_block){
    block * mb = (block *) partition + meta_block;
    file_metadata * meta = (file_metadata *) mb->content;
    
    //Find free block, set up last block as parent;
    int last_block = meta->last_block;
    int free_block = get_block(partition, meta->last_block);
    if(free_block == 0) return 0;
    //Update meta-info
    meta->last_block = free_block;
    meta->partitioned_size = meta->partitioned_size + 1 * BLOCK_SIZE;
    meta->date_of_mod = time(NULL);
    //Update last and new blocks.
    block * last = (block *) partition + last_block;
    block * free = (block *) partition + free_block;
    last->meta_block = meta_block;
    last->next_block = free_block;
    
    free->meta_block = meta_block;
    free->parent_block = last_block;
    
    return free_block;
}
