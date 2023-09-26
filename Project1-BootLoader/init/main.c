#include <common.h>
#include <asm.h>
#include <os/kernel.h>
#include <os/task.h>
#include <os/string.h>
#include <os/loader.h>
#include <type.h>

#define VERSION_BUF 50

int version = 2; // version must between 0 and 9
char buf[VERSION_BUF];
uint16_t task_num;
// Task info array
task_info_t tasks[TASK_MAXNUM];

static int bss_check(void)
{
    for (int i = 0; i < VERSION_BUF; ++i)
    {
        if (buf[i] != 0)
        {
            return 0;
        }
    }
    return 1;
}

static void init_jmptab(void)
{
    volatile long (*(*jmptab))() = (volatile long (*(*))())KERNEL_JMPTAB_BASE;

    jmptab[CONSOLE_PUTSTR]  = (long (*)())port_write;
    jmptab[CONSOLE_PUTCHAR] = (long (*)())port_write_ch;
    jmptab[CONSOLE_GETCHAR] = (long (*)())port_read_ch;
    jmptab[SD_READ]         = (long (*)())sd_read;
}

static void init_task_info(void)
{
    // TODO: [p1-task4] Init 'tasks' array via reading app-info sector
    // NOTE: You need to get some related arguments from bootblock first
    // Init tasks
    task_info_t *tasks_mem_ptr = (task_info_t *)(0x5020015c);
    // copy task_info from mem to bss
    // since mem will be overwritten by the first app
    task_num = *(uint16_t *)0x502001fe;
    memcpy((uint8_t *)tasks, (uint8_t *)tasks_mem_ptr, task_num * sizeof(task_info_t));
}

/************************************************************/
/* Do not touch this comment. Reserved for future projects. */
/************************************************************/

int main(void)
{
    // Check whether .bss section is set to zero
    int check = bss_check();

    // Init jump table provided by kernel and bios(ΦωΦ)
    init_jmptab();

    // Init task information (〃'▽'〃)
    init_task_info();

    // Output 'Hello OS!', bss check result and OS version
    char output_str[] = "bss check: _ version: _\n\r";
    char output_val[2] = {0};
    int i, output_val_pos = 0;

    output_val[0] = check ? 't' : 'f';
    output_val[1] = version + '0';
    for (i = 0; i < sizeof(output_str); ++i)
    {
        buf[i] = output_str[i];
        if (buf[i] == '_')
        {
            buf[i] = output_val[output_val_pos++];
        }
    }

    bios_putstr("Hello OS!\n\r");
    bios_putstr(buf);

    //project1-task2
    /*  int c;
        while((c=bios_getchar())){
            if((c!=-1)){
                if(c=='\r')
            {
                bios_putstr("\n\r");
            }
            else 
                {bios_putchar(c);}
            }
        }
        */
    // TODO: Load tasks by either task id [p1-task3] or task name [p1-task4],
    //   and then execute them.
      //project1-task3
      /*
      int c;
      while((c=bios_getchar())){
          
            if(c!=-1){
              bios_putchar(c);
              bios_putstr("\n\r");
              if(c<'0'+task_num && c>='0'){
                ((void (*)())load_task_img(c-'0'))();
              }
            else if(c=='\r')
                    bios_putstr("\n\r");
            else 
                    bios_putstr("Invalid taskid\n\r");
          }
      }
      */
    //project1-task4

    char namebuf[16];
    int c;
    int j=0;
    while(1){
    while((c=bios_getchar())){
        if(c!=-1){
        if(((c>='a'&& c<='z')||(c>='0'&&c<='9')) && c!='\r'){
            bios_putchar(c);
            if(c!='\r')
                namebuf[j++]=c;
        }
        else if(c=='\r'){
            namebuf[j]='\0';
            bios_putstr("\n\r");
            int checkpoint=0;
            for(int i=0;i<task_num;i++){
                if(strcmp(namebuf, tasks[i].name)==0)
                    {checkpoint=1;
                    break;
                    }
                }
            if(checkpoint){
                ((void (*)())load_task_img(namebuf))();
                }
            else 
                bios_putstr("Invalid taskname!\n\r");
            j=0;
            for(;namebuf[j]!='\0';namebuf[j++]='\0'){;}
            j=0;
            }
        }
         
      }
    }

    // Infinite while loop, where CPU stays in a low-power state (QAQQQQQQQQQQQ)
    while (1)
    {
        asm volatile("wfi");
    }

    return 0;
}
