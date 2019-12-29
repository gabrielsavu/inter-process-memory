#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "memory.h"




int main() {


    printf("hello\n");
    __heap_structure *heap = ol_init();

    unsigned i;
    for (i = 0; i < 10; i ++) {
        if (fork() == 0) {
            __section_structure *s = ol_malloc(heap, 4);

            printf("%d: %p\n", i, s);
            return 0;
        }
    }
    for (i = 0; i < 10; i ++) {
        wait(NULL);
    }
    ol_destroy(heap);
    return 0;
}