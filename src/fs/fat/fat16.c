#include "fat16.h"
#include "status.h"
#include "disk/disk.h"
#include "disk/diskstreamer.h"
#include "string/string.h"
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

struct fat_item_descriptor
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
    struct disk_stream* fat_read_stream;
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
    memset(private, 0, sizeof(struct fat_private));
private
    ->cluster_read_stream = diskstreamer_new(disk->id);
private
    ->fat_read_stream = diskstreamer_new(disk->id);
private
    ->directory_stream = diskstreamer_new(disk->id);
}

int fat16_sector_to_absolute(struct disk *disk, int sector)
{
    return sector * disk->sector_size;
}
//fat16 resolve function returns EIO for now
int fat16_resolve(struct disk* disk)
{
    
}
//
void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode)
{
    return 0;
}