// This file contains functions and definitions common to all
// solutions of the reader-writer problem

#include <string.h>
#include <stdbool.h>
#include <threads.h> // Preferred C11 feature over POSIX pthread

typedef int crc_t; // naive redundancy check logic for proof of concept

void calculate_crc(char *buf, crc_t *crc);
bool is_buffer_valid(char * buf, crc_t *expected_crc);

extern const int MAX_BUFFER_LEN;

// below is a randomly generated list of words
extern const char* POSSIBLE_WORDS[];
extern const int NUM_POSSIBLE_WORDS;

typedef struct thread_data {
    mtx_t* reader_mutex;
    mtx_t* writer_mutex;
    char *buffer;
    crc_t *buffer_crc;
    int id;
    bool exit_requested;
} thread_data_t;
