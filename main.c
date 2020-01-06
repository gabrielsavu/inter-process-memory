#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "memory.h"

int main() {

    void *heap = ol_init();


    printf("Heap: %p\n", heap);

    /*
    int *a = ol_malloc(heap, 4);
    int *b = ol_malloc(heap, 4);
    int *c = ol_malloc(heap, 4);

    ol_free(heap, b);
    ol_free(heap, c);
    int *d = ol_malloc(heap, 4);*/


    unsigned i;
    for (i = 0; i < 20; i ++) {
        pid_t c_cpid = fork();
        if (c_cpid < 0) {
            printf("Fork error.");
            return 0;
        }
        if (c_cpid == 0) {
            void *s = ol_malloc(heap, 4);
            return 0;
        }
    }
    for (i = 0; i < 20; i ++) {
        wait(NULL);
    }
    ol_print_heap(heap);
    //getchar();
    ol_destroy(heap);
    return 0;
}