#ifndef QUOTIENT_FILTER_FILE_H
#define QUOTIENT_FILTER_FILE_H

#include "quotient-filter.h"

/**
 * Initializes a quotient filter with capacity 2^q in disk.
 * Increasing r improves the filter's accuracy but uses more space.
 *
 * Returns false if q == 0, r == 0, q+r > 64, or file allocation failed.
 */
bool qf_initfile(quotient_filter *qf, uint32_t q, uint32_t r, const char *filename);


/**
 * Read a quotient filter in disk.
 *
 * Returns false file open failed.
 */
bool qf_usefile(quotient_filter *qf, const char *filename);


/**
 * Update quotient filter from memory to disk.
 *
 * Returns false when free failed.
 */
bool qf_closefile(quotient_filter *qf);


#endif