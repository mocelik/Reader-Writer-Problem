#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <threads.h> // Preferred C11 feature over POSIX pthread

bool exit_requested = false;

int reader_thread(void *data) {
    while(!exit_requested) {
        thrd_sleep(&(struct timespec){.tv_sec=1}, NULL);
    }
}

int writer_thread(void *data) {
    while(!exit_requested) {
        thrd_sleep(&(struct timespec){.tv_sec=1}, NULL);
    }
}

int main(int argc, char **argv) {
    int numReaders = 3;
    int numWriters = 1;

    thrd_t reader_threads[numReaders];
    thrd_t writer_threads[numWriters];

    for (int i=0; i < numReaders; i++) {
        thrd_create(&reader_threads[i], reader_thread, NULL);
    }

    for (int i=0; i < numWriters; i++) {
        thrd_create(&writer_threads[i], writer_thread, NULL);
    }

    printf("Press enter to exit.");
    getchar();

    exit_requested = true;
    printf("\nExit requested. Waiting for %d reader(s) to exit... ", numReaders);
    fflush(stdout);
    for (int i=0; i < numReaders; i++) {
        thrd_join(reader_threads[i], NULL);
    }

    printf("Readers exited, waiting for %d writer(s) to exit... ", numWriters);
    fflush(stdout);
    for (int i=0; i < numWriters; i++) {
        thrd_join(writer_threads[i], NULL);
    }
    printf("Writers exited.\nDone.\n");

    return 0;
}
