#ifndef DISK_H
#define DISK_H
#include "fs/file.h"

typedef unsigned int MYOS_DISK_TYPE;

//Physical hard disk on the pc
#define MYOS_DISK_TYPE_REAL 0
struct disk
{
    MYOS_DISK_TYPE type;
    int sector_size;
    //id of the disk
    int id;
    
    struct filesystem* filesystem;

    //private data of our filesystem
    void* fs_private;
};

void disk_search_and_initialize();
struct disk* disk_get(int index);
int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buff);


#endif