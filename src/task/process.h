#ifndef PROCESS_H
#define PROCESS_H
#include <stdint.h>
#include "config.h"
#include "task.h"

struct process{
    uint16_t id;
    //the filename of the process
    char filename[MYOS_MAX_PATH];
    //the task of the process
    struct task* task;

    //the memory allocations of the program
    void* allocations[MYOS_MAX_PROGRAM_ALLOCATIONS];

    //THE PHYSICAL POINTER TO THE PROCESS MEMORY
    void* ptr;
    //physical pointer to the stack memory
    void* stack;
    //size of the data pointed by ptr
    uint32_t size;

};
int process_load_for_slot(const char* filename, struct process** process, int process_slot);

#endif