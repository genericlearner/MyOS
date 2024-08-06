#include "pathparser.h"
#include "kernel.h"
#include "string/string.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "status.h"


static int path_validate_format(const char* filename)
{
    if(filename==NULL)
    {
        
        return 0;
    }
    int length=strnlen(filename, MYOS_MAX_PATH);

    if(length < 3)
    {
        
        return 0;
    }
    if(!isdigit(filename[0]))
    {
       
        return 0;
    }
    return (memorycmp((void*) & filename[1],":/",2) == 0);
}

//The function below makes 0:/test.txt to test.txt
static int path_get_drive_by_path(const char** path)
{
    if(!path_validate_format(*path))
    {
        
        return -EBADPATH;
    }
    int drive_no = chartonum(*path[0]);

    //add 3 bytes to skip drive number
    *path+=3;
    return drive_no;
}

//creayes the root path
static struct path_root* create_root(int drive_no)
{
    struct path_root* path_r=kzalloc(sizeof(struct path_root));
    path_r->drive_no=drive_no;
    return path_r;
}

//optimization required..
//passing ex/example.txt 
static const char* get_path_part(const char** path)
{
    char* path_part= kzalloc(MYOS_MAX_PATH);
    int i=0;
    while(**path!='/'&&**path!=0x00)
    {
        path_part[i]=**path;
        *path+=1;
        i++;
    }
    if(**path=='/')
    {
        *path+=1;
    }
    if(i==0)
    {

        kfree(path_part);
        path_part=0;
    }

    return path_part;
}


//The parse_path_part function takes in the last_part and path to 
struct path_part* parse_path_part(struct path_part* last_part, const char** path)
{
    const char* path_part_str = get_path_part(path);
    if(!path_part_str)
    {
        return 0;
    }

    struct path_part* part=kzalloc(sizeof(struct path_part));
    part->part=path_part_str;
    part->next=0x00;

    if(last_part)
    {
        last_part->next=part;
    }


    return part;
}

void pathparser_free(struct path_root* root)
{
    struct path_part* part = root->first;
    while(part)
    {
        struct path_part* next_part=part->next;
        kfree((void*) part->part);
        kfree(part);
        part=next_part;
    }
    kfree(root);
}

struct path_root* pathparser_parse(const char* path, const char* current_directory_path)
{
    int res=0;
    const char* temp_path=path;
    struct path_root* path_root=0;

    if(strlen(path)>MYOS_MAX_PATH)
    {
        goto out;
    }

    res= path_get_drive_by_path(&temp_path);
   
   
    if(res<0)
    {
        goto out;
    }

    path_root = create_root(res);
    if(!path_root)
    {
     
        goto out;
    }
    struct path_part* first_part=parse_path_part(NULL, &temp_path);
    if(!first_part)
    {
        goto out;
    }
    path_root->first=first_part;

    struct path_part* part = parse_path_part(first_part,&temp_path);
    while(part)
    {
        part = parse_path_part(part, &temp_path);
    }

out:
    return path_root;

}