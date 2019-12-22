//
// Created by savu on 14.12.2019.
//

#include "memory.h"
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#define SHM_NAME "memory"
#define HEAP_SIZE 1024*4

__heap_structure ol_init() {
    int shm_fd;
    void *shm_ptr;
    __heap_structure heap;
    heap.start = HEAP_FAILED;

    // Creating the shader memory file.
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd < 0) {
        return heap;
    }

    // Resize it for the heap size.
    if (ftruncate(shm_fd, HEAP_SIZE) == -1) {
        shm_unlink(SHM_NAME);
        return heap;
    }
    heap.header.heap_size = HEAP_SIZE;

    // Mapping the shm to the process memory.
    shm_ptr = mmap(0, HEAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd , 0);
    if (shm_ptr == MAP_FAILED) {
        shm_unlink(SHM_NAME);
        return heap;
    }
    heap.start = shm_ptr;
    heap.first_section = heap.start + heap.header.heap_size;
    return heap;
}

__section_structure ol_malloc(size_t size) {
    __section_structure section;


}

