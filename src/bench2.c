#include <openssl/rand.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "gqf.h"
#include "gqf_int.h"
#include "quotient-filter.h"

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
    uint64_t start_time, end_time;
    uint64_t qf_time = 0, cqf_time = 0;
    printf("Inserting %lu elements into QF and CQF :\n", nkeys);

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

    // CQF
    start_time = rdtsc();
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
    end_time = rdtsc();
    cqf_time = end_time - start_time;
    printf("CQF insertion done in %lu ns.\n", cqf_time);

    // QF
    start_time = rdtsc();
    for (uint64_t i = 0; i < nkeys; i++) {
        int ret = qf_insert(&qf, keys[i]);
        if (!ret) {
            fprintf(stderr, "failed insertion for key: %lx.\n", keys[i]);
            abort();
        }
    }
    end_time = rdtsc();
    qf_time = end_time - start_time;
    printf("QF insertion done in %lu ns.\n", qf_time);
}