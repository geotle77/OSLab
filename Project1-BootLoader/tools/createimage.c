#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "deflate/tinylibdeflate.h"

#define IMAGE_FILE "./image"
#define ARGS "[--extended] [--vm] <bootblock> <executable-file> ..."

#define SECTOR_SIZE 512
#define BOOT_LOADER_SIG_OFFSET 0x1fe
#define OS_SIZE_LOC (BOOT_LOADER_SIG_OFFSET - 2)
#define BOOT_LOADER_SIG_1 0x55
#define BOOT_LOADER_SIG_2 0xaa
#define BUFFSIZE 10000
#define NBYTES2SEC(nbytes) (((nbytes) / SECTOR_SIZE) + ((nbytes) % SECTOR_SIZE != 0))
static char Data_File_Buf[BUFFSIZE];
static char Compressorbuf[BUFFSIZE];
/* TODO: [p1-task4] design your own task_info_t */
typedef struct {
    char name[16];
    int offset;
    int size;
    uint64_t entrypoint;
} task_info_t;

#define TASK_MAXNUM 16
static task_info_t taskinfo[TASK_MAXNUM];

/* structure to store command line options */
static struct {
    int vm;
    int extended;
} options;

/* prototypes of local functions */
static void create_image(int nfiles, char *files[]);
static void error(char *fmt, ...);
static void read_ehdr(Elf64_Ehdr *ehdr, FILE *fp);
static void read_phdr(Elf64_Phdr *phdr, FILE *fp, int ph, Elf64_Ehdr ehdr);
static uint64_t get_entrypoint(Elf64_Ehdr ehdr);
static uint32_t get_filesz(Elf64_Phdr phdr);
//static uint32_t get_memsz(Elf64_Phdr phdr);
static void write_segment(Elf64_Phdr phdr, FILE *fp, FILE *img,int *phyaddr);
static void write_kernel_segment(Elf64_Phdr phdr, FILE *fp, FILE *img, int *phyaddr,int *compressed_size);
static void write_padding(FILE *img, int *phyaddr, int new_phyaddr);
static void write_img_info(int nbytes_kernel, task_info_t *taskinfo,short tasknum, FILE *img);

int main(int argc, char **argv)
{
    char *progname = argv[0];

    /* process command line options */
    options.vm = 0;
    options.extended = 0;
    while ((argc > 1) && (argv[1][0] == '-') && (argv[1][1] == '-')) {
        char *option = &argv[1][2];

        if (strcmp(option, "vm") == 0) {
            options.vm = 1;
        } else if (strcmp(option, "extended") == 0) {
            options.extended = 1;
        } else {
            error("%s: invalid option\nusage: %s %s\n", progname,
                  progname, ARGS);
        }
        argc--;
        argv++;
    }
    if (options.vm == 1) {
        error("%s: option --vm not implemented\n", progname);
    }
    if (argc < 3) {
        /* at least 3 args (createimage bootblock main) */
        error("usage: %s %s\n", progname, ARGS);
    }
    create_image(argc - 1, argv + 1);
    
    return 0;
}

/* TODO: [p1-task4] assign your task_info_t somewhere in 'create_image' */
static void create_image(int nfiles, char *files[])
{
    int tasknum = nfiles - 2;// -2 because of kernel and bootblock
    int nbytes_kernel = 0;
    int phyaddr = 0;
    int kernel_size = 0;
    FILE *fp = NULL, *img = NULL;
    Elf64_Ehdr ehdr;
    Elf64_Phdr phdr;

    /* open the image file */
    img = fopen(IMAGE_FILE, "w");
    assert(img != NULL);

    /* for each input file */
    for (int fidx = 0; fidx < nfiles; ++fidx) {

        int taskidx = fidx - 3;

        /* open input file */
        fp = fopen(*files, "r");
        assert(fp != NULL);

        /* read ELF header */
        read_ehdr(&ehdr, fp);
        printf("0x%04lx: %s\n", ehdr.e_entry, *files);
        if(strcmp(*files,"decompressor")==0){
            strcpy(taskinfo[tasknum-1].name,*files);
            taskinfo[tasknum-1].offset = phyaddr;
            taskinfo[tasknum-1].entrypoint= get_entrypoint(ehdr);
            printf("taskinfo[%d].name=%s\t\n",tasknum-1,taskinfo[tasknum-1].name);
            printf("taskinfo[%d].offset=%x\t\n",tasknum-1,taskinfo[tasknum-1].offset);
            printf("taskinfo[%d].entrypoint=%lx\t\n",tasknum-1,taskinfo[tasknum-1].entrypoint);
        }
         if(taskidx >=0){ //初始化taskinfo数组
            strcpy(taskinfo[taskidx].name, *files);
            taskinfo[taskidx].offset = phyaddr;
            taskinfo[taskidx].entrypoint= get_entrypoint(ehdr);
            printf("taskinfo[%d].name=%s\t\n",taskidx,taskinfo[taskidx].name);
            printf("taskinfo[%d].offset=%x\t\n",taskidx,taskinfo[taskidx].offset);
            printf("taskinfo[%d].entrypoint=%lx\t\n",taskidx,taskinfo[taskidx].entrypoint);
        }
        /* for each program header */
        for (int ph = 0; ph < ehdr.e_phnum; ph++) {

            /* read program header */
            read_phdr(&phdr, fp, ph, ehdr);

            if (phdr.p_type != PT_LOAD) continue;


            if(strcmp(*files, "main") == 0){
                write_kernel_segment(phdr, fp, img, &phyaddr,&kernel_size);
                printf("kernel_size=%d\t\n",kernel_size);
            }
            /* write segment to the image */
            else 
                write_segment(phdr, fp, img, &phyaddr);

            /* update nbytes_kernel */
            if (strcmp(*files, "main") == 0) {
                nbytes_kernel += kernel_size;
            }
            if(strcmp(*files, "decompressor") == 0){
                nbytes_kernel += get_filesz(phdr);
                taskinfo[tasknum-1].size=get_filesz(phdr);
            }
            else if(taskidx>=0){
                taskinfo[taskidx].size = get_filesz(phdr);}
        }
        
        /* write padding bytes */
        /**
         * TODO:
         * 1. [p1-task3] do padding so that the kernel and every app program
         *  occupies the same number of sectors
         * 2. [p1-task4] only padding bootblock is allowed!
         */
        // printf("the  phyaddr is %x\t\n",phyaddr);
        if (strcmp(*files, "bootblock") == 0) {
            write_padding(img, &phyaddr, SECTOR_SIZE);
        }
        //project1-task3
        /*else{
             write_padding(img, &phyaddr, (1+fidx*15)*SECTOR_SIZE);
             if(taskidx>=1) printf("finishing  padding:%s\t\n",taskinfo[taskidx].name);
         }
         */

        printf("the newest phyaddr is %x\t\n",phyaddr);
        fclose(fp);
        files++;
    }
    write_img_info(nbytes_kernel, taskinfo, tasknum, img);

    fclose(img);
}

static void read_ehdr(Elf64_Ehdr * ehdr, FILE * fp)
{
    int ret;

    ret = fread(ehdr, sizeof(*ehdr), 1, fp);
    assert(ret == 1);
    assert(ehdr->e_ident[EI_MAG1] == 'E');
    assert(ehdr->e_ident[EI_MAG2] == 'L');
    assert(ehdr->e_ident[EI_MAG3] == 'F');
}

static void read_phdr(Elf64_Phdr * phdr, FILE * fp, int ph,
                      Elf64_Ehdr ehdr)
{
    int ret;

    fseek(fp, ehdr.e_phoff + ph * ehdr.e_phentsize, SEEK_SET);
    ret = fread(phdr, sizeof(*phdr), 1, fp);
    assert(ret == 1);
    if (options.extended == 1) {
        printf("\tsegment %d\n", ph);
        printf("\t\toffset 0x%04lx", phdr->p_offset);
        printf("\t\tvaddr 0x%04lx\n", phdr->p_vaddr);
        printf("\t\tfilesz 0x%04lx", phdr->p_filesz);
        printf("\t\tmemsz 0x%04lx\n", phdr->p_memsz);
    }
}

static uint64_t get_entrypoint(Elf64_Ehdr ehdr)
{
    return ehdr.e_entry;
}

static uint32_t get_filesz(Elf64_Phdr phdr)
{
    return phdr.p_filesz;
}

/*static uint32_t get_memsz(Elf64_Phdr phdr)
{
    return phdr.p_memsz;
}*/
static void write_segment(Elf64_Phdr phdr, FILE *fp, FILE *img, int *phyaddr)
{
    if (phdr.p_memsz != 0 && phdr.p_type == PT_LOAD) {
        /* write the segment itself */
        /* NOTE: expansion of .bss should be done by kernel or runtime env! */
        if (options.extended == 1) {
            printf("\t\twriting 0x%04lx bytes\n", phdr.p_filesz);
        }
        fseek(fp, phdr.p_offset, SEEK_SET);
        while (phdr.p_filesz-- > 0) {
            fputc(fgetc(fp), img);
            (*phyaddr)++;
        }
    }
}
static void write_kernel_segment(Elf64_Phdr phdr, FILE *fp, FILE *img, int *phyaddr,int *compressed_size)
{
    if (phdr.p_memsz != 0 && phdr.p_type == PT_LOAD) {
        /* write the segment itself */
        /* NOTE: expansion of .bss should be done by kernel or runtime env! */
        if (options.extended == 1) {
            printf("\t\twriting 0x%04lx bytes\n", phdr.p_filesz);
        }
        fseek(fp, phdr.p_offset, SEEK_SET);
        int i=0;
        int data_len = get_filesz(phdr);
        while (phdr.p_filesz-- > 0) {
            Data_File_Buf[i++]=(char)(fgetc(fp));
        }
        printf("the datalen is %x\n",data_len);
        //compress the kernel 
        deflate_set_memory_allocator((void * (*)(int))malloc, free);
        struct libdeflate_compressor * compressor = deflate_alloc_compressor(1);
        int out_nbytes = deflate_deflate_compress(compressor, Data_File_Buf, data_len, Compressorbuf, BUFFSIZE);
        printf("original: %x\n", data_len);
        printf("compressed: %x\n", out_nbytes);
        i=0;
        *compressed_size = out_nbytes;
        fseek(img, *phyaddr, SEEK_SET);
        while (out_nbytes -- > 0) {
            fputc((uint)Compressorbuf[i++], img);
            (*phyaddr)++;
        }
    }
}

static void write_padding(FILE *img, int *phyaddr, int new_phyaddr)
{
    if (options.extended == 1 && *phyaddr < new_phyaddr) {
        printf("\t\twrite 0x%04x bytes for padding\n", new_phyaddr - *phyaddr);
    }

    while (*phyaddr < new_phyaddr) {
        fputc(0, img);
        (*phyaddr)++;
    }
}

static void write_img_info(int nbytes_kernel, task_info_t *taskinfo,
                           short tasknum, FILE * img)
{
    // TODO: [p1-task3] & [p1-task4] write image info to some certain places
    uint16_t kernel_sectors = NBYTES2SEC(nbytes_kernel);
    uint64_t task_info_size = sizeof(task_info_t) * tasknum;
    uint16_t compressed_kernel_size = (nbytes_kernel-(uint16_t)taskinfo[tasknum-1].size);
    fseek(img, OS_SIZE_LOC, SEEK_SET);  // set fileptr to OS_SIZE_LOC
    if(fwrite(&kernel_sectors,sizeof(uint16_t), 1 , img)==1)     // write os size
            printf("the os kernel sctors is %d,writing in %x\t\n",kernel_sectors,OS_SIZE_LOC);
    
    fseek(img, BOOT_LOADER_SIG_OFFSET, SEEK_SET);
    if(fwrite(&tasknum, 2 , 1 , img)==1) // write tasknum
        printf("the tasknum is %d,wrting in %X\t\n",tasknum,OS_SIZE_LOC+2);
    
    fseek(img, OS_SIZE_LOC-task_info_size-2, SEEK_SET);
    if(fwrite(&compressed_kernel_size, sizeof(uint16_t), 1, img)==1)
        printf("the decompressor_size is %x,wrting in %lx\t\n",compressed_kernel_size,OS_SIZE_LOC-task_info_size-2);
    
    
    fseek(img, OS_SIZE_LOC-task_info_size, SEEK_SET);
    if(fwrite(taskinfo, sizeof(task_info_t), tasknum, img)==tasknum) // write task info;
        printf("the taskinfo wrting in %lx\t\n",OS_SIZE_LOC-task_info_size);
    printf("\n\n");
    
    for(int i=0;i<tasknum;i++){
    printf("name : %s\t\n",taskinfo[i].name);
    printf("the taskinfo offset is %x\t\n", taskinfo[i].offset);
    printf("the taskinfo size is %x\t\n",taskinfo[i].size);
    printf("the taskinfo entrypoint is %lx\t\n",taskinfo[i].entrypoint);
    printf("\n\n");
    }
    // write task info
    // NOTE: os size, infomation about app-info sector(s) ...
}

/* print an error message and exit */
static void error(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    if (errno != 0) {
        perror(NULL);
    }
    exit(EXIT_FAILURE);
}
