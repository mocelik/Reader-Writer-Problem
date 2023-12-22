#include <stdio.h>
#include <time.h>

#include "common.h"

// prints current time after printing supplied user string
// function used during debugging
static void print_time_now(const char *usr_string) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char s[64];
    strftime(s, sizeof(s), "%c", tm);
    printf("%s %s\n", usr_string, s);
}

int naive_reader_thread(void *data) {
    static int num_active_readers = 0;
    thread_data_t *tdata=(thread_data_t*) data;
    struct timespec reader_work_period = {.tv_nsec = 500 * 1000 * 1000};
    int exit_status = 0;

    while(!tdata->exit_requested) {

        mtx_lock(tdata->reader_mutex);
        num_active_readers++;
        if (num_active_readers == 1) {
            // 'n' readers act as 1 writer
            // let the first reader lock the writer mutex
            // and let the last reader unlock it
            mtx_lock(tdata->writer_mutex);
        }
        mtx_unlock(tdata->reader_mutex);

        printf("Reader[%d] read buffer [%s]\n", tdata->id, tdata->buffer);
        if (!is_buffer_valid(tdata->buffer, tdata->buffer_crc)) {
            printf("Reader[%d] detected error! Buffer has been incorrectly modified.\n", tdata->id);
            tdata->exit_requested = true;
            exit_status = 1;
        }

        mtx_lock(tdata->reader_mutex);
        num_active_readers--;
        if (num_active_readers == 0) {
            mtx_unlock(tdata->writer_mutex);
        }
        mtx_unlock(tdata->reader_mutex);

        if (!tdata->exit_requested)
            thrd_sleep(&reader_work_period, NULL);
    }
    return exit_status;
}

int naive_writer_thread(void *data) {
    thread_data_t *tdata = (thread_data_t*) data;
    struct timespec writer_work_period = {.tv_sec = 1};
    int next_word_idx = 0;

    while(!tdata->exit_requested) {
        const char *next_word = POSSIBLE_WORDS[next_word_idx++ % NUM_POSSIBLE_WORDS];

        struct timespec wait_until_ts;
        timespec_get(&wait_until_ts, TIME_UTC);
        wait_until_ts.tv_sec++; // wait 1 second from now

        int mtx_status = mtx_timedlock(tdata->writer_mutex, &wait_until_ts);
        if (mtx_status == thrd_timedout) {
            printf("Writer[%d] could not acquire lock and starved. Exiting.\n", tdata->id);
            return 1;
        } else if (mtx_status == thrd_error) {
            printf("Writer[%d] encountered error. Exiting.\n", tdata->id);
            return 1;
        }

        printf("Writer[%d] modifying buffer from [%s] to [%s]\n", tdata->id, tdata->buffer, next_word);
        strncpy(tdata->buffer, next_word, MAX_BUFFER_LEN);
        calculate_crc(tdata->buffer, tdata->buffer_crc);

        mtx_unlock(tdata->writer_mutex);

        thrd_sleep(&writer_work_period, NULL);
    }
    return 0;
}
