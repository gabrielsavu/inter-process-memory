#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "memory.h"

int main() {


    __heap_structure heap;
    ol_init(&heap);

    printf("%p", heap.start);
    return 0;
}