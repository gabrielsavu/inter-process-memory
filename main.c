#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "memory.h"

int main() {

    void *heap = ol_init();

    printf("Heap: %p\n", heap);
    void *a = ol_malloc(heap, 4);
    void *b = ol_malloc(heap, 4);
    //void *c = ol_malloc(heap, 4);


    /*
    unsigned i;
    for (i = 0; i < 10; i ++) {
        if (fork() == 0) {
            void *s = ol_malloc(heap, 4);
            return 0;
        }
    }
    for (i = 0; i < 10; i ++) {
        wait(NULL);
    }
    */
    //ol_print_heap(heap);
    //ol_destroy(heap);
    return 0;
}