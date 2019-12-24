#include <openssl/rand.h>
#include <stdio.h>
#include <stdlib.h>

#include "gqf.h"
#include "gqf_file.h"
#include "gqf_int.h"
#include "quotient-filter-file.h"
#include "quotient-filter.h"


static inline uint64_t rand64()
{
    return ((uint64_t) rand() << 32) | rand();
}

struct timespec a, b;

void qf_test()
{
    quotient_filter qf;

    // Test random insert & lookup
    uint32_t q = 28;
    uint32_t r = 64 - q;
    uint64_t nkeys = (3 * (1 << q) / 4);
    if (!qf_init(&qf, q, r)) {
        fprintf(stderr, "Can't allocate set.\n");
        abort();
    }
    uint64_t *keys = calloc(nkeys, sizeof(uint64_t));
    RAND_bytes((unsigned char *) keys, sizeof(*keys) * nkeys);
    printf("Testing QF with %lu random insertion and lookup ", nkeys);
    for (uint64_t i = 0; i < nkeys; i++) {
        if (!qf_insert(&qf, keys[i])) {
            fprintf(stderr, "QF failed to insert for key: %lx.\n", keys[i]);
            abort();
        }
        if (!qf_may_contain(&qf, keys[i])) {
            fprintf(stderr, "QF failed to lookup for key: %lx.\n", keys[i]);
            abort();
        }

        if (i % 10000000 == 0)
            printf(".");
    }
    printf(" validated\n");

    // Check QF consistency
    if (qf_is_consistent(&qf)) {
        printf("QF is consistent.\n");
    } else {
        printf("QF consistency check failed.\n");
        abort();
    }

    // Test remove
    printf("Testing QF with %lu removal of inserted elements ", nkeys);
    for (uint64_t i = 0; i < nkeys; i++) {
        if (!qf_remove(&qf, keys[i])) {
            fprintf(stderr, "QF failed to remove for key: %lx.\n", keys[i]);
            abort();
        }

        if (i % 10000000 == 0)
            printf(".");
    }
    printf(" validated\n");
    qf_destroy(&qf);

    printf("\n");

    // Test sequential insert & lookup
    q = 16;
    r = 64 - q;
    nkeys = (3 * (1 << q) / 4);
    qf_init(&qf, q, r);
    printf("Testing QF with %lu sequential insertion and lookup ", nkeys);
    uint64_t key = rand64();
    for (uint32_t i = 0; i < nkeys; i++) {
        if (!qf_insert(&qf, key + i)) {
            fprintf(stderr, "QF failed to insert for key: %lx.\n", key + i);
            abort();
        }
        if (!qf_may_contain(&qf, key + i)) {
            fprintf(stderr, "QF failed to lookup for key: %lx.\n", key + i);
            abort();
        }

        if (i % 2000 == 0)
            printf(".");
    }
    printf(" validated\n");

    // Check QF consistency
    if (qf_is_consistent(&qf)) {
        printf("QF is consistent.\n");
    } else {
        printf("QF consistency check failed.\n");
        abort();
    }

    // Test sequential remove
    printf("Testing QF with %lu removal of inserted elements ", nkeys);
    for (uint64_t i = 0; i < nkeys; i++) {
        if (!qf_remove(&qf, key + i)) {
            fprintf(stderr, "QF failed to remove for key: %lx.\n", keys[i]);
            abort();
        }

        if (i % 2000 == 0)
            printf(".");
    }
    printf(" validated\n");
    qf_destroy(&qf);

    free(keys);
}

void cqf_test()
{
    CQF cqf;

    // Test random insert & lookup
    uint32_t q = 28;
    uint32_t r = 64 - q;
    uint64_t key_bits = q + r;
    uint64_t value_bits = 0;
    uint64_t nslots = (1ULL << q);
    uint64_t nkeys = (3 * (1 << q) / 4);
    if (!cqf_malloc(&cqf, nslots, key_bits, value_bits, QF_HASH_INVERTIBLE,
                    0)) {
        fprintf(stderr, "Can't allocate set.\n");
        abort();
    }
    cqf_set_auto_resize(&cqf, true);
    uint64_t *keys = calloc(nkeys, sizeof(uint64_t));
    RAND_bytes((unsigned char *) keys, sizeof(*keys) * nkeys);
    printf("Testing CQF with %lu random insertion and lookup ", nkeys);
    for (uint64_t i = 0; i < nkeys; i++) {
        int ret = cqf_insert(&cqf, keys[i], 0, 1, QF_NO_LOCK | QF_KEY_IS_HASH);
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

        int count = cqf_count_key_value(&cqf, keys[i], 0, QF_KEY_IS_HASH);
        if (!count) {
            fprintf(stderr, "CQF fail to lookup key : %lx\n", keys[i]);
            abort();
        }

        if (i % 10000000 == 0)
            printf(".");
    }
    printf(" validated\n");

    // Test remove
    printf("Testing CQF with %lu removal of inserted elements ", nkeys);
    for (uint64_t i = 0; i < nkeys; i++) {
        int ret =
            cqf_delete_key_value(&cqf, keys[i], 0, QF_NO_LOCK | QF_KEY_IS_HASH);
        if (ret < 0) {
            fprintf(stderr, "failed remove for key: %lx.\n", keys[i]);
            if (ret == QF_NO_SPACE)
                fprintf(stderr, "CQF is full.\n");
            else if (ret == QF_COULDNT_LOCK)
                fprintf(stderr, "TRY_ONCE_LOCK failed.\n");
            else
                fprintf(stderr, "Does not recognise return value.\n");
            abort();
        }

        if (i % 10000000 == 0)
            printf(".");
    }
    printf(" validated\n");
    cqf_destroy(&cqf);

    free(keys);
}

int main()
{
    srand(0);
    qf_test();
    printf("\n------------------------------------------------\n\n");
    cqf_test();

    return 0;
}