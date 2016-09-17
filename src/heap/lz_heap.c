#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <stddef.h>
#include <sys/queue.h>

#include "lzapi.h"
#include "lz_heap.h"

struct lz_heap_page_s;
typedef struct lz_heap_page_s lz_heap_page;

struct lz_heap_page_s {
    SLIST_ENTRY(lz_heap_page_s) next;
    char    data[];
};

struct lz_heap_s {
    size_t page_size;             /* page size */

    SLIST_HEAD(, lz_heap_page_s) page_list_free;
    SLIST_HEAD(, lz_heap_page_s) page_list_used;
};


static lz_heap_page *
_lz_heap_page_new(lz_heap * heap) {
    lz_heap_page * page;

    page       = malloc(heap->page_size + sizeof(lz_heap_page));

    SLIST_INSERT_HEAD(&heap->page_list_free, page, next);

    return page;
};


static lz_heap *
_lz_heap_new(size_t elts, size_t size) {
    lz_heap * heap;

    if (!(heap = malloc(sizeof(lz_heap)))) {
        return NULL;
    }

    heap->page_size = size;

    SLIST_INIT(&heap->page_list_free);
    SLIST_INIT(&heap->page_list_used);

    while (elts-- > 0) {
        _lz_heap_page_new(heap);
    }

    return heap;
} /* _lz_heap_new */

static void
_lz_heap_free(lz_heap * heap, void * d) {
    lz_heap_page * page;

    if (lz_unlikely(heap == NULL)) {
        return;
    }

    lz_assert(d != NULL);

    page = (lz_heap_page *)((char *)(d - offsetof(lz_heap_page, data)));
    lz_assert(page != NULL);
    lz_assert(page->data == d);

    SLIST_REMOVE(&heap->page_list_used, page, lz_heap_page_s, next);
    SLIST_INSERT_HEAD(&heap->page_list_free, page, next);
}

void
lz_heap_free(lz_heap * heap, void * data) {
    return _lz_heap_free(heap, data);
}

static void *
_lz_heap_alloc(lz_heap * heap) {
    lz_heap_page * page;

    if (SLIST_EMPTY(&heap->page_list_free)) {
        _lz_heap_page_new(heap);
    }

    page = SLIST_FIRST(&heap->page_list_free);
    lz_assert(page != NULL);

    SLIST_REMOVE(&heap->page_list_free, page, lz_heap_page_s, next);
    SLIST_INSERT_HEAD(&heap->page_list_used, page, next);

    return page->data;
}

void *
lz_heap_alloc(lz_heap * heap) {
    return _lz_heap_alloc(heap);
}

lz_heap *
lz_heap_new(size_t size, size_t elts) {
    return _lz_heap_new(elts, size);
}

