#ifndef MEMORYALLOC_MEMORY_H
#define MEMORYALLOC_MEMORY_H

#include <pthread.h>

/**
 * This function will create a shm file and map it to the process memory.
 * Also, it will initialize the mutex and some heap-header elements.
 * Technically this structure of the heap allows for more the one heap in the program.
 * But I was too lazy to generate different shm names.
 * @return the address of the heap.
 */
void *ol_init(void);

/**
 * It will destroy the mutex, unmap the memory allocated and unlink the shm file.
 * After this the heap is lost and all the memory freed.
 * @param heap the address of the heap structure.
 */
void ol_destroy(void *heap);

/**
 * It allocates n-bytes of memory in the heap.
 * @param heap the address of the heap structure.
 * @param size the desired size to be allocated
 * @param name the name of the section you wish to identify it.
 * It can be passed as NULL. If it is passed as NULL,
 * you can free the memory only with the exact address of the memory.
 * Otherwise you can pass the name of it.
 * !! THE NAME SHOULD BE UNIQUE.
 * !! If it is not, it will free the first section with the passed name.
 * !! No more the 10 characters.
 * @return the address of the allocated memory in heap.
 */
void *ol_malloc(void *heap, size_t size, char *name);

/**
 * This does what it says. It frees the memory.
 * @param heap the address of the heap structure.
 * @param option If this parameter has the 1 value it will free the memory after it's name.
 * @param address The address of the memory.
 */
void ol_free(void *heap, short int option, void *address);

/**
* It prints the content of the heap.
* @param heap the address of the heap structure.
*/
void ol_print_heap(void *heap);


#endif //MEMORYALLOC_MEMORY_H
