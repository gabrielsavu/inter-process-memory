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
short int ol_init(__heap_structure *heap) {
    int shm_fd;
    void *shm_ptr;
    heap->start = HEAP_FAILED;

    // Creating the shader memory file.
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd < 0) {
        return -1;
    }
    heap->header.shm_fd = shm_fd;

    // Resize it for the heap size.
    if (ftruncate(shm_fd, HEAP_SIZE) == -1) {
        shm_unlink(SHM_NAME);
        return -1;
    }
    heap->header.heap_size = HEAP_SIZE;

    // Mapping the shm to the process memory.
    shm_ptr = mmap(0, HEAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd , 0);
    if (shm_ptr == MAP_FAILED) {
        shm_unlink(SHM_NAME);
        return -1;
    }
    heap->start = shm_ptr;
    heap->header.first_section = heap->start + heap->header.heap_size;
    return 0;
}


int find_section_free(__heap_structure *heap) {
    void *sections_start = heap->start + heap->header.heap_size;
    void *first_section_start = heap->header.first_section;

    long long where = first_section_start - sections_start;

    if (where < 0) {
        return -1;
    }
    else if (where == 0) {
        return where;
    }

}

__section_structure ol_malloc(__heap_structure *heap, size_t size) {
    __section_structure section;





}

