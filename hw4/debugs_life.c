#include <unistd.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include "elf64.h"


#define STB_LOCAL 0
#define STB_GLOBAL 1


typedef struct Elf64_skeleton_struct
{
    //elf header
    Elf64_Ehdr ehdr;

    //section header
    Elf64_Shdr*  sht;

    //program header
    Elf64_Phdr* pht;

    //symtable
    Elf64_Sym* symtab;
    int symtab_size; 
    
    //shstrtab - Section header string table
    char* shstrtab;

    //strtab
    char* strtab;
} Elf64_skeleton;

void cleanElf(Elf64_skeleton* elf_ptr){
    free(elf_ptr->pht);
    free(elf_ptr->sht);
    free(elf_ptr->shstrtab);
    free(elf_ptr->strtab);
    free(elf_ptr->symtab);
}

int parseElf(Elf64_skeleton* elf_ptr, int fd){

    //get elf header
    read(fd, (void*)(elf_ptr->ehdr.e_ident), EI_NIDENT);
    read(fd, (void*)&(elf_ptr->ehdr.e_type), sizeof(Elf64_Ehdr)-sizeof(elf_ptr->ehdr.e_ident));
    
    //NULL all pointers of elf
    elf_ptr->pht = NULL; 
    elf_ptr->sht = NULL; 
    elf_ptr->shstrtab = NULL;
    elf_ptr->strtab = NULL;
    elf_ptr->symtab = NULL;

    //parse program headers
    if((elf_ptr->pht =(Elf64_Phdr*) malloc(sizeof(Elf64_Phdr)*elf_ptr->ehdr.e_phnum))==NULL){
        perror("Malloc Error, Program Header Table");
        cleanElf(elf_ptr);
        exit(1);
    }
    lseek(fd , elf_ptr->ehdr.e_phoff , SEEK_SET);
    if(read(fd ,(void*) elf_ptr->pht , (elf_ptr->ehdr.e_phentsize*elf_ptr->ehdr.e_phnum))!=elf_ptr->ehdr.e_phentsize*elf_ptr->ehdr.e_phnum){
        perror("read failed : PHDT");
        cleanElf(elf_ptr);
        exit(1);
    }


    //parse section headers
    if((elf_ptr->sht = malloc(sizeof(Elf64_Shdr) * (elf_ptr->ehdr.e_shnum))) == NULL){
        perror("Malloc Error , Section Header Table");
        cleanElf(elf_ptr);
        exit(1);
    }
    lseek(fd , elf_ptr->ehdr.e_shoff , SEEK_SET);
    if(read(fd , elf_ptr->sht,elf_ptr->ehdr.e_shentsize*elf_ptr->ehdr.e_shnum)!=elf_ptr->ehdr.e_shentsize*elf_ptr->ehdr.e_shnum){
        perror("read failed : SHDT");
        cleanElf(elf_ptr);
        exit(1);
    }

    
    
    //allocate shstrtab and read it form file
    if((elf_ptr->shstrtab = (char*)malloc(sizeof(char)*(elf_ptr->sht[elf_ptr->ehdr.e_shstrndx].sh_size)))==NULL){
        perror("Malloc Error , Section Header String Table");
        cleanElf(elf_ptr);
        exit(1);
    }
    lseek(fd, elf_ptr->sht[elf_ptr->ehdr.e_shstrndx].sh_offset , SEEK_SET);
    if(read(fd , elf_ptr->shstrtab, elf_ptr->sht[elf_ptr->ehdr.e_shstrndx].sh_size) <= 0){
        perror("read failed : SHSTRTAB");
        cleanElf(elf_ptr);
        exit(1);
    }
    
    //search symtab and strtab in sht and allocate them from memory
    for (int  i = 0; i < elf_ptr->ehdr.e_shnum; i++)
    {
        if(strcmp(elf_ptr->shstrtab+elf_ptr->sht[i].sh_name,".symtab")==0){
            if((elf_ptr->symtab = malloc(elf_ptr->sht[i].sh_size))==NULL){
                perror("Malloc Error , Symbol Table");
                cleanElf(elf_ptr);
                exit(1);
            }
            lseek(fd ,  elf_ptr->sht[i].sh_offset , SEEK_SET);
            if(read(fd , elf_ptr->symtab , elf_ptr->sht[i].sh_size) != elf_ptr->sht[i].sh_size){
                perror("read failed : SYMTAB");
                cleanElf(elf_ptr);
                exit(1);
            }
            elf_ptr->symtab_size = elf_ptr->sht[i].sh_size / elf_ptr->sht[i].sh_entsize;
        }
        if(strcmp(elf_ptr->shstrtab+elf_ptr->sht[i].sh_name,".strtab")==0){
            if((elf_ptr->strtab = malloc(elf_ptr->sht[i].sh_size))==NULL){
                perror("Malloc Error , String Table");
                cleanElf(elf_ptr);
                exit(1);
            }
            lseek(fd , elf_ptr->sht[i].sh_offset , SEEK_SET);
            if(read(fd , elf_ptr->strtab , elf_ptr->sht[i].sh_size) != elf_ptr->sht[i].sh_size){
                perror("read failed : STRTAB");
                cleanElf(elf_ptr);
                exit(1);
            }
        }
    }
    
    return 1;
    
}

pid_t run_target(const char* prog_name ,  char * const * args){
    pid_t pid;

    pid = fork();

    if(pid > 0){
        return pid;
    }
    else if(pid == 0){
        if(ptrace(PTRACE_TRACEME,0,NULL,NULL) < 0){
            perror("ptrace");
            exit(1);
        }
        execv(prog_name, args);
    }
    else{
        perror("fork");
        exit(1);
    }
}

int run_breakpoint(pid_t child_pid , unsigned long addr)
{
    int wait_status;
    struct user_regs_struct regs;

    
    /* Look at the word at the address we're interested in */
    unsigned long data = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)addr, NULL);
    /* Write the trap instruction 'int 3' into the address */
    unsigned long data_trap = (data & 0xFFFFFFFFFFFFFF00) | 0xCC;
    ptrace(PTRACE_POKETEXT, child_pid, (void*)addr, (void*)data_trap);

    do
    {
        /* Let the child run to the breakpoint and wait for it to reach it */
        ptrace(PTRACE_CONT, child_pid, NULL, NULL);
        wait(&wait_status);
        ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
        if(WIFEXITED(wait_status)){
            return -1;
        }
    } while (regs.rip != (unsigned long long)(addr + 1));
    
    /* Remove the breakpoint by restoring the previous data*/
    ptrace(PTRACE_POKETEXT, child_pid, (void*)addr, (void*)data);
    regs.rip -= 1;
    ptrace(PTRACE_SETREGS, child_pid, 0, &regs);
    return 0;
}

//NOTE: DEBUGGED PROG IS STOPPED
void debug_function_Syscalls(pid_t child_pid){
    int wait_status;
    struct user_regs_struct regs;

    /* Get Address of function Exit*/
    unsigned long addr_exit;
    ptrace(PTRACE_GETREGS, child_pid , NULL ,&regs);
    addr_exit = ptrace(PTRACE_PEEKTEXT , child_pid , (void*)regs.rsp , NULL);

    /* Look at the word at the exit address we're interested in */
    unsigned long data = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)addr_exit, NULL);

    /* Write the trap instruction 'int 3' into the exit address */
    unsigned long data_trap = (data & 0xFFFFFFFFFFFFFF00) | 0xCC;
    ptrace(PTRACE_POKETEXT, child_pid, (void*)addr_exit, data_trap);
    do
    {
        ptrace(PTRACE_SYSCALL, child_pid ,NULL , NULL );
        wait(&wait_status);
        if (WIFEXITED(wait_status))
        {
            perror("ptrace syscall");
            exit(1);
        }
        ptrace(PTRACE_GETREGS , child_pid , NULL , &regs);
        if (regs.rip == (unsigned long long)(addr_exit + 1) )
        {
            break;
        }
        ptrace(PTRACE_SYSCALL, child_pid ,NULL , NULL );
        wait(&wait_status);
        ptrace(PTRACE_GETREGS , child_pid , NULL , &regs);
        if ((long long)regs.rax < 0)
        {
            printf("PRF:: syscall in %llx returned with %lld\n", regs.rip - 2 , regs.rax);
        }
    } while (1);
    /* Remove the breakpoint by restoring the previous data and set rdx = 5 */
    ptrace(PTRACE_POKETEXT, child_pid, (void*)addr_exit, (void*)data);
    regs.rip -= 1;
    ptrace(PTRACE_SETREGS, child_pid, 0, &regs);    
}




int main(int argc, char * const *argv)
{
    
    if(argc < 3){
        fprintf(stderr ,"Error: Insufficient number off Arguments : %s <function name> <program name>\n",argv[0]);
        exit(1);
    }

    //open file 
    int file_fd = open(argv[2],O_RDONLY);
    if(file_fd == -1){
        fprintf(stderr, "Error: File openning failure, file not found : %s\n", argv[2]);
        exit(1);
    }

    Elf64_skeleton prog_elf;
    
    //parse file
    if(parseElf(&prog_elf, file_fd) == -1){
        cleanElf(&prog_elf);
        exit(1);
    }
    
    //find the the function in symtab
    Elf64_Sym* func = NULL;
    for (int i = 0; i < prog_elf.symtab_size; i++)
    {
        if(strcmp(argv[1],prog_elf.strtab + prog_elf.symtab[i].st_name)==0){
            func = &prog_elf.symtab[i];
            if (ELF64_ST_BIND(func->st_info) == STB_LOCAL)
            {
                printf("PRF:: local found!\n");
                cleanElf(&prog_elf);
                return 0;
            }
            if (ELF64_ST_BIND(func->st_info) != STB_GLOBAL)
            {
                perror("unkown bind type");
                cleanElf(&prog_elf);
                exit(1);
            }
        }
    }
    if(func == NULL){
        printf("PRF:: not found!\n");
        cleanElf(&prog_elf);
        return 0;
    }
    

    
    //get func adress
    unsigned long addr = 0;
    addr = func->st_value;
    /*
    for (int i = 0; i < prog_elf.ehdr.e_shnum; i++)
    {
        if(strcmp(prog_elf.shstrtab + prog_elf.sht[i].sh_name, ".text")){
            addr = prog_elf.sht[i].sh_offset + func->st_value;
        }
    }
    if(addr == 0){
        perror(".text not found");
        cleanElf(&prog_elf);
        exit(1);
    }*/

    //run debbuged program
    pid_t buggy_pid;
    buggy_pid = run_target(argv[2], argv + 2);
    wait(NULL);
    //wait for func in program and debbug it
    while(run_breakpoint(buggy_pid,addr)==0){
        debug_function_Syscalls(buggy_pid);
    }        

    //clean data and exit
    cleanElf(&prog_elf);
    return 0;
}
