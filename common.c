#include "common.h"

const int MAX_BUFFER_LEN = 100;

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