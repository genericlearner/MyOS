#include "fat16.h"
#include "status.h"
#include "disk/disk.h"
#include "disk/diskstreamer.h"
#include "string/string.h"
#include "memory/heap/kheap.h"
#include "kernel.h"
#include "config.h"
#include "memory/memory.h"
#include <stdint.h>


#define MYOS_FAT16_STRUCTURE 0x29
#define MYOS_FAT16_FAT_ENTRY_SIZE 0x02
#define MYOS_FAT16_BAD_SECTOR 0xFF7
#define MYOS_FAT16_UNUSED 0x00


typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1
//fat directory entry attributes bitmask


#define FAT_FILE_READ_ONLY 0x01
#define FAT_FILE_HIDDEN 0x02 
#define FAT_FILE_SYSTEM 0x04
#define FAT_FILE_VOLUME_LABEL 0x08
#define FAT_FILE_SUBDIRECTORY 0x10 
#define FAT_FILE_ARCHIVED 0x20
#define FAT_FILE_DEVICE 0x40
#define FAT_FILE_RESERVED 0x80 



struct fat_header_extended
{
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
}__attribute__((packed));

//represents info from the bootloader
struct fat_header
{
    uint8_t short_jmp_ins[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t sectors_big;

}__attribute__((packed));


struct fat_h
{
    struct fat_header primary_header;
    union fat_h_e
    {   
        struct fat_header_extended extended_header;

    }shared;
};
//structures for files/directories
struct fat_directory_item
{
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation_time_tenths_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;
    uint32_t filesize;
}__attribute__((packed));

struct fat_directory
{
    struct fat_directory_item* item;
    int total;
    int sector_pos;
    int end_sector_pos;


};

struct fat_item
{
    union
    {
        struct fat_director_item* item;
        struct fat_directory* directory;
    };

    FAT_ITEM_TYPE type;
};

struct fat_file_descriptor
{
    struct fat_item* item;
    uint32_t pos;
};

struct fat_private
{
    struct fat_h header;
    struct fat_directory root_directory;

    //used to stream data clusters
    struct disk_stream* cluster_read_stream;
    //used to stream the file allocation table
    struct disk_stream* fat_read_stream;

    //used in situation where we stream the directory
   


    struct disk_stream* directory_stream;
};
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
static void fat16_init_private(struct disk *disk, struct fat_private *private)
{
    memoryset(private, 0, sizeof(struct fat_private));
    private->cluster_read_stream = diskstreamer_new(disk->id);
    private->fat_read_stream = diskstreamer_new(disk->id);
    private->directory_stream = diskstreamer_new(disk->id);
}

int fat16_sector_to_absolute(struct disk* disk, int sector)
{
    return sector * disk->sector_size;
}

int fat16_get_total_items_for_directory(struct disk* disk, uint32_t directory_start_sector)
{
    struct fat_directory_item item;
    struct fat_directory_item empty_item;
    memoryset(&empty_item,0,sizeof(empty_item));

    struct fat_private* fat_private = disk->fs_private;

    int res = 0;
    int i = 0;
    int directory_start_pos = directory_start_sector * disk->sector_size;
    struct disk_stream* stream = fat_private->directory_stream;
    if(diskstreamer_seek(stream, directory_start_pos) != MYOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }
    while(1)
    {
        if(diskstreamer_read(stream, &item, sizeof(item)) != MYOS_ALL_OK)
        {
            res = -EIO;
            goto out;
        }

        if(item.filename[0] == 0x00)
        {
            break;
        }
        
        if(item.filename[0] == 0xE5)
        {
            continue;
        }
        i++;
    }
    
    res = i;
out:
    return res;
}

int fat16_get_root_directory(struct disk* disk, struct fat_private* fat_private, struct fat_directory* directory)
{
    int res = 0;
    struct fat_header* primary_header = &fat_private->header.primary_header;
    int root_dir_sector_pos = (primary_header->fat_copies*primary_header->sectors_per_fat) + primary_header->reserved_sectors;
    int root_dir_entries = fat_private->header.primary_header.root_dir_entries;
    int root_dir_size = (root_dir_entries * sizeof(struct fat_directory_item));
    int total_Sectors = root_dir_size / disk->sector_size;
    if(root_dir_size % disk->sector_size)
    {
        total_Sectors +=1;
    }

    int total_items = fat16_get_total_items_for_directory(disk, root_dir_sector_pos);

    struct fat_directory_item* dir = kzalloc(root_dir_size);
    if(!dir)
    {
        res = -ENOMEM;
        goto out;
    }

    struct disk_stream* stream = fat_private->directory_stream;
    if(diskstreamer_seek(stream, fat16_sector_to_absolute(disk, root_dir_sector_pos))!= MYOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }
    
    if(diskstreamer_read(stream, dir, root_dir_size) != MYOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }
    directory->item = dir;
    directory->total = total_items;
    directory->sector_pos = root_dir_sector_pos;
    directory->end_sector_pos = root_dir_sector_pos + (root_dir_size / disk->sector_size);


out:
    return res;
}
//fat16 resolve function returns EIO for now
int fat16_resolve(struct disk* disk)
{
    
    int res = 0;
    struct fat_private* fat_private = kzalloc(sizeof(struct fat_private));
    fat16_init_private(disk, fat_private);

    disk->fs_private = fat_private;
    disk->filesystem = &fat16_fs;
    
    struct disk_stream* stream = diskstreamer_new(disk->id);

    if(!stream)
    {
        res = -ENOMEM;
        goto out;

    }

    if(diskstreamer_read(stream, &fat_private->header,sizeof(fat_private->header))!= MYOS_ALL_OK)
    {
        res = -EIO;
        goto out;

    }

    if(fat_private->header.shared.extended_header.signature != 0x29)
    {
        res = -EFSNOTUS;
        goto out;
    }

    if(fat16_get_root_directory(disk, fat_private, &fat_private->root_directory) != MYOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    


out:
    if(stream)
    {
        diskstreamer_close(stream);
    }
    if(res < 0)
    {
        kfree(fat_private);
        disk->fs_private = 0;

    }
    return res;
}

void fat16_to_proper_string(char** out, const char* in )
{
    while(*in != 0x00 ||  *in != 0x20)
    {
        **out = *in;
        *out += 1;
        in +=1;
    }
    if(*in == 0x20)
    {
        **out = 0x00;
    }
}

void fat16_get_full_relative_filename(struct fat_directory_item* item, char* out, int max_len)
{
    memoryset(out, 0x00, max_len);
    char*out_tmp = out;
    fat16_to_proper_string(&out_tmp, (const char*)item->filename);
    if(item->ext[0] != 0x00 && item->ext[0] != 0x20)
    {
        *out_tmp++ = '.';
        fat16_to_proper_string(&out_tmp, (const char*)item->ext);
        
    }


}
struct fat_directory_item* fat16_clone_directory_item(struct fat_directory_item* item, int size)
{   struct fat_directory_item* new_item = 0;
    if(size < sizeof(struct fat_directory_item))
    {
        return 0;
    }
     new_item= kzalloc(size);
    if(!new_item)
    {
        return 0;
    }
    memorycopy(new_item, item, size);
    return new_item;
}

struct fat_directory* fat16_load_fat_directory(struct disk* disk, struct fat_directory_item* item)
{
    int res =0;
    struct fat_directory* directory = 0;
    struct fat_private* fat_private = disk->fs_private;;
    if(!(item->attribute & FAT_FILE_SUBDIRECTORY))
    {
        res = -EINVARG;
        goto out;
    }

    directory = kzalloc(sizeof(struct fat_directory));;
    if(!directory)
    {
        res = -ENOMEM;
        goto out;
    }
    int cluster = fat16_get_first_cluster(item);
    int cluster_sector = fat16_cluster_to_sector(disk, cluster);
    int total_items = fat16_get_total_items_for_directory(disk, cluster_sector);
    directory->total = total_items;
    int directory_size = total_items * sizeof(struct fat_directory_item);
    directory->item = kzalloc(directory_size);
    if(!directory->item)
    {
        res = -ENOMEM;
        goto out;
    }
    res = fat16_read_internal(disk, cluster, 0x00, directory_size, directory->item);
    if(res != MYOS_ALL_OK)
    {
        goto out;
    }

out:
   if (res != MYOS_ALL_OK)
   {
       fat16_free_directory(directory);
   }
}

struct fat_item* fat16_new_fat_item_for_directory_item(struct disk* disk, struct fat_directory_item* item)
{
    struct fat_item* f_item = kzalloc(sizeof(struct fat_item));
    if(!f_item)
    {
        return 0;
    }
    if(item->attribute & FAT_FILE_SUBDIRECTORY)
    {
        f_item->directory = fat16_load_fat_directory(disk, item);
        f_item->type = FAT_ITEM_TYPE_DIRECTORY;
    }

    f_item->type = FAT_ITEM_TYPE_FILE;
    f_item->item = fat16_clone_directory_item(item, sizeof(struct fat_directory_item));

    return f_item;
    
    
}
struct fat_itme* fat16_find_item_in_directory(struct disk* disk, struct fat_directory* directory, const char* name)
{
    struct fat_item* f_item = 0;
    char tmp_filename[MYOS_MAX_PATH];

    for(int i =0;i<directory->total;i++)
    {
        fat16_get_full_relative_filename(&directory->item[i], tmp_filename, sizeof(tmp_filename));

        if(istrncmp(tmp_filename, name, sizeof(tmp_filename))==0)
        {
            f_item = fat16_new_fat_item_for_directory_item(disk, &directory->item[i]);
        }
    }
    return f_item;
}

struct fat_item* fat16_get_directory_entry(struct disk* disk, struct path_part* path)
{
    struct fat_private* fat_private = disk->fs_private;
    struct fat_item* current_item = 0;
    struct fat_item* root_item
     = fat16_find_item_in_directory(disk, &fat_private->root_directory, path->part);
    if(!root_item)
    {
        goto out;

    }

out:
    return current_item;
}
//
void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode)
{
    if(mode != FILE_MODE_READ)
    {
        return ERROR(-ERDONLY);
    }

    struct fat_file_descriptor* descriptor = kzalloc(sizeof(struct fat_file_descriptor));

    if(!descriptor)
    {
        return ERROR(-ENOMEM);
    }

    descriptor->item = fat16_get_directory_entry(disk, path);

    if(!descriptor->item)
    {
        
        return ERROR(-EIO);
    }

    descriptor->pos = 0;

    return descriptor;
}