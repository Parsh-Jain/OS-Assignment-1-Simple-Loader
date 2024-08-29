#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
/*
 * Release memory and other cleanups
 */
void loader_cleanup(void* load_addr) {

}
/*
 * Load and run the ELF executable file
 */

const char* get_p_type_name(uint32_t p_type) {
    switch (p_type) {
        case PT_NULL: return "PT_NULL";
        case PT_LOAD: return "PT_LOAD";
        case PT_DYNAMIC: return "PT_DYNAMIC";
        case PT_INTERP: return "PT_INTERP";
        case PT_NOTE: return "PT_NOTE";
        case PT_SHLIB: return "PT_SHLIB";
        case PT_PHDR: return "PT_PHDR";
        case PT_TLS: return "PT_TLS";
        default: return "Unknown";
    }
}
void load_and_run_elf(char** exe) {

    fd = open(*exe, O_RDONLY);

    if (fd < 0) {
        printf("Failed to open ELF file\n");
        exit(1);
    }

    // 1. Load entire binary content into the memory from the ELF file.
    size_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    // void* file_content = malloc(file_size);
    // if (!file_content) {
    //     printf("Failed to allocate memory\n");
    //     close(fd);
    //     exit(1);
    // }

    // if (read(fd, file_content, file_size) != file_size) {
    //     printf("Failed to read the entire file\n");
    //     free(file_content);
    //     close(fd);
    //     exit(1);
    // }

    ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
    read(fd, ehdr, sizeof(Elf32_Ehdr));
    phdr = (Elf32_Phdr*)malloc(sizeof(Elf32_Phdr)*(ehdr->e_phnum));

    lseek(fd, ehdr->e_phoff, SEEK_SET);
    read(fd, phdr, sizeof(Elf32_Phdr) * ehdr->e_phnum);
    
    for (int i = 0; i < ehdr->e_phnum; i++) {
        printf("Program Header %d: p_type = %u (%s)\n", i, phdr[i].p_type, get_p_type_name(phdr[i].p_type));
        if (phdr[i].p_type == PT_LOAD) {
            // 3. Allocate memory of the size "p_memsz" using mmap function
            void *virtual_mem = mmap((void*)phdr[i].p_vaddr, phdr[i].p_memsz,
                               PROT_READ | PROT_WRITE | PROT_EXEC,
                               MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
            if (virtual_mem == MAP_FAILED) {
                printf("mmap failed\n");
                exit(1);
            }
            if (ehdr->e_entry >= phdr[i].p_vaddr && ehdr->e_entry < phdr[i].p_vaddr + phdr[i].p_memsz) {
                size_t x = lseek(fd, phdr[i].p_offset, SEEK_SET);
                
                if(x==-1){
                    printf("error");
                    exit(1);
                }

                size_t z = read(fd, virtual_mem, phdr[i].p_memsz);
                if(z != phdr[i].p_memsz){
                        printf("error2");
                        exit(1);
                }
                
                int (*_start)(void) = (int (*)(void))(ehdr->e_entry);
                
                int result = _start();
                printf("inside\n");
                
                printf("User _start return value: %d\n", result);
                return;
            }
        }
    }
    printf("e_entry not valid\n");

}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <ELF Executable>\n", argv[0]);
        exit(1);
    }

    load_and_run_elf(argv);

    loader_cleanup(NULL);

    return 0;
}
