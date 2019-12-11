#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "quotient-filter-file.h"

#define LOW_MASK(n) ((1ULL << (n)) - 1ULL)

/**
 * Initializes a quotient filter with capacity 2^q in disk.
 * Increasing r improves the filter's accuracy but uses more space.
 *
 * Returns false if q == 0, r == 0, q+r > 64, or file allocation failed.
 */
bool qf_initfile(quotient_filter *qf, uint32_t q, uint32_t r, const char *filename)
{
	if (q == 0 || r == 0 || q + r > 64)
        return false;

    qf->qbits = q;
    qf->rbits = r;
    qf->elem_bits = qf->rbits + 3;
    qf->index_mask = LOW_MASK(q);
    qf->rmask = LOW_MASK(r);
    qf->elem_mask = LOW_MASK(qf->elem_bits);
    qf->entries = 0;
    qf->max_size = 1 << q;

	int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
	uint64_t total_bytes = qf_table_size(q, r);

	if(posix_fallocate(fd, 0, total_bytes) < 0){
		fprintf(stderr, "Couldn't fallocate file.");
		exit(0);
	}

	qf->table = mmap(NULL, total_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(qf->table == MAP_FAILED) {
		perror("Couldn't mmap file");
		exit(0);
	}

	if(madvise(qf->table, total_bytes, MADV_RANDOM) <0){
		perror("Couldn't fallocate file in madvise");
		exit(0);
	}

	return qf->table != NULL;
}


/**
 * TODO :
 * Read a quotient filter in disk.
 *
 * Returns false when file open failed.
 */
bool qf_usefile(quotient_filter *qf, const char *filename)
{
	return false;
}

/**
 * Update quotient filter from memory to disk.
 *
 * Returns false when free failed.
 */
bool qf_closefile(quotient_filter *qf)
{
	assert(qf->table != NULL);

	uint64_t size = qf_table_size(qf->qbits, qf->rbits);
	if(msync(qf->table, size, MS_SYNC) < 0){
		perror("Couldn't sync file to disk");
		exit(0);
	}

	if(munmap(qf->table, size) == -1){
		perror("Couldn't munmap file");
		exit(0);
	}

	return true;
}
