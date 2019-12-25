#include <openssl/rand.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "gqf.h"
#include "gqf_file.h"
#include "gqf_int.h"
#include "quotient-filter-file.h"
#include "quotient-filter.h"

#define diff_in_nsec(start, end)                  \
    ((uint64_t) 1e9 * end.tv_sec + end.tv_nsec) - \
        ((uint64_t) 1e9 * start.tv_sec + start.tv_nsec)
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Command option funciton ---------------------------------------
enum mode { MEM = 0, DISK = 1 };
int mode = MEM;  // create the QF in-memory by default

void cmdoption(int *argc, char ***argv)
{
    char ch;
    while ((ch = getopt(*argc, *argv, "dm")) != EOF) {
        switch (ch) {
        case 'd':
            mode = DISK;
            break;
        case 'm':
            mode = MEM;
            break;
        default:
            exit(0);
        }
    }
    *argc -= optind;
    *argv += optind;
}
// End of command option funciton --------------------------------


int main(int argc, char **argv)
{
    // Read command option
    cmdoption(&argc, &argv);

    char qf_filename[64], cqf_filename[64];
    sprintf(qf_filename, "qf_%s_benchmark", (mode == MEM) ? "mem" : "disk");
    sprintf(cqf_filename, "cqf_%s_benchmark", (mode == MEM) ? "mem" : "disk");
    FILE *cqfile = fopen(cqf_filename, "w");
    FILE *qfile = fopen(qf_filename, "w");

    srand(0);
    int qbits = 26;
    int rbits = 64 - qbits;
    uint64_t key_bits = qbits + rbits;
    uint64_t nslots = (1ULL << qbits);
    uint64_t nkeys = (uint64_t) nslots * 0.95;
    uint64_t *keys = NULL;
    uint64_t value_bits = 0;
    uint64_t key_count = 1;
    struct timespec start_time, end_time;

    // Initialize ( decide QF/CQF is stored in mem or disk )
    CQF cqf;
    quotient_filter qf;
    if (mode == MEM) {
        if (!cqf_malloc(&cqf, nslots, key_bits, value_bits, QF_HASH_INVERTIBLE,
                        0)) {
            fprintf(stderr, "Can't allocate set.\n");
            abort();
        }
        if (!qf_init(&qf, qbits, rbits)) {
            fprintf(stderr, "Can't allocate set.\n");
            abort();
        }
    } else if (mode == DISK) {
        if (!cqf_initfile(&cqf, nslots, key_bits, value_bits,
                          QF_HASH_INVERTIBLE, 0, "data.cqf")) {
            fprintf(stderr, "Can't allocate set.\n");
            abort();
        }
        if (!qf_initfile(&qf, qbits, rbits, "data.qf")) {
            fprintf(stderr, "Can't allocate set.\n");
            abort();
        }
    }
    cqf_set_auto_resize(&cqf, true);


    // Generate random values
    keys = calloc(nkeys, sizeof(uint64_t));
    RAND_bytes((unsigned char *) keys, sizeof(*keys) * nkeys);
    printf("Insert %lu elements into QF and CQF within %s :\n\n", nkeys,
           (mode == DISK) ? "DISK" : "RAM");

    // QF insert / lookup
    double probe = 0.05 * nslots;
    int load_factor = 0;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for (uint64_t i = 0; i < nkeys; i++) {
        int ret = qf_insert(&qf, keys[i]);
        if (!ret) {
            fprintf(stderr, "failed insertion for key: %lx.\n", keys[i]);
            abort();
        }

        // Record performance each 5% load factor
        if (i >= probe) {
            clock_gettime(CLOCK_MONOTONIC, &end_time);

            // calculate insertion perf (million/thousand operation per second)
            load_factor += 5;
            uint64_t nanosec = diff_in_nsec(start_time, end_time);
            double oper_sec = (mode == MEM) ? (0.05 * nslots) / (nanosec / 1000)
                                            : (0.05 * nslots) / (nanosec / 1e6);
            fprintf(qfile, "%d %lf ", load_factor, oper_sec);
            printf("QF insert %.2lf%s times per second at %d%% load factor\n",
                   oper_sec, (mode == MEM) ? "M" : "k", load_factor);

            // do & calculate lookup perf (million/thousand operation per
            // second)
            clock_gettime(CLOCK_MONOTONIC, &start_time);
            uint64_t j;
            for (j = 0; j <= 0.05 * nslots; j++) {
                if (!qf_may_contain(&qf, keys[rand() % (uint64_t) probe])) {
                    fprintf(stderr, "QF fail to lookup key : %lx\n", keys[i]);
                    abort();
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &end_time);
            nanosec = diff_in_nsec(start_time, end_time);
            oper_sec = (mode == MEM) ? (0.05 * nslots) / (nanosec / 1000)
                                     : (0.05 * nslots) / (nanosec / 1e6);
            fprintf(qfile, "%lf\n", oper_sec);
            printf("QF lookup %.2lf%s times per second at %d%% load factor\n",
                   oper_sec, (mode == MEM) ? "M" : "k", load_factor);

            probe = MIN(probe + 0.05 * nslots, nkeys - 1);
            clock_gettime(CLOCK_MONOTONIC, &start_time);
        }
    }

    // CQF insert / lookup
    probe = 0.05 * nslots;
    load_factor = 0;
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

        // Record performance each 5% load factor
        if (i >= probe) {
            clock_gettime(CLOCK_MONOTONIC, &end_time);

            // calculate insertion perf (million/thousand operation per second)
            load_factor += 5;
            uint64_t nanosec = diff_in_nsec(start_time, end_time);
            double oper_sec = (mode == MEM) ? (0.05 * nslots) / (nanosec / 1000)
                                            : (0.05 * nslots) / (nanosec / 1e6);
            fprintf(cqfile, "%d %lf ", load_factor, oper_sec);
            printf("CQF insert %.2lf%s times per second at %d%% load factor\n",
                   oper_sec, (mode == MEM) ? "M" : "k", load_factor);

            // do & calculate lookup perf (million/thousand operation per
            // second)
            clock_gettime(CLOCK_MONOTONIC, &start_time);
            uint64_t j;
            for (j = 0; j <= 0.05 * nslots; j++) {
                int count = cqf_count_key_value(
                    &cqf, keys[rand() % (uint64_t) probe], 0, QF_KEY_IS_HASH);
                if (!count) {
                    fprintf(stderr,
                            "CQF fail to lookup key : %lx, index. %lu\n",
                            keys[i], keys[i]);
                    abort();
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &end_time);
            nanosec = diff_in_nsec(start_time, end_time);
            oper_sec = (mode == MEM) ? (0.05 * nslots) / (nanosec / 1000)
                                     : (0.05 * nslots) / (nanosec / 1e6);
            fprintf(cqfile, "%lf\n", oper_sec);
            printf("CQF lookup %.2lf%s times per second at %d%% load factor\n",
                   oper_sec, (mode == MEM) ? "M" : "k", load_factor);

            probe = MIN(probe + 0.05 * nslots, nkeys - 1);
            clock_gettime(CLOCK_MONOTONIC, &start_time);
        }
    }

    // Done
    if (mode == DISK) {
        cqf_closefile(&cqf);
        qf_closefile(&qf);
    }
    fclose(qfile);
    fclose(cqfile);
    free(keys);
}