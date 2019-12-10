/*
 * ============================================================================
 *
 *        Authors:  Prashant Pandey <ppandey@cs.stonybrook.edu>
 *                  Rob Johnson <robj@vmware.com>
 *
 * ============================================================================
 */

#ifndef _GQF_H_
#define _GQF_H_

#include <inttypes.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct counting_quotient_filter counting_quotient_filter;
typedef counting_quotient_filter QF;

/* CQFs support three hashing modes:

         - DEFAULT uses a hash that may introduce false positives, but
this can be useful when inserting large keys that need to be
hashed down to a small fingerprint.  With this type of hash,
you can iterate over the hash values of all the keys in the
CQF, but you cannot iterate over the keys themselves.

         - INVERTIBLE has no false positives, but the size of the hash
output must be the same as the size of the hash input,
e.g. 17-bit keys hashed to 17-bit outputs.  So this mode is
generally only useful when storing small keys in the CQF.  With
this hashing mode, you can use iterators to enumerate both all
the hashes in the CQF, or all the keys.

         - NONE, for when you've done the hashing yourself.  WARNING: the
           CQF can exhibit very bad performance if you insert a skewed
                 distribution of intputs.
*/

enum qf_hashmode { QF_HASH_DEFAULT, QF_HASH_INVERTIBLE, QF_HASH_NONE };

/* The CQF supports concurrent insertions and queries.  Only the
         portion of the CQF being examined or modified is locked, so it
         supports high throughput even with many threads.

         The CQF operations support 3 locking modes:

         - NO_LOCK: for single-threaded applications or applications
that do their own concurrency management.

         - WAIT_FOR_LOCK: Spin until you get the lock, then do the query
or update.

         - TRY_ONCE_LOCK: If you can't grab the lock on the first try,
return with an error code.
*/
#define QF_NO_LOCK (0x01)
#define QF_TRY_ONCE_LOCK (0x02)
#define QF_WAIT_FOR_LOCK (0x04)

/* It is sometimes useful to insert a key that has already been
         hashed. */
#define QF_KEY_IS_HASH (0x08)

/******************************************
         The CQF defines low-level constructor and destructor operations
         that are designed to enable the application to manage the memory
         used by the CQF.
*******************************************/

/***********************************
Functions for modifying the CQF.
***********************************/

#define QF_NO_SPACE (-1)
#define QF_COULDNT_LOCK (-2)
#define QF_DOESNT_EXIST (-3)



#define QF_INVALID (-4)
#define QFI_INVALID (-5)



#ifdef __cplusplus
}
#endif

#endif /* _GQF_H_ */
