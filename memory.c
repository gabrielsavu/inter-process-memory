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
#define HEAP_SIZE 1024*10

/*
 * CODE LIST
 * 0 - uh oh... something went wrong.
 * 1 - the heap is clear, no need to update the last section, only the first section in heap header.
 * 2 - you are at the last section in the heap.
 * 3 - you are in between 2 sections, some math required.
 */
typedef struct {
    void *ptr;
    unsigned char result;
} __operational_result;


__heap_structure *ol_init(void) {
    int shm_fd;
    void *shm_ptr;
    __heap_structure *heap;

    // Creating the shader memory file.
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd < 0) {
        return NULL;
    }

    // Resize it for the heap size.
    if (ftruncate(shm_fd, HEAP_SIZE) == -1) {
        shm_unlink(SHM_NAME);
        return NULL;
    }
    // Mapping the shared memory file to the process memory.
    shm_ptr = mmap(0, HEAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd , 0);
    if (shm_ptr == MAP_FAILED) {
        shm_unlink(SHM_NAME);
        return NULL;
    }

    heap = shm_ptr;
    heap->header.shm_fd = shm_fd;
    heap->header.heap_size = HEAP_SIZE;
    heap->start = shm_ptr;
    heap->header.first_section = NULL;

    // Initialize mutex.
    if (pthread_mutex_init(&(heap->header.mutex), NULL)) {
        shm_unlink(SHM_NAME);
        return NULL;
    }
    return heap;
}

void ol_destroy(__heap_structure *heap) {
    if (heap == NULL) {
        return;
    }
    munmap(heap, heap->header.heap_size);
    shm_unlink(SHM_NAME);
}

__operational_result find_section_free(__heap_structure *heap, size_t length) {

    __section_structure *section = heap->header.first_section;
    __operational_result result;
    result.ptr = NULL;
    result.result = 0;

    // Check if there is a first section.
    if (section == NULL) {
        result.ptr = heap->start + sizeof(heap->header);
        result.result = 1;
        return result;
    }

    while (section != NULL) {
        __section_structure *next = section->header.next;
        // Check if the section has a next.
        if (next == NULL) {
            result.ptr = section;
            result.result = 2;
            return result;
        }
        // If the length between the 2 section in greater then the length needed then put the section there.
        if ((next - (section + sizeof(section->header) + section->header.section_size)) >= length) {
            printf("here! ");
            result.ptr = section;
            result.result = 3;
            return result;
        }
        section = section->header.next;
    }
    return result;
}

__section_structure *ol_malloc(__heap_structure *heap, size_t size) {
    pthread_mutex_lock(&(heap->header.mutex));

    __operational_result operational = find_section_free(heap, size);

    if (operational.result == 0) {
        pthread_mutex_unlock(&(heap->header.mutex));
        return NULL;
    }
    else if (operational.result == 1) {
        heap->header.first_section = operational.ptr;
        __section_structure *section = operational.ptr;
        section->header.next = NULL;
        section->header.prev = NULL;
        section->header.section_size = size;
        section->data = NULL;
        pthread_mutex_unlock(&(heap->header.mutex));
        return section;
    }
    else if (operational.result == 2) {
        __section_structure *last_section = operational.ptr;
        size_t sizeof_last_section = sizeof(last_section->header) + last_section->header.section_size;
        __section_structure *new_section = last_section + sizeof_last_section;
        last_section->header.next = new_section;
        new_section->header.next = NULL;
        new_section->header.prev = last_section;
        new_section->header.section_size = size;
        new_section->data = NULL;
        pthread_mutex_unlock(&(heap->header.mutex));
        return new_section;
    }
    else {
        __section_structure *last_section = operational.ptr;
        size_t sizeof_last_section = sizeof(last_section->header) + last_section->header.section_size;

        __section_structure *new_section = last_section + sizeof_last_section;
        new_section->header.section_size = size;
        new_section->header.prev = last_section;
        new_section->header.next = last_section->header.next;
        new_section->header.section_size = size;
        new_section->data = NULL;

        last_section->header.next = new_section;
        pthread_mutex_unlock(&(heap->header.mutex));
        return new_section;
    }
}

void ol_free(__heap_structure *heap, __section_structure *section) {
    if (section == NULL || heap == NULL) {
        return;
    }
    __section_structure *prev = section->header.prev;
    __section_structure *next = section->header.next;
    if (section == heap->header.first_section) {
        heap->header.first_section = next;
    }

    if (prev != NULL) {
        prev->header.next = next;
    }
    if (next != NULL) {
        next->header.prev = prev;
    }
}