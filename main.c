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
    
    mtx_init(&reader_mutex, mtx_timed);
    mtx_init(&writer_mutex, mtx_timed);

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
        thrd_create(&reader_threads[i], naive_reader_thread, &reader_data[i]);
    }

    for (int i=0; i < num_writers; i++) {
        writer_data[i].reader_mutex = &reader_mutex;
        writer_data[i].writer_mutex = &writer_mutex;
        writer_data[i].buffer = shared_buffer;
        writer_data[i].buffer_crc = &buffer_crc;
        writer_data[i].id = i;
        writer_data[i].exit_requested = false;
        thrd_create(&writer_threads[i], naive_writer_thread, &writer_data[i]);
    }

    // wait until user presses [enter] to exit
    getchar();

    for (int i=0; i < num_readers; i++)
        reader_data[i].exit_requested = true;
    for (int i=0; i < num_writers; i++)
        writer_data[i].exit_requested = true;

    printf("\nExit requested.\nWaiting for %d reader(s) to exit... ", num_readers);
    fflush(stdout);
    int total_reader_errors = 0;
    for (int i=0; i < num_readers; i++) {
        int exit_status = 0;
        thrd_join(reader_threads[i], &exit_status);
        total_reader_errors += exit_status;
    }

    printf("Done.\nWaiting for %d writer(s) to exit... ", num_writers);
    fflush(stdout);
    int total_writer_errors = 0;
    for (int i=0; i < num_writers; i++) {
        int exit_status = 0;
        thrd_join(writer_threads[i], &exit_status);
        total_writer_errors += exit_status;
    }

    mtx_destroy(&reader_mutex);
    mtx_destroy(&writer_mutex);

    printf("Done.\n");
    if (total_reader_errors != 0) {
        printf("There were %d reader thread errors.\n", total_reader_errors);
    }
    if (total_writer_errors != 0) {
        printf("There were %d writer thread errors.\n", total_writer_errors);
    }
    if (total_reader_errors == 0 && total_writer_errors == 0) {
        printf("All threads exited successfully.\n");
    }

    return 0;
}
