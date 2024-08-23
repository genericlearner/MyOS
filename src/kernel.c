#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "string/string.h"
#include "disk/disk.h"
#include "fs/pathparser.h"
#include "fs/file.h"
#include "disk/diskstreamer.h"


uint16_t* video_mem=0;
uint16_t terminal_row=0;
uint16_t terminal_col=0;

uint16_t terminal_make_char(char c, char color)
{
    return (color<<8)| c;
}
void terminal_putchar(int x, int y, char c, char colour)
{
    video_mem[(x*VGA_WIDTH)+y]=terminal_make_char(c,colour);
}

void terminal_writechar(char c, char colour)
{
    if(c=='\n')
        {   
            terminal_col=0;
            terminal_row+=1;
            return;
        }
    terminal_putchar(terminal_row,terminal_col,c,colour);
    terminal_col+=1;
    if(terminal_col>=VGA_WIDTH)
    {
        terminal_col=0;
        terminal_row+=1;
    }
}

void terminal_initialize()
{
    video_mem = (uint16_t*)(0xB8000);
    terminal_col=0;
    terminal_row=0;
    for(int x=0;x<VGA_HEIGHT;x++)
    {
        for(int y=0;y<VGA_WIDTH;y++)
        {
            terminal_putchar(x,y,' ',0);
        }
    }
}



void print(const char* str)
{
    size_t len=strlen(str);
    for(int i=0;i<len;i++)
    {
        terminal_writechar(str[i],15);
        
    }

}

static struct paging_4gb_chunk* kernel_chunk=0;
void kernel_main()
{
    terminal_initialize();
    print("Hello\nWorld");

    kheap_init();
    fs_init();
    //search and initialize disk
    disk_search_and_initialize();
    

    idt_init();



    kernel_chunk=paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT|PAGING_ACCESS_FROM_ALL);

    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));

    

    enable_paging();
    
     
    enable_interrupts();

    int fd = fopen("0:/hello.txt", "r");
   if (fd)
    {
       
        print("\nWe opened hello.txt\n");
        char buf[13];
        fread(buf, 13, 1, fd);
        buf[13] = 0x00;
        print(buf);
    }
    while(1){}
   
    
    //This program initializes the terminal by cleaning it and allowing protocols. Moreover, it initializes the kernel heap, the idt Structure, the paging, 
    //The paging switch, Then at last enable the interrupts. 
    //Kernel chunk is the memory assigned for paging by the memory set.
}