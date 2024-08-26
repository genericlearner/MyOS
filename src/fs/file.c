#include "file.h"
#include "config.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "kernel.h"
#include "disk/disk.h"
#include "fat/fat16.h"
#include "string/string.h"
#include "status.h"

//The "file" file defines the filesystem and methods for a filesystem.



//Defines the array to store filesystems and file descriptors for the filesystem
struct filesystem* filesystems[MYOS_MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[MYOS_MAX_FILE_DESCRIPTORS];

//searches the file systems array to find free space 
//returns the pointer to the free position
static struct filesystem** fs_get_free_filesystem()
{
    int i=0;
    for(i=0;i<MYOS_MAX_FILESYSTEMS;i++)
    {
        if(filesystems[i] == 0)
        {
            return &filesystems[i];
        }
    }
    return 0;
}

//The insert function uses the fs_get_free_filesystem to find free space in the filesystems array
//Then allocates the free space with the pointer to the filesystem 
void fs_insert_filesystem (struct filesystem* filesystem)
{
    struct filesystem** fs;
    fs = fs_get_free_filesystem();
    
    if(!fs)
    {
        print("problem inserting filesystem");
        while(1){}
    }
    *fs =filesystem;

}

//The static load function initializes and loads the 
static void fs_static_load()
{
    fs_insert_filesystem(fat16_init());

}
//The fs load function assigns memory to the filesystem and calls static load 
void fs_load()
{
    memoryset(filesystems,0,sizeof(filesystems));
    fs_static_load();
}
//Initializes the filesystem by assigning memory to the file descriptors and calling the load function
void fs_init()
{
    memoryset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load();
}
static void file_free_descriptor(struct file_descriptor* desc)
{
    file_descriptors[desc->index-1] = 0x00;
    kfree(desc);
}
//The function searches the array of file descriptors to allocate and assigns memory from the kernel memory
//Then it assigns the index for the file descriptor 
static int file_new_descriptor(struct file_descriptor** desc_out)
{
    int res = -ENOMEM;
    for(int i=0;i<MYOS_MAX_FILE_DESCRIPTORS;i++)
    {
        if(file_descriptors[i] == 0)
        {
            struct file_descriptor* desc = kzalloc(sizeof(struct file_descriptor));
            //descriptors start at 1
            desc->index = i+1;
            file_descriptors[i] = desc;
            *desc_out =desc;
            res = 0;
            break;
        }
    }
    return res;
}
//getting file descriptor by using the array and index stored in the struct
static struct file_descriptor* file_get_descriptor(int fd)
{
    if(fd<= 0|| fd >= MYOS_MAX_FILE_DESCRIPTORS)
    {
        return 0;
    }
    int index = fd -1 ;
    return file_descriptors[index];
}

//The fs_resolve function uses the disk to find filesystem in the filesystem array
//The resolve function 
//The function returns the pointer to the filesystem
struct filesystem* fs_resolve(struct disk* disk)
{
    struct filesystem* fs = 0;
    for(int i=0;i < MYOS_MAX_FILESYSTEMS;i++)
    {
        if(filesystems[i]!=0 && filesystems[i]->resolve(disk)==0)
        {
            fs = filesystems[i];
            break;

        }
    }
    return fs;
}

FILE_MODE file_get_mode_by_string(const char* str)
{
    FILE_MODE mode = FILE_MODE_INVALID;
    if(strncmp(str, "r",1) == 0)
    {
        mode = FILE_MODE_READ;
    }
    else if(strncmp(str, "w", 1) == 0)
    {
        mode = FILE_MODE_WRITE;

    }
    else if(strncmp(str,"a", 1) == 0)
    {
        mode = FILE_MODE_APPEND;
    }
    
    return mode;
}
//The open function needs to be set up
int fopen(const char* filename, const char* mode_str)
{
    int res = 0;
    struct path_root* root_path = pathparser_parse(filename,NULL);
    if(!root_path)
    {
        res = -EINVARG;
        goto out;
    }
    //cannot be just root path
    if(!root_path->first)
    {
        res = -EINVARG;
        goto out;
    }
    //checking for invalid drives/ if it exists
    struct disk* disk = disk_get(root_path->drive_no);

    if(!disk)
    {
        res = -EIO;
        goto out;
    }

    if(!disk->filesystem)
    {
        res = -EIO;
        goto out;
    }

    FILE_MODE mode = file_get_mode_by_string(mode_str);
    if(mode == FILE_MODE_INVALID)
    {
        res = -EINVARG;
        goto out;
    }

    void* descriptor_private_data = disk->filesystem->open(disk, root_path->first, mode);
    if(ISERR(descriptor_private_data))
    {
        res = ERROR_I(descriptor_private_data);
        goto out;
    }

    struct file_descriptor* desc = 0;
    res = file_new_descriptor(&desc);
    if(res<0)
    {
        goto out;
    }

    desc->filesystem = disk->filesystem;
    desc->private = descriptor_private_data;
    desc->disk = disk;
    res = desc->index;

out:
    if(res < 0)
        res = 0;
        
    return res;
}





