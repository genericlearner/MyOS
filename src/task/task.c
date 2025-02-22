#include "task.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "status.h"
#include "kernel.h"
#include "process.h"
struct task* current_task = 0;
struct task* task_tail = 0;
struct task* task_head = 0;
int task_init(struct task* task, struct process* process);
struct task* task_current(){
    return current_task;
}
struct task* task_new(struct process* process){
    int res =0;
    struct task* task = kzalloc(sizeof(struct task));
    if (!task)
    {
        res = -ENOMEM;
        goto out;
    }
    res = task_init(task, process);
    if(res != MYOS_ALL_OK)
    {
        goto out;
    }
    if(task_head == 0)
    {
        task_head = task;
        task_tail = task;
        goto out;
    }

    task_tail->next = task;
    task->prev = task_tail;
    task_tail = task;
    
out:
    if (ISERR(res))
    {
        task_free(task);
        
        return ERROR(res);
    }
    return task;
}

struct task* task_get_next(){
    
    if(current_task->next == 0)
    {
        return task_head;
    }
    return current_task->next;
}
static void task_list_remove(struct task* task)
{
    if (task->prev)
    {
        task->prev->next = task->next;
    }

    if (task == task_head)
    {
        task_head = task->next;
    }
    if (task == task_tail)
    {
        task_tail = task->prev;
    }
    if(task == current_task)
    {
        current_task = task_get_next();
    }
}
int task_free(struct task* task)
{
    if (!task)
    {
        return -EINVARG;
    }
    if (task->page_directory)
    {
        paging_free_4gb(task->page_directory);
    }
    task_list_remove(task);
    kfree(task);
    return MYOS_ALL_OK;
}

int task_switch(struct task* task)
{
    if (!task)
    {
        return -EINVARG;
    }
    current_task = task;
    paging_switch(task->page_directory->directory_entry);
    return MYOS_ALL_OK;
}

int task_page(){
    user_registers();
    task_switch(current_task);
    return MYOS_ALL_OK;
}

void task_run_frist_ever_task(){
    if(current_task){
        panic("no current task");
    }
    task_switch(task_head);
    task_return(&task_head->registers);
}


int task_init(struct task* task, struct process* process)
{
    memset(task, 0, sizeof(struct task));

    task->page_directory = paging_new_4gb(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (!task->page_directory)
    {
        return -EIO;
    }
    task->registers.ip = MYOS_PROGRAM_VIRTUAL_ADDRESS;
    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.esp = MYOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START;

    task->process = process;
    return 0;
}