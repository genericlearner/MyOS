#include "fat16.h"
#include "status.h"
#include "string/string.h"

//This is a file to resolve the information for fat16 file system

int fat16_resolve(struct disk* disk);
void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode);


//This struct of type filesystem created for fat16 fs, it resolves from the fat16 resolve function with function pointers
//The function pointer open, crreates 
struct filesystem fat16_fs =
{
    .resolve = fat16_resolve,
    .open = fat16_open
};

//Initializes the fat16 filesystem by defining the filesystem name as FAT16 and returns the pointer to the fat16 filesystem
struct filesystem* fat16_init()
{
    strcpy(fat16_fs.name,"FAT16");
    return &fat16_fs;
}

//fat16 resolve function returns EIO for now
int fat16_resolve(struct disk* disk)
{
    return -EIO;
}
//
void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode)
{
    return 0;
}