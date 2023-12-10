#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <threads.h> // Preferred C11 feature over POSIX pthread

const int MAX_BUFFER_LEN = 100;

// below is a randomly generated list of words
const char* POSSIBLE_WORDS[] = { "low", "agent", "relative", "abuse", "dynamic",
"director", "service", "turkey", "horizon", "coat", "shadow", "franchise", "convention",
"refuse", "embryo", "barrier", "burn", "employ", "realize", "knock", "square", "rack",
"habit", "light", "misery", "mix", "idea", "bathtub", "folk", "inn", "substitute",
"understand", "limit", "pack", "me", "venture", "budge", "ditch", "divide", "threaten",
"petty", "pot", "brush", "difficulty", "warning", "us", "south", "cellar", "company",
"control", "berry", "straight", "admiration", "chain", "trivial", "flight", "veil",
"injection", "occasion", "joint", "jet", "arise", "shark", "insist", "knock", "chapter",
"memorandum", "care", "decade", "access", "revise", "rob", "portrait", "invisible", "love",
"transaction", "conceive", "material", "wound", "agony", "operation", "cash", "looting",
"unrest", "patience", "association", "deck", "dog", "intention", "cool", "understanding",
"substitute", "note", "freight", "courage", "muscle", "sign", "manufacture", "business" };
const int NUM_POSSIBLE_WORDS =  sizeof(POSSIBLE_WORDS) / sizeof(POSSIBLE_WORDS[0]);

typedef int crc_t; // naive redundancy check logic for proof of concept

typedef struct thread_data {
    mtx_t* reader_mutex;
    mtx_t* writer_mutex;
    char *buffer;
    crc_t *buffer_crc;
    int id;
    bool exit_requested;
} thread_data_t;

size_t my_strnlen(const char* s, size_t n) {
    char* found = memchr(s, '\0', n);
    return found ? (size_t)(found-s) : n;
}

void calculate_crc(char *buf, crc_t *crc) {
    *crc = 0;
    for (size_t i=0; i < my_strnlen(buf, MAX_BUFFER_LEN); i++)
        *crc += buf[i];
}

bool is_buffer_valid(char * buf, crc_t *expected_crc) {
    crc_t actual_crc;
    calculate_crc(buf, &actual_crc);
    return actual_crc == *expected_crc;
}

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
