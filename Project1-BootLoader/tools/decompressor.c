#include <common.h>
#include <asm.h>
#include <os/kernel.h>
#include <os/task.h>
#include <os/string.h>
#include <os/loader.h>
#include <type.h>
#include"deflate/tinylibdeflate.h"

#define BUFFSIZE 10000
#define KERNEL_LOC 0x50201000
#define BOOT_LOADER_SIG_OFFSET 0x502001fe
#define DECOMPRESS_LOC 0x52040000

int main(int argc, char *argv[]) {
    int restore_nbytes = 0;
    uint16_t tasknum = 0;
    tasknum=*((uint16_t*) (BOOT_LOADER_SIG_OFFSET));
    task_info_t *task_info = (task_info_t*) (BOOT_LOADER_SIG_OFFSET-2 - sizeof(task_info_t));
    char* kernel_offset=(char*)(DECOMPRESS_LOC+((uint64_t)task_info->size));
    uint16_t compressed_kernel_size=*((uint16_t*) (BOOT_LOADER_SIG_OFFSET-2-tasknum*sizeof(task_info_t)-2));
    memcpy((uint8_t *)0x50205000, (uint8_t *)(kernel_offset), (uint64_t)compressed_kernel_size);
    struct libdeflate_decompressor * decompressor = deflate_alloc_decompressor();
    int checkpoint=deflate_deflate_decompress(decompressor,(uint8_t*)0x50205000, (int)compressed_kernel_size, (char*)KERNEL_LOC,4000, &restore_nbytes);
     
    if(checkpoint==1){
        bios_putstr("An error occurred during decompression.\n");
    }else if(checkpoint==3){
        bios_putstr("Decompression failed.\n");
    }
    return 0;


}