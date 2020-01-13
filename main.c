#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include "src/memory.h"

int main(int argc, char **argv) {


    /**
     * This example is not a great one because we allocate
     * all the memory first and the release it.
     */
    void *heap = ol_init();

    unsigned i;
    for (i = 1; i < argc; i ++) {
        pid_t c_pid = fork();
        if (c_pid < 0) {
            perror("Fork failed");
            return errno;
        }
        if (c_pid == 0) {
            int choice = (int) argv[i][0] - 48;
            int elem = atoi(argv[i] + 2);
            if (choice == 1) {
                ol_malloc(heap, elem, NULL);
            }

            exit(0);
        }

    }
    for (i = 1; i <= argc; i ++) {
        wait(NULL);
    }

    for (i = 1; i < argc; i ++) {
        int choice = (int) argv[i][0] - 48;
        int elem = atoi(argv[i] + 2);
        if (choice == 0) {
            char *temp = malloc(10 * sizeof(char));
            sprintf(temp, "%d", elem);
            ol_free(heap, 1, temp);
        }
    }

    ol_print_heap(heap);
    ol_destroy(heap);
    return 0;
}