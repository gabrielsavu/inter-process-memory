#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "internals.h"

void *ol_init(void) {
    int shm_fd;
    void *shm_ptr;
    __heap_header *heap;

    // Creating the shader memory file.
    shm_fd = shm_open(SHM_HEAP_NAME, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    if (shm_fd < 0) {
        return NULL;
    }

    // Resize it for the heap size.
    if (ftruncate(shm_fd, HEAP_SIZE) == -1) {
        shm_unlink(SHM_HEAP_NAME);
        return NULL;
    }
    // Mapping the shared memory file to the process memory.
    shm_ptr = mmap(0, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        shm_unlink(SHM_HEAP_NAME);
        return NULL;
    }

    heap = shm_ptr;
    heap->shm_fd = shm_fd;
    heap->heap_size = HEAP_SIZE;
    heap->first_section = NULL;
    heap->min.prev_section = NULL;
    heap->min.free_size = HEAP_SIZE - sizeof(__heap_header);

    pthread_mutexattr_t attr;

    if (pthread_mutexattr_init(&attr)) {
        shm_unlink(SHM_HEAP_NAME);
        return NULL;
    }

    if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) {
        shm_unlink(SHM_HEAP_NAME);
        return NULL;
    }

    if (pthread_mutex_init(&(heap->mutex), &attr)) {
        shm_unlink(SHM_HEAP_NAME);
        return NULL;
    }

    if(pthread_mutexattr_destroy(&attr)) {
        shm_unlink(SHM_HEAP_NAME);
        return NULL;
    }
    return heap;
}

void ol_destroy(void *heap) {
    if (heap == NULL) {
        return;
    }
    __heap_header *heap_header = heap;
    pthread_mutex_destroy(&(heap_header->mutex));
    munmap(heap, heap_header->heap_size);
    shm_unlink(SHM_HEAP_NAME);
}

short int __resize_heap(void *heap) {

    __heap_header *heap_header = heap;
    size_t actual_size = heap_header->heap_size;
    size_t new_size = actual_size * 2;
    int shm_fd = heap_header->shm_fd;

    munmap(heap + sizeof(pthread_mutex_t), heap_header->heap_size - sizeof(pthread_mutex_t));

    // Resize it for the new heap size.
    if (ftruncate(shm_fd, new_size) == -1) {
        shm_unlink(SHM_HEAP_NAME);
        return 0;
    }

    __heap_header *new_heap;
    void *shm_ptr;

    // Mapping the shared memory file to the process memory.
    shm_ptr = mmap(heap + sizeof(pthread_mutex_t), new_size - sizeof(pthread_mutex_t), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd , 0);
    if (shm_ptr == MAP_FAILED) {
        shm_unlink(SHM_HEAP_NAME);
        return 0;
    }

    new_heap = shm_ptr;
    new_heap->heap_size = new_size;
    return 1;
}

void *__is_in_heap(void *heap, void *address, size_t size) {
    __heap_header *heap_header = heap;
    void *end_of_section = address + sizeof(__section_header) + size;
    size_t length_between = end_of_section - heap;

    if (length_between > heap_header->heap_size) {
        return NULL;
    }
    return address;
}

__operational_result __find_section_free(void *heap, size_t size) {
    __heap_header *heap_header = heap;
    void *section = heap_header->first_section;
    __operational_result result;
    result.ptr = NULL;
    result.result = 0;



    // Check if there is a first section.
    if (section == NULL) {
        result.ptr = __is_in_heap(heap, (void *) heap + sizeof(__heap_header), size);
        result.result = 1;

        // Update the min section in the heap.
        heap_header->min.prev_section = result.ptr;
        heap_header->min.free_size = (heap + heap_header->heap_size) - result.ptr;
        return result;
    }

    // If there is space between header and first section.
    size_t space_before_first = (void *) section - ((void *) heap + sizeof(__heap_header));
    if (space_before_first >= size) {
        result.ptr = (void *) section;
        result.result = 2;
        return result;
    }

    if (heap_header->min.free_size >= size + sizeof(__section_header)) {
        if (heap_header->min.prev_section != NULL) {
            void *min = heap_header->min.prev_section;
            if (min != NULL) {
                __section_header *min_header = min;
                result.ptr = min;
                // We are between two sections.
                if (min_header->next != NULL) {
                    result.result = 4;
                }
                else {
                    result.result = 3;
                }

                return result;
            }
        }
    }

    __section_header *h_section = section;

    while (h_section != NULL) {


        __section_header *next = h_section->next;
        // Check if the section has a next.
        if (next == NULL) {
            result.ptr = h_section;
            result.result = 3;
            return result;
        }

        size_t space_between_sections = ((void *) next - ((void *) h_section + sizeof(__section_header) + h_section->section_size));

        // If the length between the 2 section in greater then the length needed then put the section there.
        if (space_between_sections >= size) {
            result.ptr = h_section;
            result.result = 4;
            return result;
        }
        h_section = h_section->next;
    }
    return result;
}

void *ol_malloc(void *heap, size_t size, char *name) {
    pthread_mutex_lock(&(((__heap_header *) heap)->mutex));
    __heap_header *heap_header = heap;
    __operational_result operational;

    retry:

    operational = __find_section_free(heap, size);

    if (operational.result == 0) {
        pthread_mutex_unlock(&(heap_header->mutex));
        return NULL;
    } else if (operational.result == 1) {
        __section_header *first_section = operational.ptr;
        if (first_section == NULL) {
            short int resize_result = __resize_heap(heap);
            if (resize_result == 1) goto retry;
            pthread_mutex_unlock(&(heap_header->mutex));
            return NULL;
        }
        heap_header->first_section = first_section;
        first_section->next = NULL;
        first_section->prev = NULL;
        first_section->section_size = size;
        if (name != NULL) memcpy(first_section->name, name, strlen(name) + 1);
        else sprintf(first_section->name, "%d", heap_header->counter);
        heap_header->counter ++;

        pthread_mutex_unlock(&(heap_header->mutex));
        return (void *) first_section + sizeof(__section_header);
    } else if (operational.result == 2) {
        __section_header *old_first_section = operational.ptr;
        __section_header *new_first_section = (void *) heap + sizeof(__heap_header);

        heap_header->first_section = new_first_section;

        new_first_section->next = old_first_section;
        new_first_section->prev = NULL;
        new_first_section->section_size = size;
        if (name != NULL) memcpy(new_first_section->name, name, strlen(name) + 1);
        else sprintf(new_first_section->name, "%d", heap_header->counter);
        heap_header->counter ++;

        old_first_section->prev = new_first_section;
        pthread_mutex_unlock(&(heap_header->mutex));
        return (void *) new_first_section + sizeof(__section_header);

    } else if (operational.result == 3) {
        __section_header *last_section = operational.ptr;

        size_t sizeof_last_section = sizeof(__section_header) + last_section->section_size;
        __section_header *new_section = (void *) last_section + sizeof_last_section;
        if (__is_in_heap(heap, new_section, size) == NULL) {
            short int resize_result = __resize_heap(heap);
            if (resize_result == 1) goto retry;
            pthread_mutex_unlock(&(heap_header->mutex));
            return NULL;
        }

        last_section->next = new_section;
        new_section->next = NULL;
        new_section->prev = last_section;
        new_section->section_size = size;
        if (name != NULL) memcpy(new_section->name, name, strlen(name) + 1);
        else sprintf(new_section->name, "%d", heap_header->counter);
        heap_header->counter ++;

        // Recalculate the min free section in the heap.

        size_t space_to_end = (heap + heap_header->heap_size) -
                              ((void *) new_section + sizeof(__section_header) + new_section->section_size);
        if (space_to_end <= heap_header->min.free_size) {
            heap_header->min.free_size = space_to_end;
            heap_header->min.prev_section = new_section;
        }

        pthread_mutex_unlock(&(heap_header->mutex));
        return (void *) new_section + sizeof(__section_header);
    } else {
        __section_header *last_section = operational.ptr;
        size_t sizeof_last_section = sizeof(__section_header) + last_section->section_size;

        __section_header *new_section = (void *) last_section + sizeof_last_section;

        __section_header *last_section_next = last_section->next;
        last_section_next->prev = new_section;

        new_section->section_size = size;
        new_section->prev = last_section;
        new_section->next = last_section_next;
        if (name != NULL) memcpy(new_section->name, name, strlen(name) + 1);
        else sprintf(new_section->name, "%d", heap_header->counter);
        heap_header->counter ++;

        last_section->next = new_section;

        // Recalculate the min free section in the heap.

        size_t space_to_next = (void *) new_section->next -
                               ((void *) new_section + sizeof(__section_header) + new_section->section_size);
        if (space_to_next <= heap_header->min.free_size) {
            heap_header->min.free_size = space_to_next;
            heap_header->min.prev_section = new_section;
        }


        pthread_mutex_unlock(&(heap_header->mutex));
        return (void *) new_section + sizeof(__section_header);
    }
}

void ol_free(void *heap, short int option, void *data) {
    if (data == NULL || heap == NULL) {
        return;
    }
    __heap_header *heap_header = heap;

    pthread_mutex_lock(&(heap_header->mutex));

    if (option == 1) {
        __section_header *section = heap_header->first_section;
        while (section != NULL) {
            __section_header *next = section->next;
            if (strcmp(section->name, data) == 0) {
                data = (void *) section + sizeof(__section_header);
                goto free;
            }
            section = next;
        }
        pthread_mutex_unlock(&(heap_header->mutex));
        return;
    }

    free:
    {

        __section_header *section_header = data - sizeof(__section_header);

        // Recalculate the min free section in the heap.

        size_t space_to_next;

        // Last element in the heap.
        if (section_header->next == NULL) space_to_next = (heap + heap_header->heap_size) - (void *) section_header;
            // we are between two sections.
        else space_to_next = section_header->next - (void *) section_header;

        if (section_header->section_size + sizeof(__section_header) <= heap_header->min.free_size) {
            heap_header->min.free_size = space_to_next;
            heap_header->min.prev_section = section_header->prev;
        }

        __section_header *prev = section_header->prev;
        __section_header *next = section_header->next;
        if (section_header == heap_header->first_section) {
            heap_header->first_section = next;
        }

        if (prev != NULL) {
            prev->next = next;
        }
        if (next != NULL) {
            next->prev = prev;
        }


        pthread_mutex_unlock(&(heap_header->mutex));
    }
}


void ol_print_heap(void *heap) {
    printf("---------\n");
    __heap_header *heap_header = heap;
    __section_header *section = heap_header->first_section;
    while(section != NULL) {
        printf("Address: %p, Name: %s\n", section, section->name);
        section = section->next;
    }
    printf("Size: %lu\n", heap_header->heap_size);
    printf("---------\n");
}
