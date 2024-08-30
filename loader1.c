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

void load_and_run_elf(char** exe) {

    fd = open(*exe, O_RDONLY);

    if (fd < 0) {
        printf("Failed to open ELF file\n");
        exit(1);
    }

    // 1. Load entire binary content into the memory from the ELF file.
    size_t file_size = lseek(fd, 0, SEEK_END);
    if (!file_size) {
        printf("ERROR\n");
        exit(1);    
    }
    lseek(fd, 0, SEEK_SET);

    ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
    read(fd, ehdr, sizeof(Elf32_Ehdr));
    phdr = (Elf32_Phdr*)malloc(sizeof(Elf32_Phdr)*(ehdr->e_phnum));

    lseek(fd, ehdr->e_phoff, SEEK_SET);
    read(fd, phdr, sizeof(Elf32_Phdr) * ehdr->e_phnum);
    

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            // 3. Allocate memory of the size "p_memsz" using mmap function
            void *virtual_mem = mmap(NULL, phdr[i].p_memsz,
                               PROT_READ | PROT_WRITE | PROT_EXEC,
                               MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
            if (virtual_mem == MAP_FAILED) {
                printf("mmap failed\n");
                exit(1);
            }
            if (ehdr->e_entry >= phdr[i].p_vaddr && ehdr->e_entry < phdr[i].p_vaddr + phdr[i].p_memsz) {
                off_t x = lseek(fd, phdr[i].p_offset, SEEK_SET);
                
                if(x < 0){
                    printf("error");
                    exit(1);
                }

                size_t z = read(fd, virtual_mem, phdr[i].p_memsz);
                if(z != phdr[i].p_memsz || z <= 0){
                        printf("error2");
                        exit(1);
                }
                
                int (*_start)(void) = (int (*)(void))(ehdr->e_entry);
                // _start* f = (_start*)(ehdr->e_entry);
                printf("inside\n");

                int result = _start();
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
