//
// Created by savu on 02.01.2020.
//

#ifndef MEMORYALLOC_INTERNALS_H
#define MEMORYALLOC_INTERNALS_H

#include <glob.h>
#include <pthread.h>

#define SHM_HEAP_NAME "memory"
#define HEAP_SIZE (1024)

/*
 * CODE LIST
 * 0 - uh oh... something went wrong.
 * 1 - the heap is clear, no need to update the last section, only the first section in heap header.
 * 2 - there is space between first section and the beginning of sections.
 * 3 - you are at the last section in the heap.
 * 4 - you are in between 2 sections, some math required.
 */
typedef struct {
    void *ptr;
    unsigned char result;
} __operational_result;

//----------------------------------------------------------------------------------------------

/*
 * |----------------------------------------------------------------------------------------------|
 * |          HEADER       |                  SECTIONS                                            |
 * |  (size, first section |  (each section contains: a header in which there is                  |
 * |  shm file descriptor, |  the next available section and the previous available section,      |
 * |  mutex)               |  the data and the size of it).                                       |
 * |----------------------------------------------------------------------------------------------|
 */

typedef struct {
    void *prev_section;
    size_t free_size;
} __min_section;

typedef struct {
    size_t section_size; /** The size of the section data. */
    void *prev; /** Previous section in the chain. */
    void *next; /** Next section in the chain. */
} __section_header;


typedef struct {
    pthread_mutex_t mutex; /** The mutex. Every time when changes are made the mutex is on. */
    size_t heap_size; /** The size of the entire heap. */
    void *first_section; /** The first section that is in the heap. */
    int shm_fd; /** Shared memory file descriptor. */
    __min_section min; /** The minimum section in the heap. */
} __heap_header;


//----------------------------------------------------------------------------------------------

#endif //MEMORYALLOC_INTERNALS_H
