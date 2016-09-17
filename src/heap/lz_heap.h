#ifndef __LZ_HEAP_H__
#define __LZ_HEAP_H__

#include "liblz.h"

struct lz_heap_s;

typedef struct lz_heap_s lz_heap;


/**
 * @brief creates a new heap context
 *
 * @param size the size of the data to be allocated
 * @param nelem the number of elements allocated with the size
 *
 * @return NULL on error
 */
LZ_EXPORT lz_heap * lz_heap_new(size_t size, size_t nelem);


/**
 * @brief returns a single pre-allocated segment of memory from the heap list,
 *        if there happens to be no entries left, one will be allocated, added to the
 *        heap and returned.
 *
 * @param heap
 *
 * @return a block of data
 */
LZ_EXPORT void * lz_heap_alloc(lz_heap * heap);


/**
 * @brief removes the data entry, and places it back into a unused queue.
 *
 * @param heap
 * @param d data that was returned from lz_heap_alloc()
 */
LZ_EXPORT void lz_heap_free(lz_heap * heap, void * d);

#endif

