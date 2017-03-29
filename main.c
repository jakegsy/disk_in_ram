
#include "filesys_types.h"
#include "misc_ops.h"
#include "file_io.h"

//Design makes a lot of use of relative index, which refers to the block number in the partition.
//Which is convenient since bmaps and blocks have the same byte-size.

void print_partition(void * partition, int num);
void print_block(block * b, char content);
void print_block_map(block_map * bmap);
void print_file(file_metadata * md);

int main(int argc, const char * argv[]) {
    my_file_system * filesys = create_new_file_system(1000000);
    //Begins with 12500 blocks, or 100 megabytes...
    void * first_partition = filesys->first_partition;
    void * second_partition = filesys->second_partition;
    initialize_partition(first_partition, filesys->size);
    initialize_partition(second_partition, filesys->size);
    //Now that all partitions are properly initialized...
    print_partition(first_partition,1);
    print_partition(second_partition,2);
    printf("As we can see, 2 blocks are taken up for the master_block and a block_map\n");
    block * b1 = (block *) first_partition;
    block * b2 = (block *) second_partition;
    block_map * bmap1 = (block_map *) first_partition;
    block_map * bmap2 = (block_map *) second_partition;
    master_block * mb1 = (master_block *) first_partition;
    master_block * mb2 = (master_block *) second_partition;
    
    
    
    int root_file_p1 = create_file(first_partition,"\\",100,-1,1);
    int root_file_p2 = create_file(second_partition,"\\",100,-1,1);
    int folder_file = create_file(first_partition, "Some folder", 100,-1,1);
    int index_f1 = create_file(first_partition, "My first_file", 10, root_file_p1, 0);
    int index_f2 = create_file(second_partition, "My second_file", 10, root_file_p2, 0);
    file_metadata * folder_meta = (file_metadata *) (b1 + folder_file)->content;
    file_metadata * root_fm1 = (file_metadata *) (b1 + root_file_p1)->content;
    file_metadata * root_fm2 = (file_metadata *) (b2 + root_file_p2)->content;
    
    file_process * fp1 = open_file(first_partition, index_f1, 'w');
    for(int i=0;i<180000;i++){
        write_one('W', fp1);
    }
    write_flush(fp1);
    rewind_to_top(fp1);
    write_one('F',fp1);
    write_flush(fp1);
    printf("Now I've written 180000 'W's to a file, and rewinding to a top, and writing 'F'\n\n");
    print_file(fp1->meta);
    printf("Approximately 24 blocks were taken up for 180000 chars, which is the same size expected \n\n");
    printf("Moving to a new parent...\n\n");
    move_to_new_parent(fp1, folder_file);
    print_partition(first_partition, 1);
    printf("As we can see, there are no changes in space.\n");
    printf("Verifying that it's not in the old parent... %d\n",search_in_folder_block((f_block *)first_partition + root_fm1->start_block, index_f1));
    printf("Verifying that it's in the new parent... %d\n",search_in_folder_block((f_block *)first_partition + folder_meta->start_block, index_f1));
    printf("Copying back to old parent... \n\n");
    int new_index_f1 = copy_to_new_parent(fp1, root_file_p1);
    print_partition(first_partition, 1);
    printf("As we can see, now this operation takes up additional 24 blocks because of a copy\n\n");
    file_process * f1_r = open_file(first_partition, index_f1, 'r');
    int counter = 0;
    for(int i=0;i<180000;i++){
        char curr = read_one(f1_r);
        if(curr=='W')counter++;
    }
    printf("%d of 'W' detected in original file\n\n",counter);
    
    file_process * f1_r_copy = open_file(first_partition, new_index_f1, 'r');
    counter = 0;
    for(int i=0;i<180000;i++){
        char curr = read_one(f1_r_copy);
        if(curr=='W')counter++;
    }
    close_file(fp1);
    close_file(f1_r_copy);
    close_file(f1_r);
    printf("%d of 'W' detected in copied file\n\n",counter);
    print_file(f1_r_copy->meta);
    printf("Copy within a partition is a success\n");
    printf("Copy between partitions is assumed as well because copying within a partition is structured such that it is the same function that is used to copy between partitions\n");
    
    printf("Next up, we will try saving and loading from partition 1 to partition 2\n");
    save_partition_to_disk(filesys, 1);
    print_partition(first_partition, 1);
    load_partition_from_disk(filesys, 2);
    printf("If the copying and saving was perfect, the same relative index would work\n");
    file_process * fp12_r = open_file(second_partition, index_f1, 'r');
    counter = 0;
    for(int i=0;i<180000;i++){
        char curr = read_one(fp12_r);
        if(curr == 'W'){counter +=1;}
    }
    printf("%d of 'W' detected in loaded partition file\n\n",counter);
    printf("And if we compared partition information... \n");
    print_partition(first_partition, 1);
    print_partition(second_partition, 2);
    print_block_map(bmap1);
    print_block_map(bmap2);
    printf("To show that it is not just a clone with similar address fields, I will recursively delete from folder in second_partition\n");
    delete_file(second_partition,folder_file);
    print_partition(first_partition, 1);
    print_partition(second_partition, 2);
    //printf("Can find %d\n",search_in_folder_block((f_block *)first_partition + root_fm1->start_block, index_f1));
    
    //move_to_new_parent(fp1, folder_file);
    //print_partition(first_partition);
    //printf("Can find %d\n",search_in_folder_block((f_block *)first_partition + root_fm1->start_block, index_f1));
    //printf("Can find %d\n",search_in_folder_block((f_block *)first_partition + folder_meta->start_block, index_f1));
    //Move file works.
    //int new_block = copy_to_new_parent(fp1, folder_file);
    //printf("Can find %d\n",search_in_folder_block((f_block *)first_partition + folder_meta->start_block, new_block));
    //printf("Can find %d\n",search_in_folder_block((f_block *)first_partition + root_fm1->start_block, index_f1));
    //print_partition(first_partition);
    //print_partition(second_partition);
    //print_file(fp1->meta);
    //Natural expansion of file test with expand_file
    
    
    //printf("Reading out of permission: %c Printing\n",read_one(fp1));
    //file_process * fp2_r = open_file(first_partition, index_f1, 'r');
    //counter = 0;
    //for(int i=0;i<180000;i++){
    //    char curr = read_one(fp2_r);
    //    if(curr == 'W'){counter +=1;}
    //}
    //printf("Counter : %d\n",counter);

    //print_file(fp1->meta);
    //print_partition(first_partition);
    //save_partition_to_disk(filesys, 1);
    //load_partition_from_disk(filesys, 2);
    //void * loaded_partition = filesys->second_partition;
    //print_partition(loaded_partition,3);
    //print_partition(first_partition,1);
    //block_map * bm22 = (block_map *) loaded_partition + 1;
    //block_map * bm11 = (block_map *) first_partition + 1;
    //print_block_map(bm22);
    //print_block_map(bm11);
    //block * b22 = (block *) loaded_partition;
    //block * b11 = (block *) first_partition;
    //print_block(b22+2, 'y');
    //print_block(b11+2, 'y');
    //print_block(b22+index_f1,'y');
    //print_block(b11+index_f1,'y');
    //file_process * fp1_r = open_file(second_partition, index_f1, 'r');
    //counter = 0;
    //for(int i=0;i<180000;i++){
    //   char curr = read_one(fp1_r);
    //    if(curr == 'W'){counter +=1;}
    //}
    //printf("Counter : %d\n",counter);
    //close_file(fp1_r);
    //saves and loads perfectly.
    
    //print_partition(first_partition);
    //delete_file(first_partition, index_f1);
    //print_partition(first_partition);
    //Delete singular file works as planned. Try on recursive.
    //Wrote as many as predicted.
    /*
    create_file(first_partition, "second file", 1000, root_file_p1, 0);
    create_file(first_partition, "third file", 10000, root_file_p1, 0);
    print_partition(first_partition);
    delete_file(first_partition,root_file_p1);
    print_partition(first_partition);
    //Recursive delete works as planned.
     */
    
    return 0;
}





void print_partition(void * partition, int num){
    printf("=========Partition %d information=======\n",num);
    master_block * mb = (master_block *) partition;
    printf("Free blocks/Total blocks : %d/%d\n", mb->total_free_blocks,
           mb->total_num_blocks);
    printf("Free maps/Total maps: %d/%d\n",mb->total_free_maps,mb->total_num_maps);
    printf("Free map index : %d\n", mb->current_free_map_block);
    printf("Start data block : %d\n",mb->start_data_block);
    printf("=========End of partition information==== \n");
}

void print_block(block * b, char content){
    printf("----Block %d ----\n",b->this_block);
    printf("Meta block : %d\n",b->meta_block);
    printf("Next block : %d\n",b->next_block);
    printf("Parent block : %d\n",b->parent_block);
    printf("Free index : %d\n",b->free_index);
    if(content){
        for(int i=0;i<BLOCK_CONTENT_SIZE;i++){
            printf("%c",b->content[i]);
        }
    }
    printf("----EOB----\n");
}

void print_block_map(block_map * bmap){
    printf("------Block Map %d------\n",bmap->block_id);
    printf("Free Blocks/Total Blocks: %d/%d\n",bmap->free_blocks,bmap->num_blocks);
    printf("-------------------------\n");
}


void print_file(file_metadata * md){
    printf("@@@@@@@@ Printing file '%s' @@@@@@@@@@\n\n",md->name);
    printf("Actual size/Partitioned size: %ld/%ld\n",md->actual_size,md->partitioned_size);
    printf("Start/This/Last/Parent : %d/%d/%d/%d\n",
           md->start_block,md->this_block,md->last_block,md->parent);
    printf("Num blocks : %d\n",get_num_blocks((int) md->partitioned_size));
    printf("Last Modified/Created At : %ld/%ld\n\n",md->date_of_mod,md->date_of_creation);
    printf("@@@@@@@@@End of File@@@@@@@@@@\n");
    
}

