//
// Created by savu on 14.12.2019.
//

#ifndef MEMORYALLOC_MEMORY_H
#define MEMORYALLOC_MEMORY_H

#include <lzma.h>

/*
 * |----------------------------------------------------------------------------------------------
 * |          HEADER       |                  SECTIONS
 * |  (size, first section |  (each section contains: a header in which there is
 * |  ....., etc)          |  the next available section in number of bytes and the size of the
 *                         |  current section, and the data).
 * |----------------------------------------------------------------------------------------------
 */

#define HEAP_FAILED 0x0

typedef struct {
    // The size of the section data.
    size_t section_size;
    // Number of bytes of the next available section.
    size_t next_section;
} __section_header;

typedef struct {
    // The header of the section.
    __section_header header;
    // Data inside the section (the size of it is in the header).
    void *data;
} __section_structure;

typedef struct {
    // The size of the entire heap.
    size_t heap_size;
    // Address with first section starts.
    size_t first_section;
} __heap_header;

typedef struct {
    // The header of the heap.
    __heap_header header;
    // The first section that is in the heap.
    __section_structure *first_section;
    // The starting address of the heap.
    void *start;
} __heap_structure;

__heap_structure ol_init();
void ol_destroy(__heap_structure *);
__section_structure ol_malloc(size_t);
void ol_free(__section_structure);
void *ol_get(__section_structure);

#endif //MEMORYALLOC_MEMORY_H
