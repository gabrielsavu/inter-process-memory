//
// Created by savu on 14.12.2019.
//

#ifndef MEMORYALLOC_MEMORY_H
#define MEMORYALLOC_MEMORY_H

#include <inttypes.h>
#include <pthread.h>

/*
 * |----------------------------------------------------------------------------------------------
 * |          HEADER       |                  SECTIONS
 * |  (size, first section |  (each section contains: a header in which there is
 * |  ....., etc)          |  the next available section in number of bytes and the size of the
 * |                       |  current section, and the data).
 * |----------------------------------------------------------------------------------------------
 */

#define HEAP_FAILED 0x0

typedef struct {
    size_t section_size; /** The size of the section data. */
    void *prev; /** Previous section in the chain. */
    void *next; /** Next section in the chain. */
} __section_header;

/* **WARNING** Every data added to this structure MUST be added to the header.
 * All the calculation depends on this structure.
 */

typedef struct {
    __section_header header; /** The header of the section. */
    void *data; /** Data inside the section (the size of it is in the header). */
} __section_structure;

typedef struct {
    size_t heap_size; /** The size of the entire heap. */
    __section_structure *first_section; /** The first section that is in the heap. */
    int shm_fd; /** Shared memory file descriptor. */
    pthread_mutex_t mutex; /** The mutex... */
} __heap_header;


/* **WARNING** Every data added to this structure MUST be added to the header.
 * All the calculation depends on this structure.
 */
typedef struct {
    __heap_header header; /** The header of the heap. */
    void *start; /** The starting address of the heap. */
} __heap_structure;

__heap_structure *ol_init(void);
void ol_destroy(__heap_structure *);
__section_structure *ol_malloc(__heap_structure *, size_t);
void ol_free(__heap_structure *, __section_structure *);
void *ol_get(__section_structure);

/*
#define ol_set_buffer(section, type_t, val) ({ \
    unsigned char bar; \
    section->data = (type_t*)val; \
    })
*/
#endif //MEMORYALLOC_MEMORY_H
