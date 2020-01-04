//
// Created by savu on 14.12.2019.
//

#ifndef MEMORYALLOC_MEMORY_H
#define MEMORYALLOC_MEMORY_H

#include <inttypes.h>
#include <pthread.h>


void *ol_init(void);
void ol_destroy(void *);
void *ol_malloc(void *, size_t);
void ol_free(void *, void *);
void ol_print_heap(void *);


#endif //MEMORYALLOC_MEMORY_H
