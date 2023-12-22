#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <threads.h> // Preferred C11 feature over POSIX pthread

#include "common.h"
#include "naive_solution.h"

int main(int argc, char **argv) {
    int num_readers = 10;
    int num_writers = 2;
    char shared_buffer[MAX_BUFFER_LEN];
    crc_t buffer_crc;
    strncpy(shared_buffer, "Initial Buffer Data", MAX_BUFFER_LEN);
    calculate_crc(shared_buffer, &buffer_crc);

    thrd_t reader_threads[num_readers];
    thrd_t writer_threads[num_writers];
    thread_data_t reader_data[num_readers];
    thread_data_t writer_data[num_writers];
    mtx_t reader_mutex;
    mtx_t writer_mutex;
    
    mtx_init(&reader_mutex, mtx_plain);
    mtx_init(&writer_mutex, mtx_plain);

    memset(reader_data, 0, sizeof(thread_data_t));
    memset(writer_data, 0, sizeof(thread_data_t));

    printf("Press enter to start. Afterwards, press enter to exit.\n");
    getchar();

    printf("Starting %d reader(s) and %d writer(s).\n", num_readers, num_writers);

    for (int i=0; i < num_readers; i++) {
        reader_data[i].reader_mutex = &reader_mutex;
        reader_data[i].writer_mutex = &writer_mutex;
        reader_data[i].buffer = shared_buffer;
        reader_data[i].buffer_crc = &buffer_crc;
        reader_data[i].id = i;
        reader_data[i].exit_requested = false;
        thrd_create(&reader_threads[i], reader_thread, &reader_data[i]);
    }

    for (int i=0; i < num_writers; i++) {
        writer_data[i].reader_mutex = &reader_mutex;
        writer_data[i].writer_mutex = &writer_mutex;
        writer_data[i].buffer = shared_buffer;
        writer_data[i].buffer_crc = &buffer_crc;
        writer_data[i].id = i;
        writer_data[i].exit_requested = false;
        thrd_create(&writer_threads[i], writer_thread, &writer_data[i]);
    }

    // wait until user presses [enter] to exit
    getchar();

    for (int i=0; i < num_readers; i++)
        reader_data[i].exit_requested = true;
    for (int i=0; i < num_writers; i++)
        writer_data[i].exit_requested = true;

    printf("\nExit requested. Waiting for %d reader(s) to exit... ", num_readers);
    fflush(stdout);
    for (int i=0; i < num_readers; i++) {
        thrd_join(reader_threads[i], NULL);
    }

    printf("Readers exited, waiting for %d writer(s) to exit... ", num_writers);
    fflush(stdout);
    for (int i=0; i < num_writers; i++) {
        thrd_join(writer_threads[i], NULL);
    }

    mtx_destroy(&reader_mutex);
    mtx_destroy(&writer_mutex);

    printf("Writers exited.\nDone.\n");

    return 0;
}
