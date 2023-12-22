#include <stdio.h>

#include "common.h"

int reader_thread(void *data) {
    static int num_active_readers = 0;
    thread_data_t *tdata=(thread_data_t*) data;

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
        }

        mtx_lock(tdata->reader_mutex);
        num_active_readers--;
        if (num_active_readers == 0) {
            mtx_unlock(tdata->writer_mutex);
        }
        mtx_unlock(tdata->reader_mutex);

        if (!tdata->exit_requested)
            thrd_sleep(&(struct timespec){.tv_nsec=500 * 1000 * 1000}, NULL);
    }
}

int writer_thread(void *data) {
    thread_data_t *tdata = (thread_data_t*) data;
    int next_word_idx = 0;

    while(!tdata->exit_requested) {
        const char *next_word = POSSIBLE_WORDS[next_word_idx++ % NUM_POSSIBLE_WORDS];

        mtx_lock(tdata->writer_mutex);

        printf("Writer[%d] modifying buffer from [%s] to [%s]\n", tdata->id, tdata->buffer, next_word);
        strncpy(tdata->buffer, next_word, MAX_BUFFER_LEN);
        calculate_crc(tdata->buffer, tdata->buffer_crc);

        mtx_unlock(tdata->writer_mutex);

        thrd_sleep(&(struct timespec){.tv_sec=1}, NULL);
    }
}
