#ifndef DISK_H
#define DISK_H

typedef unsigned int MYOS_DISK_TYPE;

//Physical hard disk on the pc
#define MYOS_DISK_TYPE_REAL 0
struct disk
{
    MYOS_DISK_TYPE type;
    int sector_size;
};

void disk_search_and_initialize();
struct disk* disk_get(int index);
int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buff);


#endif