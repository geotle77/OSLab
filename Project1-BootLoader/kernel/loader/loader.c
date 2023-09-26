#include <os/task.h>
#include <os/string.h>
#include <os/kernel.h>
#include <type.h>
#define NUM_OF_BLCOKS 15
#define APP_INFO_ADDR 0x5020015c
#define APP_NUM_ADDR 0x502001fe
#define TASK_INFO_SIZE 32
//uint64_t load_task_img(int taskid)
//{
    /**
     * TODO:
     * 1. [p1-task3] load task from image via task id, and return its entrypoint
     * 
     * 2. [p1-task4] load task via task name, thus the arg should be 'char *taskname'
     */
    //uint64_t pro_entry_address=TASK_MEM_BASE + TASK_SIZE * taskid;
    //bios_sd_read(TASK_MEM_BASE + TASK_SIZE * taskid, 15, 1 + 15 * (taskid + 1));
    //return pro_entry_address;
//}
uint64_t load_task_img(char *task_name)
{
    int taskid =  strcmp(task_name, "bss")   == 0 ? 0 :
                 strcmp(task_name, "auipc") == 0 ? 1 :
                 strcmp(task_name, "data")  == 0 ? 2 :
                 strcmp(task_name, "2048")  == 0 ? 3 : -1;
    int task_num=(int)*((uint16_t*) APP_NUM_ADDR);
    if(taskid==-1){
        bios_putstr("reading taskname faild!\t\n");
    }
    if(task_num == 5){
        bios_putstr("reading task_num success!\t\n");
        }
    if(taskid<task_num){
    task_info_t *task_info = (task_info_t*)(APP_INFO_ADDR + (uint64_t)(TASK_INFO_SIZE*taskid));
    if(strcmp(task_name, task_info->name) == 0){
        bios_putstr("reading task_info_t success!\t\n");
        }
    int blocknums=NBYTES2SEC(task_info->offset%SECTOR_SIZE+task_info->size);
    uint64_t pro_start_memaddr=task_info->entrypoint;
    bios_sd_read(task_info->entrypoint,blocknums,task_info->offset / SECTOR_SIZE);
    memcpy((uint8_t *)pro_start_memaddr, (uint8_t *)(task_info->entrypoint + task_info->offset%SECTOR_SIZE), (uint64_t)task_info->size);
    return pro_start_memaddr;
    }
    else 
        return -1;
}