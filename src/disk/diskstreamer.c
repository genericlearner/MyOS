#include "diskstreamer.h"
#include "memory/heap/kheap.h"
#include "config.h"
#include <stdbool.h>
//initializing a disk streamer
struct disk_stream* diskstreamer_new(int disk_id)
{
    struct disk* disk =disk_get(disk_id);
    if(!disk)
    {
        return 0;
    }
    struct disk_stream* streamer= kzalloc(sizeof(struct disk_stream));

    streamer->pos=0;
    streamer->disk=disk;

    return streamer;
};

int diskstreamer_seek(struct disk_stream* stream, int pos)
{
    stream->pos=pos;
    return 0;
}
//read the stream
//using recursion to read through the total size of the stream. This could be dangerous and needs to be optimized
int diskstreamer_read(struct disk_stream* stream, void* out, int total)
{
    int sector = stream->pos / MYOS_SECTOR_SIZE;
    int offset = stream->pos % MYOS_SECTOR_SIZE;
    int total_read = total;
    bool overflow = (offset + total) >= MYOS_SECTOR_SIZE;
    char buf[MYOS_SECTOR_SIZE];

    if(overflow)
    {
        total_read -=(offset + total_read) - MYOS_SECTOR_SIZE;
    }

    int res = disk_read_block(stream->disk,sector,1,buf);
    if(res<0)
    {
        goto out;
    }
    
    for(int i=0; i <total_read; i++)
    {
        *(char*)out++ = buf[offset+i];
    }
    //adjust the stream

    stream->pos += total_read;
    
    if(overflow)
    {
        res=diskstreamer_read(stream, out, total-MYOS_SECTOR_SIZE);
    }
    out:
        return res;

}

//closes the stream;
void diskstreamer_close (struct disk_stream* stream)
{
    kfree(stream);
}