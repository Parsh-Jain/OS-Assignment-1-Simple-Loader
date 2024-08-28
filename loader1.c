#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <elf.h>
#include <string.h>

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

/*
 * release memory and other cleanups
 */
void loader_cleanup(void* load_addr) {
    // Free the loaded ELF content
    free(load_addr);
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char* exe) {
    fd = open(exe, O_RDONLY);

    if (fd < 0) {
        perror("Failed to open ELF file");
        exit(1);
    }

    // 1. Load entire binary content into the memory from the ELF file.
    off_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    void* file_content = malloc(file_size);
    if (!file_content) {
        perror("Failed to allocate memory");
        close(fd);
        exit(1);
    }

    if (read(fd, file_content, file_size) != file_size) {
        perror("Failed to read the entire file");
        free(file_content);
        close(fd);
        exit(1);
    }

    close(fd);

    // Initialize ELF header and program header
    ehdr = (Elf32_Ehdr*)file_content;
    phdr = (Elf32_Phdr*)((char*)file_content + ehdr->e_phoff);



    // 2. Iterate through the PHDR table and find the section of PT_LOAD
    Elf32_Addr entry_point = ehdr->e_entry; // Entry point address
    void *virtual_mem;
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            // 3. Allocate memory of the size "p_memsz" using mmap function
           virtual_mem = mmap(NULL, phdr[i].p_memsz,
                                 PROT_READ | PROT_WRITE | PROT_EXEC,
                                  MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
            if (virtual_mem == MAP_FAILED) {
                perror("mmap failed");
                free(file_content);
                exit(EXIT_FAILURE);
            }

            // Copy the segment content to the allocated memory
            memcpy(virtual_mem, (char*)file_content + phdr[i].p_offset, phdr[i].p_filesz);

            // Zero out any remaining memory in the segment
            // memset((char*)virtual_mem + phdr[i].p_filesz, 0, phdr[i].p_memsz - phdr[i].p_filesz);
        }
    }

    // 4. Navigate to the entrypoint address
    // how to navigate ???


    
    // 5. Call the "_start" method
    int result = _start();
    printf("User _start return value = %d\n",result);


    // Clean up
    free(file_content);
}

/*
 * Main function
 */
int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <ELF Executable> \n", argv[0]);
        exit(1);
    }

    // 1. Carry out necessary checks on the input ELF file (can be extended)
    load_and_run_elf(argv[1]);

    // 3. Invoke the cleanup routine inside the loader
    loader_cleanup(NULL); // No specific load address to clean in this case

    return 0;
}
