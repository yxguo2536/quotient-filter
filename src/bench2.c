#include <openssl/rand.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "gqf.h"
#include "gqf_int.h"
#include "gqf_file.h"
#include "quotient-filter.h"
#include "quotient-filter-file.h"


#define diff(start, end) ((uint64_t) 1e9*end.tv_sec + end.tv_nsec) - ((uint64_t) 1e9*start.tv_sec + start.tv_nsec);
static __inline__ unsigned long long rdtsc(void)
{
    unsigned hi, lo;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((unsigned long long) lo) | (((unsigned long long) hi) << 32);
}

int main(int argc, char **argv)
{
    int qbits = 24;
    int rbits = 64 - qbits;
    uint64_t key_bits = qbits + rbits;
    uint64_t nslots = (1ULL << qbits);
    uint64_t nkeys = (nslots >> 1) + (nslots >> 2);  // load factor = 0.75
    uint64_t *keys = NULL;
    uint64_t value_bits = 0;
    uint64_t key_count = 1;
    struct timespec start_time, end_time;
    uint64_t qf_time = 0, cqf_time = 0;

    // Initialize
    CQF cqf;
    if (!cqf_malloc(&cqf, nslots, key_bits, value_bits, QF_HASH_INVERTIBLE,
                    0)) {
        fprintf(stderr, "Can't allocate set.\n");
        abort();
    }
    cqf_set_auto_resize(&cqf, true);
    quotient_filter qf;
    if (!qf_init(&qf, qbits, rbits)) {
        fprintf(stderr, "Can't allocate set.\n");
        abort();
    }

    // Generate random values 
    keys = calloc(nkeys, sizeof(uint64_t));
    RAND_bytes((unsigned char *) keys, sizeof(*keys) * nkeys);

    // in-memory CQF insert
    printf("Insert %lu elements into QF and CQF within RAM :\n\n", nkeys);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for (uint64_t i = 0; i < nkeys; i++) {
        int ret = cqf_insert(&cqf, keys[i], 0, key_count,
                             QF_NO_LOCK | QF_KEY_IS_HASH);
        if (ret < 0) {
            fprintf(stderr, "failed insertion for key: %lx.\n", keys[i]);
            if (ret == QF_NO_SPACE)
                fprintf(stderr, "CQF is full.\n");
            else if (ret == QF_COULDNT_LOCK)
                fprintf(stderr, "TRY_ONCE_LOCK failed.\n");
            else
                fprintf(stderr, "Does not recognise return value.\n");
            abort();
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    cqf_time = diff(start_time, end_time);
    printf("CQF insertion done in %.5f sec.\n", (double) 1e-9*cqf_time);

    // in-memory QF insert
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for (uint64_t i = 0; i < nkeys; i++) {
        int ret = qf_insert(&qf, keys[i]);
        if (!ret) {
            fprintf(stderr, "failed insertion for key: %lx.\n", keys[i]);
            abort();
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    qf_time = diff(start_time, end_time);
    printf("QF insertion done in %.5f sec.\n", (double) 1e-9*qf_time);

    // -------------------------------------------------------

    printf("---------------------------------------------\n");
    printf("Lookup %lu elements in QF and CQF within RAM :\n\n", nkeys);

    // in-memory CQF lookup
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for(uint64_t i = 0; i<nkeys; i++) {
        int count = cqf_count_key_value(&cqf, keys[i], 0, QF_KEY_IS_HASH);
        if(!count){
            fprintf(stderr, "CQF fail to lookup key : %lx, index. %lu\n", keys[i], keys[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    cqf_time = diff(start_time, end_time);
    printf("CQF lookup done in %.5f sec.\n", (double) 1e-9*cqf_time);

    // in-memory QF lookup
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for(uint64_t i = 0; i<nkeys; i++) {
        if(!qf_may_contain(&qf, keys[i])) {
            fprintf(stderr, "QF fail to lookup key : %lx\n", keys[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    qf_time = diff(start_time, end_time);
    printf("QF lookup done in %.5f sec.\n", (double) 1e-9*qf_time);

    cqf_destroy(&cqf);
    qf_destroy(&qf);
    





    // =======================================================
    printf("\n---------------------------------------------\n\n");





    // Initialize
    if(!cqf_initfile(&cqf, nslots, key_bits, value_bits, QF_HASH_INVERTIBLE,
                    0, "data.cqf")) {
        fprintf(stderr, "Can't allocate set.\n");
        abort();
    }
    if(!qf_initfile(&qf, qbits, rbits, "data.qf")) {
        fprintf(stderr, "Can't allocate set.\n");
        abort();   
    }

    // in-disk CQF insert
    printf("Insert %lu elements into QF and CQF within disk :\n\n", nkeys);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for (uint64_t i = 0; i < nkeys; i++) {
        int ret = cqf_insert(&cqf, keys[i], 0, key_count,
                             QF_NO_LOCK | QF_KEY_IS_HASH);
        if (ret < 0) {
            fprintf(stderr, "failed insertion for key: %lx.\n", keys[i]);
            if (ret == QF_NO_SPACE)
                fprintf(stderr, "CQF is full.\n");
            else if (ret == QF_COULDNT_LOCK)
                fprintf(stderr, "TRY_ONCE_LOCK failed.\n");
            else
                fprintf(stderr, "Does not recognise return value.\n");
            abort();
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    cqf_time = diff(start_time, end_time);
    printf("CQF insertion done in %.5f sec.\n", (double) 1e-9*cqf_time);

    // in-disk QF insert
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for (uint64_t i = 0; i < nkeys; i++) {
        int ret = qf_insert(&qf, keys[i]);
        if (!ret) {
            fprintf(stderr, "failed insertion for key: %lx.\n", keys[i]);
            abort();
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    qf_time = diff(start_time, end_time);
    printf("QF insertion done in %.5f sec.\n", (double) 1e-9*qf_time);

    // -------------------------------------------------------

    printf("---------------------------------------------\n");

    // in-disk CQF lookup
    printf("Lookup %lu elements in QF and CQF within disk :\n\n", nkeys);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for(uint64_t i = 0; i<nkeys; i++) {
        int count = cqf_count_key_value(&cqf, keys[i], 0, QF_KEY_IS_HASH);
        if(!count){
            fprintf(stderr, "CQF fail to lookup key : %lx, index. %lu\n", keys[i], keys[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    cqf_time = diff(start_time, end_time);
    printf("CQF lookup done in %.5f sec.\n", (double) 1e-9*cqf_time);

    // in-disk QF lookup
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for(uint64_t i = 0; i<nkeys; i++) {
        if(!qf_may_contain(&qf, keys[i])) {
            fprintf(stderr, "QF fail to lookup key : %lx\n", keys[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    qf_time = diff(start_time, end_time);
    printf("QF lookup done in %.5f sec.\n", (double) 1e-9*qf_time);


    cqf_closefile(&cqf);
    qf_closefile(&qf);
}